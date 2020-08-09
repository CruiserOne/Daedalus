/*
** Daedalus (Version 3.3) File: threed.cpp
** By Walter D. Pullen, Astara@msn.com, http://www.astrolog.org/labyrnth.htm
**
** IMPORTANT NOTICE: Daedalus and all Maze generation and general
** graphics routines used in this program are Copyright (C) 1998-2018 by
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
** This file contains general graphics routines, that operate on a monochrome
** or color bitmap being treated as a 3D or 4D bitmap, unrelated to Mazes.
**
** Created: 3/12/1990.
** Last code change: 11/29/2018.
*/

#include <stdio.h>
#include <math.h>
#include "util.h"
#include "graphics.h"
#include "color.h"
#include "threed.h"


// Offset table for 3D/4D orthogonal and 2D diagonal directions.
CONST int woff3[DIRS4 + 4] = { 0, 0, 0, 0, 0, 0,-1, 1,  0, 0, 0, 0};
CONST int xoff3[DIRS4 + 4] = { 0,-1, 0, 1, 0, 0, 0, 0, -1,-1, 1, 1};
CONST int yoff3[DIRS4 + 4] = {-1, 0, 1, 0, 0, 0, 0, 0, -1, 1, 1,-1};
CONST int zoff3[DIRS4 + 4] = { 0, 0, 0, 0,-1, 1, 0, 0,  0, 0, 0, 0};


/*
******************************************************************************
** 3D Bitmap Routines
******************************************************************************
*/

// Allocate a 3D bitmap of a given size. The z levels will be arranged in a
// grid with no more than w levels per row.

flag CMap3::FAllocateCube(int x, int y, int z, int w)
{
  if (x < 0 || y < 0 || z < 0 ||
    x > xCubeMax || y > yCubeMax || z > zCubeMax) {
    PrintSz_E("Can't create 3D bitmap that large!\n");
    return fFalse;
  }
  if (!FAllocate(x * Min(w, z), y * ((z + w-1) / w), NULL))
    return fFalse;
  m_x3 = x; m_y3 = y; m_z3 = z; m_w3 = w;
  return fTrue;
}


// Make a 3D bitmap be a given size, allocating or reallocating if necessary.

flag CMap3::FCubeSizeSet(int xnew, int ynew, int znew, int wnew)
{
  if (!FBitmapSizeSet(xnew * Min(wnew, znew),
    ynew * ((znew + wnew-1) / wnew)))
    return fFalse;
  m_x3 = xnew; m_y3 = ynew; m_z3 = znew; m_w3 = wnew;
  return fTrue;
}


// Make a 4D bitmap be a given size, allocating or reallocating if necessary.
// A 4D bitmap is a w*z grid of levels, each x*y pixels in size.

flag CMap3::FTesseractSizeSet(int wnew, int xnew, int ynew, int znew)
{
  if (!FBitmapSizeSet((Even(wnew)-1) * xnew, (Even(znew)-1) * ynew))
    return fFalse;
  m_x3 = xnew; m_y3 = ynew; m_z3 = znew; m_w3 = wnew;
  return fTrue;
}


// Force coordinates to be within the bounds of a 3D bitmap, moving them to
// the edge of the bitmap if not within it already.

void CMap3::Legalize3(int *x, int *y, int *z) CONST
{
  if (*x < 0)
    *x = 0;
  else if (*x >= m_x3)
    *x = m_x3-1;
  if (*y < 0)
    *y = 0;
  else if (*y >= m_y3)
    *y = m_y3-1;
  if (*z < 0)
    *z = 0;
  else if (*z >= m_z3)
    *z = m_z3-1;
}


// Set all pixels within a 3D rectangle in a 3D bitmap.

