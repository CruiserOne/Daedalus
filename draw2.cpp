/*
** Daedalus (Version 3.5) File: draw2.cpp
** By Walter D. Pullen, Astara@msn.com, http://www.astrolog.org/labyrnth.htm
**
** IMPORTANT NOTICE: Daedalus and all Maze generation and general
** graphics routines used in this program are Copyright (C) 1998-2024 by
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
** This file contains advanced perspective graphics routines, unrelated to
** Mazes.
**
** Created: 7/17/1990.
** Last code change: 10/30/2024.
*/

#include <stdio.h>
#include <math.h>
#include "util.h"
#include "graphics.h"
#include "color.h"
#include "threed.h"
#include "draw.h"


/*
******************************************************************************
** Perspective Routines
******************************************************************************
*/

// Draw the background for the FRenderPerspective functions. This handles both
// monochrome and color bitmaps.

flag FRenderInitialize(CMap &b, real theta, real phi)
{
  int h, i, d, x, y, y1, y2, y3, y4;
  real rx, ry, rz, rST, rCT, rSP, rCP;
  flag fColor = b.FColor();
  KV kv;

  // Figure out the height of the horizon line, and the visible sections of
  // ground and sky.
  h = (int)(ds.rHoriz - ds.rScale*RTanD(phi) + rRound) - ds.nHoriz + 1;
  if (h < 0)
    h = 0;
  else if (h >= b.m_y)
    h = b.m_y;
  y1 = 0; y2 = h-1; y3 = h; y4 = b.m_y-1;
  if (phi > 90.0 || phi <= -90.0) {
    SwapN(y1, y3);
    SwapN(y2, y4);
  }

  // Draw the ground and sky sections in the appropriate colors.
  if (!fColor) {
    b.BitmapOn();
    if (ds.fSkyShade && y2 >= 0)
      b.Block(0, y1, b.m_x-1, y2, fOff);
    if (ds.fGroundShade)
      for (y = y3; y <= y4; y++)
        for (x = 0; x < b.m_x; x++)
          if (!FOdd(x ^ y))
            b.Set0(x, y);
  } else {
    for (y = y1; y <= y2; y++)
      b.LineX(0, b.m_x-1, y, KvBlendN(ds.kvSkyHi, ds.kvSkyLo,
        y - y1, y2 - y1 + 1));
    for (y = y3; y <= y4; y++)
      b.LineX(0, b.m_x-1, y, KvBlendN(ds.kvGroundHi, ds.kvGroundLo,
        y - y3, y4 - y3 + 1));
  }

  // Draw stars in the sky.
  if (ds.fStar) {
    // If the list of stars hasn't been generated yet, do so first.
    if (ds.rgstar == NULL) {
      ds.rgstar = RgAllocate(istarMax, STAR);
      if (ds.rgstar == NULL)
        return fFalse;
      for (i = 0; i < istarMax; i++) {
        ds.rgstar[i].x = Rnd(0, 32767);
        do {
          ds.rgstar[i].y = Rnd(0, 32767);
        } while (Rnd(0, 9999) > (int)(RCosD((real)((int)ds.rgstar[i].y -
          16384) / 16384.0 * rDegQuad) * 10000.0));
        ds.rgstar[i].kv = KvStarRandom();
      }
    }
    // Rotate star coordinates projecting them upon the polar sky.
    RotateR2Init(rST, rCT, theta);
    RotateR2Init(rSP, rCP, phi);
    for (i = 0; i < ds.cStar; i++) {
      x = (int)ds.rgstar[i].x;
      y = (int)ds.rgstar[i].y;
      rx = RSinD((real)x * rDegMax / 32768.0) * 1000.0;
      ry = RCosD((real)x * rDegMax / 32768.0) * 1000.0;
      rz = RTanD((real)y * rDegHalf / 32768.0 - rDegQuad) * 1000.0;
      if (!ds.fSkyAll && rz <= 0.0)
        continue;
      RotateR2(&rx, &ry, rST, rCT);
      if (phi != 0.0)
        RotateR2(&ry, &rz, rSP, rCP);
      if (ry <= 0.0)
        continue;
      CalculateCoordinate(&x, &y, rx, ry, rz);
      kv = fColor ? ds.rgstar[i].kv : fOff;
      b.Set(x, y, kv);
      if (ds.nStarSize < 0 ? FOdd(i) :
        RgbR(kv) + RgbG(kv) + RgbB(kv) >= ds.nStarSize)
        for (d = 0; d < DIRS; d++)
          b.Set(x+xoff[d], y+yoff[d], kv);
    }
  }

  // Draw the horizon line itself.
  if (ds.nHoriz > 0)
    b.Block(0, h, b.m_x-1, h + ds.nHoriz - 1, !fColor ? fOff : ds.kvTrim);
  return fTrue;
}


// Draw a border around the image for the FRenderPerspective functions. This
// handles both monochrome and color bitmaps.

void RenderFinalize(CMap &b)
{
  flag fColor = b.FColor();

  if (ds.nBorder > 0)
    b.BoxAll(ds.nBorder, ds.nBorder, !fColor ? fOff : ds.kvTrim);
}


// Draw a stereoscopic 3D version of a wireframe or patch list to a color or
// monochrome bitmap. This involves drawing "left eye" and "right eye"
// versions of the scene from slightly different points of view.

