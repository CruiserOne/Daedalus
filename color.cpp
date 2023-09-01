/*
** Daedalus (Version 3.4) File: color.cpp
** By Walter D. Pullen, Astara@msn.com, http://www.astrolog.org/labyrnth.htm
**
** IMPORTANT NOTICE: Daedalus and all Maze generation and general
** graphics routines used in this program are Copyright (C) 1998-2023 by
** Walter D. Pullen. Permission is granted to freely use, modify, and
** distribute these routines provided these credits and notices remain
** unmodified with any altered or distributed versions of the program.
** The user does have all rights to Mazes and other graphic output
** they make in Daedalus, like a novel created in a word processor.
**
** More formally: This program is free software; you can redistribute it
** and/or modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation; either version 2 of the
** License, or (at your option) any later version. This program is
** distributed in the hope that it will be useful and inspiring, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details, a copy of which is in the
** LICENSE.HTM included with Daedalus, and at http://www.gnu.org
**
** This file contains general graphics routines, that operate on a color
** bitmap, unrelated to Mazes.
**
** Old last code change: 6/29/1990.
** Last code change: 8/29/2023.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "util.h"
#include "graphics.h"
#include "color.h"


#define cchAscii 16
CONST char rgchAscii[cchAscii + 1] = "#MN@O$7%?*;:+,. ";

// Color RGB values and names.

CONST KV rgkv[cColor] = {
  kvBlack,  kvMaroon,  kvDkGreen, kvMaize,
  kvDkBlue, kvPurple,  kvDkCyan,  kvLtGray,
  kvGray,   kvRed,     kvGreen,   kvYellow,
  kvBlue,   kvMagenta, kvCyan,    kvWhite,

  kvMaize,  kvDkBlue,  kvPurple,  kvDkCyan,
  kvLtGray, kvGray,    kvGreen,   kvMagenta,
  kvCyan,   kvDkGray,  kvOrange,  kvPink,
  kvBrown};

CONST char *rgszColor[cColor] = {
  "Black",  "Maroon",  "DkGreen", "Maize",
  "DkBlue", "Purple",  "DkCyan",  "LtGray",
  "Gray",   "Red",     "Green",   "Yellow",
  "Blue",   "Magenta", "Cyan",    "White",

  "Olive",  "Navy",    "Violet",  "Teal",
  "Silver", "Grey",    "Lime",    "Fuchsia",
  "Aqua",   "DkGray",  "Orange",  "Pink",
  "Brown"};

CS cs = {
  // Color replace settings
  fTrue, 0, nDegMax, kvBlack, kvRed, crpRing,
  // Macro accessible only settings
  fFalse, 299587114L, fFalse};


/*
******************************************************************************
** Color Primitives
******************************************************************************
*/

// Return the RGB value of a pixel in a color bitmap.

KV CCol::Get(int x, int y) CONST
{
  byte *pb;

  if (!FLegal(x, y))
    return kvBlack;
  pb = _Pb(x, y);
  return _Get(pb);
}


// Set the RGB value of a pixel in a color bitmap.

void CCol::Set(int x, int y, KV kv)
{
  byte *pb;

  if (!FLegal(x, y))
    return;
  pb = _Pb(x, y);
  if (gs.fTraceDot && FVisible())
    ScreenDot(x, y, fOff, kv);
  _Set(pb, RgbR(kv), RgbG(kv), RgbB(kv));
}


// Invert a pixel in a bitmap.

void CCol::Inv(int x, int y)
{
  byte *pb;
  KV kv;

  if (!FLegal(x, y))
    return;
  pb = _Pb(x, y);
  kv = _Get(pb) ^ kvWhite;
  if (gs.fTraceDot && FVisible())
    ScreenDot(x, y, fOff, kv);
  _Set(pb, RgbR(kv), RgbG(kv), RgbB(kv));
}


// Allocate a new color bitmap of a given size.

flag CCol::FAllocate(int x, int y, CONST CMap *pcOld)
{
  long cb;

  if (x < 0 || y < 0 || x > xColmap || y > yColmap) {
    PrintSzNN_E("Can't create color bitmap larger than %d by %d!\n",
      xColmap, yColmap);
    return fFalse;
  }
  cb = CbColmap(x, y);
  if (cb < 0) {
    PrintSzNN_E("Can't allocate color bitmap of size %d by %d!\n", x, y);
    return fFalse;
  }
  m_rgb = (byte *)PAllocate(cb);
  if (m_rgb == NULL)
    return fFalse;
  m_x = x; m_y = y;
  m_clRow = CbColmapRow(x) >> 2;
  m_cfPix = 24;

  // Inherit the client fields (i.e. 3D dimensions) from passed in bitmap.

  if (pcOld != NULL)
    Copy3(*pcOld);
  else {
    m_x3 = wClient1Def; m_y3 = wClient2Def;
    m_z3 = wClient3Def; m_w3 = wClient4Def;
  }
  return fTrue;
}


// Make a color bitmap be a given size, reallocating or allocating in the
// first place if necessary.

flag CCol::FBitmapSizeSet(int xnew, int ynew)
{
  CCol c2;

  if (!FNull()) {
    // Check if bitmap will have same dimensions, or same number of bytes.
    if (m_x == xnew && m_y == ynew)
      return fTrue;
    if (CbColmap(m_x, m_y) == CbColmap(xnew, ynew)) {
      m_x = xnew; m_y = ynew;
      m_clRow = CbColmapRow(xnew) >> 2;
      return fTrue;
    }
  }
  if (!c2.FAllocate(xnew, ynew, !FNull() ? this : NULL))
    return fFalse;
  CopyFrom(c2);
  return fTrue;
}


// Lighten or darken an RGB color. The returned color ranges from black
// (r = -1.0) to the original color (r = 0.0) to white (r = 1.0).

KV KvShade(KV kv, real r)
{
  Assert(r >= -1.0 && r <= 1.0);
  if (r < 0.0) {
    return Rgb(
      (int)((real)(RgbR(kv))*(1.0 + r)),
      (int)((real)(RgbG(kv))*(1.0 + r)),
      (int)((real)(RgbB(kv))*(1.0 + r)));
  }
  return Rgb(
    255 - (int)((255.0 - (real)(RgbR(kv)))*(1.0 - r)),
    255 - (int)((255.0 - (real)(RgbG(kv)))*(1.0 - r)),
    255 - (int)((255.0 - (real)(RgbB(kv)))*(1.0 - r)));
}


// Blend two colors. Return a color half way between them.

KV KvBlend(KV kv1, KV kv2)
{
  return Rgb(
    (RgbR(kv1) + RgbR(kv2)) >> 1,
    (RgbG(kv1) + RgbG(kv2)) >> 1,
    (RgbB(kv1) + RgbB(kv2)) >> 1);
}


// Blend two colors with a specified bias given to one color. The returned
// color ranges from the first color (r = 0.0) to the second (r = 1.0).

KV KvBlendR(KV kv1, KV kv2, real r)
{
  Assert(FBetween(r, 0.0, 1.0));
  return Rgb((int)((real)(RgbR(kv2) - RgbR(kv1)) * r) + RgbR(kv1),
    (int)((real)(RgbG(kv2) - RgbG(kv1)) * r) + RgbG(kv1),
    (int)((real)(RgbB(kv2) - RgbB(kv1)) * r) + RgbB(kv1));
}


// Blend two colors with a specified bias given to one color. The returned
// color ranges from the first color (n1 = 0) to the second (n1 = n2).

KV KvBlendN(KV kv1, KV kv2, int n1, int n2)
{
  Assert(FBetween(n1, 0, n2));
  if (n2 == 0)
    n2 = 1;
  return Rgb((RgbR(kv2) - RgbR(kv1)) * n1 / n2 + RgbR(kv1),
    (RgbG(kv2) - RgbG(kv1)) * n1 / n2 + RgbG(kv1),
    (RgbB(kv2) - RgbB(kv1)) * n1 / n2 + RgbB(kv1));
}


// Return the difference between two RGB colors. Note the difference between a
// color and itself is black (like how X - X = 0), and the difference between
// a color and black is itself (like how X - 0 = X).

KV KvDiff(KV kv1, KV kv2)
{
  int nR, nG, nB;

  nR = NAbs(RgbR(kv2) - RgbR(kv1));
  nG = NAbs(RgbG(kv2) - RgbG(kv1));
  nB = NAbs(RgbB(kv2) - RgbB(kv1));
  return Rgb(nR, nG, nB);
}


// Return a color from a predefined range of colors.

KV Hue(int d)
{
  int nR, nG, nB, i;
  long l;

  l = (long)d * cs.nRainbowDistance / nDegMax +
    (long)cs.nRainbowStart * (nHueMax / nDegMax);

  // Do a blend between two colors.
  // d = 0 is color #1, d = 3600 is color #2
  if (!cs.fRainbowBlend) {
    while (l >= (nHueMax << 1))
      l -= (nHueMax << 1);
    while (l < 0)
      l += (nHueMax << 1);
    d = (int)l;
    if (d >= nHueMax)
      d = (nHueMax << 1) - 1 - d;
    return !cs.fGraphNumber ? KvBlendN(cs.kvBlend1, cs.kvBlend2, d, nHueMax) :
      (cs.kvBlend2 - cs.kvBlend1) * d / nHueMax + cs.kvBlend1;
  }

  // Do a blend around the colors of the rainbow.
  // d = 0 is red, d = 1200 is green, d = 2400 is blue
  while (l >= nHueMax)
    l -= nHueMax;
  while (l < 0)
    l += nHueMax;
  d = (int)l;
  i = NAbs(d - nHueHalf);
  if (i > nHue13)
    nR = 255;
  else if (i < nHue16)
    nR = 0;
  else {
    i = i - nHue16;
    if (cs.fGraphNumber)
      i = (i + (Sq(nHue16) - Sq(nHue16 - i)) / nHue16) >> 1;
    nR = NMultDiv(i, 255, nHue16);
  }
  i = NAbs(d - nHue13);
  if (i < nHue16)
    nG = 255;
  else if (i > nHue13)
    nG = 0;
  else {
    i = nHue13 - i;
    if (cs.fGraphNumber)
      i = (i + (Sq(nHue16) - Sq(nHue16 - i)) / nHue16) >> 1;
    nG = NMultDiv(i, 255, nHue16);
  }
  i = NAbs(d - nHue23);
  if (i < nHue16)
    nB = 255;
  else if (i > nHue13)
    nB = 0;
  else {
    i = nHue13 - i;
    if (cs.fGraphNumber)
      i = (i + (Sq(nHue16) - Sq(nHue16 - i)) / nHue16) >> 1;
    nB = NMultDiv(i, 255, nHue16);
  }
  return Rgb(nR, nG, nB);
}


// Return a color given its Hue, Saturation, and Luminosity.

KV Hsl(int h, int s, int l)
{
  KV kv;

  kv = HueD(h);
  kv = KvShade(kv, (real)l / 1000.0);
  kv = KvBlendN(kvGray, kv, s, 1000);
  return kv;
}


/*
******************************************************************************
** Color Bitmap Editing
******************************************************************************
*/

// Set all pixels within a rectangle in a color bitmap to a color.

void CCol::Block(int x1, int y1, int x2, int y2, KV kv)
{
  int y;

  Legalize(&x1, &y1); Legalize(&x2, &y2);
  SortN(&x1, &x2);
  SortN(&y1, &y2);
  for (y = y1; y <= y2; y++)
    LineX(x1, x2, y, kv);
}


// Copy pixels in a rectangle from one color bitmap to another. If the bitmaps
// are the same, the rectangles should not overlap.

void CCol::BlockMove(CONST CMap &c, int x1, int y1, int x2, int y2,
  int x0, int y0)
{
  CONST CCol &c1 = dynamic_cast<CONST CCol &>(c);

  Assert(c.FColor());
  BlockMove(c1, x1, y1, x2, y2, x0, y0, -1);
}


// Copy pixels in a rectangle from one color bitmap to another, using the
// given operator. If bitmaps are the same, the rectangles should not overlap.