void CMap3::CubeBlock(int x1, int y1, int z1, int x2, int y2, int z2, KV kv)
{
  int z;

  if ((x1 < 0 && x2 < 0) || (x1 >= m_x3 && x2 >= m_x3) ||
    (y1 < 0 && y2 < 0) || (y1 >= m_y3 && y2 >= m_y3) ||
    (z1 < 0 && z2 < 0) || (z1 >= m_z3 && z2 >= m_z3))
    return;
  Legalize3(&x1, &y1, &z1); Legalize3(&x2, &y2, &z2);
  SortN(&z1, &z2);
  for (z = z1; z <= z2; z++)
    Block(X2(x1, z), Y2(y1, z), X2(x2, z), Y2(y2, z), kv);
}


// Copy pixels in a 3D rectangle from one 3D bitmap to another 3D bitmap. If
// the bitmaps are the same, the rectangles should not overlap.

void CMap3::CubeMove(CONST CMap3 &t, int x1, int y1, int z1,
  int x2, int y2, int z2, int x0, int y0, int z0)
{
  int x, y, z, xT, yT, zT;

  t.Legalize3(&x1, &y1, &z1); t.Legalize3(&x2, &y2, &z2);
  SortN(&x1, &x2);
  SortN(&y1, &y2);
  SortN(&z1, &z2);

  // Normal case: Copy one pixel at a time from source to dest rectangle.
  for (z = z1; z <= z2; z++)
    for (y = y1; y <= y2; y++)
      for (x = x1; x <= x2; x++) {
        xT = x0+(x-x1); yT = y0+(y-y1); zT = z0+(z-z1);
        if (FLegalCube(xT, yT, zT))
          Set3(xT, yT, zT, t.Get3(x, y, z));
      }
}


// Resize a 3D bitmap to be a given size, by either truncating or appending
// off pixels to its east, south, and bottom faces.

flag CMap3::FCubeResizeTo(int xnew, int ynew, int znew)
{
  CMap3 *tNew;

  if (m_x3 == xnew && m_y3 == ynew && m_z3 == znew)
    return fTrue;
  tNew = Create3();
  if (!tNew->FAllocateCube(xnew, ynew, znew, m_w3))
    return fFalse;
  tNew->BitmapOff();
  if (!FZero3())
    tNew->CubeMove(*this, 0, 0, 0, m_x3-1, m_y3-1, m_z3-1, 0, 0, 0);
  CopyFrom(*tNew);
  tNew->Destroy3();
  return fTrue;
}


// Resize a 3D bitmap by a given offset, by either truncating or appending off
// pixels to its west, north, and top faces.

flag CMap3::FCubeShiftBy(int xnew, int ynew, int znew)
{
  CMap3 *tNew;
  int x, y, z;

  if (xnew == 0 && ynew == 0 && znew == 0)
    return fTrue;
  tNew = Create3();
  if (!tNew->FAllocateCube(m_x3 + xnew, m_y3 + ynew, m_z3 + znew, m_w3))
    return fFalse;
  for (z = 0; z < tNew->m_z3; z++)
    for (y = 0; y < tNew->m_y3; y++)
      for (x = 0; x < tNew->m_x3; x++)
        tNew->Set3(x, y, z, Get3(x - xnew, y - ynew, z - znew));
  CopyFrom(*tNew);
  tNew->Destroy3();
  return fTrue;
}


// Resize a 3D bitmap smaller, by trimming all blank planes from its six
// faces.

flag CMap3::FCubeCollapse()
{
  int xnew, ynew, znew, x, y, z;

  // Trim off pixels from the east, south, and bottom faces.
  for (znew = m_z3-1; znew >= 0; znew--)
    for (y = 0; y < m_y3; y++)
      for (x = 0; x < m_x3; x++)
        if (Get3(x, y, znew))
          goto LNext1;
LNext1:
  for (ynew = m_y3-1; ynew >= 0; ynew--)
    for (z = 0; z < znew; z++)
      for (x = 0; x < m_x3; x++)
        if (Get3(x, ynew, z))
          goto LNext2;
LNext2:
  for (xnew = m_x3-1; xnew >= 0; xnew--)
    for (z = 0; z < znew; z++)
      for (y = 0; y < ynew; y++)
        if (Get3(xnew, y, z))
          goto LNext3;
LNext3:
  if (!FCubeResizeTo(xnew + 1, ynew + 1, znew + 1))
    return fFalse;

  // Trim off pixels from the west, north, and top faces.
  for (znew = 0; znew < m_z3; znew++)
    for (y = 0; y < m_y3; y++)
      for (x = 0; x < m_x3; x++)
        if (Get3(x, y, znew))
          goto LNext4;
LNext4:
  for (ynew = 0; ynew < m_y3; ynew++)
    for (z = znew; z < m_z3; z++)
      for (x = 0; x < m_x3; x++)
        if (Get3(x, ynew, z))
          goto LNext5;
LNext5:
  for (xnew = 0; xnew < m_x3; xnew++)
    for (z = znew; z < m_z3; z++)
      for (y = ynew; y < m_y3; y++)
        if (Get3(xnew, y, z))
          goto LNext6;
LNext6:
  return FCubeShiftBy(-xnew, -ynew, -znew);
}