flag FRenderPerspectiveStereo(CMap &b, COOR *coor, long ccoor,
  PATCH *patch, long cpatch)
{
  int mSav = ds.hormin, nSav = ds.vermin, dSav = ds.theta, x, f, m, n;
  flag fWire = coor != NULL, fRet = fFalse;
  CMap *bs = NULL;
  CCol *c, *cs;

  x = !ds.fStereo3D ? b.m_x >> 1 : b.m_x;
  bs = b.Create();
  if (bs == NULL || !bs->FBitmapSizeSet(x, b.m_y))
    goto LExit;
  f = FOdd(b.m_x) && !ds.fStereo3D;
  m = NCosRD(ds.nStereo, ds.theta); n = NSinRD(ds.nStereo, ds.theta);

  // Draw left eye view of scene.
  ds.hormin += m; ds.vermin += n; //ds.theta += 1;
  if (!(fWire ? FRenderPerspectiveWireCore(*bs, coor, ccoor) :
    FRenderPerspectivePatchCore(*bs, patch, cpatch)))
    goto LExit;
  if (ds.fStereo3D && b.FColor()) {
    c = dynamic_cast<CCol *>(&b);
    cs = dynamic_cast<CCol *>(bs);
    cs->ColmapGrayscale();
    cs->ColmapOrAndKv(kvCyan, 0);
  }
  b.BlockMove(*bs, 0, 0, x-1, b.m_y-1, 0, 0);

  // Draw right eye view of scene.
  ds.hormin -= m*2; ds.vermin -= n*2; //ds.theta -= 1*2;
  if (!(fWire ? FRenderPerspectiveWireCore(*bs, coor, ccoor) :
    FRenderPerspectivePatchCore(*bs, patch, cpatch)))
    goto LExit;
  if (!ds.fStereo3D)
    b.BlockMove(*bs, 0, 0, x-1, b.m_y-1, x+f, 0);
  else if (b.FColor()) {
    cs->ColmapGrayscale();
    cs->ColmapOrAndKv(kvRed, 0);
    c->ColmapOrAnd(*cs, 1);
  }

  // Draw line down middle if outer bitmap is odd sized.
  if (f)
    b.LineY(x, 0, b.m_y-1, ds.kvTrim);
  ds.hormin = mSav; ds.vermin = nSav; ds.theta = dSav;

  fRet = fTrue;
LExit:
  if (bs != NULL)
    bs->Destroy();
  return fRet;
}


// Return a randomly chosen color for a star or meteor.

KV KvStarRandom()
{
  int nR, nG, nB, nR0, nR1, nG0, nG1, nB0, nB1;

  // Start with a monochrome brightness
  nR = nG = nB = Rnd(0, 255);

  // Then add in a little coloring if appropriate
  if (ds.lStarColor != 0 && Rnd(0, 98) < (int)(ds.lStarColor / 1000000L)) {
    switch (Rnd(0, 2)) {
    case 0:
      nR0 = (int)(ds.lStarColor / 100000L % 10);
      nR1 = (int)(ds.lStarColor / 10000L  % 10);
      nR += Rnd(0, (nR0 + nR1) * 511 / 18) - (nR0 * 511 / 18);
      EnsureBetween(nR, 0, 255);
      break;
    case 1:
      nG0 = (int)(ds.lStarColor / 1000L % 10);
      nG1 = (int)(ds.lStarColor / 100L  % 10);
      nG += Rnd(0, (nG0 + nG1) * 511 / 18) - (nG0 * 511 / 18);
      EnsureBetween(nG, 0, 255);
      break;
    case 2:
      nB0 = (int)(ds.lStarColor / 10L % 10);
      nB1 = (int)(ds.lStarColor       % 10);
      nB += Rnd(0, (nB0 + nB1) * 511 / 18) - (nB0 * 511 / 18);
      EnsureBetween(nB, 0, 255);
      break;
    }
  }
  return Rgb(nR, nG, nB);
}


// Compute the viewing angle to use for the perspective renderings. Usually
// this is a user setting, but may also be set to point at the origin.

void RenderGetAngle(real *ptheta, real *pphi)
{
  real theta, phi;

  if (ds.nFaceOrigin <= 0) {
    theta = (real)ds.theta;
    phi   = (real)ds.phi;
  } else {
    theta = rDeg34 - GetAngle(0, 0, ds.hormin, ds.vermin);
    if (theta < 0.0)
      theta += rDegMax;
    phi = GetAngle(0, 0, NSqr(Sq(ds.hormin)+Sq(ds.vermin)), ds.depmin);
    if (phi >= rDegHalf)
      phi -= rDegMax;
    if (ds.nFaceOrigin > 1) {
      ds.theta = (int)theta;
      ds.phi = (int)phi;
    }
  }
  *ptheta = theta; *pphi = phi;
}


// Move the point (x1, y1, z1) along the given line so that y1 = 0. Called
// from RenderPatchPerspective().

void IgnoreNegativeR(real *x1, real *y1, real *z1, real x2, real y2, real z2)
{
  if (y2 != *y1) {
    *x1 -= *y1 * (x2 - *x1) / (y2 - *y1);
    *z1 -= *y1 * (z2 - *z1) / (y2 - *y1);
  }
  *y1 = 0.0;
}


// Do perspective projection of a 3D point onto the 2D screen. Called from
// FRenderPerspectivePatch().