void CCol::BlockMove(CONST CCol &c1, int x1, int y1, int x2, int y2,
  int x0, int y0, int nOp)
{
  byte *pbSrc, *pbDst;
  int x, y, xT, yT, nR, nG, nB;

  SortN(&x1, &x2);
  SortN(&y1, &y2);

  // Fast case: Straight copying to a destinatinon rectangle that's entirely
  // on the bitmap means can quickly copy one row at a time.
  if (nOp < 0 && FLegal(x0, y0) && FLegal(x0+(x2-x1), y0+(y2-y1)) &&
    c1.FLegal(x1, y1) && c1.FLegal(x2, y2))
    for (y = y1; y <= y2; y++) {
      pbSrc = c1._Pb(x1, y);
      pbDst = _Pb(x0, y0+(y-y1));
      for (x = x1; x <= x2; x++) {
        c1._Get(pbSrc, &nR, &nG, &nB);
        _Set(pbDst, nR, nG, nB);
        pbSrc += cbPixelC; pbDst += cbPixelC;
      }
    }

  // Standard case: Copy one pixel at a time using the given operator.
  else
    for (y = y1; y <= y2; y++)
      for (x = x1; x <= x2; x++) {
        xT = x0+(x-x1); yT = y0+(y-y1);
        if (FLegal(xT, yT))
          switch (nOp) {
          case 0: Set(xT, yT, c1.Get(x, y) & Get(xT, yT)); break;
          case 1: Set(xT, yT, c1.Get(x, y) | Get(xT, yT)); break;
          case 2: Set(xT, yT, c1.Get(x, y) ^ Get(xT, yT)); break;
          default: Set(xT, yT, c1.Get(x, y)); break;
          }
      }
}


// Draw a horizontal line on a color bitmap.

void CCol::LineX(int x1, int x2, int y, KV kv)
{
  byte *pb;
  int x, nR, nG, nB;

  // Don't worry about pixels that are off the bitmap.
  if (y < 0 || y >= m_y)
    return;
  SortN(&x1, &x2);
  if (x1 < 0)
    x1 = 0;
  if (x2 >= m_x)
    x2 = m_x-1;

  // Quickly draw the line, where things like the memory address only need to
  // be calculated once.
  pb = _Pb(x1, y);
  nR = RgbR(kv); nG = RgbG(kv); nB = RgbB(kv);
  for (x = x1; x <= x2; x++) {
    _Set(pb, nR, nG, nB);
    pb += cbPixelC;
  }
  if (gs.fTraceDot && FVisible())
    for (x = x1; x <= x2; x++)
      ScreenDot(x, y, fOff, kv);
}


// Draw a vertical line on a color bitmap.

void CCol::LineY(int x, int y1, int y2, KV kv)
{
  byte *pb;
  int y, nR, nG, nB;

  // Don't worry about pixels that are off the bitmap.
  if (x < 0 || x >= m_x)
    return;
  SortN(&y1, &y2);
  if (y1 < 0)
    y1 = 0;
  if (y2 >= m_y)
    y2 = m_y-1;

  // Quickly draw the line, where things like the memory address only need to
  // be calculated once.
  pb = _Pb(x, y1);
  nR = RgbR(kv); nG = RgbG(kv); nB = RgbB(kv);
  for (y = y1; y <= y2; y++) {
    _Set(pb, nR, nG, nB);
    pb += m_clRow << 2;
  }
  if (gs.fTraceDot && FVisible())
    for (y = y1; y <= y2; y++)
      ScreenDot(x, y, fOff, kv);
}


// Draw a line between any two points on a color bitmap.

void CCol::Line(int x1, int y1, int x2, int y2, KV kv)
{
  int x = x1, y = y1, dx = x2 - x1, dy = y2 - y1, xInc, yInc, xInc2, yInc2,
    d, dInc, z, zMax;
  flag fAllLegal = FLegal(x1, y1) && FLegal(x2, y2);

  // Determine slope.
  if (NAbs(dx) >= NAbs(dy)) {
    xInc = NSgn(dx); yInc = 0;
    xInc2 = 0; yInc2 = NSgn(dy);
    zMax = NAbs(dx); dInc = NAbs(dy);
    d = zMax - (!FOdd(dx) && x1 > x2);
  } else {
    xInc = 0; yInc = NSgn(dy);
    xInc2 = NSgn(dx); yInc2 = 0;
    zMax = NAbs(dy); dInc = NAbs(dx);
    d = zMax - (!FOdd(dy) && y1 > y2);
  }
  d >>= 1;

  // Loop over long axis, adjusting short axis for slope as needed.
  for (z = 0; z <= zMax; z++) {
    if (fAllLegal || FLegal(x, y))
      Set(x, y, kv);
    x += xInc; y += yInc; d += dInc;
    if (d >= zMax) {
      x += xInc2; y += yInc2; d -= zMax;
    }
  }
}


// Draw a filled in triangle on a color bitmap, defined by three points.

flag CCol::FTriangle(int x1, int y1, int x2, int y2, int x3, int y3,
  KV kv, int nTrans)
{
  MM rgmm[zBitmapStack], *pmm = NULL, *xminmax;
  int ymin, ymax, y, x, i;
  real rTrans = (real)nTrans / 10000.0;

  if (m_y <= zBitmapStack)
    xminmax = rgmm;
  else {
    pmm = RgAllocate(m_y, MM);
    if (pmm == NULL)
      return fFalse;
    xminmax = pmm;
  }
  ymin = NMax(NMin(Min(y1, y2), y3), 0);
  ymax = NMin(NMax(Max(y1, y2), y3), m_y - 1);
  for (y = ymin; y <= ymax; y++) {
    xminmax[y].min = xColmap;
    xminmax[y].max = -1;
  }
  LineSetup(xminmax, x1, y1, x2, y2);
  LineSetup(xminmax, x2, y2, x3, y3);
  LineSetup(xminmax, x3, y3, x1, y1);

  // Draw one horizontal line per row.
  if (rTrans <= 0.0) {
    for (y = ymin; y <= ymax; y++)
      LineX(xminmax[y].min, xminmax[y].max, y, kv);
  } else {
    for (y = ymin; y <= ymax; y++) {
      i = Min(xminmax[y].max, m_x - 1);
      for (x = Max(xminmax[y].min, 0); x <= i; x++)
        Set(x, y, KvBlendR(kv, Get(x, y), rTrans));
    }
  }
  if (pmm != NULL)
    DeallocateP(pmm);
  return fTrue;
}


// Draw a filled in four sided quadrilateral on a color bitmap.

flag CCol::FQuadrilateral(int x1, int y1, int x2, int y2, int x3, int y3,
  int x4, int y4, KV kv, int nTrans)
{
  MM rgmm[zBitmapStack], *pmm = NULL, *xminmax;
  int ymin, ymax, y, x, i;
  real rTrans = (real)nTrans / 10000.0;

  if (m_y <= zBitmapStack)
    xminmax = rgmm;
  else {
    pmm = RgAllocate(m_y, MM);
    if (pmm == NULL)
      return fFalse;
    xminmax = pmm;
  }
  ymin = NMax(NMin(NMin(Min(y1, y2), y3), y4), 0);
  ymax = NMin(NMax(NMax(Max(y1, y2), y3), y4), m_y - 1);
  for (y = ymin; y <= ymax; y++) {
    xminmax[y].min = xColmap;
    xminmax[y].max = -1;
  }
  LineSetup(xminmax, x1, y1, x2, y2);
  LineSetup(xminmax, x2, y2, x3, y3);
  LineSetup(xminmax, x3, y3, x4, y4);
  LineSetup(xminmax, x4, y4, x1, y1);

  // Since one horizontal line is drawn per row, the quadrilateral should be
  // convex, or at least not concave any along the horizontal axis.
  if (rTrans <= 0.0) {
    for (y = ymin; y <= ymax; y++)
      LineX(xminmax[y].min, xminmax[y].max, y, kv);
  } else {
    for (y = ymin; y <= ymax; y++) {
      i = Min(xminmax[y].max, m_x - 1);
      for (x = Max(xminmax[y].min, 0); x <= i; x++)
        Set(x, y, KvBlendR(kv, Get(x, y), rTrans));
    }
  }
  if (pmm != NULL)
    DeallocateP(pmm);
  return fTrue;
}


// Flood an irregular shape in a color bitmap with one color, where the shape
// area is defined by another color.

flag CCol::FFill(int x, int y, KV kv, KV kvArea, flag fFlood)
{
  PT *rgpt;
  long ipt = 0;
  int xnew, ynew, d, dMax;

  if (!FLegal(x, y))
    return fTrue;
  if (F64K())
    return fFalse;
  if (kvArea < 1)
    kvArea = Get(x, y);
  if (kv == kvArea || Get(x, y) != kvArea)
    return fTrue;
  rgpt = RgAllocate(m_x*m_y, PT);
  if (rgpt == NULL)
    return fFalse;
  dMax = DIRS + fFlood*DIRS;
LSet:
  Set(x, y, kv);
  loop {
    for (d = 0; d < dMax; d++) {
      xnew = x + xoff[d]; ynew = y + yoff[d];
      if (FLegalFill(xnew, ynew, kvArea)) {
        FillPush(x, y);
        x = xnew; y = ynew;
        goto LSet;
      }
    }
    if (ipt <= 0)
      break;
    FillPop(x, y);
  }
  DeallocateP(rgpt);
  return fTrue;
}


/*
******************************************************************************
** Color Bitmap Transformations
******************************************************************************
*/

// Set all pixels in a color bitmap to a specified color.

void CCol::BitmapSet(KV kv)
{
  int il;
  dword *pl, l;
  flag fTrace;

  if (kv == kvBlack || kv == kvWhite || kv == ~0) {

    // For the simplest colors, set 32 bits at a time.
    l = kv == kvBlack ? 0 : dwSet;
    pl = (dword *)_Pb(0);
    for (il = m_y * m_clRow; il > 0; il--)
      *pl++ = l;
  } else {

    // For normal colors, go set one pixel at a time.
    fTrace = gs.fTraceDot; gs.fTraceDot = fFalse;
    Block(0, 0, m_x-1, m_y-1, kv);
    gs.fTraceDot = fTrace;
  }
  if (gs.fTraceDot && FVisible())
    UpdateDisplay();
}


// Convert all colors in a color bitmap to shades of gray with the same
// brightness.

void CCol::ColmapGrayscale()
{
  int x, y, n, nR, nG, nB, nRT, nGT, nBT;
  byte *pb;

  // Simple case: Treat RGB channels as the same brightness.
  if (cs.lGrayscale < 0) {
    for (y = 0; y < m_y; y++) {
      pb = _Pb(0, y);
      for (x = 0; x < m_x; x++) {
        _Get(pb, &nR, &nG, &nB);
        n = (nR + nG + nB + 1) / 3;
        _Set(pb, n, n, n);
        pb += cbPixelC;
      }
    }

  // Advanced case: Each of the RGB channels has its own brightness.
  } else {
    nRT = cs.lGrayscale / 1000000;
    nGT = cs.lGrayscale / 1000 % 1000;
    nBT = cs.lGrayscale % 1000;
    for (y = 0; y < m_y; y++) {
      pb = _Pb(0, y);
      for (x = 0; x < m_x; x++) {
        _Get(pb, &nR, &nG, &nB);
        nR = nR * nRT / 1000;
        nG = nG * nGT / 1000;
        nB = nB * nBT / 1000;
        n = nR + nG + nB;
        _Set(pb, n, n, n);
        pb += cbPixelC;
      }
    }
  }
}


// Adjust the brightness of the colors in a color bitmap.

void CCol::ColmapBrightness(int nOp, real rFactor, int nLimit)
{
  int x, y, r, g, b, sum;
  KV kv;

  switch (nOp) {

  // Brighten all by factor.
  case 0:
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++) {
        Set(x, y, KvShade(Get(x, y), rFactor));
      }
    break;

  // Brighten all by offset.
  case 1:
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++) {
        kv = Get(x, y);
        r = RgbR(kv); g = RgbG(kv); b = RgbB(kv);
        if (nLimit >= 0) {
          r = Min(r + nLimit, 255);
          g = Min(g + nLimit, 255);
          b = Min(b + nLimit, 255);
        } else {
          r = Max(r + nLimit, 0);
          g = Max(g + nLimit, 0);
          b = Max(b + nLimit, 0);
        }
        Set(x, y, Rgb(r, g, b));
      }
    break;

  // Brighten all by multiplier.
  case 2:
    if (!cs.fGraphNumber) {
      for (y = 0; y < m_y; y++)
        for (x = 0; x < m_x; x++) {
          kv = Get(x, y);
          r = RgbR(kv); g = RgbG(kv); b = RgbB(kv);
          if (rFactor >= 0.0) {
            r = (int)Min((long)((real)r * rFactor), 255);
            g = (int)Min((long)((real)g * rFactor), 255);
            b = (int)Min((long)((real)b * rFactor), 255);
          } else {
            r = 255-(int)Min((long)((real)(255-r) * -rFactor), 255);
            g = 255-(int)Min((long)((real)(255-g) * -rFactor), 255);
            b = 255-(int)Min((long)((real)(255-b) * -rFactor), 255);
          }
          Set(x, y, Rgb(r, g, b));
        }
    } else {
      for (y = 0; y < m_y; y++)
        for (x = 0; x < m_x; x++) {
          kv = Get(x, y);
          kv = (KV)((real)kv * rFactor);
          Set(x, y, kv);
        }
    }
    break;

  // Limit brightness to value.
  case 3:
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++) {
        kv = Get(x, y);
        r = RgbR(kv); g = RgbG(kv); b = RgbB(kv);
        sum = r + g + b;
        if (nLimit >= 0) {
          if (sum > nLimit) {
            r = NMultDiv(r, nLimit, sum);
            g = NMultDiv(g, nLimit, sum);
            b = NMultDiv(b, nLimit, sum);
          }
        } else {
          if (sum < -nLimit) {
            r = 255-NMultDiv(255-r, 255*3 + nLimit, 255*3 - sum);
            g = 255-NMultDiv(255-g, 255*3 + nLimit, 255*3 - sum);
            b = 255-NMultDiv(255-b, 255*3 + nLimit, 255*3 - sum);
          }
        }
        Set(x, y, Rgb(r, g, b));
      }
    break;

  default:
    Assert(fFalse);
  }
}