// Given a coordinates in a 3D bitmap, and an indicator for an axis to focus
// on, return that value transformed appropriately. Used by FCubeFlip().

int CMap3::CubeFlipCore(int x, int y, int z, char ch) CONST
{
  switch (ch) {
  case 'x': return x;
  case 'y': return y;
  case 'z': return z;
  case 'X': return m_x3 - x - 1;
  case 'Y': return m_y3 - y - 1;
  case 'Z': return m_z3 - z - 1;
  }
  Assert(fFalse);
  return 0;
}


// Transform a 3D bitmap by remapping each of its axes as specified.

flag CMap3::FCubeFlip(CONST char *rgch)
{
  CMap3 *tNew;
  int x, y, z, x2, y2, z2;
  flag fColor = FColor();

  // Parameter rgch is three characters long, where each indicates how to
  // transform the x, y, and z axes of the 3D bitmap.
  if ((!FBetween(rgch[0], 'X', 'Z') && !FBetween(rgch[0], 'x', 'z')) ||
    (!FBetween(rgch[1], 'X', 'Z') && !FBetween(rgch[1], 'x', 'z')) ||
    (!FBetween(rgch[2], 'X', 'Z') && !FBetween(rgch[2], 'x', 'z')) ||
    rgch[3] != chNull) {
    PrintSz_E("Bad CubeFlip axis or string not 3 characters long.\n");
    return fFalse;
  }

  // Prepare to allocate a new 3D bitmap to contain the transformed pixels.
  x = CubeFlipCore(m_x3, m_y3, m_z3, ChUncap(rgch[0]));
  y = CubeFlipCore(m_x3, m_y3, m_z3, ChUncap(rgch[1]));
  z = CubeFlipCore(m_x3, m_y3, m_z3, ChUncap(rgch[2]));

  tNew = Create3();
  if (tNew == NULL)
    return fFalse;
  if (!tNew->FAllocateCube(x, y, z, m_w3))
    return fFalse;
  if (!fColor)
    tNew->BitmapOff();

  // Copy each pixel to its new coordinate.
  for (z = 0; z < m_z3; z++)
    for (y = 0; y < m_y3; y++)
      for (x = 0; x < m_x3; x++) {
        x2 = CubeFlipCore(x, y, z, rgch[0]);
        y2 = CubeFlipCore(x, y, z, rgch[1]);
        z2 = CubeFlipCore(x, y, z, rgch[2]);
        if (fColor)
          tNew->Set3(x2, y2, z2, Get3(x, y, z));
        else if (Get3(x, y, z))
          tNew->Set31(x2, y2, z2);
      }
  CopyFrom(*tNew);
  tNew->Destroy3();
  return fTrue;
}


// Flip or rotate a 3D bitmap across or around a given axis. Called from the
// 3D Bitmap dialog and the Flip3D operation.