void CalculateCoordinate(int *h, int *v, real x, real y, real z)
{
  real rLimit = 32000.0, r;

  if (y < 1.0)
    y = 1.0;

  // Calculate horizontal pixel. The farther away y is, the less effect x has
  // on the horizontal distance left or right from the middle of the screen.
  r = ds.rScale*x / y;
  EnsureBetween(r, -rLimit, rLimit);
  *h = (ds.xmax >> 1) + (int)r;

  // Calculate vertical pixel. The farther away y is, the less effect z has on
  // the vertical distance above or below the horizon line.
  r = ds.rScale*z / y;
  EnsureBetween(r, -rLimit, rLimit);
  *v = (int)(ds.rHoriz - r);
}


// Move the point (x1, y1) along the given line so that y1 = 0. Called from
// ClipOutside().

void IgnoreNegative(real *x1, real *y1, real x2, real y2)
{
  if (y2 != *y1)
    *x1 -= *y1 * (x2 - *x1) / (y2 - *y1);
  *y1 = 0.0;
}


// Move the point (x1, y1) along the given line so that y1 = n. Called from
// ClipOutside().

void IgnoreGreaterThan(real *x1, real *y1, real x2, real y2, real n)
{
  if (y2 != *y1)
    *x1 += (n - *y1) * (x2 - *x1) / (y2 - *y1);
  *y1 = n;
}


// Clip a line segment so that it lies inside a rectangle, specifically from
// (0, 0) to (xmax, ymax). Called from FRenderPerspectiveWire().

void ClipOutside(real xmax, real ymax, real *x1, real *y1, real *x2, real *y2)
{
  if (*x1 < 0)
    IgnoreNegative(y1, x1, *y2, *x2);
  if (*x2 < 0)
    IgnoreNegative(y2, x2, *y1, *x1);
  if (*y1 < 0)
    IgnoreNegative(x1, y1, *x2, *y2);
  if (*y2 < 0)
    IgnoreNegative(x2, y2, *x1, *y1);
  if (*x1 > xmax)
    IgnoreGreaterThan(y1, x1, *y2, *x2, xmax);
  if (*x2 > xmax)
    IgnoreGreaterThan(y2, x2, *y1, *x1, xmax);
  if (*y1 > ymax)
    IgnoreGreaterThan(x1, y1, *x2, *y2, ymax);
  if (*y2 > ymax)
    IgnoreGreaterThan(x2, y2, *x1, *y1, ymax);
}


// Do perspective projection of a 3D point onto the 2D screen. Called from
// FRenderPerspectiveWire().

void CalculateCoordinateR(real *h, real *v, real x, real y, real z)
{
  real r;

  if (y < 1.0)
    y = 1.0;

  // Calculate horizontal pixel. The farther away y is, the less effect x has
  // on the horizontal distance left or right from the middle of the screen.
  r = ds.rScale*x / y;
  *h = (real)(ds.xmax >> 1) + r;

  // Calculate vertical pixel. The farther away y is, the less effect z has on
  // the vertical distance above or below the horizon line.
  r = ds.rScale*z / y;
  *v = (real)ds.rHoriz - r;
}


/*
******************************************************************************
** Render Perspective Wireframe
******************************************************************************
*/

#define FCompareWire(coor1, coor2) \
  ((coor1).y1 + (coor1).y2 > (coor2).y1 + (coor2).y2)

// Given a list of lines forming a heap style of binary tree, where a parent
// is greater than its two children, and the index of a random value line in
// it, push the line down through the tree until the heap condition is met
// again. Called from FRenderPerspectiveWire() to implement heap sort.

void PushdownWire(COOR *coor, long p, long size)
{
  COOR coorSwap;

  while (p < size && (FCompareWire(coor[p >> 1], coor[p]) ||
    (p < size-1 && FCompareWire(coor[p >> 1], coor[p + 1])))) {
    p += (p < size-1 && FCompareWire(coor[p], coor[p + 1]));
    coorSwap = coor[p]; coor[p] = coor[p >> 1]; coor[p >> 1] = coorSwap;
    p <<= 1;
  }
}


// Draw a perspective scene composed of a set of line segments in 3D space.
// Implements the Render Wireframe Perspective command for both monochrome and
// color bitmaps.