// Replace one color in a bitmap with another color or a pattern.

void CCol::ColmapReplace(KV kvFrom, KV kvTo, int x1, int y1, int x2, int y2)
{
  int x, y;
  byte *pb, nR, nG, nB;

  if (kvTo < 0 && kvTo > -crpUnknown) {
    ColmapReplacePattern(kvFrom, (int)(-kvTo), x1, y1, x2, y2);
    return;
  }

  // Quickly replace all instances of one color with another.
  nR = RgbR(kvTo); nG = RgbG(kvTo); nB = RgbB(kvTo);
  for (y = 0; y < m_y; y++) {
    pb = _Pb(0, y);
    for (x = 0; x < m_x; x++) {
      if (_Get(pb) == kvFrom)
        _Set(pb, nR, nG, nB);
      pb += cbPixelC;
    }
  }
}


// Replace one color in a bitmap with a special color pattern.

void CCol::ColmapReplacePattern(KV kvFrom, int nMode,
  int x1, int y1, int x2, int y2)
{
  int x, y, h, v, l;
  KV kv;
  real len;

  h = m_x >> 1;
  v = m_y >> 1;
  switch (nMode) {
  case crpRing: // O
    len = LengthN(m_x, m_y, h, v) / rHueMax;
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++)
        if (Get(x, y) == kvFrom)
          Set(x, y, Hue((int)(LengthN(h, v, x, y) / len + rRound)));
    break;
  case crpSpoke: // X
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++)
        if (Get(x, y) == kvFrom)
          Set(x, y, HueR(GetAngle(h, v, x, y)));
    break;
  case crpFade: // *
    len = LengthN(m_x, m_y, h, v) + rMinute;
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++)
        if (Get(x, y) == kvFrom)
          Set(x, y, KvShade(HueR(GetAngle(h, v, x, y)),
            (0.5 - LengthN(x, y, h, v) / len) * 2.0));
    break;
  case crpY: // |
    for (y = 0; y < m_y; y++) {
      kv = Hue(NMultDiv(y, nHueMax, m_y));
      for (x = 0; x < m_x; x++)
        if (Get(x, y) == kvFrom)
          Set(x, y, kv);
    }
    break;
  case crpX: // -
    for (x = 0; x < m_x; x++) {
      kv = Hue(NMultDiv(x, nHueMax, m_x));
      for (y = 0; y < m_y; y++)
        if (Get(x, y) == kvFrom)
          Set(x, y, kv);
    }
    break;
  case crpDiagUL: // "\"
    h = m_x + m_y;
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++)
        if (Get(x, y) == kvFrom)
          Set(x, y, Hue(NMultDiv(x + y, nHueMax, h)));
    break;
  case crpDiagUR: // /
    h = m_x + m_y;
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++)
        if (Get(x, y) == kvFrom)
          Set(x, y, Hue(NMultDiv(x + m_y-y-1, nHueMax, h)));
    break;
  case crpSquare: // #
    l = Max(h, v);
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++)
        if (Get(x, y) == kvFrom)
          Set(x, y, Hue(NMultDiv(Max(NAbs(h-x), NAbs(v-y)), nHueMax, l)));
    break;
  case crpDiamond: // +
    l = Max(h, v) << 1;
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++)
        if (Get(x, y) == kvFrom)
          Set(x, y, Hue(NMultDiv(NAbs(h-x) + NAbs(v-y), nHueMax, l)));
    break;
  case crpSpiral: // @
    len = LengthN(x1, y1, h, v);
    if (len <= 0)
      len = rDegMax;
    if (y1 > v || (y1 == v && x1 > h))
      neg(len);
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++)
        if (Get(x, y) == kvFrom)
          Set(x, y, HueR(GetAngle(h, v, x, y) +
            LengthN(h, v, x, y) / len * rDegMax));
    break;
  case crpRing2: // o
    len = LengthN(x1, y1, 0, 0);
    len = RMax(len, LengthN(x1, y1, m_x-1, 0));
    len = RMax(len, LengthN(x1, y1, 0, m_y-1));
    len = RMax(len, LengthN(x1, y1, m_x-1, m_y-1)) / rHueMax;
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++)
        if (Get(x, y) == kvFrom)
          Set(x, y, Hue((int)(LengthN(x1, y1, x, y) / len + rRound)));
    break;
  case crpRings: // 8
    len = RMin(LengthN(x1, y1, 0, 0), LengthN(x2, y2, 0, 0));
    len = RMax(len, RMin(LengthN(x1, y1, m_x-1, 0),
      LengthN(x2, y2, m_x-1, 0)));
    len = RMax(len, RMin(LengthN(x1, y1, 0, m_y-1),
      LengthN(x2, y2, 0, m_y-1)));
    len = RMax(len, RMin(LengthN(x1, y1, m_x-1, m_y-1),
      LengthN(x2, y2, m_x-1, m_y-1))) / rHueMax;
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++)
        if (Get(x, y) == kvFrom)
          Set(x, y, Hue((int)(RMin(LengthN(x1, y1, x, y),
            LengthN(x2, y2, x, y)) / len + rRound)));
    break;
  case crpSpoke2: // x
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++)
        if (Get(x, y) == kvFrom)
          Set(x, y, HueR(GetAngle(x1, y1, x, y)));
    break;
  case crpRainbow: // ?
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++)
        if (Get(x, y) == kvFrom)
          Set(x, y, kvRandomRainbow);
    break;
  case crpRandom: // ??
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++)
        if (Get(x, y) == kvFrom)
          Set(x, y, kvRandom);
    break;
  case crpRandom2: // ?????
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++)
        if (Get(x, y) == kvFrom)
          Set(x, y, Rgb(Rnd(RgbR(cs.kvBlend1), RgbR(cs.kvBlend2)),
            Rnd(RgbG(cs.kvBlend1), RgbG(cs.kvBlend2)),
            Rnd(RgbB(cs.kvBlend1), RgbB(cs.kvBlend2))));
    break;
  default:
    Assert(fFalse);
  }
}


// Exchange two colors in a color bitmap with each other.

void CCol::ColmapExchange(KV kv1, KV kv2)
{
  int x, y;
  KV kv;

  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x; x++) {
      kv = _Get(x, y);
      if (kv == kv1)
        Set(x, y, kv2);
      else if (kv == kv2)
        Set(x, y, kv1);
    }
}


// Combine a color with all pixels in a bitmap using the specified operator.

void CCol::ColmapOrAndKv(KV kv, int nOp)
{
  int x, y, nR, nG, nB;
  KV kvT;

  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x; x++) {
      kvT = _Get(x, y);
      switch (nOp) {

      // Boolean bit combining
      case 0: kvT &= kv; break;
      case 1: kvT |= kv; break;
      case 2: kvT ^= kv; break;

      // Add/Sub/Average/Min/Max/Mul (separate rgb channels)
      case 3: // +
        nR = NMin(RgbR(kvT) + RgbR(kv), 255);
        nG = NMin(RgbG(kvT) + RgbG(kv), 255);
        nB = NMin(RgbB(kvT) + RgbB(kv), 255);
        kvT = Rgb(nR, nG, nB);
        break;
      case 4: kvT = KvDiff(kvT, kv);  break;
      case 5: kvT = KvBlend(kvT, kv); break;
      case 6: // Min
        nR = NMin(RgbR(kvT), RgbR(kv));
        nG = NMin(RgbG(kvT), RgbG(kv));
        nB = NMin(RgbB(kvT), RgbB(kv));
        kvT = Rgb(nR, nG, nB);
        break;
      case 7: // Max
        nR = NMax(RgbR(kvT), RgbR(kv));
        nG = NMax(RgbG(kvT), RgbG(kv));
        nB = NMax(RgbB(kvT), RgbB(kv));
        kvT = Rgb(nR, nG, nB);
        break;
      case 8: // *
        nR = NMin(RgbR(kvT) * RgbR(kv), 255);
        nG = NMin(RgbG(kvT) * RgbG(kv), 255);
        nB = NMin(RgbB(kvT) * RgbB(kv), 255);
        kvT = Rgb(nR, nG, nB);
        break;

      // Add/Sub/Average/Min/Max/Mul (single number)
      case 9:  kvT = NMin(kvT + kv, kvWhite); break;
      case 10: kvT = NMax(kvT - kv, kvBlack); break;
      case 11: kvT = (kvT + kv) >> 1; break;
      case 12: kvT = NMin(kvT, kv); break;
      case 13: kvT = NMax(kvT, kv); break;
      case 14: kvT = NMin(kvT * kv, kvWhite); break;
      default: Assert(fFalse);
      }
      Set(x, y, kvT);
    }
}


// Combine one color bitmap with another using the specified operator.

void CCol::ColmapOrAnd(CONST CCol &c2, int nOp)
{
  int x, y, nR, nG, nB;
  KV kv1, kv2, kv;

  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x; x++) {
      kv1 = Get(x, y); kv2 = c2.Get(x, y);
      switch (nOp) {
      case 0: kv = kv1 & kv2; break;
      case 1: kv = kv1 | kv2; break;
      case 2: kv = kv1 ^ kv2; break;
      case 3: // +
        nR = NMin(RgbR(kv1) + RgbR(kv2), 255);
        nG = NMin(RgbG(kv1) + RgbG(kv2), 255);
        nB = NMin(RgbB(kv1) + RgbB(kv2), 255);
        kv = Rgb(nR, nG, nB);
        break;
      case 4: kv = KvDiff(kv1, kv2);  break;
      case 5: kv = KvBlend(kv1, kv2); break;
      case 6: kv = NMin(kv1 + kv2, kvWhite); break;
      case 7: kv = NMax(kv1 - kv2, kvBlack); break;
      case 8: kv = (kv1 + kv2) >> 1; break;
      default: Assert(fFalse);
      }
      Set(x, y, kv);
    }
}


// Blend one color bitmap with another, based on the transparency proportions
// in a third bitmap.

void CCol::ColmapAlpha(CONST CCol &c2, CONST CCol &c3)
{
  int x, y, nR, nG, nB;
  KV kv1, kv2, kv3;

  if (!cs.fGraphNumber) {
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++) {
        kv1 = Get(x, y); kv2 = c2.Get(x, y); kv3 = c3.Get(x, y);
        nR = RgbR(kv1) + (RgbR(kv2) - RgbR(kv1)) * RgbR(kv3) / 255;
        nG = RgbG(kv1) + (RgbG(kv2) - RgbG(kv1)) * RgbG(kv3) / 255;
        nB = RgbB(kv1) + (RgbB(kv2) - RgbB(kv1)) * RgbB(kv3) / 255;
        Set(x, y, Rgb(nR, nG, nB));
      }
  } else {
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++) {
        kv1 = Get(x, y); kv2 = c2.Get(x, y); kv3 = c3.Get(x, y);
        Set(x, y, kv1 + (kv2 - kv1) * kv3 / 4095);
      }
  }
}


// Flip the pixels in a color bitmap horizontally.

void CCol::BitmapFlipX()
{
  int x, y;
  KV kv;

  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x >> 1; x++) {
      kv = Get(x, y);
      Set(x, y, Get(m_x-1-x, y));
      Set(m_x-1-x, y, kv);
    }
}


// Resize a color bitmap to be a given size. The old contents of the bitmap
// are stretched or compacted to fit within the new dimensions.