flag CMap3::FCubeFlip2(int nAxis, int nOp)
{
  CONST char *pch;

  Assert(FBetween(nAxis, 0, 2) && FBetween(nOp, 0, 3));
  switch (nAxis*4 + nOp) {
  case 0:  pch = "Xyz"; break; // X Flip
  case 1:  pch = "xzY"; break; // X Rotate right
  case 2:  pch = "xZy"; break; // X Rotate left
  case 3:  pch = "xYZ"; break; // X Rotate across
  case 4:  pch = "xYz"; break; // Y Flip
  case 5:  pch = "zyX"; break; // Y Rotate right
  case 6:  pch = "Zyx"; break; // Y Rotate left
  case 7:  pch = "XyZ"; break; // Y Rotate across
  case 8:  pch = "xyZ"; break; // Z Flip
  case 9:  pch = "Yxz"; break; // Z Rotate right
  case 10: pch = "yXz"; break; // Z Rotate left
  case 11: pch = "XYz"; break; // Z Rotate across
  default: Assert(fFalse); return fFalse;
  }
  return FCubeFlip(pch);
}


// Flood an irregular volume in a 3D bitmap with on or off pixels.

flag CMap3::FCubeFill(int x, int y, int z, KV o)
{
  PT *rgpt;
  long ipt = 0;
  int xnew, ynew, znew, d;

  if (!FLegalFillCube(x, y, z, o))
    return fTrue;
  if (F64K())
    return fFalse;
  rgpt = RgAllocate(m_x3 * m_y3 * m_z3, PT);
  if (rgpt == NULL)
    return fFalse;
LSet:
  Set3(x, y, z, o);
  loop {
    for (d = 0; d < DIRS3; d++) {
      xnew = x + xoff3[d]; ynew = y + yoff3[d]; znew = z + zoff3[d];
      if (FLegalFillCube(xnew, ynew, znew, o)) {
        FillPush(X2(x, z), Y2(y, z));
        x = xnew; y = ynew; z = znew;
        goto LSet;
      }
    }
    if (ipt <= 0)
      break;
    FillPop(x, y);
    z = Z3(x, y); x = X3(x); y = Y3(y);
  }
  DeallocateP(rgpt);
  return fTrue;
}


// Copy a monochrome bitmap to a color bitmap, where off and on pixels in the
// monochrome bitmap are mapped to the passed in colors. All off pixels
// reachable from a start point or points will be filled with a blend of
// colors indicating their distance from the nearest start point.

int CCol3::ColmapGraphDistance3(CONST CMon3 &t, CONST CMon3 &t2,
  KV kv0, KV kv1, int x, int y, int z)
{
  BFSS *bfss;
  int count = 0, xnew, ynew, znew, d;
  long iLo = 0, iHi, iMax = 0, i;
  flag f;

  if (!t.FLegalCube(x, y, z) || t.Get3(x, y, z)) {
    if (!t.FBitmapFind(&x, &y, fOff)) {
      PrintSz_W("There are no open sections to graph.\n");
      return -2;
    }
  } else {
    x = X2(x, z); y = Y2(y, z);
  }
  if (t.F64K())
    return -2;
  if (!FColmapGetFromBitmap(t, kv0, kv1))
    return -1;
  bfss = RgAllocate(m_x*m_y, BFSS);
  if (bfss == NULL)
    return -1;
  Set(x, y, kv1);
  BfssPush(iMax, x, y, 0);

  // Potentially allow all off pixels in a 2nd monochrome bitmap to act as
  // start points to flood from, if it exists and is same size as main bitmap.
  f = fFalse;
  if (!t2.FNull() && t2.m_x == t.m_x && t2.m_y == t.m_y)
    for (ynew = 0; ynew < t2.m_y; ynew++)
      for (xnew = 0; xnew < t2.m_x; xnew++)
        if (!t2.Get(xnew, ynew)) {
          // Reject 2nd bitmap if it contains an off pixel not off in main.
          if (t.Get(xnew, ynew))
            goto LBegin;
        } else if (!t.Get(xnew, ynew))
          f = fTrue;
  // Accept 2nd bitmap if it contains at least one on pixel off in the main.
  if (f)
    for (ynew = 0; ynew < t2.m_y; ynew++)
      for (xnew = 0; xnew < t2.m_x; xnew++)
        if (!t2.Get(xnew, ynew)) {
          Assert(!t.Get(xnew, ynew));
          Set(xnew, ynew, kv1);
          BfssPush(iMax, xnew, ynew, 0);
        }

LBegin:
  // Flood bitmap from start points, marking distances for each pixel touched.
  iHi = iMax;
  while (iLo < iHi) {
    count++;
    for (i = iLo; i < iHi; i++) {
      x = X3(bfss[i].x); y = Y3(bfss[i].y); z = Z3(bfss[i].x, bfss[i].y);
      for (d = 0; d < DIRS3; d++) {
        xnew = x + xoff3[d]; ynew = y + yoff3[d]; znew = z + zoff3[d];
        if (FLegalCube(xnew, ynew, znew) && Get3(xnew, ynew, znew) != kv1) {
          Set3(xnew, ynew, znew, kv1);
          BfssPush(iMax, X2(xnew, znew), Y2(ynew, znew), count);
        }
      }
    }
    iLo = iHi; iHi = iMax;
  }

  // Map distance numbers to colors.
  if (!cs.fGraphNumber) {
    for (i = 0; i < iMax; i++)
      Set3(X3(bfss[i].x), Y3(bfss[i].y), Z3(bfss[i].x, bfss[i].y),
        Hue(NMultDiv(bfss[i].parent, nHueMax, count)));
  } else {
    for (i = 0; i < iMax; i++)
      Set3(X3(bfss[i].x), Y3(bfss[i].y), Z3(bfss[i].x, bfss[i].y),
        bfss[i].parent);
  }
  DeallocateP(bfss);
  return count - 1;
}