flag FRenderPerspectiveWireCore(CMap &b, COOR *coor, long ccoor)
{
  COOR *coorT, coorSwap;
  long ccoor2 = 0, ccoor3 = 0, i;
  int xmax, ymax, x1, y1, x2, y2;
  real xpos, ypos, zpos, yminCoor, ymaxCoor, theta, phi, rS, rC,
    rx1, ry1, rx2, ry2, rxmax, rymax, rDist, rUnit;
  flag fColor = b.FColor(), fSort = ds.fWireSort;
  KV kv;

  xmax = b.m_x; ymax = b.m_y; ds.xmax = xmax;
  rxmax = (real)xmax; rymax = (real)ymax;
  xpos  = (real)ds.hormin;
  ypos  = (real)ds.vermin;
  zpos  = (real)ds.depmin;
  RenderGetAngle(&theta, &phi);
  ds.rHoriz = (real)((ymax >> 1) + ds.verv);
  rUnit = (real)ds.nWireDistance * ds.ryScale;

  // Get starting set of coordinates.

  coorT = RgAllocate(Max(ccoor, 1), COOR);
  if (coorT == NULL)
    return fFalse;
  for (i = 0; i < ccoor; i++)
    coorT[i] = coor[i];
  if (ds.fReflect && ccoor > 0) {
    yminCoor = ymaxCoor = coorT[0].y1;
    for (i = 0; i < ccoor; i++) {
      yminCoor = Min(yminCoor, coorT[i].y1);
      yminCoor = Min(yminCoor, coorT[i].y2);
      ymaxCoor = Max(ymaxCoor, coorT[i].y1);
      ymaxCoor = Max(ymaxCoor, coorT[i].y2);
    }
    for (i = 0; i < ccoor; i++) {
      neg(coorT[i].y1);
      neg(coorT[i].y2);
    }
    if (yminCoor >= 0)
      for (i = 0; i < ccoor; i++) {
        coorT[i].y1 += ymaxCoor + yminCoor;
        coorT[i].y2 += ymaxCoor + yminCoor;
      }
  }

  // Rotate and adjust coordinates so viewing location is central.

  RotateR2Init(rS, rC, theta);
  for (i = 0; i < ccoor; i++) {
    coorT[i].x1 = (coorT[i].x1 - xpos)*ds.rxScale;
    coorT[i].y1 = (coorT[i].y1 - ypos)*ds.ryScale;
    coorT[i].z1 = (coorT[i].z1 - zpos)*ds.rzScale;
    RotateR2(&coorT[i].x1, &coorT[i].y1, rS, rC);
    coorT[i].x2 = (coorT[i].x2 - xpos)*ds.rxScale;
    coorT[i].y2 = (coorT[i].y2 - ypos)*ds.ryScale;
    coorT[i].z2 = (coorT[i].z2 - zpos)*ds.rzScale;
    RotateR2(&coorT[i].x2, &coorT[i].y2, rS, rC);
  }
  if (phi != 0.0) {
    RotateR2Init(rS, rC, phi);
    for (i = 0; i < ccoor; i++) {
      RotateR2(&coorT[i].y1, &coorT[i].z1, rS, rC);
      RotateR2(&coorT[i].y2, &coorT[i].z2, rS, rC);
    }
  }

  // Drop or adjust all coordinate pairs that are behind the viewer.

  for (i = 0; i < ccoor; i++) {
    if (coorT[i].y1 >= 0.0 || coorT[i].y2 >= 0.0) {
      if (coorT[i].y1 < 0.0)
        IgnoreNegativeR(&coorT[i].x1, &coorT[i].y1, &coorT[i].z1,
          coorT[i].x2, coorT[i].y2, coorT[i].z2);
      if (coorT[i].y2 < 0.0)
        IgnoreNegativeR(&coorT[i].x2, &coorT[i].y2, &coorT[i].z2,
          coorT[i].x1, coorT[i].y1, coorT[i].z1);
      coorT[ccoor2] = coorT[i];
      coorT[ccoor2].z1 = coorT[i].z1 - (real)ymax + ds.rHoriz;
      coorT[ccoor2].z2 = coorT[i].z2 - (real)ymax + ds.rHoriz;
      ccoor2++;
    }
  }
  PrintSzL("Visible number of coordinate pairs: %ld\n", ccoor2);

  // Heap sort the list of lines.

  if (fSort) {
    // No need to sort if all lines are the same color.
    for (i = 1; i < ccoor2; i++)
      if (coorT[i].kv != coorT[i-1].kv)
        break;
    if (i >= ccoor2)
      fSort = fFalse;
  }
  if (fSort) {
    for (i = (ccoor2 - 1) >> 1; i >= 0; i--)
      PushdownWire(coorT, i << 1, ccoor2);
    for (i = ccoor2 - 1; i > 0; i--) {
      coorSwap = coorT[i]; coorT[i] = coorT[0]; coorT[0] = coorSwap;
      PushdownWire(coorT, 0, i);
    }
#ifdef DEBUG
    for (i = 1; i < ccoor2; i++)
      if (FCompareWire(coorT[i], coorT[i-1])) {
        PrintSzN_E("Line %d was sorted incorrectly.\n", i);
        break;
      }
#endif
  }

  // Draw the lines on the screen.

  FRenderInitialize(b, theta, phi);
  for (i = 0; i < ccoor2; i++) {
    CalculateCoordinateR(&rx1, &ry1, coorT[i].x1, coorT[i].y1, coorT[i].z1);
    CalculateCoordinateR(&rx2, &ry2, coorT[i].x2, coorT[i].y2, coorT[i].z2);
    if ((rx1 >= 0.0 || rx2 >= 0.0) && (ry1 >= 0.0 || ry2 >= 0.0) &&
      (rx1 < rxmax || rx2 < rxmax) && (ry1 < rymax || ry2 < rymax)) {
      ClipOutside(rxmax-1.0, rymax-1.0, &rx1, &ry1, &rx2, &ry2);
      x1 = (int)rx1; y1 = (int)ry1; x2 = (int)rx2; y2 = (int)ry2;
      if (b.FLegal(x1, y1) && b.FLegal(x2, y2)) {

        // Draw one or more lines to indicate near vs. far coordinates.
        kv = !fColor ? fOff : coorT[i].kv;
        b.Line(x1, y1, x2, y2, kv);
        rDist = (coorT[i].y1 + coorT[i].y2) / 2.0;
        if (ds.nWireWidth >= 1 && rDist < rUnit*3.0) {
          b.Line(x1+1, y1,   x2+1, y2,   kv);
          if (ds.nWireWidth >= 2 && rDist < rUnit*2.0) {
            b.Line(x1,   y1+1, x2,   y2+1, kv);
            if (ds.nWireWidth >= 3 && rDist < rUnit) {
              b.Line(x1-1, y1,   x2-1, y2,   kv);
              b.Line(x1,   y1-1, x2,   y2-1, kv);
            } else
              b.Line(x1+1, y1+1, x2+1, y2+1, kv);
          }
        }
        ccoor3++;
      }
    }
  }
  PrintSzL("Drawn number of coordinate pairs: %ld\n", ccoor3);

  RenderFinalize(b);
  DeallocateP(coorT);
  return fTrue;
}