flag CCol::FBitmapZoomTo(int xnew, int ynew, flag fPreserve)
{
  CCol cNew;
  int x, y, x1, y1, x2, y2, nR, nG, nB, i;
  KV kv, kv2;

  if (!cNew.FAllocate(xnew, ynew, this))
    return fFalse;
  if (FNull() || cNew.FNull())
    goto LDone;
  if (fPreserve && xnew <= m_x && ynew <= m_y) {

    // Preserve on pixels when shrinking case: If the bitmap isn't growing in
    // either dimension, then there are always one or more source pixels to
    // map to a destination pixel. Make each destination pixel the mixture of
    // colors of all the source pixels that map to it.

    for (y = 0; y < ynew; y++)
      for (x = 0; x < xnew; x++) {
        nR = nG = nB = i = 0;
        y2 = (y+1) * m_y / ynew;
        for (y1 = y * m_y / ynew; y1 < y2; y1++) {
          x2 = (x+1) * m_x / xnew;
          for (x1 = x * m_x / xnew; x1 < x2; x1++) {
            kv = Get(x1, y1);
            nR += RgbR(kv); nG += RgbG(kv); nB += RgbB(kv);
            i++;
          }
        }
        cNew.Set(x, y,
          Rgb((nR + (i >> 1)) / i, (nG + (i >> 1)) / i, (nB + (i >> 1)) / i));
      }
  } else if (fPreserve && xnew >= m_x && ynew >= m_y) {

    // Preserve on pixels when shrinking case #2: If the bitmap isn't
    // shrinking in either dimension, then each source pixel maps to one or
    // more destination pixels. Make each destination pixel a blend of the
    // four source pixels that are closest to it.

    if (!cs.fGraphNumber) {
      for (y = 0; y < ynew; y++)
        for (x = 0; x < xnew; x++) {
          x1 = (x * m_x << 8) / xnew;
          y1 = (y * m_y << 8) / ynew;
          x2 = x1 >> 8; y2 = y1 >> 8;
          kv = KvBlendN(Get(x2, y2), Get(x2+1, y2), x1 & 255, 256);
          kv2 = KvBlendN(Get(x2, y2+1), Get(x2+1, y2+1), x1 & 255, 256);
          cNew.Set(x, y, KvBlendN(kv, kv2, y1 & 255, 256));
        }
    } else {
      for (y = 0; y < ynew; y++)
        for (x = 0; x < xnew; x++) {
          x1 = (x * m_x << 8) / xnew;
          y1 = (y * m_y << 8) / ynew;
          x2 = x1 >> 8; y2 = y1 >> 8;
          kv = Get(x2, y2);
          kv += (Get(x2+1, y2) - kv) * (x1 & 255) >> 8;
          kv2 = Get(x2, y2+1);
          kv2 += (Get(x2+1, y2+1) - kv2) * (x1 & 255) >> 8;
          cNew.Set(x, y, kv + ((kv2 - kv) * (y1 & 255) >> 8));
        }
    }
  } else {

    // Normal case: Make each destination pixel a copy of the one source pixel
    // that best maps to it.

    for (y = 0; y < ynew; y++)
      for (x = 0; x < xnew; x++)
        cNew.Set(x, y, Get(x * m_x / xnew, y * m_y / ynew));
  }
LDone:
  CopyFrom(cNew);
  return fTrue;
}


// Zoom a color bitmap and its contents by the given factors. Each source
// pixel becomes an x by y block in the destination, where x and y can be
// different for even and odd numbered rows and columns.

flag CCol::FBitmapBias(int xbias1, int xbias2, int ybias1, int ybias2)
{
  CCol cNew;
  int x, y;

  if (!cNew.FAllocate(
    (m_x >> 1)*(xbias1 + xbias2) + FOdd(m_x)*xbias1,
    (m_y >> 1)*(ybias1 + ybias2) + FOdd(m_y)*ybias1, this))
    return fFalse;
  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x; x++)
      cNew.Block((x >> 1)*(xbias1 + xbias2) + FOdd(x)*xbias1,
        (y >> 1)*(ybias1 + ybias2) + FOdd(y)*ybias1,
        ((x + 1) >> 1)*(xbias1 + xbias2) + FOdd(x + 1)*xbias1 - 1,
        ((y + 1) >> 1)*(ybias1 + ybias2) + FOdd(y + 1)*ybias1 - 1,
        Get(x, y));
  CopyFrom(cNew);
  return fTrue;
}


// Blur the colors in a bitmap together slightly.

flag CCol::FColmapBlur(flag fTorus)
{
  CCol cCopy;
  int x, y, xnew, ynew, d, cd, nR, nG, nB, nRT, nGT, nBT;
  byte *pb, *pb2, *pbT;
  flag fLegal;

  if (!cCopy.FBitmapCopy(*this))
    return fFalse;
  for (y = 0; y < m_y; y++) {
    pb = _Pb(0, y); pb2 = cCopy._Pb(0, y);
    for (x = 0; x < m_x; x++) {

      // For each pixel, average its color with its neighboring pixels.
      cd = 1;
      _Get(pb2, &nR, &nG, &nB);
      for (d = 0; d < DIRS2; d++) {
        xnew = x + xoff[d]; ynew = y + yoff[d];
        fLegal = FLegal(xnew, ynew);
        if (fLegal || fTorus) {
          if (fTorus && !fLegal)
            Legalize2(&xnew, &ynew);
          cd++;
          pbT = cCopy._Pb(xnew, ynew);
          cCopy._Get(pbT, &nRT, &nGT, &nBT);
          nR += nRT; nG += nGT; nB += nBT;
        }
      }
      d = cd >> 1;
      nR = (nR + d) / cd; nG = (nG + d) / cd; nB = (nB + d) / cd;
      _Set(pb, nR, nG, nB);
      pb += cbPixelC; pb2 += cbPixelC;
    }
  }
  return fTrue;
}


// Make a color bitmap have as high contrast as possible.

void CCol::ColmapContrast(flag fRainbow)
{
  int x, y, nR, nG, nB, rgn[3], i, iMin, iMax, nMin, nMax;
  byte *pb;

  if (!fRainbow) {

    // Standard contrast: Each pixel maps to the nearest of one of eight
    // colors, where all RGB values become 0 or 255.

    for (y = 0; y < m_y; y++) {
      pb = _Pb(0, y);
      for (x = 0; x < m_x; x++) {
        _Get(pb, &nR, &nG, &nB);
        nR = nR < 128 ? 0 : 255;
        nG = nG < 128 ? 0 : 255;
        nB = nB < 128 ? 0 : 255;
        _Set(pb, nR, nG, nB);
        pb += cbPixelC;
      }
    }
  } else {

    // Rainbow contrast: Each pixel maps to the nearest rainbow color to it.
    // A rainbow color has at least one RGB component of 255 and another 0,
    // so increase the highest RGB component to 255 and drop the lowest to 0.

    for (y = 0; y < m_y; y++) {
      pb = _Pb(0, y);
      for (x = 0; x < m_x; x++) {
        _Get(pb, &rgn[0], &rgn[1], &rgn[2]);
        nMax = -1; nMin = 256;
        for (i = 0; i < 3; i++)
          if (rgn[i] > nMax) {
            nMax = rgn[i]; iMax = i;
          }
        for (i = 0; i < 3; i++)
          if (rgn[i] <= nMin) {
            nMin = rgn[i]; iMin = i;
          }
        rgn[iMax] = 255; rgn[iMin] = 0;
        _Set(pb, rgn[0], rgn[1], rgn[2]);
        pb += cbPixelC;
      }
    }
  }
}


// Transform the pixels in a color bitmap in some special manner.

flag CCol::FColmapTransform(int nOp, int nFactor)
{
  CCol cCopy;
  int xmid, ymid, x, y, x2, y2;
  real rFactor = (real)-nFactor, rx, ry, rS, rC, rl, rlMax;

  if (!cCopy.FBitmapCopy(*this))
    return fFalse;
  xmid = m_x >> 1; ymid = m_y >> 1;
  rlMax = (real)Min(xmid, ymid);
  switch (nOp) {

  // Rotate all by degree.
  case 0:
    RotateR2Init(rS, rC, rFactor);
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++) {
        rx = (real)(x - xmid); ry = (real)(y - ymid);
        RotateR2(&rx, &ry, rS, rC);
        x2 = xmid + (int)rx; y2 = ymid + (int)ry;
        Set(x, y, cCopy.Get(x2, y2));
      }
    break;

  // Twist middle by degree.
  case 1:
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++) {
        rx = (real)(x - xmid); ry = (real)(y - ymid);
        rl = LengthR(0, 0, rx, ry);
        if (rl >= rlMax)
          continue;
        rl = 1.0 - (rl / rlMax);
        RotateR(&rx, &ry, Sq(rl) * rFactor);
        x2 = xmid + (int)rx; y2 = ymid + (int)ry;
        Set(x, y, cCopy.Get(x2, y2));
      }
    break;

  // Replace number with hue.
  case 2:
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++)
        Set(x, y, Hue(cCopy.Get(x, y) * nFactor / 100 % nHueMax));
    break;

  default:
    Assert(fFalse);
  }
  return fTrue;
}


CONST int rgnSprite[16] =
  {15, 0, 1, 4, 2, 12, 5, 9, 3, 7, 13, 8, 6, 11, 10, 14};

// Zoom a bitmap by a certain magnification based on the size and using the
// contents of a second bitmap. Use the status of a pixel and its four
// neighbors in a monochrome bitmap to determine which section of the second
// bitmap to use for the zoomed version of each pixel.

flag CMap::FZoomSprite(CMap *bMaz, CONST CMap &bPic)
{
  CMap *bT = NULL;
  flag fColor = FColor();
  int x1 = m_x, y1 = m_y, x2, y2, xd, yd, x, y, d, n, zp;

  x2 = bPic.m_x;
  y2 = bPic.m_y;
  zp = x2 >> 2;
  if (x2 < 1 || y2 < 1 || (x2 & 3) != 0 || y2 % zp != 0)
    return fFalse;
  if (!fColor) {
    bT = Create();
    if (bT == NULL)
      return fFalse;
    if (!bT->FBitmapCopy(*this)) {
      bT->Destroy();
      return fFalse;
    }
  } else
    bT = bMaz;
  if (!FBitmapSizeSet(x1 * zp, y1 * zp))
    return fFalse;
  BitmapOff();
  for (y = 0; y < y1; y++)
    for (x = 0; x < x1; x++) {
      for (n = 0, d = DIRS1; d >= 0; d--)
        n = (n << 1) | bT->Get(x + xoff[d], y + yoff[d]);
      n = rgnSprite[n] + (!bT->Get(x, y) * 16);
      xd = (n & 3) * zp; yd = (n >> 2) * zp;

      // Copy the section to the destination. If the second bitmap doesn't
      // have enough rows, use a solid black or white block.

      if (yd >= y2) {
        if (yd < x2)
          Block(x * zp, y * zp, x * zp + zp-1, y * zp + zp-1, kvWhite);
        continue;
      }
      BlockMove(bPic, xd, yd, xd + zp-1, yd + zp-1, x * zp, y * zp);
    }
  if (!fColor && bT != NULL)
    bT->Destroy();
  return fTrue;
}


// Replace the contents of a color bitmap with the pixels within an arbitrary
// quadrilateral in another bitmap. The source quadrilateral pixels get
// stretched to fit within the rectangle bounds of the destination bitmap.

void CCol::ColmapStretchToEdge(CONST CCol &c2, int x1, int y1,
  int x2, int y2, int x3, int y3, int x4, int y4)
{
  int xmax = m_x, ymax = m_y, x, y, m1, n1, m2, n2;

  if (FNull() || c2.FNull())
    return;
  c2.Legalize(&x1, &y1);
  c2.Legalize(&x2, &y2);
  c2.Legalize(&x3, &y3);
  c2.Legalize(&x4, &y4);
  for (y = 0; y < ymax; y++) {

    // Compute the source coordinates in the quadrilateral that map to the
    // current row in the destination.
    m1 = x1 + y * (x3 - x1) / ymax;
    n1 = y1 + y * (y3 - y1) / ymax;
    m2 = x2 + y * (x4 - x2) / ymax;
    n2 = y2 + y * (y4 - y2) / ymax;

    for (x = 0; x < xmax; x++)
      Set(x, y,
        c2.Get(m1 + x * (m2 - m1) / xmax, n1 + x * (n2 - n1) / xmax));
  }
}


// Display information about the pixels in a color bitmap, such as number and
// percentage of pure black pixels, white pixels, and grayscale pixels, along
// with the amount of red, green, and blue in the average pixel.