/*
******************************************************************************
** 3D Bitmap File Routines
******************************************************************************
*/

// Load a Daedalus 3D bitmap from a file, into a new 3D bitmap.

flag CMon3::FReadCube(FILE *file, int w)
{
  int x, y, z, xT, nCur;
  char ch, ch2;

  ch = getbyte(); ch2 = getbyte();
  if (ch != 'D' || ch2 != '3') {
    PrintSz_W("This file does not look like a Daedalus 3D bitmap.\n");
    return fFalse;
  }
  ch2 = getbyte();
  fscanf(file, "%d%d%d", &x, &y, &z);
  if (!FCubeSizeSet(x, y, z, w))
    return fFalse;
  BitmapOff();
  skipcrlf(); skipcrlf();
  if (ch2 == 'C') {

    // Supercompressed: 1 character = 6 pixels, plus run length encoding.
    for (z = 0; z < m_z3; z++) {
      for (y = 0; y < m_y3; y++) {
        x = 0;
        loop {
          ch = getbyte();
          if (ch < ' ') {
            skiplf();
            break;
          }
          if (ch < 'a') {
            // This character encodes 6 pixels.
            nCur = (int)(ch - '!');
            for (xT = 5; xT >= 0; xT--) {
              if (FOdd(nCur))
                Set31(x + xT, y, z);
              nCur >>= 1;
            }
            x += 6;
          } else {
            // Check for the duplicate row marker '|'.
            if (ch == '|') {
              Assert(x == 0 && y > 0);
              for (nCur = 0, ch = getbyte(); FDigitCh(ch); ch = getbyte())
                nCur = nCur * 10 + (ch - '0');
              loop {
                BlockMove(*this, X2(0, z), Y2(y-1, z),
                  X2(m_x3-1, z), Y2(y-1, z), X2(0, z), Y2(y, z));
                if (--nCur <= 0)
                  break;
                y++;
              }
              skipcrlf();
              break;
            }
            // This character contains some multiple of 6 on or off pixels.
            if (ch >= 'n') {
              xT = X2(x, z);
              LineX(xT, xT + (ch - 'n' + 2) * 6 - 1, Y2(y, z), fOn);
              x += (ch - 'n' + 2) * 6;
            } else
              x += (ch - 'a' + 2) * 6;
          }
        }
      }
      skipcrlf();
    }
  } else if (ch2 == 'B') {

    // Compressed: 1 character = 4 pixels.
    for (z = 0; z < m_z3; z++) {
      for (y = 0; y < m_y3; y++) {
        x = 0;
        loop {
          ch = getbyte();
          if (ch < ' ') {
            skiplf();
            break;
          }
          nCur = NHex(ch);
          for (xT = 3; xT >= 0; xT--) {
            if (x + xT < m_x3 && FOdd(nCur))
              Set31(x + xT, y, z);
            nCur >>= 1;
          }
          x += 4;
        }
      }
      skipcrlf();
    }
  } else {

    // Not compressed: 1 character = 1 pixel.
    for (z = 0; z < m_z3; z++) {
      for (y = 0; y < m_y3; y++) {
        x = 0;
        loop {
          ch = getbyte();
          if (ch < ' ') {
            skiplf();
            break;
          }
          if (ch >= chOn)
            Set31(x, y, z);
          x++;
        }
      }
      skipcrlf();
    }
  }
  return fTrue;
}