// Like FRenderPerspectiveWireCore but handles stereoscopic rendering of two
// images side by side if that setting is in effect.

flag FRenderPerspectiveWire(CMap &b, COOR *coor, long ccoor)
{
  if (ds.nStereo != 0)
    return FRenderPerspectiveStereo(b, coor, ccoor, NULL, 0);
  return FRenderPerspectiveWireCore(b, coor, ccoor);
}


/*
******************************************************************************
** Render Perspective Patches
******************************************************************************
*/

#define FComparePatch(pat1, pat2) ((pat1).rDistance > (pat2).rDistance)

// Given a list of patches forming a heap style of binary tree, where a parent
// is greater than its two children, and the index of a random value patch in
// it, push the patch down through the tree until the heap condition is met
// again. Called from FRenderPerspectivePatch() to implement heap sort.

void PushdownPatch(PATCH *patch, long p, long size)
{
  PATCH patT;

  while (p < size && (FComparePatch(patch[p >> 1], patch[p]) ||
    (p < size-1 && FComparePatch(patch[p >> 1], patch[p + 1])))) {
    p += (p < size-1 && FComparePatch(patch[p], patch[p + 1]));
    patT = patch[p]; patch[p] = patch[p >> 1]; patch[p >> 1] = patT;
    p <<= 1;
  }
}


// Draw a perspective scene composed of a set of triangular and quadrilateral
// patches in 3D space. Implements the Render Wireframe Perspective command
// for both monochrome and color bitmaps.