long CCol::BitmapCount() CONST
{
  char sz[cchSzMax], sz1[cchSzMax], sz2[cchSzMax], sz3[cchSzMax];
  long tot, bl = 0, wh = 0, gr = 0, r = 0, g = 0, b = 0, l;
  int x, y;
  KV kv;

  tot = m_x*m_y;
  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x; x++) {
      kv = Get(x, y);
      if (kv == kvBlack)
        bl++;
      else if (kv == kvWhite)
        wh++;
      else if (RgbR(kv) == RgbG(kv) && RgbG(kv) == RgbB(kv))
        gr++;
      r += RgbR(kv); g += RgbG(kv); b += RgbB(kv);
    }
  sprintf(S(sz1),
    "Total pixels: %ld\nBlack: %ld (%.2f%%), White: %ld (%.2f%%)",
    tot, bl, tot ? (real)bl/(real)tot*100.0 : 0.0,
    wh, tot ? (real)wh/(real)tot*100.0 : 0.0);
  l = tot-bl-wh-gr;
  sprintf(S(sz2), ", Grayscale: %ld (%.2f%%), Color: %ld (%.2f%%)\n",
    gr, tot ? (real)gr/(real)tot*100.0 : 0.0,
    l, tot ? (real)l/(real)tot*100.0 : 0.0);
  l = r + g + b;
  sprintf(S(sz3), "Red: %.2f%%, Green: %.2f%%, Blue: %.2f%%, Total: %.2f%%\n",
    tot ? (real)r/255.0/(real)tot*100.0 : 0.0,
    tot ? (real)g/255.0/(real)tot*100.0 : 0.0,
    tot ? (real)b/255.0/(real)tot*100.0 : 0.0,
    tot ? (real)l/765.0/(real)tot*100.0 : 0.0);
  sprintf(S(sz), "%s%s%s", sz1, sz2, sz3);
  PrintSz_N(sz);
  return wh;
}


// Search a color bitmap for a pixel that has the specified characteristic.

long CCol::ColmapFind(int nOp) CONST
{
  int x, y, n, i;
  byte *pb;
  KV kvMax, kv;

  switch (nOp) {

  // Return the maximum value pixel in a color bitmap.
  case 0:
    kvMax = 0;
    for (y = 0; y < m_y; y++) {
      pb = _Pb(0, y);
      for (x = 0; x < m_x; x++) {
        kv = _Get(pb);
        if (kv > kvMax)
          kvMax = kv;
      }
    }
    return kvMax;

  // Return a random color pixel that's adjacent to a black pixel.
  case 1:
    for (i = 1; i <= DIRS; i++) {
      n = 0;
      for (y = 1; y < m_y-1; y++)
        for (x = 1; x < m_x-1; x++)
          if (Get(x, y) > kvBlack &&
            ((Get(x, y-1) == kvBlack) + (Get(x-1, y) == kvBlack) +
            (Get(x, y+1) == kvBlack) + (Get(x+1, y) == kvBlack) == i))
            n++;
      if (n < 1)
        continue;
      n = Rnd(1, n);
      for (y = 1; y < m_y-1; y++)
        for (x = 1; x < m_x-1; x++)
          if (Get(x, y) > kvBlack &&
            ((Get(x, y-1) == kvBlack) + (Get(x-1, y) == kvBlack) +
            (Get(x, y+1) == kvBlack) + (Get(x+1, y) == kvBlack) == i)) {
            n--;
            if (n < 1)
              return LFromWW(x, y);
          }
    }
    return LFromWW(m_x >> 1, m_y >> 1);

  default:
    Assert(fFalse);
  }
  return 0;
}


// Zoom a color bitmap by 200% plus one extra pixel. Boundaries or edges
// between different colored pixels in the old bitmap, become lines of pixels
// in the new whose color is the difference between those colors.

flag CCol::FBitmapAccentBoundary()
{
  CCol cNew;
  int x, y, xnew, ynew;
  KV kv0, kv1, kv2, kv3, kvT;

  if (!cNew.FAllocate((m_x << 1) + 1, (m_y << 1) + 1, this))
    return fFalse;
  cNew.BitmapOff();
  for (y = -1; y < m_y; y++) {
    ynew = (y + 1) << 1;
    for (x = -1; x < m_x; x++) {
      xnew = (x + 1) << 1;
      kv0 = Get(x, y);   kv1 = Get(x+1, y);
      kv2 = Get(x, y+1); kv3 = Get(x+1, y+1);
      if (kv0 != kv3 || kv1 != kv3 || kv2 != kv3) {

        // When 3+ colors meet at a corner, pick the brightest difference.
        kvT = KvDiff(kv0, kv3);
        kvT = Max(kvT, KvDiff(kv1, kv3));
        kvT = Max(kvT, KvDiff(kv2, kv3));
        kvT = Max(kvT, KvDiff(kv1, kv2));
        cNew.Set(xnew, ynew, kvT);
      }
      if (kv1 != kv3)
        cNew.Set(xnew+1, ynew, KvDiff(kv1, kv3));
      if (kv2 != kv3)
        cNew.Set(xnew, ynew+1, KvDiff(kv2, kv3));
    }
  }
  CopyFrom(cNew);
  return fTrue;
}


// Change a color bitmap so that each pixel's RGB components are set to the
// maximum difference between that pixel and all its neighbors.

flag CCol::FColmapAccentContrast(flag fCorner)
{
  CCol cCopy;
  int x, y, d, dMax = DIRS + fCorner*DIRS, xT, yT,
    nR, nG, nB, nRT, nGT, nBT, nRHi, nGHi, nBHi;
  KV kv, kvT;

  if (!cCopy.FBitmapCopy(*this))
    return fFalse;
  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x; x++) {
      nRHi = nGHi = nBHi = 0;
      kv = cCopy.Get(x, y);
      nR = RgbR(kv); nG = RgbG(kv); nB = RgbB(kv);
      for (d = 0; d < dMax; d++) {
        xT = x + xoff[d]; yT = y + yoff[d];
        if (FLegal(xT, yT)) {
          kvT = cCopy.Get(xT, yT);
          nRT = RgbR(kvT); nGT = RgbG(kvT); nBT = RgbB(kvT);
          nRT = (nR - nRT); nGT = (nG - nGT); nBT = (nB - nBT);
          if (nRT > nRHi)
            nRHi = nRT;
          if (nGT > nGHi)
            nGHi = nGT;
          if (nBT > nBHi)
            nBHi = nBT;
        }
      }
      Set(x, y, Rgb(nRHi, nGHi, nBHi));
    }
  return fTrue;
}


CONST int rggrfSeen[9] = {NBinary(1,1,0,0,0,0,0,1), NBinary(1,1,0,0,0,1,1,1),
  NBinary(0,0,0,0,0,1,1,1), NBinary(1,1,1,1,0,0,0,1), 0,
  NBinary(0,0,0,1,1,1,1,1), NBinary(0,1,1,1,0,0,0,0),
  NBinary(0,1,1,1,1,1,0,0), NBinary(0,0,0,1,1,1,0,0)};

// Highlight all pixels visible via line of sight, within a certain range from
// a starting point, with colors from another bitmap. Implements the DrawSeen
// operation.

void CCol::DrawSeen(CONST CMon &b, CONST CCol &c2, int x0, int y0, int dr,
  flag fCorner)
{
  int m1, n1, m2, n2, m, n, rSq, xnew, ynew;
  flag fCircle = dr < 0, fAllLegal = FLegal(x0, y0);
  int x, y, dx, dy, xInc, yInc, xInc2, yInc2, d, dInc, z, zMax,
    dm, dn, igrf, e;

  if (fCircle) {
    neg(dr);
    rSq = Sq(dr);
  }
  m1 = NMax(x0 - dr, 0); n1 = NMax(y0 - dr, 0);
  m2 = NMin(x0 + dr, m_x - 1); n2 = NMin(y0 + dr, m_y - 1);

  // Loop over each potentially visible pixel within the maximum range.
  for (n = n1; n <= n2; n++)
    for (m = m1; m <= m2; m++) {
      if (fCircle && Sq(m - x0) + Sq(n - y0) > rSq)
        continue;
      if (b.Get(m, n) || Get(m, n) == c2.Get(m, n))
        continue;
      dm = NSgn(m - x0); dn = NSgn(n - y0);
      igrf = dn * 3 + dm + 4;

      // For each pixel, check for walls along a line from the viewer to it.
      x = x0; y = y0;
      dx = m - x0; dy = n - y0;
      if (NAbs(dx) >= NAbs(dy)) {
        xInc = NSgn(dx); yInc = 0;
        xInc2 = 0; yInc2 = NSgn(dy);
        zMax = NAbs(dx); dInc = NAbs(dy);
        d = zMax - (!FOdd(dx) && x0 > m);
      } else {
        xInc = 0; yInc = NSgn(dy);
        xInc2 = NSgn(dx); yInc2 = 0;
        zMax = NAbs(dy); dInc = NAbs(dx);
        d = zMax - (!FOdd(dy) && y0 > n);
      }
      d >>= 1;

      // Loop over long axis, adjusting short axis for slope as needed.
      for (z = 0; z <= zMax; z++) {
        if (fAllLegal || (FBetween(x, -1, m_x) && FBetween(y, -1, m_y))) {
          if (b.GetFast(x, y))
            break;
          if (FLegal(x, y))
            Set(x, y, c2.Get(x, y));

          // Set wall pixels adjacent to this space.
          for (e = 0; e < DIRS2; e++) {
            if ((rggrfSeen[igrf] & (1 << e)) == 0)
              continue;
            xnew = x + xoff[e]; ynew = y + yoff[e];
            if (!FBetween(xnew, m1, m2) || !FBetween(ynew, n1, n2) ||
              (fCircle && Sq(xnew - x0) + Sq(ynew - y0) > rSq))
              continue;
            if (b.GetFast(xnew, ynew))
              Set(xnew, ynew, c2.Get(xnew, ynew));
          }
        }
        x += xInc; y += yInc; d += dInc;
        if (d >= zMax) {
          x += xInc2; y += yInc2; d -= zMax;

          // Don't let line hop diagonally between set pixels.
          if (!fCorner && b.GetFast(x - dm, y) && b.GetFast(x, y - dn))
            break;
        }
      }
    }
}


// Create a simple random dot stereogram in a color bitmap, based on the two
// level image in a monochrome bitmap. A second color bitmap may be used for
// the random dots.

flag CCol::FColmapStereogram(CONST CMon &b, CONST CCol &cRandom,
  int width, int height)
{
  int x, y, h, w, wDrop = NAbs(height);
  flag fDeep = height < 0, fTexture, f;

  if (!FBitmapSizeSet(b.m_x, b.m_y))
    return fFalse;
  fTexture = !cRandom.FNull() && cRandom.m_x == m_x && cRandom.m_y == m_y;

  for (y = 0; y < m_y; y++) {
    for (x = 0; x < m_x; x++) {
      f = b._Get(x, y);
      h = f ? height : 0;
      w = width - h;
      if (x - w < 0 || (f == fDeep && b.Get(x - wDrop, y) == !fDeep))
        Set(x, y, fTexture ? cRandom.Get(x, y) : kvRandomRainbow);
      else
        Set(x, y, Get(x - w, y));
    }
  }
  return fTrue;
}


// Treat the color bitmap as a board of the "Life" cellular automaton, and
// create the next generation. A second bitmap contains indexes of the colors
// to use for each cell in the main bitmap.