// Save a 3D bitmap to a Daedalus 3D bitmap file.

void CMon3::WriteCube(FILE *file, int nCompress, flag fClip) CONST
{
  int x, y, z, xT, xmax, nCur, nSav, cn, cRow = 0;
  flag fBlank;
  byte bT;

  fprintf(file, "D3%c\n%d %d %d\n\n", '@' + nCompress, m_x3, m_y3, m_z3);
  if (nCompress >= 3) {

    // Supercompressed: 1 character = 6 pixels, plus run length encoding.
    for (z = 0; z < m_z3; z++) {
      for (y = 0; y <= m_y3; y++) {

        // Check if current row same as previous. If so write '|' dup marker.
        if (y > 0) {
          if (y < m_y3) {
            for (x = 0; x < m_x3; x++)
              if (Get3(x, y, z) != Get3(x, y-1, z))
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
        if (y >= m_y3)
          continue;

        // Output a unique row.
        xmax = m_x3 - 1;
        if (fClip)
          while (xmax >= 0 && !Get3(xmax, y, z))
            xmax--;
        fBlank = (xmax < 0);
        cn = 0;
        for (x = 0; x <= xmax + 6; x += 6) {
          // Compute a number encoding the next 6 pixels in this row.
          nCur = 0;
          for (xT = 0; xT < 6; xT++)
            nCur = (nCur << 1) + (x + xT < m_x3 && Get3(x + xT, y, z));
          // Check for whether ready to write next character/run to file.
          if (x > 0 && (nCur != nSav ||
            (nCur != 0 && nCur != 63) || cn >= 14 || x > xmax)) {
            if (cn <= 1)
              putbyte('!' + nSav);
            else {
              Assert(nSav == 0 || nSav == 63);
              bT = nSav == 0 ? 'a' - 2 + cn : 'n' - 2 + cn;
              putbyte(bT);
            }
            cn = 0;
          }
          nSav = nCur;
          cn++;
        }
        putbyte('\n');
      }
      putbyte('\n');
    }
  } else if (nCompress == 2) {

    // Compressed: 1 character = 4 pixels.
    for (z = 0; z < m_z3; z++) {
      for (y = 0; y < m_y3; y++) {
        xmax = m_x3 - 1;
        if (fClip)
          while (xmax >= 0 && !Get3(xmax, y, z))
            xmax--;
        for (x = 0; x <= xmax; x += 4) {
          nCur = 0;
          for (xT = 0; xT < 4; xT++)
            nCur = (nCur << 1) + (x + xT < m_x3 && Get3(x + xT, y, z));
          putbyte(ChHex(nCur));
        }
        putbyte('\n');
      }
      putbyte('\n');
    }
  } else {

    // Not compressed: 1 character = 1 pixel.
    for (z = 0; z < m_z3; z++) {
      for (y = 0; y < m_y3; y++) {
        xmax = m_x3 - 1;
        if (fClip)
          while (xmax >= 0 && !Get3(xmax, y, z))
            xmax--;
        for (x = 0; x <= xmax; x++)
          putbyte(Get3(x, y, z) ? chOn : chOff);
        putbyte('\n');
      }
      putbyte('\n');
    }
  }
}

/* threed.cpp */