flag FRenderPerspectivePatchCore(CMap &b, PATCH *patch, long cpatch)
{
  PATCH *patchT, patT;
  PATR patrT;
  long cpatch2 = 0, cpatch3 = 0, count, i, j;
  int x1, y1, x2, y2, x3, y3, x4, y4, cpt, xmax, ymax, k;
  flag fSquare, fTouch, fTest;
  KV kv;
  CVector vLight, vEye, v;
  real xpos, ypos, zpos, yminCoor, ymaxCoor, theta, phi, rS, rC, rT;
  char sz[cchSzDef];
  flag fColor = b.FColor();

  xmax = b.m_x; ymax = b.m_y;
  ds.xmax = xmax;
  xpos  = (real)ds.hormin;
  ypos  = (real)ds.vermin;
  zpos  = (real)ds.depmin;
  RenderGetAngle(&theta, &phi);
  ds.rHoriz = (real)((ymax >> 1) + ds.verv);
  vLight = ds.vLight;

  // Get starting set of patches.

  patchT = RgAllocate(Max(cpatch, 1), PATCH);
  if (patchT == NULL)
    return fFalse;
  for (i = 0; i < cpatch; i++)
    patchT[i] = patch[i];

  if (ds.fReflect) {
    yminCoor = ymaxCoor = patchT[0].p[0].y;
    for (i = 0; i < cpatch; i++)
      for (k = patchT[i].cpt-1; k >= 0; k--) {
        yminCoor = Min(yminCoor, patchT[i].p[k].y);
        ymaxCoor = Max(ymaxCoor, patchT[i].p[k].y);
      }
    for (i = 0; i < cpatch; i++) {
      for (k = patchT[i].cpt-1; k >= 0; k--)
        patchT[i].p[k].y = ymaxCoor + yminCoor - patchT[i].p[k].y;
    }
  } else {
    for (i = 0; i < cpatch; i++) {
      cpt = patchT[i].cpt-1;
      patrT = patchT[i].p[1];
      patchT[i].p[1] = patchT[i].p[cpt];
      patchT[i].p[cpt] = patrT;
      fTest = patchT[i].p[0].fLine;
      for (k = 0; k < cpt; k++)
        patchT[i].p[k].fLine = patchT[i].p[k + 1].fLine;
      patchT[i].p[cpt].fLine = fTest;
    }
  }
  fTouch = ds.fEdges && ds.fTouch;

  // Rotate and adjust patches so viewing location is central.

  RotateR2Init(rS, rC, theta);
  for (i = 0; i < cpatch; i++)
    for (k = patchT[i].cpt-1; k >= 0; k--) {
      patchT[i].p[k].x = (patchT[i].p[k].x - xpos)*ds.rxScale;
      patchT[i].p[k].y = (patchT[i].p[k].y - ypos)*ds.ryScale;
      patchT[i].p[k].z = (patchT[i].p[k].z - zpos)*ds.rzScale;
      RotateR2(&patchT[i].p[k].x, &patchT[i].p[k].y, rS, rC);
    }
  RotateR2(&vLight.m_x, &vLight.m_y, rS, rC);
  if (phi != 0.0) {
    RotateR2Init(rS, rC, phi);
    for (i = 0; i < cpatch; i++)
      for (k = patchT[i].cpt-1; k >= 0; k--)
        RotateR2(&patchT[i].p[k].y, &patchT[i].p[k].z, rS, rC);
    RotateR2(&vLight.m_y, &vLight.m_z, rS, rC);
  }

  // Drop all patches that are behind the viewer.

  for (i = 0; i < cpatch; i++) {
    cpt = patchT[i].cpt;
    if (patchT[i].p[0].y < 0.0 || patchT[i].p[1].y < 0.0 ||
      patchT[i].p[2].y < 0.0 || (cpt >= 3 && patchT[i].p[3].y < 0.0))
      continue;

    // Drop patches facing away from the viewer.
    if (!ds.fRight) {
      v.Normal(patchT[i].p[0].x, patchT[i].p[0].y, patchT[i].p[0].z,
        patchT[i].p[1].x, patchT[i].p[1].y, patchT[i].p[1].z,
        patchT[i].p[2].x, patchT[i].p[2].y, patchT[i].p[2].z);
      vEye.Set(patchT[i].p[0].x, patchT[i].p[0].y, patchT[i].p[0].z);
      if (vEye.Dot(v) > 0.0)
        continue;
    }
    patchT[cpatch2] = patchT[i];
    for (k = 0; k < cpt; k++)
      patchT[cpatch2].p[k].z =
        patchT[i].p[k].z - (real)ymax + ds.rHoriz;
    cpatch2++;
    if (cpt <= 3)
      cpatch3++;
  }
  sprintf(S(sz),
    "Visible number of patches: %ld (%ld triangle, %ld square)\n",
    cpatch2, cpatch3, cpatch2 - cpatch3);
  PrintSz(sz);

  // Sort patches in order from farthest away to closest.

  for (i = 0; i < cpatch2; i++) {
    // Calculate the distance of the center of the patch from viewing plane.
    rT = 0.0;
    for (j = patchT[i].cpt-1; j >= 0; j--)
      rT += Sq(patchT[i].p[j].y) + Sq(patchT[i].p[j].z) +
        Sq(patchT[i].p[j].x);
    patchT[i].rDistance = rT / patchT[i].cpt;
  }
  // Heap sort the list of patches.
  for (i = (cpatch2 - 1) >> 1; i >= 0; i--)
    PushdownPatch(patchT, i << 1, cpatch2);
  for (i = cpatch2 - 1; i > 0; i--) {
    patT = patchT[i]; patchT[i] = patchT[0]; patchT[0] = patT;
    PushdownPatch(patchT, 0, i);
  }
#ifdef DEBUG
  for (i = 1; i < cpatch2; i++)
    if (FComparePatch(patchT[i], patchT[i - 1])) {
      PrintSzN_E("Patch %d was sorted incorrectly.\n", i);
      break;
    }
#endif

  // Draw the patches on the screen.

  FRenderInitialize(b, theta, phi);
  count = cpatch3 = 0;
  for (i = 0; i < cpatch2; i++) {
    fSquare = patchT[i].cpt >= 4;
    CalculateCoordinate(&x1, &y1,
      patchT[i].p[0].x, patchT[i].p[0].y, patchT[i].p[0].z);
    CalculateCoordinate(&x2, &y2,
      patchT[i].p[1].x, patchT[i].p[1].y, patchT[i].p[1].z);
    CalculateCoordinate(&x3, &y3,
      patchT[i].p[2].x, patchT[i].p[2].y, patchT[i].p[2].z);
    if (fSquare) {
      CalculateCoordinate(&x4, &y4,
        patchT[i].p[3].x, patchT[i].p[3].y, patchT[i].p[3].z);
    } else {
      x4 = x1; y4 = y1;
    }
    if (((x1 >= 0)  || (x2 >= 0)    || (x3 >= 0)    || (x4 >= 0))    &&
      ((x1 <= xmax) || (x2 <= xmax) || (x3 <= xmax) || (x4 <= xmax)) &&
      ((y1 >= 0)    || (y2 >= 0)    || (y3 >= 0)    || (y4 >= 0))    &&
      ((y1 <= ymax) || (y2 <= ymax) || (y3 <= ymax) || (y4 <= ymax))) {

      // Figure out the color of this patch based on the light vector.
      if (ds.fShading) {
        v.Normal(patchT[i].p[0].x, patchT[i].p[0].y, patchT[i].p[0].z,
          patchT[i].p[1].x, patchT[i].p[1].y, patchT[i].p[1].z,
          patchT[i].p[2].x, patchT[i].p[2].y, patchT[i].p[2].z);
        if (!fColor)
          k = -(int)(vLight.Angle(v) / rPi * 16.99);
        else
          kv = KvShade(patchT[i].kv,
            (vLight.Angle(v) - rPiHalf) / rPiHalf);
      } else {
        if (!fColor)
          k = 1;
        else
          kv = patchT[i].kv;
      }
      if (!fColor) {

        // Draw a monochrome patch.
        if (fSquare)
          b.FQuadrilateral(x1, y1, x2, y2, x3, y3, x4, y4, k, 0);
        else
          b.FTriangle(x1, y1, x2, y2, x3, y3, k, 0);
        if (ds.fEdges) {
          if (patchT[i].p[0].fLine)
            b.Line(x1, y1, x2, y2, fOff);
          if (patchT[i].p[1].fLine)
            b.Line(x2, y2, x3, y3, fOff);
          if (fSquare) {
            if (patchT[i].p[2].fLine)
              b.Line(x3, y3, x4, y4, fOff);
            if (patchT[i].p[3].fLine)
              b.Line(x4, y4, x1, y1, fOff);
          } else
            if (patchT[i].p[2].fLine)
              b.Line(x3, y3, x1, y1, fOff);
        }
      } else {

        // Draw a color patch.
        if (ds.nFog > 0) {
          rT = 0.0;
          for (k = patchT[i].cpt-1; k >= 0; k--)
            rT += patchT[i].p[k].y;
          rT = rT / patchT[i].cpt / 10.0 / (real)ds.nFog;
          kv = KvBlendR(kv, ds.kvTrim, Min(rT, 1.0));
        }
        if (fSquare)
          b.FQuadrilateral(x1, y1, x2, y2, x3, y3, x4, y4,
            kv, patchT[i].nTrans);
        else
          b.FTriangle(x1, y1, x2, y2, x3, y3, kv, patchT[i].nTrans);
        if (ds.fEdges) {
          if (patchT[i].p[0].fLine)
            b.Line(x1, y1, x2, y2, ds.kvTrim);
          if (patchT[i].p[1].fLine)
            b.Line(x2, y2, x3, y3, ds.kvTrim);
          if (fSquare) {
            if (patchT[i].p[2].fLine)
              b.Line(x3, y3, x4, y4, ds.kvTrim);
            if (patchT[i].p[3].fLine)
              b.Line(x4, y4, x1, y1, ds.kvTrim);
          } else
            if (patchT[i].p[2].fLine)
              b.Line(x3, y3, x1, y1, ds.kvTrim);
        }
      }
      if (fTouch) {
        patchT[i].p[0].x = (real)x1; patchT[i].p[0].y = (real)y1;
        patchT[i].p[1].x = (real)x2; patchT[i].p[1].y = (real)y2;
        patchT[i].p[2].x = (real)x3; patchT[i].p[2].y = (real)y3;
        patchT[i].p[3].x = (real)x4; patchT[i].p[3].y = (real)y4;
      }
      count++;
      cpatch3 += !fSquare;
    } else if (fTouch)
      for (k = 0; k < 4; k++)
        patchT[i].p[k].fLine = fFalse;
  }
  sprintf(S(sz), "Drawn number of patches: %ld (%ld triangle, %ld square)\n",
    count, cpatch3, count - cpatch3); PrintSz(sz);

  // Touch up edges on the screen. Very slow, but helps avoid blemishes formed
  // by later patches that slightly overlap edges of earlier patches.

  if (fTouch) {
    for (j = 0; j < cpatch2 - 1; j++) {
      cpt = patchT[j].cpt;
      for (k = 0; k < cpt; k++)
        for (i = j + 1; i < cpatch2; i++)
          if (patchT[j].p[k].fLine) {
            fTouch = fTrue;
            for (count = 0; count < 4; count++)
              fTouch = fTouch &&
                patchT[j].p[k].x <= patchT[i].p[count].x &&
                patchT[j].p[(k + 1) % cpt].x <= patchT[i].p[count].x;
            fTest = fTouch;
            fTouch = fTrue;
            for (count = 0; count < 4; count++)
              fTouch = fTouch &&
                patchT[j].p[k].y <= patchT[i].p[count].y &&
                patchT[j].p[(k + 1) % cpt].y <= patchT[i].p[count].y;
            fTest = fTest || fTouch;
            fTouch = fTrue;
            for (count = 0; count < 4; count++)
              fTouch = fTouch &&
                patchT[j].p[k].x >= patchT[i].p[count].x &&
                patchT[j].p[(k + 1) % cpt].x >= patchT[i].p[count].x;
            fTest = fTest || fTouch;
            fTouch = fTrue;
            for (count = 0; count < 4; count++)
              fTouch = fTouch &&
                patchT[j].p[k].y >= patchT[i].p[count].y &&
                patchT[j].p[(k + 1) % cpt].y >= patchT[i].p[count].y;
            patchT[j].p[k].fLine = fTest || fTouch;
          }
    }
    for (i = 0; i < cpatch2; i++) {
      cpt = patchT[i].cpt;
      for (k = 0; k < cpt; k++)
        if (patchT[i].p[k].fLine) {
          x1 = (int)patchT[i].p[k].x;
          y1 = (int)patchT[i].p[k].y;
          x2 = (int)patchT[i].p[(k + 1) % cpt].x;
          y2 = (int)patchT[i].p[(k + 1) % cpt].y;
          b.Line(x1, y1, x2, y2, !fColor ? fOff : ds.kvTrim);
        }
    }
  }

  RenderFinalize(b);
  DeallocateP(patchT);
  return fTrue;
}