long CCol::ColmapLifeGenerate(CCol &c2, flag fTorus)
{
  CCol cT;
  int rgk[8], x, y, i, n, k, xT, yT;
  byte *pb;
  long count = 0L, cbRow, l;

  // If the second bitmap doesn't exist in the right size, it can't contain
  // valid color information, so recreate it with random colors.
  if (c2.FNull() || c2.m_x != m_x+2 || c2.m_y != m_y+2) {
    if (!c2.FBitmapSizeSet(m_x+2, m_y+2))
      return -1;
    c2.BitmapSet(kvBlack);
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++)
        if (Get(x, y) > kvBlack) {
          k = Rnd(0, nHueMax-1);
          c2.Set(x+1, y+1, Rgb(k & 255, k >> 8, 1));
          Set(x, y, Hue(k));
        }
    UpdateDisplay();
  }

  // For torus worlds, copy the edge cells to the opposite sides. The extra
  // pixel width around the edges allows quick counting of neighboring cells,
  // without having to check for the edge of the bitmap.
  if (fTorus) {
    for (y = 1; y <= c2.m_y-2; y++) {
      c2.Set(0, y, c2.Get(c2.m_x-2, y));
      c2.Set(c2.m_x-1, y, c2.Get(1, y));
    }
    for (x = 1; x <= c2.m_x-2; x++) {
      c2.Set(x, 0, c2.Get(x, c2.m_y-2));
      c2.Set(x, c2.m_y-1, c2.Get(x, 1));
    }
    c2.Set(0,        0,        c2.Get(c2.m_x-2, c2.m_y-2));
    c2.Set(c2.m_x-1, 0,        c2.Get(1,        c2.m_y-2));
    c2.Set(0,        c2.m_y-1, c2.Get(c2.m_x-2, 1       ));
    c2.Set(c2.m_x-1, c2.m_y-1, c2.Get(1,        1       ));
  }

  if (!cT.FBitmapCopy(c2))
    return -1;
  cbRow = c2.m_clRow << 2;
  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x; x++) {

      // Quickly count the number of neighboring live cells.
      l = cT._Ib(x+1, y+1);
      pb = cT._Pb(l);
      k = (*(pb+1) << 8) | *(pb+2);
      n = *(pb-cbRow) + *(pb+cbRow) + *(pb-3) + *(pb+3) +
        *(pb-cbRow-3) + *(pb-cbRow+3) + *(pb+cbRow-3) + *(pb+cbRow+3);
      if (*pb) {
        if (gs.grfLifeDie & (1 << n)) {

          // A cell dies.
          pb = c2._Pb(l);
          _Set(pb, 0, 0, 0);
          Set(x, y, kvBlack);
          count++;
        }
      } else {
        if (gs.grfLifeBorn & (1 << n)) {

          // A new cell is born.
          n = 0;
          for (i = 0; i < DIRS2; i++) {
            xT = x + xoff[i]; yT = y + yoff[i];
            pb = cT._Pb(xT+1, yT+1);
            if (*pb)
              rgk[n++] = (*(pb+1) << 8) | *(pb+2);
          }

          // Find the midpoint of three colors of the rainbow of the parent
          // cells, and make a new color of the rainbow based on them.
          k = (rgk[0] + rgk[1] + rgk[2] + 1) / 3;
          if (NAbs(k - rgk[0]) + NAbs(k - rgk[1]) +
            NAbs(k - rgk[2]) > nHue23) {
            k += nHue13 * ((rgk[0] >= nHueHalf) + (rgk[1] >= nHueHalf) +
              (rgk[2] >= nHueHalf) >= 2 ? 1 : -1);
            if (k < 0)
              k += nHueMax;
            else if (k >= nHueMax)
              k -= nHueMax;
          }
          pb = c2._Pb(l);
          c2._Set(pb, k & 255, k >> 8, 1);
          Set(x, y, Hue(k));
          count++;
        }
      }
    }
  return count;
}


// Treat the color bitmap as a board of the "Evolution" cyclic cellular
// automaton, and create the next generation. A second bitmap contains indexes
// of the colors to use for each cell in the main bitmap.

long CCol::Evolution(CCol &c2, int col,
  flag fContinue, flag fTorus, flag fCorner)
{
  CCol cT;
  KV rgkv[32];
  byte *pb;
  int x, y, i;
  long count = 0L, cbRow, l;

  if (!c2.FBitmapSizeSet(m_x+2, m_y+2))
    return -1;
  Assert(col < 32);
  for (i = 0; i < col; i++)
    rgkv[i] = Hue(NMultDiv(i, nHueMax, col));

  // For the initial generation, give every pixel a random color.
  if (!fContinue) {
    c2.BitmapSet(kvWhite);
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++) {
        i = Rnd(0, col-1);
        c2.Set(x+1, y+1, i);
        Set(x, y, rgkv[i]);
      }
    return 0;
  }

  // For torus worlds, copy the edge cells to the opposite sides. The extra
  // pixel width around the edges allows quick checking of four neighboring
  // cells, without having to check for the edge of the bitmap.
  if (fTorus) {
    for (y = 1; y <= c2.m_y-2; y++) {
      c2.Set(0, y, c2.Get(c2.m_x-2, y));
      c2.Set(c2.m_x-1, y, c2.Get(1, y));
    }
    for (x = 1; x <= c2.m_x-2; x++) {
      c2.Set(x, 0, c2.Get(x, c2.m_y-2));
      c2.Set(x, c2.m_y-1, c2.Get(x, 1));
    }
  } else {
    // For non-torus worlds, copy the edge cells to the same sides, to prevent
    // any color flowing to/from off the edges of the bitmap.
    for (y = 1; y <= c2.m_y-2; y++) {
      c2.Set(0, y, c2.Get(1, y));
      c2.Set(c2.m_x-1, y, c2.Get(c2.m_x-2, y));
    }
    for (x = 1; x <= c2.m_x-2; x++) {
      c2.Set(x, 0, c2.Get(x, 1));
      c2.Set(x, c2.m_y-1, c2.Get(x, c2.m_y-2));
    }
  }

  if (!cT.FBitmapCopy(c2))
    return -1;
  cbRow = c2.m_clRow << 2;
  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x; x++) {

      // Quickly check if a neighboring cell is one color higher.
      l = cT._Ib(x+1, y+1);
      pb = cT._Pb(l) + 2;
      i = *pb + 1;
      if (i >= col)
        i = 0;
      if (*(pb-cbRow) == i || *(pb-3) == i ||
        *(pb+cbRow) == i || *(pb+3) == i || (fCorner && (
        *(pb-cbRow-3) == i || *(pb-cbRow+3) == i ||
        *(pb+cbRow-3) == i || *(pb+cbRow+3) == i))) {
        count++;

        // Set the current cell's color to the neighbor cell's color.
        *(c2._Pb(l) + 2) = i;
        Set(x, y, rgkv[i]);
      }
    }
  return count;
}


#define cMandy 1536

// Create a Mandelbrot set fractal in a color bitmap, looking upon the
// specified rectangle of coordinates.

void CCol::Mandelbrot(real xmin, real ymin, real xmax, real ymax,
  int nLimit, flag fShip)
{
  KV rgkv[cMandy], kv;
  byte *pb;
  real x, y, xInc, yInc, m, n, z;
  int iMax = Min(nLimit, cMandy), xa, ya, color, i;

  // Define the set of colors to use for the various depths.
  if (!cs.fGraphNumber) {
    for (i = iMax-1; i >= 0; i--)
      rgkv[i] = Hue(NMultDiv(i, nHueMax, cMandy));
  } else {
    for (i = iMax-1; i >= 0; i--)
      rgkv[i] = iMax - i;
  }

  BitmapSet(kvBlack);
  xInc = (xmax - xmin) / m_x;
  yInc = (ymax - ymin) / m_y;
  for (ya = 0, y = ymin; ya < m_y; ya++, y += yInc) {
    pb = _Pb(0, ya);
    for (xa = 0, x = xmin; xa < m_x; xa++, x += xInc) {
      m = x; n = y;
      if (!fShip) {
        for (color = 0; color < nLimit; color++) {
          z = m * n;
          m *= m; n *= n;
          if (m + n > 4.0)
            break;
          m = m - n + x;
          n = z + z + y;
        }
      } else {
        for (color = 0; color < nLimit; color++) {
          z = m * n;
          m *= m; n *= n;
          if (m + n > 4.0)
            break;
          m = m - n + x;
          n = z + z + y;
          if (m < 0)
            neg(m);
          if (n < 0)
            neg(n);
        }
      }

      // If the depth limit wasn't reached, set the pixel to a color.
      if (color < nLimit) {
        while (color >= cMandy)
          color -= cMandy;
        kv = rgkv[color];
        _Set(pb, RgbR(kv), RgbG(kv), RgbB(kv));
      }
      pb += cbPixelC;
    }
  }
}


/*
******************************************************************************
** Color Bitmap IO Routines
******************************************************************************
*/

// Copy a monochrome bitmap to a color bitmap. Off and on pixels in the
// monochrome bitmap are mapped to the given colors.

flag CCol::FColmapGetFromBitmap(CONST CMon &b, KV kv0, KV kv1)
{
  int x, y, nR, nG, nB;
  byte *pb;

  if (!FBitmapSizeSet(b.m_x, b.m_y))
    return fFalse;
  BitmapSet(kv0);
  nR = RgbR(kv1); nG = RgbG(kv1); nB = RgbB(kv1);
  for (y = 0; y < m_y; y++) {
    pb = _Pb(0, y);
    for (x = 0; x < m_x; x++) {
      if (b.Get(x, y))
        _Set(pb, nR, nG, nB);
      pb += cbPixelC;
    }
  }
  Copy3(b);
  return fTrue;
}


// Copy a color bitmap to a monochrome bitmap, in a specified fashion.

flag CCol::FColmapPutToBitmap(CMon &b, int nOp) CONST
{
  int x, y;
  KV kv;

  if (!b.FBitmapSizeSet(m_x, m_y))
    return fFalse;
  b.BitmapOff();
  switch (nOp) {

  // Non-black pixels in the color bitmap become white, all others black.
  case 0:
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++)
        if (Get(x, y) > 0)
          b.Set1(x, y);
    break;

  // Light colors in the color bitmap become white, dark colors become black.
  case 1:
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++) {
        kv = Get(x, y);
        if (RgbR(kv) + RgbG(kv) + RgbB(kv) > 127*3+1)
          b.Set1(x, y);
      }
    break;

  // Dither: Colors become white or black in proportion to their brightness.
  case 2:
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++) {
        kv = Get(x, y);
        if (FDither((RgbR(kv) + RgbG(kv) + RgbB(kv)) / 45, x, y))
          b.Set1(x, y);
      }
    break;

  default:
    Assert(fFalse);
  }
  b.Copy3(*this);
  return fTrue;
}


// Combine a monochrome bitmap with a color bitmap using the specified
// operator, where off and on pixels in the monochrome bitmap are treated as
// the given colors.

void CCol::ColmapOrAndFromBitmap(CONST CMon &b, KV kv0, KV kv1, int nOp)
{
  int x, y, x1, y1;
  KV kv;

  x1 = Min(m_x, b.m_x); y1 = Min(m_y, b.m_y);
  for (y = 0; y < y1; y++)
    for (x = 0; x < x1; x++) {
      kv = b.Get(x, y) ? kv1 : kv0;
      switch (nOp) {
      case 0: Set(x, y, Get(x, y) & kv); break;
      case 1: Set(x, y, Get(x, y) | kv); break;
      case 2: Set(x, y, Get(x, y) ^ kv); break;
      default: Assert(fFalse);
      }
    }
}


// Blend three monochrome bitmaps together into one color bitmap. Each pixel
// in the color bitmap will be one of eight colors.

flag CCol::FColmapBlendFromBitmap(CONST CMon *b1, CONST CMon *b2,
  CONST CMon *b3)
{
  int x, y;

  if (b2 == NULL || b2->FNull())
    b2 = b1;
  if (b3 == NULL || b3->FNull())
    b3 = b2;
  x = Min(b1->m_x, b2->m_x); x = Min(x, b3->m_x);
  y = Min(b1->m_y, b2->m_y); y = Min(y, b3->m_y);
  if (!FBitmapSizeSet(x, y))
    return fFalse;

  // Bitmap #1 is treated as red on black, bitmap #2 is green on black, and
  // bitmap #3 is blue on black, where these RGB values are merged together.
  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x; x++)
      Set(x, y, Rgb(b1->Get(x, y)*255, b2->Get(x, y)*255, b3->Get(x, y)*255));
  return fTrue;
}


// Make a color bitmap an equal blending together of N other color bitmaps.
// The N other bitmaps are stacked vertically within a second bitmap. Black
// pixels are skipped, and make that subbitmap not contribute to the result.

void CCol::ColmapBlendBitmaps(CONST CCol &c)
{
  int cc, x, y, i, nT, ccol, nR, nG, nB;
  KV kv;

  BitmapOff();
  cc = c.m_y + (m_y - 1) / m_y;
  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x; x++) {
      ccol = nR = nG = nB = 0;
      for (i = 0; i < cc; i++) {
        kv = c.Get(x, i * m_y + y);
        if (kv == kvBlack)
          continue;
        ccol++;
        nR += RgbR(kv);
        nG += RgbG(kv);
        nB += RgbB(kv);
      }
      if (ccol <= 0)
        continue;
      nT = ccol >> 1;
      nR = (nR + nT) / ccol;
      nG = (nG + nT) / ccol;
      nB = (nB + nT) / ccol;
      kv = Rgb(nR, nG, nB);
      if (kv != kvBlack)
        Set(x, y, kv);
    }
}


// Copy a monochrome bitmap to a color bitmap, where off and on pixels in the
// monochrome bitmap are mapped to the passed in colors. All off pixels
// reachable from a start point or points will be filled with a blend of
// colors indicating their distance from the nearest start point.

int CCol::ColmapGraphDistance(CONST CMon &b, CONST CMon &b2,
  KV kv0, KV kv1, int x, int y, flag fCorner)
{
  BFSS *bfss;
  int count = 0, xnew, ynew, d, j = DIRS + fCorner*DIRS;
  long iLo = 0, iHi, iMax = 0, i;

  if ((!b.FLegal(x, y) || b.Get(x, y)) && !b.FBitmapFind(&x, &y, fOff)) {
    PrintSz_W("There are no open sections to graph.\n");
    return -2;
  }
  if (b.F64K())
    return -2;
  if (!FColmapGetFromBitmap(b, kv0, kv1))
    return -1;
  bfss = RgAllocate(m_x*m_y, BFSS);
  if (bfss == NULL)
    return -1;
  Set(x, y, kv1);
  BfssPush(iMax, x, y, 0);

  // Potentially allow all off pixels in a 2nd monochrome bitmap to act as
  // start points to flood from.
  if (b.FBitmapSubset(b2))
    for (ynew = 0; ynew < b2.m_y; ynew++)
      for (xnew = 0; xnew < b2.m_x; xnew++)
        if (!b2.Get(xnew, ynew)) {
          Assert(!b.Get(xnew, ynew));
          Set(xnew, ynew, kv1);
          BfssPush(iMax, xnew, ynew, 0);
        }

  // Flood bitmap from start points, marking distances for each pixel touched.
  iHi = iMax;
  while (iLo < iHi) {
    count++;
    for (i = iLo; i < iHi; i++) {
      x = bfss[i].x; y = bfss[i].y;
      for (d = 0; d < j; d++) {
        xnew = x + xoff[d]; ynew = y + yoff[d];
        if (FLegal(xnew, ynew) && Get(xnew, ynew) != kv1) {
          Set(xnew, ynew, kv1);
          BfssPush(iMax, xnew, ynew, count);
        }
      }
    }
    iLo = iHi; iHi = iMax;
  }

  // Map distance numbers to colors.
  if (!cs.fGraphNumber) {
    for (i = 0; i < iMax; i++)
      Set(bfss[i].x, bfss[i].y,
        Hue(NMultDiv(bfss[i].parent, nHueMax, count)));
  } else {
    for (i = 0; i < iMax; i++)
      Set(bfss[i].x, bfss[i].y, bfss[i].parent);
  }
  DeallocateP(bfss);
  return count - 1;
}


#define FValidGraph(x, y) (b.Get(x, y) && Get(x, y) == kv1)

// Copy a monochrome bitmap to a color bitmap, where off and on pixels in the
// monochrome bitmap are mapped to the passed in colors. One pixel wide walls
// that don't surround inaccessible sections (and aren't linked on both ends
// to walls surrounding them) will be set to blend of colors indicating their
// distance from the farthest wall endpoint within the local wall network. In
// other words, for each wall segment this determines its "importance", or
// graphs the approximate distance it takes to walk from one side of the wall
// to the other (if even possible) by going around the end of the wall.

long CCol::ColmapGraphDistance2(CONST CMon &b, KV kv0, KV kv1, flag fCorner)
{
  PT *rgpt;
  int mpgrff[256], x, y, xnew, ynew, d, dSav, cd, grf, i;
  long cpt = 0, ipt, iMax = 0;
  KV kv;

  if (!FColmapGetFromBitmap(b, kv0, kv1))
    return -1;
  rgpt = RgAllocate(m_x*m_y, PT);
  if (rgpt == NULL)
    return -1;
  if (fCorner) {
    for (i = 0; i < 256; i++)
      mpgrff[i] = fFalse;
    for (i = 0; i < 25; i++)
      mpgrff[rggrfNeighbor[i]] = fTrue;
  }

  // Determine the set of wall endpoints to start flooding from.
  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x; x++)
      if (b.Get(x, y)) {
        if (!fCorner) {

          // Starts for orthogonal graphing are on pixels with <= 1 neighbor.
          cd = 0;
          for (d = 0; d < DIRS; d++)
            if (b.Get(x + xoff[d], y + yoff[d])) {
              if (cd > 0)
                goto LNext;
              cd = 1;
            }
        } else {

          // Starts for diagonal graphing are on pixels with no more than one
          // consecutive group of <= 3 neighbors.
          grf = 0;
          for (d = 0; d < DIRS2; d++) {
            grf <<= 1;
            if (b.Get(x + xoff[d], y + yoff[d]))
              grf |= 1;
          }
          if (!mpgrff[grf])
            goto LNext;
        }
        rgpt[cpt].x = x; rgpt[cpt].y = y;
        cpt++;
LNext:
        ;
      }

  // Flood the bitmap from the start points, marking distances for each pixel.
  while (cpt > 0) {
    iMax++;
    for (ipt = 0; ipt < cpt; ipt++) {
      x = rgpt[ipt].x; y = rgpt[ipt].y;
      if (Get(x, y) != kv1)
        goto LStop;
      cd = 0;
      if (!fCorner) {

        // For orthogonal graphing, stop flooding along a wall if the current
        // pixel has > 1 unflooded neighbor.
        for (d = 0; d < DIRS; d++) {
          xnew = x + xoff[d]; ynew = y + yoff[d];
          if (FValidGraph(xnew, ynew)) {
            if (cd > 0)
              goto LStop;
            cd = 1;
            dSav = d;
          }
        }
      } else {

        // For diagonal graphing, first get the count and arrangement of the
        // current cell's unflooded neighbors.
        grf = 0;
        for (d = 0; d < DIRS2; d++) {
          grf <<= 1;
          xnew = x + xoff[d]; ynew = y + yoff[d];
          if (FValidGraph(xnew, ynew)) {
            grf |= 1;
            cd++;
            dSav = d;
          }
        }

        // If current pixel has no more than one consecutive group of <= 3
        // neighbors, pick a direction to flood in that will keep that true.
        if (cd > 1) {
          if (mpgrff[grf])
            Set(x, y, iMax);
          for (d = 0; d < DIRS2; d++) {
            xnew = x + xoff[d]; ynew = y + yoff[d];
            if (FValidGraph(xnew, ynew)) {
              grf = 0;
              for (i = 0; i < DIRS2; i++) {
                grf <<= 1;
                if (FValidGraph(xnew + xoff[i], ynew + yoff[i])) {
                  grf |= 1;
                  cd++;
                  dSav = d;
                }
              }
              if (cd >= 1 && mpgrff[grf])
                goto LGo;
            }
          }
          goto LStop;
        }
      }
      Set(x, y, iMax);
      if (cd < 1)
        goto LStop;
LGo:
      x += xoff[dSav]; y += yoff[dSav];
      rgpt[ipt].x = x; rgpt[ipt].y = y;
      continue;
LStop:
      rgpt[ipt] = rgpt[cpt-1];
      cpt--; ipt--;
    }
  }

  // Map distance numbers to colors.
  if (!cs.fGraphNumber)
    for (y = 0; y < b.m_y; y++)
      for (x = 0; x < b.m_x; x++)
        if (b.Get(x, y)) {
          kv = Get(x, y);
          if (kv != kv1)
            Set(x, y, Hue(NMultDiv(kv, nHueMax, iMax)));
        }
  DeallocateP(rgpt);
  return iMax;
}


// Copy a monochrome bitmap to a color bitmap, with antialiasing. Off and on
// pixels in the monochrome bitmap are mapped to the given colors. The color
// bitmap will be zoomed z times smaller, where each pixel will become the
// right ratio between the off and on colors, based on the number of off and
// on pixels in the monochrome bitmap that map to that pixel.

flag CCol::FColmapAntialias(CONST CMon &b, KV kv0, KV kv1, int z)
{
  int x, y, x2, y2, xmax, ymax, n, zSq;

  xmax = b.m_x / z; ymax = b.m_y / z;
  if (!FBitmapSizeSet(xmax, ymax))
    return fFalse;
  zSq = Sq(z);
  for (y = 0; y < ymax; y++)
    for (x = 0; x < xmax; x++) {
      n = 0;
      for (y2 = 0; y2 < z; y2++)
        for (x2 = 0; x2 < z; x2++)
          n += b.Get(x*z + x2, y*z + y2);
      Set(x, y, KvBlendN(kv0, kv1, n, zSq));
    }
  return fTrue;
}


// Replace colors in a bitmap using colors from a second bitmap. The original
// bitmap contains index numbers, and the second bitmap acts as a palette.

void CCol::ColmapPalette(CONST CCol &c)
{
  int x, y, x2, y2, i;
  KV kv;

  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x; x++) {
      i = Get(x, y);
      y2 = i / c.m_x;
      x2 = i - y2 * c.m_x;
      if (y2 < c.m_y && x2 < c.m_x) {
        kv = c.Get(x2, y2);
        if (kv != kvBlack)
          Set(x, y, kv);
      }
    }
}


// Load a color bitmap from a Windows bitmap format file. It is assumed
// FReadBitmapHeader() has already been called.

flag CCol::FReadColmapCore(FILE *file, int x, int y, int z, int k)
{
  int cb, i;
  byte *pb, bR, bG, bB, ch;
  KV rgkv[256], kv;

  Assert(z == 4 || z == 8 || z == 16 || z == 24 || z == 32);
  // RgbQuad
  // Data
  if (!FBitmapSizeSet(x, y))
    return fFalse;

  // Figure out the bytes per row in this color bitmap.
  if (z == 32)
    cb = m_x << 2;
  else if (z == 24)
    cb = m_x * 3;
  else if (z == 16) {
    for (i = 0; i < 12; i++)
      skipbyte();
    cb = m_x << 1;
  } else {
    // Read in the color palette to translate indexes to RGB values.
    Assert(k <= 256);
    if (k == 0)
      k = (z == 8) ? 256 : 16;
    for (i = 0; i < k; i++) {
      bB = getbyte(); bG = getbyte(); bR = getbyte();
      skipbyte();
      rgkv[i] = Rgb(bR, bG, bB);
    }
    if (z == 8)
      cb = m_x;
    else
      cb = (m_x + 1) >> 1;
  }
  // Figure out the number of padding bytes after each row.
  cb = (4 - (cb & 3)) & 3;

  for (y = m_y-1; y >= 0; y--) {
    pb = _Pb(0, y);
    for (x = 0; x < m_x; x++) {
      if (z >= 24) {
        bB = getbyte(); bG = getbyte(); bR = getbyte();
        if (z == 32)
          skipbyte();
      } else {
        if (z == 16) {
          i = WRead(file);
          kv = Rgb((i >> 11) << 3, (i >> 5 & 63) << 2, (i & 31) << 3);
        } else if (z == 8) {
          ch = getbyte();
          kv = rgkv[ch];
        } else {
          if (!FOdd(x))
            ch = getbyte();
          i = FOdd(x) ? (ch & 15) : (ch >> 4);
          kv = rgkv[i];
        }
        bR = RgbR(kv); bG = RgbG(kv); bB = RgbB(kv);
      }
      _Set(pb, bR, bG, bB);
      pb += cbPixelC;
    }
    for (x = 0; x < cb; x++)
      skipbyte();
  }
  return fTrue;
}


// Save a color bitmap to a Windows bitmap format file.

void CCol::WriteColmap(FILE *file) CONST
{
  int x, y, cb;
  dword dw;

  cb = (4 - ((m_x*3) & 3)) & 3;
  // BitmapFileHeader
  putbyte('B'); putbyte('M');
  dw = 14+40 + 0 + m_y*(m_x*3+cb);
  putlong(dw);
  putword(0); putword(0);
  putlong(14+40 + 0);
  // BitmapInfo / BitmapInfoHeader
  putlong(40);
  putlong(m_x); putlong(m_y);
  putword(1); putword(24);
  putlong(0 /*BI_RGB*/); putlong(0);
  putlong(0); putlong(0);
  putlong(0); putlong(0);
  // RgbQuad
  // Data
  for (y = m_y-1; y >= 0; y--) {
    for (x = 0; x < m_x; x++) {
      dw = Get(x, y);
      putbyte(RgbB(dw)); putbyte(RgbG(dw)); putbyte(RgbR(dw));
    }
    for (x = 0; x < cb; x++)
      putbyte(0);
  }
}


// Load a color Targa bitmap from a file, into a new color bitmap.