// Like FRenderPerspectivePatchCore but handles stereoscopic rendering of two
// images side by side if that setting is in effect.

flag FRenderPerspectivePatch(CMap &b, PATCH *patch, long cpatch)
{
  if (ds.nStereo != 0)
    return FRenderPerspectiveStereo(b, NULL, 0, patch, cpatch);
  return FRenderPerspectivePatchCore(b, patch, cpatch);
}


/*
******************************************************************************
** Other Render Routines
******************************************************************************
*/

// Draw a scene of the same line segment list used by FRenderPerspectiveWire,
// in a simple non-perspective view looking straight down from the z-axis.
// Implements the Render Wireframe Aerial command for both color and
// monochrome bitmaps.

flag FRenderAerial(CMap &b, CONST COOR *coor, long count)
{
  long i;
  int xmin = 0, ymin = 0, xmax = 0, ymax = 0, x1, y1, x2, y2;

  // Figure out the bounds of all the line segments.
  for (i = 0; i < count; i++) {
    x1 = (int)coor[i].x1; y1 = (int)coor[i].y1;
    x2 = (int)coor[i].x2; y2 = (int)coor[i].y2;
    xmin = Min(x1, xmin); xmin = Min(x2, xmin);
    ymin = Min(y1, ymin); ymin = Min(y2, ymin);
    xmax = Max(x1, xmax); xmax = Max(x2, xmax);
    ymax = Max(y1, ymax); ymax = Max(y2, ymax);
  }

  // Resize the bitmap to encompass these bounds.
  if (!b.FBitmapSizeSet(xmax-xmin+1, ymax-ymin+1))
    return fFalse;

  // Draw all the line segments.
  b.BitmapOn();
  for (i = 0; i < count; i++) {
    x1 = (int)coor[i].x1 - xmin; y1 = (int)coor[i].y1 - ymin;
    x2 = (int)coor[i].x2 - xmin; y2 = (int)coor[i].y2 - ymin;
    b.Line(x1, y1, x2, y2, b.FColor() ? coor[i].kv : fOff);
  }
  return fTrue;
}