flag CCol::FReadColmapTarga(FILE *file)
{
  int x, y, i;
  byte ch, chR, chG, chB;

  for (i = 0; i < 12; i++)
    skipbyte();
  if (feof(file))
    return fFalse;
  fscanf(file, "%c%c", S1(&ch), S1(&chR));
  x = (int)(chR)*256 + (int)ch;
  fscanf(file, "%c%c", S1(&ch), S1(&chR));
  y = (int)(chR)*256 + (int)ch;
  if (!FBitmapSizeSet(x, y))
    return fFalse;
  skipword();
  for (y = m_y-1; y >= 0; y--) {
    for (x = 0; x < m_x; x++) {
      if (feof(file))
        return fFalse;
      fscanf(file, "%c%c%c%c", S1(&chB), S1(&chG), S1(&chR), S1(&ch));
      Set(x, y, Rgb(chR, chG, chB));
    }
  }
  return fTrue;
}


// Load a color PC Paintbrush bitmap from a file, into a new color bitmap.

flag CCol::FReadColmapPaint(FILE *file)
{
  int x1, y1, x2, y2, x, y, i;
  byte ch, chR, chG, chB;
  KV rgkv[256];

  // Read the header
  for (i = 0; i < 3; i++)
    skipbyte();
  ch = getbyte();
  if (ch != 8)
    return fFalse;
  x1 = WRead(file); y1 = WRead(file);
  x2 = WRead(file); y2 = WRead(file);
  x = x2 - x1 + 1; y = y2 - y1 + 1;
  for (i = 0; i < 116; i++)
    skipbyte();
  if (!FBitmapSizeSet(x, y))
    return fFalse;

  // Load the color indexes
  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x; x++) {
      ch = getbyte();
      if (ch < 192)
        i = 0;
      else {
        i = ch - 193;
        ch = getbyte();
      }
      Set(x, y, ch);
      while (i > 0) {
        Set(++x, y, ch);
        i--;
      }
    }

  // Load the palette
  skipbyte();
  for (i = 0; i < 256; i++) {
    fscanf(file, "%c%c%c", S1(&chR), S1(&chG), S1(&chB));
    rgkv[i] = Rgb(chR, chG, chB);
  }

  // Apply the palette
  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x; x++) {
      i = RgbR(Get(x, y));
      Set(x, y, rgkv[i]);
    }
  return fTrue;
}


// Save a color bitmap to a Targa format file.

void CCol::WriteColmapTarga(FILE *file) CONST
{
  int x, y;
  KV kv;

  fprintf(file, "%c%c%c%c%c%c%c%c",
    chNull, chNull, '\2', chNull, chNull, chNull, chNull, chNull);
  x = m_x; y = m_y;
  fprintf(file, "%c%c%c%c",
    (char)(x % 256), (char)(x / 256),
    (char)(y % 256), (char)(y / 256));
  fprintf(file, "%c%c%c%c",
    (char)(m_x % 256), (char)(m_x / 256),
    (char)(m_y % 256), (char)(m_y / 256));
  fprintf(file, " %c", chNull);
  for (y = m_y-1; y >= 0; y--) {
    for (x = 0; x < m_x; x++) {
      kv = Get(x, y);
      fprintf(file, "%c%c%c%c",
        (char)RgbB(kv), (char)RgbG(kv), (char)RgbR(kv), chNull);
    }
  }
}


// Load a Daedalus color bitmap from a file, into a new color bitmap.

flag CCol::FReadDaedalusBitmapCore(FILE *file, int x, int y)
{
  int i, cn, nCur;
  KV kv;
  char ch;

  if (!FBitmapSizeSet(x, y))
    return fFalse;
  BitmapSet(kvBlack);
  skipcrlf();
  for (y = 0; y < m_y; y++) {
    x = 0;
    ch = getbyte();
    loop {
      if (ch < ' ') {
        skiplf();
        break;
      }
      if (ch < 'a' || ch > 'z') {
        // Single pixels start with '{' or characters before 'a'.
        cn = 1;
        if (ch == '{')
          ch = getbyte();
        else if (ch == '|') {
          // Found the duplicate row marker '|'.
          Assert(x == 0 && y > 0);
          for (nCur = 0, ch = getbyte(); FDigitCh(ch); ch = getbyte())
            nCur = nCur * 10 + (ch - '0');
          BlockMove(*this, 0, y-1, m_x-1, y-1, 0, y);
          while (--nCur > 0) {
            y++;
            BlockMove(*this, 0, y-1, m_x-1, y-1, 0, y);
          }
          skipcrlf();
          break;
        }
      } else {
        // A run of 2 to 27 pixels is indicated by characters 'a' thru 'z'.
        cn = ch - 'a' + 2;
        ch = getbyte();
      }
      if (ch >= 'a') {
        // One of 27 common colors is encoded by characters 'a' thru '{'.
        i = ch - 'a';
        kv = Rgb((i / 9)*255 >> 1, (i / 3 % 3)*255 >> 1, (i % 3)*255 >> 1);
        ch = getbyte();
      } else {
        // Standard case has four characters encode a 24 bit RGB value.
        kv = 0;
        for (i = 0; i < 4; i++) {
          kv = (kv << 6) + (ch - '!');
          ch = getbyte();
        }
      }
      // Set the current run of pixels in the bitmap.
      while (cn--) {
        Set(x, y, kv);
        x++;
      }
    }
  }
  return fTrue;
}


// Save a color bitmap to a Daedalus color bitmap custom format file.

void CCol::WriteDaedalusBitmap(FILE *file, flag fClip) CONST
{
  int x, y, xmax, i, cn, nr, ng, nb, cRow = 0;
  KV kv;
  flag fBlank;

  fprintf(file, "DB\n%d %d 24\n\n", m_x, m_y);
  for (y = 0; y <= m_y; y++) {

    // Check if current row same as previous. If so write '|' dup marker.
    if (y > 0) {
      if (y < m_y) {
        for (x = 0; x < m_x; x++)
          if (Get(x, y) != Get(x, y-1))
            goto LNormal;
        cRow++;
        continue;
      }

LNormal:
      // Output number of times row was duplicated
      if (cRow > 0) {
        if (!fBlank || cRow > 1) {
          putbyte('|');
          if (cRow > 1)
            fprintf(file, "%d", cRow);
        }
        putbyte('\n');
        cRow = 0;
      }
    }
    if (y >= m_y)
      continue;

    xmax = m_x - 1;
    if (fClip)
      while (xmax >= 0 && Get(xmax, y) == kvBlack)
        xmax--;
    fBlank = (xmax < 0);
    for (x = 0; x <= xmax; x += cn) {
      kv = Get(x, y);

      // Check for runs of consecutive colors in a row.
      for (cn = 1; cn <= 26 && x + cn < m_x && Get(x + cn, y) == kv; cn++)
        ;
      // Indicate a run of 2 to 27 pixels by characters 'a' thru 'z'.
      if (cn > 1)
        putbyte('a' + cn - 2);

      nr = RgbR(kv); ng = RgbG(kv); nb = RgbB(kv);
      if ((nr == 0 || nr == 255 || nr == 127) &&
        (ng == 0 || ng == 255 || ng == 127) &&
        (nb == 0 || nb == 255 || nb == 127)) {
        // One of 27 common colors is encoded by characters 'a' thru '{'.
        if (cn <= 1)
          putbyte('{');
        putbyte('a' + ((nr+1) >> 7)*9 + ((ng+1) >> 7)*3 + ((nb+1) >> 7));
      } else {
        // Standard case requires four chars to encode a 24 bit RGB value.
        for (i = 0; i < 4; i++)
          putbyte('!' + (int)((kv >> (3-i)*6) & 0x3f));
      }
    }
    putbyte('\n');
  }
}


// Given a color, return a character approximating the color's grayscale
// value. Called from WriteTextColmap().

char ChFromKv(KV kv)
{
  int n;

  if (cs.lGrayscale < 0) {

    // Simple case: Treat RGB channels as the same brightness.
    n = RgbR(kv) + RgbG(kv) + RgbB(kv);
  } else {

    // Advanced case: Each of the RGB channels has its own brightness.
    n  = RgbR(kv) * (cs.lGrayscale / 1000000);
    n += RgbG(kv) * (cs.lGrayscale / 1000 % 1000);
    n += RgbB(kv) * (cs.lGrayscale % 1000);
    n = n * 766 / 255000;
  }
  return rgchAscii[n * cchAscii / 766];
}


// Save a color bitmap to a text file, with each color pixel written as a
// character based on the color's grayscale value.

void CCol::WriteTextColmap(FILE *file, flag fClip) CONST
{
  int x, y, x2 = m_x-1;
  char ch;

  for (y = 0; y < m_y; y++) {
    if (fClip)
      for (x2 = m_x-1; x2 >= 0 && ChFromKv(Get(x2, y)) == ' '; x2--)
        ;
    for (x = 0; x <= x2; x++) {
      ch = ChFromKv(Get(x, y));
      putbyte(ch);
    }
    putbyte('\n');
  }
}


// Write a color value to a string.

void ConvertKvToSz(KV kv, char *sz, int cch)
{
  int i;

  // If the color is one of the standard colors in the color table, use that
  // string, otherwise compose a string with RGB numbers.

  for (i = 0; i < cColor; i++)
    if (kv == rgkv[i]) {
      sprintf(SS(sz, cch), "%s", rgszColor[i]);
      return;
    }
  if (kv < 0) {
    sprintf(SS(sz, cch), "-1");  // Catch invalid colors
    return;
  }
  sprintf(SS(sz, cch), "rgb %d %d %d", RgbR(kv), RgbG(kv), RgbB(kv));
}


// Convert a color stored in a string to an actual color value, advancing the
// string pointer to after the string. This can parse all colors generated
// with ConvertKvToSz. This may call itself recursively to handle functions
// that take colors as parameters, e.g. "blend red orange".

KV ParseColorCore(char **ppch, flag fAllow)
{
  char line[cchSzDef];
  int i;

  SkipSpaces(*ppch);
  sscanf(*ppch, "%s", S(line));
  SkipToSpace(*ppch);

  // Check for standard colors in the color table.
  for (i = 0; i < cColor; i++)
    if (ChCap(line[0]) == ChCap(rgszColor[i][0]) &&
      ChCap(line[1]) == ChCap(rgszColor[i][1]) &&
      ChCap(line[2]) == ChCap(rgszColor[i][2]) && line[3] == chNull)
      return rgkv[i];

  // Check for strings that can translate to multiple colors.
  if (FEqSz(line, "???"))
    return kvRandomRainbow;
  else if (FEqSz(line, "????"))
    return kvRandom;
  else if (FEqSzI(line, "dlg"))
    return KvDialog();

  // Check for special color patterns, if they're allowed.
  if (fAllow) {
    if (FEqSz(line, "O"))
      return -crpRing;
    else if (FEqSz(line, "X"))
      return -crpSpoke;
    else if (FEqSz(line, "*"))
      return -crpFade;
    else if (FEqSz(line, "|"))
      return -crpY;
    else if (FEqSz(line, "-"))
      return -crpX;
    else if (FEqSz(line, "\\"))
      return -crpDiagUL;
    else if (FEqSz(line, "/"))
      return -crpDiagUR;
    else if (FEqSz(line, "#"))
      return -crpSquare;
    else if (FEqSz(line, "+"))
      return -crpDiamond;
    else if (FEqSz(line, "@"))
      return -crpSpiral;
    else if (FEqSz(line, "o"))
      return -crpRing2;
    else if (FEqSz(line, "8"))
      return -crpRings;
    else if (FEqSz(line, "x"))
      return -crpSpoke2;
    else if (FEqSz(line, "?"))
      return -crpRainbow;
    else if (FEqSz(line, "??"))
      return -crpRandom;
    else if (FEqSz(line, "?????"))
      return -crpRandom2;
  }

  // Numbers are treated as that index in the color table.
  if (FDigitCh(line[0])) {
    i = atoi(line);
    if (FBetween(i, 0, cColor-1))
      return rgkv[i];
  }

  // Unknown strings will be parsed via the scripting language.
  return -crpUnknown;
}


// Convert a color stored in a string to an actual color value.

KV ParseColor(char *sz, flag fAllow)
{
  KV kv;
  char *szT = sz;

  kv = ParseColorCore(&szT, fAllow);
  if (kv == -crpUnknown)
    return ParseExpression(sz);
  if (kv < 0 && kv > -crpUnknown)
    cs.nReplacePattern = -kv;
  return kv;
}


// Read a string representing a color from a file, and parse it into a value.

KV ReadColor(FILE *file)
{
  char sz[cchSzDef];
  int i;

  for (i = 0; i < cchSzDef && (sz[i] = getbyte()) >= ' '; i++)
    ;
  sz[i] = chNull;
  return ParseColor(sz, fFalse);
}

/* color.cpp */