// Given a line segment, return whether it already exists in the list of line
// segments. Called from ConvertPatchToWire() to avoid including edges between
// patches more than once.

flag FVerifyWire(COOR *coor, long count)
{
  real rT;
  long i;

  // Sort the two endpoints of the line segment.
  if (coor[count].z2 < coor[count].z1 || (coor[count].z2 == coor[count].z1
    && (coor[count].y2 < coor[count].y1 || (coor[count].y2 ==
    coor[count].y1 && coor[count].x2 < coor[count].x1)))) {
    rT = coor[count].x1;
    coor[count].x1 = coor[count].x2;
    coor[count].x2 = rT;
    rT = coor[count].y1;
    coor[count].y1 = coor[count].y2;
    coor[count].y2 = rT;
    rT = coor[count].z1;
    coor[count].z1 = coor[count].z2;
    coor[count].z2 = rT;
  }

  // Check whether the line segment already exists in the list.
  for (i = count - 1; i >= 0; i--)
    if (coor[count].x1 == coor[i].x1 && coor[count].x2 == coor[i].x2 &&
      coor[count].y1 == coor[i].y1 && coor[count].y2 == coor[i].y2 &&
      coor[count].z1 == coor[i].z1 && coor[count].z2 == coor[i].z2) {
      return fFalse;
    }
  return fTrue;
}


// Generate a wireframe list of line segments based on the list of patches in
// memory, with one line segment for each visible edge of a patch. Implements
// the Convert Patches To Wireframe command.

long ConvertPatchToWire(COOR **pcoor, CONST PATCH *patch, long cpatch)
{
  COOR *coor;
  long count = 0, i;

  if (cpatch == 0)
    return 0;
  coor = RgAllocate(cpatch*4, COOR);
  if (coor == NULL)
    return -1;
  if (*pcoor != NULL)
    DeallocateP(*pcoor);
  *pcoor = coor;
  for (i = 0; i < cpatch; i++) {
    if (patch[i].p[0].fLine) {
      coor[count].x1 = patch[i].p[0].x;
      coor[count].y1 = patch[i].p[0].y;
      coor[count].z1 = patch[i].p[0].z;
      coor[count].x2 = patch[i].p[1].x;
      coor[count].y2 = patch[i].p[1].y;
      coor[count].z2 = patch[i].p[1].z;
      count += FVerifyWire(coor, count);
    }
    if (patch[i].p[1].fLine) {
      coor[count].x1 = patch[i].p[1].x;
      coor[count].y1 = patch[i].p[1].y;
      coor[count].z1 = patch[i].p[1].z;
      coor[count].x2 = patch[i].p[2].x;
      coor[count].y2 = patch[i].p[2].y;
      coor[count].z2 = patch[i].p[2].z;
      count += FVerifyWire(coor, count);
    }
    if (patch[i].cpt <= 3) {
      if (patch[i].p[2].fLine) {
        coor[count].x1 = patch[i].p[2].x;
        coor[count].y1 = patch[i].p[2].y;
        coor[count].z1 = patch[i].p[2].z;
        coor[count].x2 = patch[i].p[0].x;
        coor[count].y2 = patch[i].p[0].y;
        coor[count].z2 = patch[i].p[0].z;
        count += FVerifyWire(coor, count);
      }
    } else {
      if (patch[i].p[2].fLine) {
        coor[count].x1 = patch[i].p[2].x;
        coor[count].y1 = patch[i].p[2].y;
        coor[count].z1 = patch[i].p[2].z;
        coor[count].x2 = patch[i].p[3].x;
        coor[count].y2 = patch[i].p[3].y;
        coor[count].z2 = patch[i].p[3].z;
        count += FVerifyWire(coor, count);
      }
      if (patch[i].p[3].fLine) {
        coor[count].x1 = patch[i].p[3].x;
        coor[count].y1 = patch[i].p[3].y;
        coor[count].z1 = patch[i].p[3].z;
        coor[count].x2 = patch[i].p[0].x;
        coor[count].y2 = patch[i].p[0].y;
        coor[count].z2 = patch[i].p[0].z;
        count += FVerifyWire(coor, count);
      }
    }
  }
  return count;
}

/* draw2.cpp */
