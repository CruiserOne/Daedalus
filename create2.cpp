/*
** Daedalus (Version 3.3) File: create2.cpp
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
** This file contains Maze creation algorithms, specifically algorithms that
** produce orthogonal Mazes, however which aren't standard 2D Mazes.
**
** Created: 11/22/1996.
** Last code change: 11/29/2018.
*/

#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "graphics.h"
#include "color.h"
#include "threed.h"
#include "maze.h"


/*
******************************************************************************
** Virtual Fractal Maze Routines
******************************************************************************
*/

enum _virtualmazetype {
  vmtNested    = 0,  // Nested cell fractal Maze
  vmtBinary    = 1,  // Binary tree Maze
  vmtDivision  = 2,  // Recursive division Maze
  vmtUnicursal = 3,  // Random Labyrinth
  vmtClassical = 4,  // Classical Labyrinth
  vmtHilbert   = 5,  // Hilbert curve Labyrinth
};

// Quickly create a 2x2 passage Maze in a bitmap. The bitmap is assumed to be
// of appropriate size to contain it. Used to quickly create fractal Mazes
// that nest 2x2 Mazes.

void CMaz::TwoByTwoGenerate()
{
  int d;

  BitmapOff();
  LineX(0, 4, 0, fOn); LineX(0, 4, 4, fOn);
  LineY(0, 1, 3, fOn); LineY(4, 1, 3, fOn);
  Set1(2, 2);

  // Randomly connect the center vertex to one of the four boundary walls.
  d = RndDir();
  Set1(2 + xoff[d], 2 + yoff[d]);
  MakeEntranceExit(0);
  UpdateDisplay();
}


// Create a section of a fractal Maze. Recursively calls itself to create the
// subsections or Mazes within each cell of the Maze at this nesting level.

void CMaz::FractalGenerate(CMaz &bT, int z, int xp, int yp)
{
  int xsMax = 1, ysMax = 1, x, y, i;

  z--;
  for (i = 0; i < z; i++) {
    xsMax *= ms.xFractal; ysMax *= ms.yFractal;
  }

  // Create the Maze for this nesting level.
  if (ms.xFractal != 2 || ms.yFractal != 2)
    bT.CreateMazePerfect();
  else
    bT.TwoByTwoGenerate();

  // Each Maze contains a top and left edge (but no bottom or right edge) with
  // a random entrance in those edges. Those entrances may or may not be
  // visible when this Maze is nested within surrounding Mazes. For the
  // outermost Maze, don't have either entrance (they'll be added later).
  if (z+1 < ms.zFractal)
    bT.Set0(0, (Rnd(1, ms.yFractal) << 1) - 1);
  else
    bT.Set1(ms.xEntrance, ms.yEntrance);

  // Render the Maze at appropriate zoom level and location.
  BlockMoveMaze(bT, 0, 0, bT.m_x-2, bT.m_y-2, xp << 1, yp << 1,
    xsMax << 1, 0, 0, ysMax << 1);

  // Recursively create a Maze within each cell of this Maze.
  if (z > 0)
    for (y = 0; y < ms.yFractal; y++)
      for (x = 0; x < ms.xFractal; x++)
        FractalGenerate(bT, z, xp + x * xsMax, yp + y * ysMax);
}


// Create a new nested fractal Maze, composed of a Maze with smaller Mazes
// recursively nested within each cell. The fractal Maze's size is defined by
// the Fractal fields in the Create Settings dialog.

flag CMaz::CreateMazeFractal()
{
  CMaz bT;
  char sz[cchSzMax];
  int xsMax = 1, ysMax = 1, i;

  // Ensure the fractal Maze will fit within a bitmap.
  for (i = 0; i < ms.zFractal; i++) {
    if ((xsMax * ms.xFractal << 1) + 1 > xBitmap ||
      (ysMax * ms.yFractal << 1) + 1 > yBitmap) {
      sprintf(S(sz),
        "%d by %d fractal Maze can't have a nesting level greater than %d.",
        ms.xFractal, ms.yFractal, i); PrintSz_W(sz);
      return fFalse;
    }
    xsMax *= ms.xFractal; ysMax *= ms.yFractal;
  }

  // Create the bitmap for the fractal Maze, and a temporary internal bitmap
  // to store the Mazes at each level.
  if (!FBitmapSizeSet((xsMax << 1) + 1, (ysMax << 1) + 1))
    return fFalse;
  if (!bT.FAllocate((ms.xFractal << 1) + 1, (ms.yFractal << 1) + 1, this))
    return fFalse;
  BitmapOff();
  UpdateDisplay();
  bT.SetXyh();

  // Create the fractal Maze here.
  i = ms.nEntrancePos; ms.nEntrancePos = epRandom;
  FractalGenerate(bT, ms.zFractal, 0, 0);
  ms.nEntrancePos = i;

  // Draw the right and bottom boundary walls.
  SetXyh();
  LineY(xh, 0, yh-1, fOn);
  LineX(0, xh, yh, fOn);
  MakeEntranceExit(0);
  return fTrue;
}


// Draw one subsection of a virtual fractal Maze using given random number
// seed. Called from FractalPartial() to create sections of the fractal Maze.

void CMaz::FractalPartialGenerate(CMaz &bT, int z, long x0, long y0,
  int xp, int yp, int nRndSeed, long *pxEntrance, long *pxExit)
{
  ulong rgl[4];
  long xTotal, yTotal, xsMax = 1, ysMax = 1, xT, yT, xEntrance, xExit;
  int xpMax = 1, ypMax = 1, xs, ys, i, nSav, nSav2, nT;
  flag fX, fY;

  for (i = 0; i < z; i++) {
    xpMax *= ms.xFractal; ypMax *= ms.yFractal;
  }
  for (i = z; i < ms.zFractal; i++) {
    xsMax *= ms.xFractal; ysMax *= ms.yFractal;
  }
  xTotal = xsMax * xpMax; yTotal = ysMax * ypMax;
  InitRndL(nRndSeed);

  // Figure out where the entrance and exit to the fractal Maze should be.
  switch (ms.nEntrancePos) {
  case 0:
    xEntrance = 0;
    xExit     = xTotal - 1;
    break;
  case 1:
    xEntrance = (xTotal - 1) >> 1;
    xExit     = xEntrance + !FOdd(xTotal);
    break;
  case 2:
    xEntrance = Rnd(0, xTotal - 1);
    xExit     = xTotal - 1 - xEntrance;
    break;
  case 3:
    xEntrance = Rnd(0, xTotal - 1);
    xExit     = Rnd(0, xTotal - 1);
    break;
  }
  *pxEntrance = xEntrance; *pxExit = xExit;

  // Leave the section blank if it's outside the bounds of the whole Maze.
  if (x0 >= 0 && x0 < xsMax && y0 >= 0 && y0 < ysMax) {
    if (ms.nFractalT <= vmtNested) {

      // Create fractal section using hunt and kill algorithm
      xT = xsMax; yT = ysMax;
      fX = fY = fFalse;

      // Get information about the Mazes surrounding starting nesting level.
      for (i = z; i < ms.zFractal; i++) {
        if (ms.xFractal != 2 || ms.yFractal != 2)
          bT.CreateMazePerfect();
        else
          bT.TwoByTwoGenerate();
        if (i > z)
          bT.Set0(0, (Rnd(1, ms.yFractal) << 1) - 1);
        else
          bT.Set1(ms.xEntrance, ms.yEntrance);
        xT /= ms.xFractal; yT /= ms.yFractal;
        xs = x0 / xT % ms.xFractal; ys = y0 / yT % ms.yFractal;

        // The top and/or left edges of the section should be solid if the
        // section is adjacent to any surrounding Maze boundary, and that
        // surrounding Maze has a wall segment at the appropriate position.
        if (y0 % yT == 0)
          fX |= bT.Get((xs << 1) + 1, ys << 1);
        if (x0 % xT == 0)
          fY |= bT.Get(xs << 1, (ys << 1) + 1);

        // Set the random number seed for this subsection.
        rgl[0] = nRndSeed; rgl[1] = i; rgl[2] = x0 / xT; rgl[3] = y0 / yT;
        InitRndRgl(rgl, 4);
      }

      // Create the fractal Maze section here.
      nSav = ms.nEntrancePos; ms.nEntrancePos = epRandom;
      FractalGenerate(bT, z, xp, yp);
      ms.nEntrancePos = nSav;
    } else {

      // Create fractal section using binary tree algorithm
      rgl[0] = nRndSeed; rgl[1] = z; rgl[2] = x0; rgl[3] = y0;
      InitRndRgl(rgl, 4);
      nSav = ms.nEntrancePos; ms.nEntrancePos = epCorner;
      nSav2 = ms.nRndRun; ms.nRndRun = 0;
      bT.CreateMazeBinary();
      ms.nEntrancePos = nSav; ms.nRndRun = nSav2;
      bT.Set1(1, 0);
      xT = (x0 > 0) << 1; yT = (y0 > 0) << 1;
      if (ms.nFractalT == vmtBinary)
        BlockMove(bT, xT, yT, bT.m_x-4 + xT, bT.m_y-4 + yT, xp << 1, yp << 1);
      else {

        // This binary tree Maze will be expanded into a unicursal Maze
        xT = (x0 < xsMax-1) << 1;
        BlockMoveMaze(bT, xT, yT,
          bT.m_x/2-2+xT + (FOdd(xpMax)*(FOdd(x0) + (x0 == xsMax-1)) << 1),
          bT.m_y/2-2+yT + (FOdd(ypMax)*(FOdd(y0) + (y0 == ysMax-1)) << 1),
          (xp + xpMax + FOdd(xpMax)*(FOdd(x0 == xsMax-1)*2 -FOdd(~x0))) << 1,
          (yp - 1 - FOdd(ypMax)*FOdd(y0)) << 1, -4, 0, 0, 4);
        if (x0 == 0 && FBetween(y0, 0, ysMax-1))
          LineY(xp << 1, yp << 1,
            (yp + ypMax - FOdd(~ypMax & (y0 == ysMax-1))) << 1, fOn);
        if (y0 == ysMax-1 && FBetween(x0, 0, xsMax-1)) {
          nT = FOdd(ypMax & (y0 == ysMax-1));
          LineX(xp << 1, (xp + xpMax) << 1, (yp + ypMax - 1 + nT) << 1, fOn);
          LineX(xp << 1, (xp + xpMax) << 1, (yp + ypMax + nT) << 1, fOff);
        }
      }
      fX = fY = fFalse;
    }
  } else {

    // Ensure the boundary walls at the right and bottom get drawn.
    fX = y0 == ysMax && FBetween(x0, 0, xsMax-1);
    fY = x0 == xsMax && FBetween(y0, 0, ysMax-1);
  }

  // Draw lines at the top and/or left of the section, if there should be no
  // passage between cells at this nesting level.
  if (fX)
    LineX(xp << 1, (xp + xpMax) << 1, yp << 1, fOn);
  if (fY)
    LineY(xp << 1, yp << 1, (yp + ypMax) << 1, fOn);

  // Draw the entrance or exit to the whole Maze if visible.
  if (y0 == 0 && x0 == xEntrance / xpMax)
    Set0(((xp + LMod(xEntrance, xpMax)) << 1) + 1, yp << 1);
  if (y0 == ysMax && x0 == xExit / xpMax)
    Set0(((xp + LMod(xExit, xpMax)) << 1) + 1, yp << 1);
}


// Create a grid of subsections of a virtual fractal Maze, where only part of
// the fractal Maze is rendered. Implements the FractalPart operation.

flag CMaz::FractalPartial(long x0, long y0, int z, int nRndSeed,
  long *pxEntrance, long *pxExit)
{
  CMaz bT;
  char sz[cchSzMax];
  int xpMax = 1, ypMax = 1, x, y, i, x1, x2, y1, y2, m1, n1;
  long xT, yT;

  // Ensure the subsections of the fractal Maze to make will fit in a bitmap.
  for (i = 0; i < z; i++) {
    if ((long)xpMax * ms.xFractal * 8 > xBitmap ||
      (long)ypMax * ms.yFractal * 8 > yBitmap) {
      sprintf(S(sz),
        "%d by %d fractal section can't have nesting level greater than %d.",
        ms.xFractal, ms.yFractal, i); PrintSz_W(sz);
      return fFalse;
    }
    xpMax *= ms.xFractal; ypMax *= ms.yFractal;
  }

  // Ensure the total size of the fractal Maze doesn't overflow 32 bits.
  xT = xpMax; yT = ypMax;
  for (i = z; i < ms.zFractal; i++) {
    if (xT * ms.xFractal / ms.xFractal != xT ||
      yT * ms.yFractal / ms.yFractal != yT) {
      sprintf(S(sz),
        "%d by %d fractal Maze can't have total nesting greater than %d.",
        ms.xFractal, ms.yFractal, i); PrintSz_W(sz);
      return fFalse;
    }
    xT *= ms.xFractal; yT *= ms.yFractal;
  }

  // Create the bitmap for the fractal Maze, and a temporary internal bitmap
  // to store the Mazes at each level.
  if (!FBitmapSizeSet(xpMax * 8, ypMax * 8))
    return fFalse;
  if (ms.nFractalT <= vmtBinary || ms.nFractalT == vmtUnicursal) {
    if (ms.nFractalT <= vmtNested) {
      if (!bT.FAllocate((ms.xFractal << 1) + 1, (ms.yFractal << 1) + 1, this))
        return fFalse;
    } else {
      if (!bT.FAllocate((xpMax << 1) + 3, (ypMax << 1) + 3, this))
        return fFalse;
    }
    BitmapOff();
    UpdateDisplay();
    bT.SetXyh();

    // Create a 4x4 grid of fractal Maze sections.
    for (y = 0; y < 4; y++)
      for (x = 0; x < 4; x++)
        FractalPartialGenerate(bT, z, x0 - 1 + x, y0 - 1 + y,
          x * xpMax, y * ypMax, nRndSeed, pxEntrance, pxExit);

    // For unicursal Mazes, do adjustments after all raw sections in place.
    if (ms.nFractalT == vmtUnicursal) {
      SetXyh();
      xl = FOdd(xpMax & ~x0)*2;
      yl = !FOdd(ypMax & ~y0)*2;
      UnicursalApply(fFalse);
      xl = yl = 0;
      m1 = (x0 - 1)*xpMax; n1 = (y0 - 1)*ypMax;
      x1 = NMax(-1 - m1, 0); x2 = NMin(xT - m1 + 1, m_x);
      if (x1 < x2) {
        LineX(x1 << 1, x2 << 1, (-1 - n1) << 1, fOff);
        LineX(x1 << 1, x2 << 1, (yT - n1 + FOdd(ypMax)) << 1, fOff);
      }
      y1 = NMax(-1 - n1, 0); y2 = NMin(yT - n1, m_y);
      if (y1 < y2)
        LineY((xT - m1 + FOdd(xpMax)) << 1, y1 << 1, y2 << 1, fOff);
      if (!FOdd(xpMax & ypMax)) {
        Set0(((xT-1 - m1) << 1) + 1, (yT-1 - n1) << 1);
        Set0(((xT-1 - m1) << 1) - 1, (yT-1 - n1) << 1);
        Set1((xT-1 - m1) << 1, ((yT-1 - n1) << 1) - 1);
      } else
        Set0(((xT - m1) << 1) - 1, (yT - n1) << 1);
      *pxEntrance = 0;
      *pxExit = xT - 2 + FOdd(xpMax);
    }

  // Recursive division and Labyrinth virtual Mazes can be made all at once.
  } else if (ms.nFractalT == vmtDivision) {
    DivisionPartial(xT, yT, (x0 - 1)*xpMax, (y0 - 1)*ypMax,
      nRndSeed, pxEntrance, pxExit);
  } else if (ms.nFractalT == vmtHilbert) {
    Assert(xT == yT);
    LabyrinthHilbertPartial(ms.zFractal, (x0 - 1)*xpMax, (y0 - 1)*ypMax);
    *pxEntrance = 0; *pxExit = xT - 1;
  } else {
    Assert(xT == yT);
    i = (xT >> 3) - 1;
    LabyrinthClassicalPartial(i, (x0 - 1)*xpMax, (y0 - 1)*ypMax);
    *pxEntrance = *pxExit = (i << 2) + 3;
  }
  return fTrue;
}


#define VX2(x) (((x) < x1 ? -1 : ((x) > x2 ? m_x : (x)-x1))*2)
#define VY2(y) (((y) < y1 ? -1 : ((y) > y2 ? m_y : (y)-y1))*2)

// Create a subsection of a virtual Maze, in which only part of the Maze is
// rendered, using the recursive division algorithm.

flag CMaz::DivisionPartial(int xs, int ys, int x1, int y1,
  int nRndSeed, long *pxEntrance, long *pxExit)
{
  ulong rgl[5];
  RC *rgrect, rect, *pr;
  int x2, y2, xEntrance, xExit, x, y;
  flag f;

  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize | femsMinSize))
    return fFalse;
  rgrect = RgAllocate(ms.nCrackPass*4, RC);
  if (rgrect == NULL)
    return fFalse;

  // Start with one rectangle in stack covering the entire Maze.
  InitRndL(nRndSeed);
  x2 = x1 + (m_x >> 1); y2 = y1 + (m_y >> 1);
  rect.x1 = rect.y1 = 0; rect.x2 = xs; rect.y2 = ys;
  pr = rgrect;
  *pr = rect;
  BitmapOff();

  // Do Edge(-x1*2, -y1*2, (xs-x1)*2, (ys-y1)*2) without overflowing 32 bits.
  if (FBetween(0,  y1, y2)) LineX(VX2(0),  VX2(xs), VY2(0),  fOn);
  if (FBetween(0,  x1, x2)) LineY(VX2(0),  VY2(0),  VY2(ys), fOn);
  if (FBetween(ys, y1, y2)) LineX(VX2(0),  VX2(xs), VY2(ys), fOn);
  if (FBetween(xs, x1, x2)) LineY(VX2(xs), VY2(0),  VY2(ys), fOn);

  // Figure out where the entrance and exit to the Maze should be.
  switch (ms.nEntrancePos) {
  case 0:
    xEntrance = 0;
    xExit     = xs - 1;
    break;
  case 1:
    xEntrance = (xs - 1) >> 1;
    xExit     = xEntrance + !FOdd(xs);
    break;
  case 2:
    xEntrance = Rnd(0, xs - 1);
    xExit     = xs - 1 - xEntrance;
    break;
  case 3:
    xEntrance = Rnd(0, xs - 1);
    xExit     = Rnd(0, xs - 1);
    break;
  }
  *pxEntrance = xEntrance; *pxExit = xExit;
  Set0((xEntrance-x1)*2+1, -y1*2);
  Set0((xExit    -x1)*2+1, (ys-y1)*2);
  UpdateDisplay();

  // Continue while there's at least one rectangle on the stack.
  while (pr >= rgrect) {
    rect = *pr;
    if (rect.x2 - rect.x1 < 2 || rect.y2 - rect.y1 < 2 ||
      rect.x1 > x2 || rect.x2 < x1 || rect.y1 > y2 || rect.y2 < y1 ||
      pr-rgrect >= ms.nCrackPass*4 - 1) {
      // If rectangle is too small, or is outside visible viewport, skip it.
      pr--;
      continue;
    }
    if (fCellMax)
      break;

    // Set the random number seed for this subrectangle.
    rgl[0] = nRndSeed;
    rgl[1] = rect.x1; rgl[2] = rect.y1; rgl[3] = rect.x2; rgl[4] = rect.y2;
    InitRndRgl(rgl, 5);

    // Determine whether to divide current rectangle horizontal or vertical.
    if (ms.nRndRun)
      f = Rnd(-50, 49) >= ms.nRndBias;
    else
      f = Rnd(0, ((rect.x2 - rect.x1) >> 1) + ((rect.y2 - rect.y1) >> 1)) +
        ms.nRndBias <= ((rect.x2 - rect.x1) >> 1);

    // Divide current rectangle and push two subrectangles on stack.
    if (f) {
      x = Rnd(rect.x1 + 1, rect.x2 - 1);
      if (FBetween(x, x1, x2)) {
        LineY(VX2(x), VY2(rect.y1)+1, VY2(rect.y2)-1, fOn);
        Set0(VX2(x), VY2(Rnd(rect.y1+1, rect.y2-1)) + 1);
      }
      pr->x1 = x; pr->y1 = rect.y1; pr->x2 = rect.x2; pr->y2 = rect.y2;
      pr++;
      pr->x1 = rect.x1; pr->y1 = rect.y1; pr->x2 = x; pr->y2 = rect.y2;
    } else {
      y = Rnd(rect.y1 + 1, rect.y2 - 1);
      if (FBetween(y, y1, y2)) {
        LineX(VX2(rect.x1)+1, VX2(rect.x2)-1, VY2(y), fOn);
        Set0(VX2(Rnd(rect.x1+1, rect.x2-1)) + 1, VY2(y));
      }
      pr->x1 = rect.x1; pr->y1 = y; pr->x2 = rect.x2; pr->y2 = rect.y2;
      pr++;
      pr->x1 = rect.x1; pr->y1 = rect.y1; pr->x2 = rect.x2; pr->y2 = y;
    }
  }
  DeallocateP(rgrect);
  return fTrue;
}


/*
******************************************************************************
** 3D Maze Routines
******************************************************************************
*/

CONST KV rgkvClar[DIRS] = {kvRed, kvBlue, kvGreen, kvYellow};


// Return the status of a pixel in a 3D Maze. Daedalus 3D Maze bitmaps don't
// have ceilings above the top level or floors below the bottom, so assume
// them here.

bit CMaz::Get3M(int x, int y, int z) CONST
{
  if (x < 0 || x >= m_x3 || y < 0 || y >= m_y3 || z < -1 || z > Odd(m_z3))
    return fOff;
  if (z == -1 || z == Odd(m_z3))
    return fOn;
  x = X2(x, z); y = Y2(y, z);
  return _Get(x, y);
}


// Return the status of a pixel in a 3D Maze, where the 3D Maze is assumed to
// be in the perspective inside view. This is like Get3M() except each floor
// extends forever throughout the plane. This prevents the Maze from appearing
// suspended in space, with just the top level sticking out of the ground.

bit CMaz::Get3I(int x, int y, int z) CONST
{
  if (x < 0 || x >= m_x3 || y < 0 || y >= m_y3 || z < 0 || z >= Odd(m_z3))
    return z & 1;
  x = X2(x, z); y = Y2(y, z);
  return _Get(x, y);
}


// Create a new perfect 3D Maze in the bitmap, stored as a sequence of 2D
// levels. The dimensions of the Maze are taken from the Bitmap Size dialog.

flag CMaz::CreateMaze3D()
{
  int tx, ty, tz, x, y, z, xnew, ynew, znew,
    fHunt = fFalse, pass = 0, d, i;
  long count;

  if (!FCubeSizeSet(m_x3, m_y3, m_z3, m_w3))
    return fFalse;
  BitmapOff();
  if (!FEnsureMazeSize(3, femsNoResize))
    return fFalse;
  tx = Even(m_x3); ty = Even(m_y3); tz = Even(m_z3);
  CubeBlock(0, 0, 0, tx-2, ty-2, tz-2, fOn);
  MakeEntranceExit(4);
  x = y = 1; z = 0;
  Set30(x, y, z);
  count = (long)((tx - 1) >> 1) * ((ty - 1) >> 1) * (tz >> 1) - 1;
  UpdateDisplay();
  if (count <= 0)
    goto LDone;

  // Use the Hunt and Kill algorithm to create the 3D Maze.
  loop {
    if (ms.nRndBias > 0 && Rnd(0, ms.nRndBias) > 0)
      i = DIRS1;
    else
      i = DIRS3-1;
    d = Rnd(0, i);
    if (!Get3(x, y, z)) {
      for (i = 0; i < DIRS3; i++) {
        xnew = x + (xoff3[d] << 1);
        ynew = y + (yoff3[d] << 1);
        znew = z + (zoff3[d] << 1);
        if (FLegalMaze3(xnew, ynew, znew) && Get3(xnew, ynew, znew))
          break;
        d++;
        if (d >= DIRS3)
          d = 0;
      }
      if (ms.fRiver)
        fHunt = (i >= DIRS3);
      else {
        fHunt = i > 0;
        if (i < DIRS3)
          pass = 0;
      }
    }
    if (!fHunt) {
      if (fCellMax)
        break;
      Set30((x + xnew) >> 1, (y + ynew) >> 1, (z + znew) >> 1);
      Set30(xnew, ynew, znew);
      x = xnew; y = ynew; z = znew;
      pass = 0;
      count--;
      if (count <= 0)
        goto LDone;
    } else {
      x += 2;
      if (x >= tx-1) {
        x = 1;
        y += 2;
        if (y >= ty-1) {
          y = 1;
          z += 2;
          if (z >= tz) {
            z = 0;
            pass++;
            if (pass > 1) {
              Assert(fFalse);
              goto LDone;
            }
            UpdateDisplay();
          }
        }
      }
    }
  }
LDone:
  return fTrue;
}


// Create a new perfect 3D Maze in the bitmap using the Binary Tree algorithm
// (technically a trinary tree in this case) by carving passages.

flag CMaz::CreateMazeBinary3D()
{
  int nPercent = 50 + ms.nRndBias, nDir, xInc, iMax,
    tx, ty, tz, x, y, z, i, d;

  if (!FCubeSizeSet(m_x3, m_y3, m_z3, m_w3))
    return fFalse;
  BitmapOff();
  if (!FEnsureMazeSize(3, femsNoResize))
    return fFalse;
  tx = Even(m_x3); ty = Even(m_y3); tz = Even(m_z3);
  CubeBlock(0, 0, 0, tx-2, ty-2, tz-2, fOn);
  MakeEntranceExit(4);
  UpdateDisplay();
  nDir = ms.nRndRun <= 0 ? -1 : ms.nRndRun >= 100;
  iMax = (tx-2) >> 1;

  for (z = 0; z < tz; z += 2)
    for (y = 1; y < ty-1; y += 2) {
      xInc = (nDir < 0 || (nDir == 0 && Rnd(0, 99) < ms.nRndRun)) ? 1 : -1;
      x = xInc < 0 ? tx-3 : 1;
      for (i = 0; i < iMax; i++) {

        // For each cell, carve a passage north, west, or up.
        if (fCellMax)
          break;
        Set30(x, y, z);;
        if (i <= 0 && y <= 1 && z <= 0)
          goto LNext;
        do {
          d = Rnd(0, 2);
        } while ((d == 0 && i <= 0) || (d == 1 && y <= 1) ||
          (d == 2 && z <= 0));
        if (d < 2)
          Set30(x - (d ^ 1)*xInc, y - d, z);
        else
          Set30(x, y, z - 1);
LNext:
        x += (xInc << 1);
      }
    }
  return fTrue;
}


// Create a new perfect 3D Maze in the bitmap using the Sidewinder algorithm,
// by carving passages.

flag CMaz::CreateMazeSidewinder3D()
{
  int nPercent = 50 + ms.nRndBias, tx, ty, tz, x, y, z, dx, f;

  if (!FCubeSizeSet(m_x3, m_y3, m_z3, m_w3))
    return fFalse;
  BitmapOff();
  if (!FEnsureMazeSize(3, femsNoResize))
    return fFalse;
  tx = Even(m_x3); ty = Even(m_y3); tz = Even(m_z3);
  CubeBlock(0, 0, 0, tx-2, ty-2, tz-2, fOn);
  MakeEntranceExit(4);
  LineX(1, tx-3, 1, fOff);
  UpdateDisplay();

  for (z = 0; z < tz; z += 2)
    for (y = 1; y < ty-1; y += 2) {
      if (y <= 1 && z <= 0)
        continue;
      for (x = 1; x < tx-1; x += 2) {

        // For each cell, decide whether to carve a passage along x axis.
        dx = 0;
        while (x < tx-3 && Rnd(0, 99) < nPercent)
          dx++, x += 2;
        if (fCellMax)
          break;

        // If no x passage, then carve a randomly positioned y or z passage.
        f = y <= 1 ? 0 : (z <= 0 ? 1 : Rnd(0, 1));
        if (dx == 0) {
          Set30(x, y-f, z-!f);
          Set30(x, y, z);
        } else {
          Set30(x - (Rnd(0, dx) << 1), y-f, z-!f);
          LineX(X2(x - (dx << 1), z), X2(x, z), Y2(y, z), fOff);
        }
      }
    }
  return fTrue;
}


// Create a new perfect 3D Maze in the bitmap using recursive division. This
// adds planes or walls, recursively dividing the Maze into smaller cubes.

flag CMaz::CreateMazeDivision3D()
{
  RC3 *rgcube, cube, *pr;
  int x, y, z, n;

  if (!FCubeSizeSet(m_x3, m_y3, m_z3, m_w3))
    return fFalse;
  BitmapOff();
  if (!FEnsureMazeSize(3, femsNoResize))
    return fFalse;
  rgcube = RgAllocate((m_x3 + m_y3 + m_z3) >> 1, RC3);
  if (rgcube == NULL)
    return fFalse;

  // Start with one cube in stack covering the entire Maze.
  cube.x1 = cube.y1 = 0; cube.z1 = -1;
  cube.x2 = m_x3-1 & ~1; cube.y2 = m_y3-1 & ~1; cube.z2 = (m_z3-1 & ~1) + 1;
  pr = rgcube;
  *pr = cube;
  BitmapOff();
  CubeBlock(0,       0,       0, cube.x2, 0,       cube.z2-1, fOn);
  CubeBlock(0,       cube.y2, 0, cube.x2, cube.y2, cube.z2-1, fOn);
  CubeBlock(0,       0,       0, 0,       cube.y2, cube.z2-1, fOn);
  CubeBlock(cube.x2, 0,       0, cube.x2, cube.y2, cube.z2-1, fOn);
  MakeEntranceExit(4);
  UpdateDisplay();

  // Continue while there's at least one cube on the stack.
  while (pr >= rgcube) {
    cube = *pr;
    x = cube.x2 - cube.x1; y = cube.y2 - cube.y1; z = cube.z2 - cube.z1;
    if ((x < 4) + (y < 4) + (z < 4) >= 2) {
      pr--;
      continue;
    }
    if (fCellMax)
      break;

    // Determine whether to divide current cube along x or y or z axis.
    if (x < 4)
      n = (Rnd(0, y + z + ms.nRndBias) <= y) + 1;
    else if (y < 4)
      n = (Rnd(0, x + z + ms.nRndBias) <= x) << 1;
    else if (z < 4)
      n = (Rnd(0, x + y) <= x);
    else {
      n = Rnd(0, x + y + z + ms.nRndBias);
      n = (n > x) + (n > x + y);
    }

    // Divide current cube and push two subcubes on stack.
    if (n <= 0) {
      x = RndSkip(cube.x1 + 2, cube.x2 - 2);
      CubeBlock(x, cube.y1+1, cube.z1+1, x, cube.y2-1, cube.z2-1, fOn);
      Set30(x, RndSkip(cube.y1+1, cube.y2-1), RndSkip(cube.z1+1, cube.z2-1));
      pr->x1 = x; pr->y1 = cube.y1; pr->z1 = cube.z1;
      pr->x2 = cube.x2; pr->y2 = cube.y2; pr->z2 = cube.z2;
      pr++;
      pr->x1 = cube.x1; pr->y1 = cube.y1; pr->z1 = cube.z1;
      pr->x2 = x; pr->y2 = cube.y2; pr->z2 = cube.z2;
    } else if (n == 1) {
      y = RndSkip(cube.y1 + 2, cube.y2 - 2);
      CubeBlock(cube.x1+1, y, cube.z1+1, cube.x2-1, y, cube.z2-1, fOn);
      Set30(RndSkip(cube.x1+1, cube.x2-1), y, RndSkip(cube.z1+1, cube.z2-1));
      pr->x1 = cube.x1; pr->y1 = y; pr->z1 = cube.z1;
      pr->x2 = cube.x2; pr->y2 = cube.y2; pr->z2 = cube.z2;
      pr++;
      pr->x1 = cube.x1; pr->y1 = cube.y1; pr->z1 = cube.z1;
      pr->x2 = cube.x2; pr->y2 = y; pr->z2 = cube.z2;
    } else {
      z = RndSkip(cube.z1 + 2, cube.z2 - 2);
      CubeBlock(cube.x1+1, cube.y1+1, z, cube.x2-1, cube.y2-1, z, fOn);
      Set30(RndSkip(cube.x1+1, cube.x2-1), RndSkip(cube.y1+1, cube.y2-1), z);
      pr->x1 = cube.x1; pr->y1 = cube.y1; pr->z1 = z;
      pr->x2 = cube.x2; pr->y2 = cube.y2; pr->z2 = cube.z2;
      pr++;
      pr->x1 = cube.x1; pr->y1 = cube.y1; pr->z1 = cube.z1;
      pr->x2 = cube.x2; pr->y2 = cube.y2; pr->z2 = z;
    }
  }
  DeallocateP(rgcube);
  return fTrue;
}


// Create a section of a 3D fractal Maze. Recursively calls itself to create
// subsections or Mazes within each cell of the Maze at this nesting level.

void CMaz::FractalGenerate3D(CMaz &bT, int w, int xp, int yp, int zp)
{
  int xFractal = ms.xFractal, yFractal = ms.xFractal, zFractal = ms.yFractal,
    xsMax = 1, ysMax = 1, zsMax = 1, x, y, z, i;

  w--;
  for (i = 0; i < w; i++) {
    xsMax *= xFractal; ysMax *= yFractal; zsMax *= zFractal;
  }

  // Create the Maze for this nesting level.
  bT.CreateMaze3D();
  bT.Set1(ms.xEntrance, ms.yEntrance);
  for (z = bT.m_z3 - 2; z >= 0; z--)
    bT.CubeMove(bT, 0, 0, z, bT.m_x3-1, bT.m_y3-1, z, 0, 0, z+1);
  bT.CubeBlock(0, 0, 0, bT.m_x3-1, bT.m_y3-1, 0, fOn);

  // Each Maze contains north & west & top faces (but not south & east &
  // bottom faces) with a random passage upon those faces. Those passages may
  // or may not be visible when this Maze is nested within surrounding Mazes.
  // For the outermost Maze, don't have any passages (they'll be added later).
  if (w+1 < ms.zFractal) {
    bT.Set30(0, (Rnd(1, yFractal) << 1) - 1, (Rnd(1, zFractal) << 1) - 1);
    bT.Set30((Rnd(1, xFractal) << 1) - 1, 0, (Rnd(1, zFractal) << 1) - 1);
    bT.Set30((Rnd(1, xFractal) << 1) - 1, (Rnd(1, yFractal) << 1) - 1, 0);
  }

  // Render the Maze at appropriate zoom level and location.
  BlockMoveMaze3(bT, 0, 0, 0, bT.m_x3-1, bT.m_y3-1, bT.m_z3-1,
    xp << 1, yp << 1, zp << 1, xsMax << 1, ysMax << 1, zsMax << 1);

  // Recursively create a Maze within each cell of this Maze.
  if (w > 0)
    for (z = 0; z < zFractal; z++)
      for (y = 0; y < yFractal; y++)
        for (x = 0; x < xFractal; x++)
          FractalGenerate3D(bT, w,
            xp + x * xsMax, yp + y * ysMax, zp + z * zsMax);
}


// Create a new 3D nested fractal Maze, composed of a Maze with smaller Mazes
// recursively nested within each cell. The fractal Maze's size is defined by
// the Fractal fields in the Create Settings dialog.

flag CMaz::CreateMazeFractal3D()
{
  CMaz bT;
  char sz[cchSzMax];
  int xFractal = ms.xFractal, yFractal = ms.xFractal, zFractal = ms.yFractal,
    xsMax = 1, ysMax = 1, zsMax = 1, i;

  // Ensure the fractal Maze will fit within a bitmap.
  for (i = 0; i < ms.zFractal; i++) {
    if (xsMax * xFractal / xFractal != xsMax ||
      ysMax * yFractal / yFractal != ysMax ||
      zsMax * zFractal / zFractal != zsMax) {
      sprintf(S(sz),
        "%d by %d by %d fractal Maze with %d levels per row "
        "can't have a nesting level greater than %d.",
        xFractal, yFractal, zFractal, m_w3, i); PrintSz_W(sz);
      return fFalse;
    }
    xsMax *= xFractal; ysMax *= yFractal; zsMax *= zFractal;
  }

  // Create the bitmap for the fractal Maze, and a temporary internal bitmap
  // to store the Mazes at each level.
  if (!FCubeSizeSet((xsMax << 1) + 1, (ysMax << 1) + 1, (zsMax << 1),
    m_w3))
    return fFalse;
  if (!bT.FAllocateCube((xFractal << 1) + 1, (yFractal << 1) + 1,
    (zFractal << 1), m_w3))
    return fFalse;
  BitmapOff();
  UpdateDisplay();

  // Create the fractal Maze here.
  i = ms.nEntrancePos; ms.nEntrancePos = epCorner;
  FractalGenerate3D(bT, ms.zFractal, 0, 0, 0);
  ms.nEntrancePos = i;

  // Draw the bottom and east and south boundary walls.
  CubeMove(*this, 0, 0, 1, m_x3-1, m_y3-1, m_z3-1, 0, 0, 0);
  CubeBlock(0, 0, m_z3-1, m_x3-1, m_y3-1, m_z3-1, fOff);
  CubeBlock(xsMax << 1, 0, 0, xsMax << 1, (ysMax << 1) - 1, (zsMax << 1) - 2,
    fOn);
  CubeBlock(0, ysMax << 1, 0, xsMax << 1, ysMax << 1, (zsMax << 1) - 2, fOn);
  MakeEntranceExit(4);
  return fTrue;
}


// Draw one subsection of a 3D virtual fractal Maze using given random number
// seed. Called from FractalPartial3D() to create sections of a fractal Maze.

void CMaz::FractalPartialGenerate3D(CMaz &bT, int z, long x0, long y0,
  long z0, int xp, int yp, int zp, int nRndSeed,
  long *pxEntrance, long *pxExit)
{
  ulong rgl[5];
  long xFractal = ms.xFractal, yFractal = ms.xFractal, zFractal = ms.yFractal,
    xTotal, yTotal, zTotal, xsMax = 1, ysMax = 1, zsMax = 1, xT, yT, zT,
    xEntrance, xExit;
  int xpMax = 1, ypMax = 1, zpMax = 1, xs, ys, zs, i, nSav, nSav2;
  flag fX, fY, fZ;

  for (i = 0; i < z; i++) {
    xpMax *= xFractal; ypMax *= yFractal; zpMax *= zFractal;
  }
  for (i = z; i < ms.zFractal; i++) {
    xsMax *= xFractal; ysMax *= yFractal; zsMax *= zFractal;
  }
  xTotal = xsMax * xpMax; yTotal = ysMax * ypMax; zTotal = zsMax * ypMax;
  InitRndL(nRndSeed);

  // Figure out where the entrance and exit to the fractal Maze should be.
  switch (ms.nEntrancePos) {
  case 0:
    xEntrance = 0;
    xExit     = xTotal - 1;
    break;
  case 1:
    xEntrance = (xTotal - 1) >> 1;
    xExit     = xEntrance + !FOdd(xTotal);
    break;
  case 2:
    xEntrance = Rnd(0, xTotal - 1);
    xExit     = xTotal - 1 - xEntrance;
    break;
  case 3:
    xEntrance = Rnd(0, xTotal - 1);
    xExit     = Rnd(0, xTotal - 1);
    break;
  }
  *pxEntrance = xEntrance; *pxExit = xExit;

  // Leave the section blank if it's outside the bounds of the whole Maze.
  if (x0 >= 0 && x0 < xsMax && y0 >= 0 && y0 < ysMax &&
    z0 >= 0 && z0 < zsMax) {
    if (ms.nFractalT <= vmtNested) {

      // Create fractal section using hunt and kill algorithm
      xT = xsMax; yT = ysMax; zT = zsMax;
      fX = fY = fZ = fFalse;

      // Get information about the Mazes surrounding starting nesting level.
      for (i = z; i < ms.zFractal; i++) {
        bT.CreateMaze3D();
        if (i > z)
          bT.Set0(0, (Rnd(1, yFractal) << 1) - 1);
        else
          bT.Set1(ms.xEntrance, ms.yEntrance);
        xT /= xFractal; yT /= yFractal; zT /= zFractal;
        xs = x0 / xT % xFractal;
        ys = y0 / yT % yFractal;
        zs = z0 / zT % zFractal;

        // The north/west/top faces of the section should be solid if the
        // section is adjacent to any surrounding Maze boundary, and that
        // surrounding Maze has a wall panel at the appropriate position.
        if (y0 % yT == 0)
          fX |= bT.Get3((xs << 1) + 1, ys << 1, zs << 1);
        if (x0 % xT == 0)
          fY |= bT.Get3(xs << 1, (ys << 1) + 1, zs << 1);
        if (z0 % zT == 0)
          fZ |= bT.Get3(xs << 1, ys << 1, (zs << 1) + 1);

        // Set the random number seed for this subsection.
        rgl[0] = nRndSeed; rgl[1] = i;
        rgl[2] = x0 / xT; rgl[3] = y0 / yT; rgl[4] = z0 / zT;
        InitRndRgl(rgl, 5);
      }

      // Create the fractal Maze section here.
      nSav = ms.nEntrancePos; ms.nEntrancePos = epRandom;
      FractalGenerate3D(bT, z, xp, yp, zp);
      ms.nEntrancePos = nSav;
    } else {

      // Create fractal section using binary tree algorithm
      rgl[0] = nRndSeed; rgl[1] = z; rgl[2] = x0; rgl[3] = y0; rgl[4] = z0;
      InitRndRgl(rgl, 5);
      nSav = ms.nEntrancePos; ms.nEntrancePos = epCorner;
      nSav2 = ms.nRndRun; ms.nRndRun = 0;
      bT.CreateMazeBinary3D();
      ms.nEntrancePos = nSav; ms.nRndRun = nSav2;
      bT.Set1(1, 0);
      xT = (x0 > 0) << 1; yT = (y0 > 0) << 1; zT = (z0 > 0) << 1;
      CubeMove(bT, xT, yT, zT, bT.m_x3-4 + xT, bT.m_y3-4 + yT, bT.m_z3-4 + zT,
        xp << 1, yp << 1, zp << 1);
      fX = fY = fZ = fFalse;
    }
  } else {

    // Ensure the boundary planes at the east and south and bottom get drawn.
    fX = y0 == ysMax && FBetween(x0, 0, xsMax-1);
    fY = x0 == xsMax && FBetween(y0, 0, ysMax-1);
    fZ = z0 == zsMax && FBetween(z0, 0, zsMax-1);
  }

  // Draw faces at the top/north/west of the section, if there should be no
  // passage between cells at this nesting level.
  if (fX)
    CubeBlock(xp << 1, yp << 1, zp << 1,
      (xp + xpMax) << 1, yp << 1, (zp + zpMax) << 1, fOn);
  if (fY)
    CubeBlock(xp << 1, yp << 1, zp << 1,
      xp << 1, (yp + ypMax) << 1, (zp + zpMax) << 1, fOn);
  if (fZ)
    CubeBlock(xp << 1, yp << 1, zp << 1,
      (xp + xpMax) << 1, (yp + ypMax) << 1, zp << 1, fOn);

  // Draw the entrance or exit to the whole Maze if visible.
  if (y0 == 0 && x0 == xEntrance / xpMax && z0 == 0)
    Set30(((xp + LMod(xEntrance, xpMax)) << 1) + 1, yp << 1,
      (zp << 1) + (ms.nFractalT == vmtNested));
  if (y0 == ysMax && x0 == xExit / xpMax && z0 == zsMax)
    Set30(((xp + LMod(xExit, xpMax)) << 1) + 1, yp << 1,
      (zp << 1) - 2 + (ms.nFractalT == vmtNested));
}


// Create a grid of subsections of a 3D virtual fractal Maze, where only part
// of the fractal Maze is rendered. Implements the FractalPart3 operation.

flag CMaz::FractalPartial3D(long x0, long y0, long z0, int w, int nRndSeed,
  long *pxEntrance, long *pxExit)
{
  CMaz bT;
  char sz[cchSzMax];
  int xFractal = ms.xFractal, yFractal = ms.xFractal, zFractal = ms.yFractal,
    xpMax = 1, ypMax = 1, zpMax = 1, x, y, z, m1, m2, n1, n2, o1, o2, i, j;
  long xT, yT, zT;

  // Ensure the subsections of the fractal Maze to make will fit in a bitmap.
  for (i = 0; i < w; i++) {
    if ((long)xpMax * xFractal * 4 > xCubeMax ||
      (long)ypMax * yFractal * 4 > yCubeMax ||
      (long)zpMax * zFractal * 4 > zCubeMax) {
      sprintf(S(sz), "%d by %d by %d "
        "fractal section can't have nesting level greater than %d.",
        xFractal, yFractal, zFractal, i); PrintSz_W(sz);
      return fFalse;
    }
    xpMax *= xFractal; ypMax *= yFractal; zpMax *= zFractal;
  }

  // Ensure the total size of the fractal Maze doesn't overflow 32 bits.
  xT = xpMax; yT = ypMax; zT = zpMax;
  for (i = w; i < ms.zFractal; i++) {
    if (xT * xFractal / xFractal != xT ||
      yT * yFractal / yFractal != yT ||
      zT * zFractal / zFractal != zT) {
      sprintf(S(sz), "%d by %d by %d "
        "fractal Maze can't have total nesting greater than %d.",
        xFractal, yFractal, zFractal, i); PrintSz_W(sz);
      return fFalse;
    }
    xT *= xFractal; yT *= yFractal; zT *= zFractal;
  }

  // Create the bitmap for the fractal Maze, and a temporary internal bitmap
  // to store the Mazes in each block.
  if (!FCubeSizeSet(xpMax * 4 + 1, ypMax * 4 + 1, zpMax * 4 - 1, m_w3))
    return fFalse;
  if (ms.nFractalT <= vmtBinary) {
    if (ms.nFractalT <= vmtNested) {
      if (!bT.FAllocateCube((xFractal << 1) + 1, (yFractal << 1) + 1,
        (zFractal << 1), m_w3))
        return fFalse;
    } else {
      if (!bT.FAllocateCube((xpMax << 1) + 3, (ypMax << 1) + 3,
        (zpMax << 1) + 3, m_w3))
        return fFalse;
    }
    BitmapOff();
    UpdateDisplay();

    // Draw some boundary floors without overflowing 32 bits.
    m1 = (x0-1)*xpMax; m2 = m1 + (m_x3 >> 1); 
    n1 = (y0-1)*ypMax; n2 = n1 + (m_y3 >> 1); 
    o1 = (z0-1)*zpMax; o2 = o1 + (m_z3 >> 1); 
    if (FBetween(0, o1, o2)) {
      for (i = -o1*2-1; i <= -o1*2+1; i += 2)
        CubeBlock(0, 0, i + (ms.nFractalT == vmtNested),
          m_x3-1, m_y3-1, i + (ms.nFractalT == vmtNested), fOn);
      if (FBetween(0, n1, n2)) {
        i = i - 2 + (ms.nFractalT == vmtNested);
        j = -n1*2+1;
        CubeBlock(FBetween(0, m1, m2) ? -m1*2+1 : 0, j, i,
          FBetween(xT, m1, m2) ? (xT-m1)*2-1 : m_x3-1, m_y3-1, i, fOff);
      }
    }
    if (FBetween(zT, o1, o2)) {
      i = (zT-o1)*2-3 + (ms.nFractalT == vmtNested);
      CubeBlock(0, 0, i, m_x3-1, m_y3-1, i, fOn);
      if (FBetween(yT, n1, n2)) {
        j = (yT-n1)*2-1;
        CubeBlock(FBetween(0, m1, m2) ? -m1*2+1 : 0, 0, i,
          FBetween(xT, m1, m2) ? (xT-m1)*2-1 : m_x3-1, j, i, fOff);
      }
    }

    // Create a 2x2x2 grid of fractal Maze sections.
    for (z = 0; z < 2; z++)
      for (y = 0; y < 2; y++)
        for (x = 0; x < 2; x++)
          FractalPartialGenerate3D(bT, w, x0 - 1 + x, y0 - 1 + y, z0 - 1 + z,
            x * xpMax, y * ypMax, z * zpMax, nRndSeed, pxEntrance, pxExit);

    // Draw the rest of boundary floors without overflowing 32 bits.
    if (ms.nFractalT == vmtNested) {
      CubeMove(*this, 0, 0, 1, m_x3-1, m_y3-1, m_z3-1, 0, 0, 0);
      if (FBetween(0, o1, o2)) {
        i = -o1*2-1;
        CubeBlock(0, 0, i, m_x3-1, m_y3-1, i, fOn);
      }
    }
    if (FBetween(zT, o1, o2)) {
      i = (zT-o1)*2-1;
      CubeBlock(0, 0, i, m_x3-1, m_y3-1, i, fOn);
    }
  } else if (ms.nFractalT != vmtHilbert) {
    DivisionPartial3D(xT, yT, zT, (x0-1)*xpMax, (y0-1)*ypMax, (z0-1)*zpMax,
      nRndSeed, pxEntrance, pxExit);
  } else {
    LabyrinthHilbertPartial3D(ms.zFractal,
      (x0-1)*xpMax, (y0-1)*ypMax, (z0-1)*zpMax);
    *pxEntrance = 0; *pxExit = (1 << ms.zFractal) - 1;
  }
  return fTrue;
}


#define VX3(x) (((x) < x1 ? -1 : ((x) > x2 ? m_x3 : (x)-x1))*2)
#define VY3(y) (((y) < y1 ? -1 : ((y) > y2 ? m_y3 : (y)-y1))*2)
#define VZ3(z) (((z) < z1 ? -1 : ((z) > z2 ? m_z3 : (z)-z1))*2)

// Create a subsection of a 3D virtual Maze, in which only part of the Maze is
// rendered, using the recursive division algorithm.

flag CMaz::DivisionPartial3D(int xs, int ys, int zs, int x1, int y1, int z1,
  int nRndSeed, long *pxEntrance, long *pxExit)
{
  ulong rgl[7];
  RC3 *rgcube, cube, *pr;
  int x2, y2, z2, xEntrance, xExit, x, y, z, n, m1, n1, o1, m2, n2, o2;

  if (!FEnsureMazeSize(3, femsNoResize))
    return fFalse;
  rgcube = RgAllocate(ms.nCrackPass*4, RC3);
  if (rgcube == NULL)
    return fFalse;

  // Start with one cube in stack covering the entire Maze.
  InitRndL(nRndSeed);
  x2 = x1 + (m_x3 >> 1); y2 = y1 + (m_y3 >> 1); z2 = z1 + (m_z3 >> 1);
  cube.x1 = 0; cube.y1 = 0; cube.z1 = 0;
  cube.x2 = xs; cube.y2 = ys; cube.z2 = zs;
  pr = rgcube;
  *pr = cube;
  BitmapOff();

  // Cube(-x1*2, -y1*2, -z1*2, (xs-x1)*2, (ys-y1)*2, (zs-z1)*2) w/o overflow.
  m1 = VX3(0); m2 = VX3(xs);
  n1 = VY3(0); n2 = VY3(ys);
  o1 = VZ3(0); o2 = VZ3(zs)-2;
  if (FBetween(0, z1, z2)) {
    for (y = o1-1; y <= o1+1; y += 2)
      CubeBlock(0, 0, y, m_x3-1, m_y3-1, y, fOn);
    CubeBlock(m1, n1, o1+1, m2, n2, o1+1, fOff);
  }
  if (FBetween(zs, z1, z2)) {
    for (y = o2+1; y >= o2-1; y -= 2)
      CubeBlock(0, 0, y, m_x3-1, m_y3-1, y, fOn);
    CubeBlock(m1, n1, o2-1, m2, n2, o2-1, fOff);
  }
  if (FBetween(0,  y1, y2)) CubeBlock(m1, n1, o1, m2, n1, o2, fOn);
  if (FBetween(ys, y1, y2)) CubeBlock(m1, n2, o1, m2, n2, o2, fOn);
  if (FBetween(0,  x1, x2)) CubeBlock(m1, n1, o1, m1, n2, o2, fOn);
  if (FBetween(xs, x1, x2)) CubeBlock(m2, n1, o1, m2, n2, o2, fOn);

  // Figure out where the entrance and exit to the Maze should be.
  switch (ms.nEntrancePos) {
  case 0:
    xEntrance = 0;
    xExit     = xs - 1;
    break;
  case 1:
    xEntrance = (xs - 1) >> 1;
    xExit     = xEntrance + !FOdd(xs);
    break;
  case 2:
    xEntrance = Rnd(0, xs - 1);
    xExit     = xs - 1 - xEntrance;
    break;
  case 3:
    xEntrance = Rnd(0, xs - 1);
    xExit     = Rnd(0, xs - 1);
    break;
  }
  *pxEntrance = xEntrance; *pxExit = xExit;
  Set30((xEntrance-x1)*2+1, -y1*2,     -z1*2);
  Set30((xExit    -x1)*2+1, (ys-y1)*2, (zs-z1)*2-2);
  UpdateDisplay();

  // Continue while there's at least one cube on the stack.
  while (pr >= rgcube) {
    cube = *pr;
    x = cube.x2 - cube.x1; y = cube.y2 - cube.y1; z = cube.z2 - cube.z1;
    if ((x < 2) + (y < 2) + (z < 2) >= 2 ||
      cube.x1 > x2 || cube.x2 < x1 || cube.y1 > y2 || cube.y2 < y1 ||
      cube.z1 > z2 || cube.z2 < z1 || pr-rgcube >= ms.nCrackPass*4 - 1) {
      // If cube is too small, or is outside visible viewport, skip it.
      pr--;
      continue;
    }
    if (fCellMax)
      break;

    // Set the random number seed for this subcube.
    rgl[0] = nRndSeed;
    rgl[1] = cube.x1; rgl[2] = cube.y1; rgl[3] = cube.z1;
    rgl[4] = cube.x2; rgl[5] = cube.y2; rgl[6] = cube.z2;
    InitRndRgl(rgl, 7);

    // Determine whether to divide current cube along x or y or z axis.
    if (x < 2)
      n = (Rnd(0, y + z + ms.nRndBias) <= y) + 1;
    else if (y < 2)
      n = (Rnd(0, x + z + ms.nRndBias) <= x) << 1;
    else if (z < 2)
      n = (Rnd(0, x + y) <= x);
    else {
      n = Rnd(0, x + y + z + ms.nRndBias);
      n = (n > x) + (n > x + y);
    }

    // Divide current cube and push two subcubes on stack.
    if (n <= 0) {
      x = Rnd(cube.x1 + 1, cube.x2 - 1);
      if (FBetween(x, x1, x2)) {
        CubeBlock(VX3(x), VY3(cube.y1)+1, VZ3(cube.z1),
          VX3(x), VY3(cube.y2)-1, VZ3(cube.z2)-2, fOn);
        Set30(VX3(x), VY3(Rnd(cube.y1, cube.y2-1))+1,
          VZ3(Rnd(cube.z1, cube.z2-1)));
      }
      pr->x1 = x; pr->y1 = cube.y1; pr->z1 = cube.z1;
      pr->x2 = cube.x2; pr->y2 = cube.y2; pr->z2 = cube.z2;
      pr++;
      pr->x1 = cube.x1; pr->y1 = cube.y1; pr->z1 = cube.z1;
      pr->x2 = x; pr->y2 = cube.y2; pr->z2 = cube.z2;
    } else if (n == 1) {
      y = Rnd(cube.y1 + 1, cube.y2 - 1);
      if (FBetween(y, y1, y2)) {
        CubeBlock(VX3(cube.x1)+1, VY3(y), VZ3(cube.z1),
          VX3(cube.x2)-1, VY3(y), VZ3(cube.z2)-2, fOn);
        Set30(VX3(Rnd(cube.x1, cube.x2-1))+1, VY3(y),
          VZ3(Rnd(cube.z1, cube.z2-1)));
      }
      pr->x1 = cube.x1; pr->y1 = y; pr->z1 = cube.z1;
      pr->x2 = cube.x2; pr->y2 = cube.y2; pr->z2 = cube.z2;
      pr++;
      pr->x1 = cube.x1; pr->y1 = cube.y1; pr->z1 = cube.z1;
      pr->x2 = cube.x2; pr->y2 = y; pr->z2 = cube.z2;
    } else {
      z = Rnd(cube.z1 + 1, cube.z2 - 1);
      if (FBetween(z, z1, z2)) {
        CubeBlock(VX3(cube.x1)+1, VY3(cube.y1)+1, VZ3(z)-1,
          VX3(cube.x2)-1, VY3(cube.y2)-1, VZ3(z)-1, fOn);
        Set30(VX3(Rnd(cube.x1, cube.x2-1))+1,
          VY3(Rnd(cube.y1, cube.y2-1))+1, VZ3(z)-1);
      }
      pr->x1 = cube.x1; pr->y1 = cube.y1; pr->z1 = z;
      pr->x2 = cube.x2; pr->y2 = cube.y2; pr->z2 = cube.z2;
      pr++;
      pr->x1 = cube.x1; pr->y1 = cube.y1; pr->z1 = cube.z1;
      pr->x2 = cube.x2; pr->y2 = cube.y2; pr->z2 = z;
    }
  }
  DeallocateP(rgcube);
  return fTrue;
}


// Replace the 3D Maze in a bitmap with a clarified version of itself. Cells
// are wider and passages between levels are indicated with diagonal lines
// representing up and down staircases. In color bitmaps, up and down
// staircases that connect with each other are given the same color, to make
// manually navigating between levels easier.

flag CMaz::FClarify3D(CMap &bClar, KV kv0, KV kv1) CONST
{
  CMon bT;
  CMap *b = &bClar;
  int tw, tx, ty, tz, x, y, z, m, n, i;
  flag fColor = bClar.FColor();

  tw = Even(m_w3); tx = Even(m_x3); ty = Even(m_y3); tz = Even(m_z3);
  x = (tx * tw / 4 - 1) * CLAR + 1;
  y = (ty * ((tz + tw-1) / tw) / 2 - 1) * CLAR + 1;
  if (!fColor && &bClar == this)
    b = &bT;
  if (!b->FBitmapSizeSet(x, y))
    return fFalse;
  b->BitmapSet(kv0);
  for (z = 0; z < tz-1; z += 2)
    for (y = 0; y < ty-1; y += 2) {
      for (x = 0; x < tx-1; x += 2) {
        m = (z % tw * tx / 4 + x / 2) * CLAR;
        n = (z / tw * ty / 2 + y / 2) * CLAR;
        if (Get3(x+1, y+1, z))
          b->Block(m, n, m+CLAR, n+CLAR, kv1);
        else {
          if (Get3(x+1, y, z))
            b->LineX(m, m+CLAR, n, kv1);
          if (Get3(x, y+1, z))
            b->LineY(m, n, n+CLAR, kv1);

          // Draw colored stairs between levels.
          if (x < tx-2 && y < ty-2) {
            i = ((x ^ y) & 2) >> 1 ^ ((z & 2) >> 1) * 3 ^ 3;
            if (!Get3M(x+1, y+1, z-1))
              b->LineZ(m+3, n+CLAR-3, n+3, 1, rgkvClar[i]);
            i ^= 3;
            if (!Get3M(x+1, y+1, z+1)) {
              b->LineZ(m+CLAR-3, n+CLAR-3, n+3, -1, rgkvClar[i]);
              if (fColor && !Get3M(x+1, y+1, z-1))
                b->Set(m+CLAR/2, n+CLAR/2,
                  KvBlend(rgkvClar[i], rgkvClar[i^3]));
            }
          }
        }
      }
    }
  if (!fColor && &bClar == this)
    bClar.CopyFrom(bT);
  return fTrue;
}


// Return the status of a pixel in a 4D Maze. Daedalus 4D Maze bitmaps don't
// have ceilings above the top levels or floors below the bottom, and don't
// have walls at the extreme ends of the 4th dimension, so assume them here.
// They also don't have solid blocks for levels that are between floors in
// both the 3rd and 4th dimensions (done to make the 4D Maze easier to
// understand in its raw form) so assume that here too.

bit CMaz::Get4M(int w, int x, int y, int z) CONST
{
  if (x < 0 || x >= m_x3 || y < 0 || y >= m_y3 ||
    z < -1 || z > Odd(m_z3) || w < -1 || w > Odd(m_w3))
    return fOff;
  if (z == -1 || z == Odd(m_z3) || w == -1 || w == Odd(m_w3) || (w & z & 1))
    return fOn;
  return Get4(w, x, y, z);
}


// Create a new perfect 4D Maze in the bitmap, stored as a grid of 2D levels.
// The dimensions of the 4D Maze are taken from the Bitmap Size dialog.

flag CMaz::CreateMaze4D()
{
  int tw, tx, ty, tz, w, x, y, z, wnew, xnew, ynew, znew,
    fHunt = fFalse, pass = 0, d, i;
  long count;

  if (!FTesseractSizeSet(m_w3, m_x3, m_y3, m_z3))
    return fFalse;
  BitmapOff();
  if (!FEnsureMazeSize(3, femsNoResize))
    return fFalse;
  tw = Even(m_w3); tx = Even(m_x3); ty = Even(m_y3); tz = Even(m_z3);
  for (z = 0; z < tz-1; z++) {
    y = Y4(0, z);
    for (w = 0; w < tw-1; w++) {
      x = X4(w, 0);
      if ((w & z & 1) == 0)
        Block(x, y, x + tx - 2, y + ty - 2, fOn);
    }
  }
  MakeEntranceExit(5);
  x = y = 1; w = z = 0;
  Set40(w, x, y, z);
  count = (long)(tw >> 1) * ((tx - 1) >> 1) * ((ty - 1) >> 1) * (tz >> 1) - 1;
  UpdateDisplay();
  if (count <= 0)
    goto LDone;

  // Use the Hunt and Kill algorithm to create the 4D Maze.
  loop {
    if (ms.nRndBias > 0 && Rnd(0, ms.nRndBias) > 0)
      i = DIRS1;
    else
      i = DIRS4-1;
    d = Rnd(0, i);
    if (!Get4(w, x, y, z)) {
      for (i = 0; i < DIRS4; i++) {
        wnew = w + (woff3[d] << 1);
        xnew = x + (xoff3[d] << 1);
        ynew = y + (yoff3[d] << 1);
        znew = z + (zoff3[d] << 1);
        if (FLegalMaze4(wnew, xnew, ynew, znew) &&
          Get4(wnew, xnew, ynew, znew))
          break;
        d = (d + 1) & (DIRS4 - 1);
      }
      if (ms.fRiver)
        fHunt = (i >= DIRS4);
      else {
        fHunt = i > 0;
        if (i < DIRS4)
          pass = 0;
      }
    }
    if (!fHunt) {
      if (fCellMax)
        break;
      Set40(
        (w + wnew) >> 1, (x + xnew) >> 1, (y + ynew) >> 1, (z + znew) >> 1);
      Set40(wnew, xnew, ynew, znew);
      w = wnew; x = xnew; y = ynew; z = znew;
      pass = 0;
      count--;
      if (count <= 0)
        goto LDone;
    } else {
      x += 2;
      if (x >= tx-1) {
        x = 1;
        y += 2;
        if (y >= ty-1) {
          y = 1;
          z += 2;
          if (z >= tz) {
            z = 0;
            w += 2;
            if (w >= tw) {
              w = 0;
              pass++;
              if (pass > 1) {
                Assert(fFalse);
                goto LDone;
              }
              UpdateDisplay();
            }
          }
        }
      }
    }
  }
LDone:
  return fTrue;
}


// Replace the 4D Maze in a bitmap with a clarified version of itself. Cells
// are wider and passages between levels and through the 4th dimension are
// indicated with lines in the cell, representing hatches or portals to an
// adjacent floor in the grid of levels. In color bitmaps, colors are used to
// highlight portals between levels, where portals that connect with each
// other are given the same color, to make manually navigating among the
// grid of levels easier.

flag CMaz::FClarify4D(CMap &bClar, KV kv0, KV kv1) CONST
{
  CMon bT;
  CMap *b = &bClar;
  int w, x, y, z, m, n, i;
  flag fColor = bClar.FColor();

  x = (m_w3 / 2 * m_x3 / 2 - 1) * CLAR + 1;
  y = (m_z3 / 2 * m_y3 / 2 - 1) * CLAR + 1;
  if (!fColor && &bClar == this)
    b = &bT;
  if (!b->FBitmapSizeSet(x, y))
    return fFalse;
  b->BitmapSet(kv0);
  for (w = 0; w < m_w3-1; w += 2)
    for (z = 0; z < m_z3-1; z += 2)
      for (y = 0; y < m_y3-1; y += 2) {
        n = (z * m_y3 / 4 + y / 2) * CLAR;
        for (x = 0; x < m_x3-1; x += 2) {
          m = (w * m_x3 / 4 + x / 2) * CLAR;
          if (Get4(w, x+1, y+1, z))
            b->Block(m, n, m+CLAR, n+CLAR, kv1);
          else {
            if (Get4(w, x+1, y, z))
              b->LineX(m, m+CLAR, n, kv1);
            if (Get4(w, x, y+1, z))
              b->LineY(m, n, n+CLAR, kv1);

            // Draw colored portals between levels in the 3rd/4th dimensions.
            if (x < m_x3-2 && y < m_y3-2) {
              i = ((y & 2) | ((x & 2) >> 1)) ^ ((w ^ z) & 2);
              if (!Get4M(w, x+1, y+1, z-1)) {
                b->LineX(m+3, m+CLAR-3, n+2, rgkvClar[i]);
                b->Set(m+CLAR/2, n+3, rgkvClar[i]);
              }
              DirInc(i);
              if (!Get4M(w-1, x+1, y+1, z)) {
                b->LineY(m+2, n+3, n+CLAR-3, rgkvClar[i]);
                b->Set(m+3, n+CLAR/2, rgkvClar[i]);
              }
              DirInc(i);
              if (!Get4M(w, x+1, y+1, z+1)) {
                b->LineX(m+3, m+CLAR-3, n+CLAR-2, rgkvClar[i]);
                b->Set(m+CLAR/2, n+CLAR-3, rgkvClar[i]);
              }
              DirInc(i);
              if (!Get4M(w+1, x+1, y+1, z)) {
                b->LineY(m+CLAR-2, n+3, n+CLAR-3, rgkvClar[i]);
                b->Set(m+CLAR-3, n+CLAR/2, rgkvClar[i]);
              }
            }
          }
        }
      }
  if (!fColor && &bClar == this)
    bClar.CopyFrom(bT);
  return fTrue;
}


// Convert a 4D Maze to a 3D Maze. The 4D Maze will be treated as a list of 3D
// Mazes placed along the X-axis. A copy of the Maze will be put in the given
// color bitmap, with portals through the 4th dimension between 3D bitmaps
// colored appropriately. The given monochrome bitmap will have on pixels at
// places where there are portals. Implements the 4D3D operation.

flag CMaz::FConvert4DTo3D(CMon &b, CCol &c,
  KV kvWall, KV kvMark, KV kvPast, KV kvFuture, KV kvBoth)
{
  CMon b2;
  int w, z;

  if (!FTesseractSizeSet(m_w3, m_x3, m_y3, m_z3))
    return fFalse;
  if (!b.FBitmapSizeSet(m_x, m_y))
    return fFalse;
  if (!c.FBitmapSizeSet(m_x, m_y))
    return fFalse;
  if (!b2.FAllocate(m_x, m_y, this))
    return fFalse;
  b.BitmapOff();
  b2.BitmapOff();
  for (w = 0; w < m_w3; w += 2) {
    for (z = 0; z < m_z3; z += 2) {

      // Copy 4D portals leading to the past to one bitmap.
      if (w > 0) {
        b.BlockMove(*this, X4(w-1, 0), Y4(0, z),
          X4(w-1, 0) + m_x3 - 2, Y4(0, z) + m_y3 - 2,
          X4(w, 0), Y4(0, z));
        b.BlockReverse(X4(w, 0), Y4(0, z),
          X4(w, 0) + m_x3 - 2, Y4(0, z) + m_y3 - 2);
      }

      // Copy 4D portals leading to the future to another bitmap.
      if (w < m_w3-2) {
        b2.BlockMove(*this, X4(w+1, 0), Y4(0, z),
          X4(w+1, 0) + m_x3 - 2, Y4(0, z) + m_y3 - 2,
          X4(w, 0), Y4(0, z));
        b2.BlockReverse(X4(w, 0), Y4(0, z),
          X4(w, 0) + m_x3 - 2, Y4(0, z) + m_y3 - 2);
      }
    }
  }
  m_x3 *= Odd(m_w3); m_z3 = Odd(m_z3); m_w3 = 1;
  b.Copy3(*this);
  c.Copy3(*this);
  c.FColmapBlendFromBitmap(&b, &b2, NULL);
  b.BitmapOr(b2);

  // Color portals through the 4th dimension appropriately.
  if (kvPast != kvRed)
    c.ColmapReplaceSimple(kvRed, kvPast);
  if (kvFuture != kvCyan)
    c.ColmapReplaceSimple(kvCyan, kvFuture);
  if (kvBoth != kvWhite)
    c.ColmapReplaceSimple(kvWhite, kvBoth);

  // Color walls and passages in the Maze appropriately.
  c.ColmapOrAndFromBitmap(*this, kvBlack, kvWhite, 1);
  if (kvWall != kvWhite)
    c.ColmapReplaceSimple(kvWhite, kvWall);
  if (kvMark != kvBlack)
    c.ColmapReplaceSimple(kvBlack, kvMark);

  return fTrue;
}


// Create a new Hypermaze in the bitmap, or a Maze where the solving object is
// a line instead of a point, and the solution a surface instead of a line.
// Like a 3D Maze this is a 3D construct stored as a sequence of 2D levels,
// where the dimensions of the Maze are taken from the Bitmap Size dialog.

flag CMaz::CreateMazeHyper()
{
  PT3 *rgpt;
  int tx, ty, tz, x, y, z, xnew, ynew, znew, d, i;
  long count, cpt = 1, ipt = 0, iptLo, iptHi;

  if (!FCubeSizeSet(m_x3, m_y3, m_z3, m_w3))
    return fFalse;
  if (F64K3())
    return fFalse;
  BitmapOff();
  tx = Even(m_x3); ty = Even(m_y3); tz = Even(m_z3);

  // Start with solid faces at the top and bottom levels.
  CubeBlock(0, 0, 0,    tx-2, ty-2, 0,    fOn);
  CubeBlock(0, 0, tz-2, tx-2, ty-2, tz-2, fOn);
  count = (long)(tx >> 1) * (ty >> 1) * (tz >> 1);
  if (count < 1)
    return fTrue;
  rgpt = RgAllocate(count, PT3);
  if (rgpt == NULL)
    return fFalse;
  for (y = 0; y < ty; y += 2)
    for (x = 0; x < tx; x += 2) {
      PutPt3(ipt, x, y, 0); PutPt3(ipt + 1, x, y, tz-2);
      ipt += 2;
      count -= 2;
    }
  cpt = ipt;
  ipt = Rnd(0, cpt-1);
  GetPt3(ipt, x, y, z);
  UpdateDisplay();

  // Use a 3D version of the Growing Tree algorithm to create the Hypermaze.
  // Grow wall beams into the Maze from both the top and bottom faces.
  loop {
    d = Rnd(0, DIRS3-1);
    for (i = 0; i < DIRS3; i++) {
      xnew = x + (xoff3[d] << 1);
      ynew = y + (yoff3[d] << 1);
      znew = z + (zoff3[d] << 1);
      if (FLegalCube(xnew, ynew, znew) && !Get3(xnew, ynew, znew))
        break;
      d++;
      if (d >= DIRS3)
        d = 0;
    }

    // When extending the Maze into a new cell, add the new cell to the list.
    // When nothing can be created from a cell, remove it from the list.
    if (i < DIRS3) {
      if (fCellMax)
        break;
      x = xnew; y = ynew; z = znew;
      Set31(x - xoff3[d], y - yoff3[d], z - zoff3[d]);
      Set31(x, y, z);
      count--;
      if (count <= 0)
        break;
      PutPt3(cpt, x, y, z);
      cpt++;
    } else {
      cpt--;
      if (cpt < 1) {
        Assert(fFalse);
        break;
      }
      rgpt[ipt] = rgpt[cpt];
    }

    // Figure out which cell to look at next.
    if (ms.fTreeRandom) {
      if (Rnd(0, ms.nTreeRiver) == 0)
        ipt = Rnd(0, cpt-1);
      else
        ipt = cpt-1;
    } else {
      if (ms.nTreeRiver >= 0) {
        iptLo = cpt-1 - ms.nTreeRiver;
        if (iptLo < 0)
          iptLo = 0;
        iptHi = cpt-1;
      } else {
        iptHi = -(ms.nTreeRiver + 1);
        if (iptHi >= cpt)
          iptHi = cpt-1;
        iptLo = 0;
      }
      ipt = Rnd(iptLo, iptHi);
    }
    GetPt3(ipt, x, y, z);
  }
  DeallocateP(rgpt);
  return fTrue;
}


/*
******************************************************************************
** N-Dimensional Maze Routines
******************************************************************************
*/

// Map a point within an n-Dimensional bitmap, whose coordinates are defined
// in an array, to X and Y pixel coordinates within a 2D bitmap. This is the
// internal representation of the ND bitmap, as opposed to how to display it.

void CMaz::MapND(CONST int *pn, int *px, int *py) CONST
{
  int x = 0, y = 0, d, i;

  // X pixel coordinate is based on even numbered dimensions.
  d = 1;
  for (i = 0; i < m_w3; i += 2) {
    x += pn[i] * d;
    d *= i < 2 ? m_x3 : m_z3;
  }

  // Y pixel coordinate is based on odd numbered dimensions.
  d = 1;
  for (i = 1; i < m_w3; i += 2) {
    y += pn[i] * d;
    d *= i < 2 ? m_y3 : m_z3;
  }

  *px = x; *py = y;
}


// Create a new perfect n-Dimensional Maze in the bitmap, stored as a 2D grid
// of (n-2)-Dimensional Maze levels, which in turn contain 2D levels. The
// dimensions of the Maze are taken from the Bitmap Size dialog, where X and Y
// are the size of the 1st and 2nd dimensions, Z is 3rd dimension and up, and
// W is the number of dimensions. Implements the ND operation.

flag CMaz::CreateMazeND()
{
  int n[dimMax], nNew[dimMax], nT[dimMax], nMax[dimMax],
    cDim = m_w3, tx, ty, tz, cDir, x, y, fHunt = fFalse, pass = 0, d, i, j;
  long count;

  x = m_x3 * LPower(m_z3, (cDim - 1) >> 1);
  y = m_y3 * LPower(m_z3, (cDim - 2) >> 1);
  if (cDim < 2 || cDim > dimMax || x < 1 || y < 1) {
    PrintSzNN_W("%d dimensional Maze of size %d is too large.", cDim, m_z3);
    return fFalse;
  }
  if (!FBitmapSizeSet(x, y))
    return fFalse;
  tx = Even(m_x3); ty = Even(m_y3); tz = Even(m_z3);
  cDir = cDim << 1;
  nMax[0] = tx; nMax[1] = ty;
  n[0] = n[1] = 1;
  for (i = 2; i < cDim; i++) {
    nMax[i] = tz;
    n[i] = 0;
  }

  // Fill out the 2D levels that passages can actually carve through.
  BitmapOff();
  loop {
    j = 0;
    for (i = 2; i < cDim; i++)
      if (FOdd(n[i]))
        j++;
    if (j < 2) {
      MapND(n, &x, &y);
      Block(x - 1, y - 1, x + tx - 3, y + ty - 3, fOn);
    }
    for (i = 2; i < cDim; i++) {
      n[i]++;
      if (n[i] <= tz-2)
        break;
      n[i] = 0;
    }
    if (i >= cDim)
      break;
  }

  // Carve the entrance and initial cell.
  MapND(n, &x, &y);
  Set0(x, y);
  n[1]--;
  MapND(n, &x, &y);
  Set0(x, y);
  n[1]++;
  count = (long)((tx - 1) >> 1) * ((ty - 1) >> 1) *
    LPower(tz >> 1, cDim-2) - 1;
  UpdateDisplay();
  if (count <= 0)
    goto LDone;

  // Use the Hunt and Kill algorithm to create the n-Dimensional Maze.
  loop {
    if (ms.nRndBias > 0 && Rnd(0, ms.nRndBias) > 0)
      i = DIRS1;
    else
      i = cDir - 1;
    d = Rnd(0, i);
    MapND(n, &x, &y);
    if (!Get(x, y)) {
      for (i = 0; i < cDir; i++) {
        for (j = 0; j < cDim; j++)
          nNew[j] = n[j];
        j = d >> 1;
        nNew[j] += (FOdd(d) << 2) - 2;
        if (!(nNew[j] < 0 || nNew[j] > nMax[j]-2)) {
          MapND(nNew, &x, &y);
          if (Get(x, y))
            break;
        }
        d = (d + 1) % cDir;
      }
      if (ms.fRiver)
        fHunt = (i >= cDir);
      else {
        fHunt = i > 0;
        if (i < cDir)
          pass = 0;
      }
    }
    if (!fHunt) {
      for (i = 0; i < cDim; i++)
        nT[i] = (n[i] + nNew[i]) >> 1;
      MapND(nT, &x, &y);
      Set0(x, y);
      MapND(nNew, &x, &y);
      Set0(x, y);
      for (i = 0; i < cDim; i++)
        n[i] = nNew[i];
      pass = 0;
      count--;
      if (count <= 0)
        goto LDone;
    } else {
      for (i = 0; i < cDim; i++) {
        n[i] += 2;
        if (n[i] <= nMax[i]-2)
          break;
        n[i] = i < 2;
      }
      if (i >= cDim) {
        pass++;
        if (pass > 1) {
          Assert(fFalse);
          goto LDone;
        }
        UpdateDisplay();
      }
    }
  }
LDone:
  for (i = 0; i < cDim; i++)
    nMax[i] -= 2 + (i < 1);
  MapND(nMax, &x, &y);
  Set0(x, y);
  return fTrue;
}


// Offset the coordinates of a pixel, based on the axis sizes of an
// n-Dimensional cube, so the cube when rendered will be centered around the
// origin, as opposed to the origin always being one of the cube's corners.

void CMaz::CenterND(CONST long *nLine, int *px, int *py) CONST
{
  int x = 0, y = 0, i;

  for (i = 0; i < m_w3; i++) {
    x += nLine[i << 1];
    y += nLine[(i << 1) + 1];
  }
  *px -= x >> 1; *py -= y >> 1;
}


// Compute the pixel coordinate of a point within an n-Dimensional cube, based
// on the horizontal and vertical offsets of each dimensional axis. This is
// for the screen display of the cube, as opposed to internal representation.

void CMaz::MapNDRender(int *px, int *py, CONST int *n,
  CONST int *nMax, CONST long *nLine) CONST
{
  int x = 0, y = 0, cDim = m_w3, i;

  for (i = 0; i < cDim; i++) {
    x += NMultDiv(nLine[i << 1],       n[i], nMax[i]-1);
    y += NMultDiv(nLine[(i << 1) + 1], n[i], nMax[i]-1);
  }
  *px = x; *py = y;
}


// Draw a n-Dimensional Maze on the bitmap. Implements the DrawND operation.

void CMaz::RenderND(CMap &b, int *px, int *py, int *px2, int *py2,
  CONST long *nCoor, CONST long *nLine, KV kv, int grf) CONST
{
  int n[dimMax], n2[dimMax], nMax[dimMax], x0 = *px, y0 = *py,
    cDim = m_w3, tx, ty, tz, x, y, x1, y1, x2, y2, xmax, ymax, i, j, k;
  KV rgkv[dimMax];
  flag fColor = b.FColor(), o;

  if (!fColor)
    o = (kv != kvBlack);
  else {
    for (i = 0; i < cDim; i++)
      rgkv[i] = Hue(i * nHueMax / cDim);
  }
  xmax = b.m_x; ymax = b.m_y;
  tx = (m_x3 - 1) >> 1; ty = (m_y3 - 1) >> 1; tz = (m_z3 + 1) >> 1;
  nMax[0] = tx; nMax[1] = ty;
  for (i = 2; i < cDim; i++)
    nMax[i] = tz;
  for (i = 0; i < cDim; i++)
    n[i] = 0;
  if (grf & 1)
    CenterND(nLine, &x0, &y0);

  loop {
    for (i = 0; i < cDim; i++)
      n2[i] = (i < 2) + (n[i] << 1);
    MapND(n2, &x1, &y1);
    if (!Get(x1, y1)) {
      MapNDRender(&x1, &y1, n, nMax, nLine);
      x1 += x0; y1 += y0;
      if (!fColor && (grf & 2) > 0 &&
        x1 > -1 && x1 <= xmax && y1 > -1 && y1 <= ymax)
        b.Block(x1-1, y1-1, x1+1, y1+1, o);

      // For each cell, draw a passage if present to cells adjacent to it.
      for (i = 0; i < cDim; i++)
        if (n[i] < nMax[i]-1) {
          n[i]++;
          for (j = 0; j < cDim; j++)
            n2[j] = (j < 2) + (n[j] << 1) - (j == i);
          MapND(n2, &x, &y);
          if (!Get(x, y)) {
            MapNDRender(&x2, &y2, n, nMax, nLine);
            x2 += x0; y2 += y0;
            if (!fColor)
              b.Line(x1, y1, x2, y2, o);
            else {
              if ((grf & 4) == 0)
                k = i;
              else {
                k = (n[i] <= nCoor[i]) - 1;
                for (j = 0; j < cDim; j++)
                  k += NAbs(n[j] - nCoor[j]);
                if (k >= cDim)
                  k = cDim - 1;
              }
              b.Line(x1, y1, x2, y2, rgkv[k]);
            }
          }
          n[i]--;
        }
    }

    // Advance to the next cell in the Maze.
    for (i = 0; i < cDim; i++) {
      n[i]++;
      if (n[i] < nMax[i])
        break;
      n[i] = 0;
    }
    if (i >= cDim)
      break;
  }

  // Draw the vertex points in a color Maze afterward, so they appear on top.
  if (fColor && (grf & 2) > 0)
    loop {
      for (i = 0; i < cDim; i++)
        n2[i] = (i < 2) + (n[i] << 1);
      MapND(n2, &x1, &y1);
      if (!Get(x1, y1)) {
        MapNDRender(&x1, &y1, n, nMax, nLine);
        x1 += x0; y1 += y0;
        if (x1 > -1 && x1 <= xmax && y1 > -1 && y1 <= ymax)
          b.Block(x1-1, y1-1, x1+1, y1+1, kv);
      }

      // Advance to the next cell in the Maze.
      for (i = 0; i < cDim; i++) {
        n[i]++;
        if (n[i] < nMax[i])
          break;
        n[i] = 0;
      }
      if (i >= cDim)
        break;
    }

  // Set x and y to screen coordinates of the current coordinate in the Maze.
  for (i = 0; i < cDim; i++)
    n[i] = nCoor[i];
  MapNDRender(px, py, n, nMax, nLine);
  *px += x0; *py += y0;

  // Set x2 and y2 to the screen coordinates of the finish cell of the Maze.
  for (i = 0; i < cDim; i++)
    n[i] = nMax[i] - 1;
  MapNDRender(px2, py2, n, nMax, nLine);
  *px2 += x0; *py2 += y0;
}


// Move a ND point to an adjacent point, based on screen coordinates where the
// user clicked. Go to the adjacent point in the ND cube closest to that
// clicked coordinate (or do nothing if the closest ND point is the current
// point). Return whether the point moved to the far corner, which is
// considered to be the solution cell. Implements the MoveND operation.

flag CMaz::MoveND(long *nCoor, CONST long *nLine, int *px, int *py,
  flag fCenter) CONST
{
  int nMax[dimMax], n[dimMax], np[dimMax], cDim = m_w3, tx, ty, tz,
    x0, y0, xMouse = *px, yMouse = *py, iNear = -1, i, ip, j, f;
  long lDist, lDistMin, x, y;

  tx = (m_x3 - 1) >> 1; ty = (m_y3 - 1) >> 1; tz = (m_z3 + 1) >> 1;
  nMax[0] = tx; nMax[1] = ty;
  for (i = 2; i < cDim; i++)
    nMax[i] = tz;
  for (i = 0; i < cDim; i++)
    n[i] = nCoor[i];
  MapNDRender(&x0, &y0, n, nMax, nLine);
  if (fCenter)
    CenterND(nLine, &x0, &y0);
  xMouse -= x0; yMouse -= y0;

  lDistMin = Sq(xMouse) + Sq(yMouse);
  for (i = 0; i < cDim << 1; i++) {
    j = i >> 1;
    f = FOdd(i) ? n[j] < nMax[j]-1 : n[j] > 0;
    if (f) {
      for (ip = 0; ip < cDim; ip++)
        np[ip] = (ip < 2) + (n[ip] << 1);
      np[j] += (FOdd(i) << 1) - 1;
      MapND(np, &x0, &y0);
      if (Get(x0, y0))
        f = fFalse;
    }
    if (f) {
      x = nLine[j << 1]       / (nMax[j] - 1);
      y = nLine[(j << 1) + 1] / (nMax[j] - 1);
      if (!FOdd(i)) {
        neg(x); neg(y);
      }
      lDist = Sq(xMouse - x) + Sq(yMouse - y);
      if (lDist < lDistMin) {
        lDistMin = lDist;
        iNear = i;
      }
    }
  }

  if (iNear >= 0) {
    n[iNear >> 1] += (FOdd(iNear) << 1) - 1;
  }
  f = fTrue;
  for (i = 0; i < cDim; i++) {
    nCoor[i] = n[i];
    if (n[i] < nMax[i] - 1)
      f = fFalse;
  }
  MapNDRender(px, py, n, nMax, nLine);
  if (fCenter)
    CenterND(nLine, px, py);
  return f;
}


/*
******************************************************************************
** Planair Maze Routines
******************************************************************************
*/

#define PZTo(sz, z, d) (int)(sz[(z)*9 + (d)*2]-'a')
#define PDir(sz, z, d) (int)(sz[(z)*9 + (d)*2 + 1]-'0')
#define PSpc(sz, z) sz[(z)*9 + 8]

#define JX(x, z) (((z) % jw) * m_x3 + (x))
#define JY(y, z) (((z) / jw) * m_y3 + (y))
#define JG(x, y, z) Get(JX(x, z), JY(y, z))
#define J0(x, y, z) Set0(JX(x, z), JY(y, z))

// Check whether a planair Maze definition string is formatted correctly.

flag CMaz::FValidPlanair(CONST char *sz) CONST
{
  char szT[cchSzMax], sz1[cchSzDef], sz2[cchSzDef], sz3[cchSzDef];
  int cch, jz, i, j, z, d, z1, z2;

  // Ensure the length of the string implies an integer number of sections.
  cch = CchSz(sz);
  if ((cch+1) % 9 > 0) {
    sprintf(S(sz1), "The Planair string length is invalid.\n");
    sprintf(S(sz2), "The length %d implies a section count of %.1f.\n",
      cch, (real)(cch + 1)/9.0);
    sprintf(S(sz3),
      "The length needs to be one less than a multiple of 9.\n");
    sprintf(S(szT), "%s%s%s", sz1, sz2, sz3);
    PrintSz_W(szT);
    return fFalse;
  }

  // Check each section for validity.
  jz = (cch+1) / 9;
  for (i = 0; i < jz; i++) {
    for (j = 0; j < DIRS; j++) {
      z = PZTo(sz, i, j);
      if (!FBetween(z, 0, jz-1)) {
        sprintf(S(sz1),
          "The Planair string has an invalid group destination.\n");
        sprintf(S(sz2), "Edge %d of group '%c' points to group '%c'.\n",
          j, (char)(i+'a'), (char)(z+'a'));
        sprintf(S(sz3),
          "This destination group should range from 'a' to '%c'.\n",
          (char)(jz-1+'a'));
        sprintf(S(szT), "%s%s%s", sz1, sz2, sz3);
        PrintSz_W(szT);
        return fFalse;
      }
      d = PDir(sz, i, j);
      if (!FBetween(d, 0, 7)) {
        sprintf(S(sz1),
          "The Planair string has an invalid edge destination.\n");
        sprintf(S(sz2), "Edge %d of group '%c' points to edge %d.\n",
          j, (char)(i+'a'), d);
        sprintf(S(sz3), "This destination edge should range from 0 to 7.\n");
        sprintf(S(szT), "%s%s%s", sz1, sz2, sz3);
        PrintSz_W(szT);
        return fFalse;
      }
      z1 = ((FOdd(j) ? m_y3 : m_x3) - 1) >> 1;
      z2 = ((FOdd(d) ? m_y3 : m_x3) - 1) >> 1;
      if (z1 != z2) {
        sprintf(S(sz1), "The Planair string has an unaligned edge link.\n");
        sprintf(S(sz2),
          "Edge %d of group '%c' points to edge %d of group '%c'.\n",
          j, (char)(i+'a'), d, (char)(z+'a'));
        sprintf(S(sz3), "These edges are of different lengths (%d and %d).\n",
          z1, z2);
        sprintf(S(szT), "%s%s%s", sz1, sz2, sz3);
        PrintSz_W(szT);
        return fFalse;
      }
    }
    if (i < jz-1 && PSpc(sz, i) != ' ') {
      sprintf(S(sz1), "The Planair string has an invalid separator.\n");
      sprintf(S(sz2), "The character after group '%c' should be a space.\n",
        (char)(i+'a'));
      sprintf(S(szT), "%s%s", sz1, sz2);
      PrintSz_W(szT);
      return fFalse;
    }
  }

  // Check for asymmetric edge connections. If one edge connects to another,
  // the other edge should connect up with the first one to avoid one way
  // passages. It's not required, but at least display a warning if so.
  for (i = 0; i < jz; i++)
    for (j = 0; j < DIRS; j++) {
      z = PZTo(sz, i, j); d = PDir(sz, i, j);
      z1 = PZTo(sz, z, d & DIRS1); z2 = PDir(sz, z, d & DIRS1);
      if (z1 != i) {
        sprintf(S(sz1), "The Planair string has a one way group link.\n");
        sprintf(S(sz2),
          "Edge %d of group '%c' points to edge %d of group '%c',\n",
          j, (char)(i+'a'), d, (char)(z+'a'));
        sprintf(S(sz3),
          "However edge %d of group '%c' points to group '%c'.\n",
          d, (char)(z+'a'), (char)(z1+'a'));
        sprintf(S(szT), "%s%s%s", sz1, sz2, sz3);
        PrintSz(szT);
        return fTrue;
      }
      if ((z2 & DIRS1) != j || (d & DIRS) != (z2 & DIRS)) {
        sprintf(S(sz1), "The Planair string has a one way edge link.\n");
        sprintf(S(sz2),
          "Edge %d of group '%c' points to edge %d of group '%c',\n",
          j, (char)(i+'a'), d, (char)(z+'a'));
        sprintf(S(sz3),
          "However edge %d of group '%c' points to edge %d.\n",
          d, (char)(z+'a'), z2);
        sprintf(S(szT), "%s%s%s", sz1, sz2, sz3);
        PrintSz(szT);
        return fTrue;
      }
    }
  return fTrue;
}


// Create a new planair Maze in a bitmap, or a Maze on the surface of a set of
// rectangles, whose edges can connect in arbitrary ways. Like a 3D Maze this
// is technically a 3D construct stored as a sequence of levels, where the
// horizontal and vertical dimensions of each rectangle are taken from the
// 3D settings in the Bitmap Size dialog.

flag CMaz::CreateMazePlanair()
{
  int jx = Even(m_x3), jy = Even(m_y3), jz, jw = m_w3,
    fHunt = fFalse, pass = 0, x, y, z, xnew, ynew, znew,
    xInc = 2, yInc = 2, zInc = 1, d, e, i, iMax, j;
  long count;

  if (!FValidPlanair(ms.szPlanair))
    return fFalse;
  jz = (CchSz(ms.szPlanair) + 1) / 9;
  if (!FCubeSizeSet(jx, jy, jz, jw))
    return fFalse;
  BitmapOff();
  if (!FEnsureMazeSize(3, femsNoResize))
    return fFalse;
  CubeBlock(0, 0, 0, jx-2, jy-2, jz-1, fOn);
  count = (long)((jx - 1) >> 1) * ((jy - 1) >> 1) * jz - 1;
  iMax = ms.fRiver ? DIRS : 1;
  x = y = 1; z = 0;
  J0(x, y, z);
  UpdateDisplay();
  if (count <= 0)
    goto LDone;

  // Use the Hunt and Kill algorithm to create the planair Maze.
  loop {
    if (!JG(x, y, z)) {
LNext:
      d = RndDir();
      for (i = fHunt ? DIRS : iMax; i > 0; i--) {
        xnew = x + xoff2[d];
        ynew = y + yoff2[d];
        if (xnew <= 0 || xnew >= jx-1 || ynew <= 0 || ynew >= jy-1) {

          // If checking a cell off the edge of a rectangle, figure out which
          // new rectangle and which edge cell to flow onto.
          znew = PZTo(ms.szPlanair, z, d);
          e = PDir(ms.szPlanair, z, d);
          j = xnew <= 0 || xnew >= jx-1 ? ynew : xnew;
          switch (e) {
          case 0: xnew = j;    ynew = 1;    break;
          case 1: xnew = 1;    ynew = j;    break;
          case 2: xnew = j;    ynew = jy-3; break;
          case 3: xnew = jx-3; ynew = j;    break;
          case 4: xnew = jx-2-j; ynew = 1;      break;
          case 5: xnew = 1;      ynew = jy-2-j; break;
          case 6: xnew = jx-2-j; ynew = jy-3;   break;
          case 7: xnew = jx-3;   ynew = jy-2-j; break;
          }
          e &= DIRS1;
        } else {

          // For movement within a rectangle, the rectangle of course doesn't
          // change, and the undo direction is simply the reverse.
          znew = z;
          e = d ^ 2;
        }
        if (JG(xnew, ynew, znew)) {
          if (fCellMax)
            break;
          J0(x+xoff[d], y+yoff[d], z);
          J0(xnew+xoff[e], ynew+yoff[e], znew);
          J0(xnew, ynew, znew);
          x = xnew; y = ynew; z = znew;
          pass = 0;
          fHunt = fFalse;
          count--;
          if (count <= 0)
            goto LDone;
          goto LNext;
        }
        DirInc(d);
      }
      fHunt = fTrue;
    }
    if (x + xInc > 0 && x + xInc < jx-1)
      x += xInc;
    else {
      neg(xInc);
      if (y + yInc > 0 && y + yInc < jy-1)
        y += yInc;
      else {
        neg(yInc);
        if (z + zInc >= 0 && z + zInc < jz)
          z += zInc;
        else {
          neg(zInc);
          pass++;
          if (pass > 1) {
            Assert(fFalse);
            goto LDone;
          }
          UpdateDisplay();
        }
      }
    }
  }
LDone:
  return fTrue;
}


/*
******************************************************************************
** Segment Maze Routines
******************************************************************************
*/

#define SList 16
#define SGroup 9

enum _segmode {
  segRiver = 1, // Op #1: River percentage.
  segBias  = 2, // Op #2: Random bias.
  segRun   = 3, // Op #3: Random run.
};

int segment[SList*SGroup] = {
  segRiver,   0,  0, 255,  95,   0,   0,   0,   0,
  segRiver,   0, 96, 255, 191, 100, 100, 100, 100,
  segBias,    0,  0, 127, 191, -20, -20, -20, -20,
  segBias,  128,  0, 255, 191,  20,  20,  20,  20,
  segRun,    64, 48, 191, 144,   4,   4,   4,   4,
  0};

#define SOp(i) segment[(i)*SGroup]
#define SX1(i) segment[(i)*SGroup + 1]
#define SY1(i) segment[(i)*SGroup + 2]
#define SX2(i) segment[(i)*SGroup + 3]
#define SY2(i) segment[(i)*SGroup + 4]
#define SVal1(i) segment[(i)*SGroup + 5]
#define SVal2(i) segment[(i)*SGroup + 6]
#define SVal3(i) segment[(i)*SGroup + 7]
#define SVal4(i) segment[(i)*SGroup + 8]

// Convert the segment Maze definition array to a list of numbers in a string.
// Called from the Create Settings dialog to show the current definition.

void SegmentGetSz(char *sz, int cchMax)
{
  int i, j;
  char *pch = sz;

  for (i = 0; i < SList && SOp(i); i++) {
    for (j = 0; j < SGroup; j++) {
      sprintf(SS(pch, cchMax - (pch - sz)), "%d%s",
        segment[i*SGroup + j], j < 8 ? " " : "  ");
      while (*pch)
        pch++;
    }
  }
  sprintf(SS(pch, cchMax - (pch - sz)), "0");
}


// Given a list of numbers in a string, fill the segment Maze definition array
// with them. The reverse of SegmentGetSz().

void SegmentParseSz(CONST char *sz)
{
  int i, j;
  CONST char *pch = sz;
  for (i = 0; i < SList*SGroup && *pch; i++) {
    SkipSpaces(pch);
    sscanf(pch, "%d", &j); segment[i] = j;
    SkipToSpace(pch);
  }
}


// Given a coordinate and an operation, return the first segment rectangle for
// that operation that overlaps the coordinate, if any.

int SegLookup(int x, int y, int op)
{
  int i;

  for (i = 0; i < SList && SOp(i) != 0; i++) {
    if (NAbs(SOp(i)) == op &&
      x >= SX1(i) && x <= SX2(i) && y >= SY1(i) && y <= SY2(i))
      return i;
  }
  return -1;
}


// Return the correct operation value for a coordinate that lies within a
// segment rectangle. A rectangle may have different values blend across it.

int SegValBlend(int x, int y, int i)
{
  int s1, s2, s3, s4, x1, x2;
  real r;

  // Simple case: The value is uniform across the rectangle.
  s1 = SVal1(i); s2 = SVal2(i); s3 = SVal3(i); s4 = SVal4(i);
  if (s1 == s2 && s2 == s3 && s3 == s4)
    return s1;

  // Complex case: The value has a gradient between the four corners.
  r = ((real)(y - SY1(i)) / (real)(SY2(i) - SY1(i)));
  x1 = s1 + (int)(r * (real)(s3 - s1));
  x2 = s2 + (int)(r * (real)(s4 - s2));
  r = ((real)(x - SX1(i)) / (real)(SX2(i) - SX1(i)));
  return x1 + (int)(r * (real)(x2 - x1));
}


// Return a random one of the four directions. A more advanced version of
// RndDir() that gets the random bias and run from any segment rectangles that
// overlap the coordinates.

int SegRndDir(int x, int y)
{
  int i, n;

  // If in the middle of a random run, return the previous direction again.
  if (ms.cRunRndSeg > 0) {
    ms.cRunRndSeg--;
    return ms.dirRndSeg;
  }

  // Randomly pick a new direction, and a new random run length if any.
  i = SegLookup(x, y, segRun);
  if (i >= 0)
    ms.cRunRndSeg = SegValBlend(x, y, i);
  else
    ms.cRunRndSeg = ms.nRndRun;
  i = SegLookup(x, y, segBias);
  if (i >= 0)
    n = SegValBlend(x, y, i);
  else
    n = ms.nRndBias;
  ms.dirRndSeg = Rnd(0, DIRS1 + NAbs(n)*2);

  // Return a standard direction.
  if (ms.dirRndSeg < DIRS)
    return ms.dirRndSeg;

  // Higher random numbers represent bias. Map them to a standard direction.
  ms.dirRndSeg = ((ms.dirRndSeg & 1) << 1) + (n > 0);
  return ms.dirRndSeg;
}


// Return whether to check all directions (as opposed to just the first)
// before entering hunting mode while creating the Maze. This is a random
// percentage taken from a segment rectangle that overlaps the coordinates.
// Like a version of the Create With River setting that changes dynamically.

flag SegRiver(int x, int y)
{
  int i, n;

  i = SegLookup(x, y, segRiver);
  if (i > -1) {
    n = SegValBlend(x, y, i);
    return Rnd(0, 99) < n;
  }
  return ms.fRiver;
}


// Create a new segment Maze in the bitmap, or a standard Maze that can have
// different textures defined for different rectangular sections in it.

flag CMaz::CreateMazeSegment()
{
  int x, y, xnew, ynew,
    xInc = 2, yInc = 2, fHunt = fFalse, pass = 0, d, i;
  long count;

  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize | femsMinSize))
    return fFalse;
  ms.cRunRndSeg = 0;
  MazeClear(fOn);
  count = (long)((xh - xl) >> 1) * ((yh - yl) >> 1) - 1;
  x = xh - 1; y = yh - 1;
  Set0(x, y);
  MakeEntranceExit(0);
  UpdateDisplay();

  // Use the Hunt and Kill algorithm to create the segmented Maze.
  loop {
    if (!Get(x, y)) {
      d = SegRndDir(x, y);
      for (i = 0; i < DIRS; i++) {
        xnew = x + xoff2[d];
        ynew = y + yoff2[d];
        if (FLegalMaze2(xnew, ynew) && Get(xnew, ynew))
          break;
        DirInc(d);
      }
      if (SegRiver(xnew, ynew))
        fHunt = (i >= DIRS);
      else {
        fHunt = i > 0;
        if (i < DIRS)
          pass = 0;
      }
    }
    if (!fHunt) {
      if (fCellMax)
        break;
      Set0((x + xnew) >> 1, (y + ynew) >> 1);
      Set0(xnew, ynew);
      x = xnew; y = ynew;
      pass = 0;
      count--;
      if (count <= 0)
        goto LDone;
    } else {
      if (x + xInc >= xl && x + xInc <= xh)
        x += xInc;
      else {
        neg(xInc);
        if (y + yInc >= yl && y + yInc <= yh)
          y += yInc;
        else {
          neg(yInc);
          pass++;
          if (pass > 1)
            goto LDone;
          UpdateDisplay();
        }
      }
    }
  }
LDone:
  return fTrue;
}


/*
******************************************************************************
** Infinite Length Maze Routines
******************************************************************************
*/

enum _infmode {
  infStart    = 0,
  infContinue = 1,
  infEnd      = 2,
  infRestart  = 3,
};

// Create or open a file in preparation of appending infinite Maze sections to
// it. Used when the Save Infinite To File setting is on.

FILE *FileOpenInfinite(flag fAppend)
{
  char sz[cchSzDef], szExt[cchSzDef], szMode[cchSzDef];
  FILE *file;

  if (ms.nInfFile != 3)
    sprintf(S(szExt), "txt");
  else
    sprintf(S(szExt), "bmp");
  szMode[0] = fAppend ? 'a' : 'w';
  szMode[1] = fAppend || ms.nInfFile != 3 ? chNull : 'b';
  szMode[2] = chNull;

  // If writing to multiple files, include numbers in the filename.
  if (ms.nInfFile == 3 || ms.nInfFileCutoff > 0)
    sprintf(S(sz), "inf%05d.%s",
      ms.nInfGen / (ms.nInfFile != 3 ? ms.nInfFileCutoff : 1), szExt);
  else
    sprintf(S(sz), "infinite.%s", szExt);
  file = FileOpen(sz, szMode);
  return file;
}


// Create an infinite Maze, or a Maze that has a finite width and no limit to
// how long it can be. The Maze is created in sections, which scroll down the
// bitmap, where two sections are visible in the bitmap at any time. This can
// start a new infinite Maze in the bitmap, add onto the existing infinite
// Maze by scrolling up and adding a new Maze section, or finish the existing
// infinite Maze in the bitmap.

flag CMaz::CreateMazeInfinite(int phase)
{
  int x, y, xnew, ynew, yhalf, xInc = 2, yInc = 2, fHunt, pass = 0, d,
    xs, xinc1, xinc2, yp;
  long count, count2;
  CMaz b2;

  x = m_x;
  if (x < 3)
    x = 3;
  y = m_y;
  if (y < 7)
    y = 7;
  else
    y -= 2*((y + 1 & 2) > 0);
  if (!FBitmapSizeSet(x, y))
    return fFalse;
  SetXyh();

  // Automatically start a new infinite Maze if not in the middle of one.
  // Automatically end the current infinite Maze if the number of sections
  // created so far reaches the Generation Cutoff setting.
  if (!ms.fInfMiddle) {
    if (phase != infStart && ms.fInfAutoStart)
      return fFalse;
    phase = infStart;
  } else {
    if (ms.nInfCutoff > 0 && ms.nInfGen >= ms.nInfCutoff - 1)
      phase = infEnd;
  }

  if (ms.fInfEller) {
    xs = (xh - xl) >> 1;
    x = xs-1;              xinc1 = LPrime(x);
    x = xs-2*ms.fTreeWall; xinc2 = LPrime(x);

    // When using Eller's algorithm, initialize the connection information for
    // the current row, and put each cell in the row in its own set. Do this
    // when starting, restarting (for lack of anything better to do although
    // it will cause loops) and if the user resizes the bitmap.

    if (ms.cnInfEller != xs || phase == infStart || phase == infRestart) {
      if (ms.cnInfEller != xs) {
        if (ms.rgnInfEller != NULL)
          DeallocateP(ms.rgnInfEller);
        ms.rgnInfEller = RgAllocate(xs*2, long);
        if (ms.rgnInfEller == NULL)
          return fFalse;
        ms.cnInfEller = xs;
      }
      for (x = 0; x < xs; x++)
        ms.rgnInfEller[x] = ms.rgnInfEller[xs + x] = x;
    }
  }

  // Restarting an infinite Maze is the same as continuing it, except the
  // number of cells created so far in the bottom section is recomputed.
  if (phase == infRestart) {
    if (!ms.fInfEller) {
      ms.fInfMiddle = fTrue;
      ms.nInfCount = 0L;
      for (y = (yh >> 1) + 2; y < yh; y += 2)
        for (x = 1; x < xh; x += 2)
          ms.nInfCount += !Get(x, y);
    }
    phase = infContinue;
  }

  x = y = 1;
  if (phase == infStart) {

    // Get ready to start a new infinite Maze.
    if (ms.fileInf != NULL) {
      fclose(ms.fileInf);
      ms.fileInf = NULL;
    }
    ms.nInfGen = ms.nInfCount = 0L;
    if (ms.nInfFile) {
      ms.fileInf = FileOpenInfinite(fFalse);
      if (ms.fileInf == NULL)
        PrintSz_E("The file could not be created.");
    }
    MazeClear(fOn);
    if (FOdd(xh))
      LineY(xh, yl, yh, fOff);
    if (FOdd(yh))
      LineX(xl, xh, yh, fOff);
    MakeEntranceExit(6);
    if (!ms.fInfEller) {
      Set0(x, y);
      fHunt = fFalse;
    } else
      yp = 1;
  } else {

    // Get ready to continue or finish an existing infinite Maze.
    if (ms.nInfFile) {
      if (ms.nInfFile == 3 ||
        (ms.nInfFileCutoff > 0 && ms.nInfGen % ms.nInfFileCutoff == 0)) {
        if (ms.fileInf != NULL)
          fclose(ms.fileInf);
        ms.fileInf = FileOpenInfinite(fFalse);
        if (ms.fileInf == NULL)
          PrintSz_E("The next file could not be created.");
      } else if (ms.fileInf == NULL) {
        ms.fileInf = FileOpenInfinite(fTrue);
        if (ms.fileInf == NULL)
          PrintSz_E("The file could not be reopened.");
      }
    }

    // Copy the bottom section to the top, and blank out a new bottom section.
    BlockMove(*this, xl, (yh >> 1) + 1, xh, yh & ~1, xl, yl);
    Block(xl, yh >> 1, xh, yh & ~1, fOn);
    if (FOdd(xh))
      LineY(xh, yh >> 1, yh & ~1, fOff);
    if (!ms.fInfEller)
      fHunt = fTrue;
    else
      yp = yh >> 1;
  }

  UpdateDisplay();
  if (phase != infEnd) {
    yhalf = yh >> 1;
    count2 = 0;
  } else {
    MakeEntranceExit(7);
    yhalf = yh;
    count2 = 1;
  }

  // Figure out how many cells need to be carved to finish the section(s).
  if (phase == infStart)
    count = (long)(xh >> 1) * ((yhalf + 1) >> 1) - 1;
  else if (phase == infContinue)
    count = (long)(xh >> 1) * ((yhalf + 1) >> 1) - ms.nInfCount;
  else
    count = (long)(xh >> 1) * (yh >> 1) - ms.nInfCount;

  // If the infinite Maze is being created with Eller's algorithm, just create
  // rows one by one to completely fill the new section.
  if (ms.fInfEller) {
    while (yp <= (yh | 1)-2) {
      EllerMakeRow(ms.rgnInfEller, ms.rgnInfEller + xs, xs, xinc1, xinc2,
        yp, 1, phase == infEnd && yp >= (yh | 1)-2);
      yp += 2;
    }
    goto LAfterMaze;
  }

  // Use a version of the Hunt and Kill algorithm to create the next section
  // of the infinite Maze. This stops when the top half is done, leaving the
  // bottom half partially created to continue with in the next iteration.
  loop {
    if (!Get(x, y)) {
      if (ms.fRiver || fHunt) {
        d = DirFindUncreated(&x, &y, fFalse);
        fHunt = d < 0;
      } else {
        d = RndDir();
        xnew = x + xoff2[d];
        ynew = y + yoff2[d];
        if (FLegalMaze2(xnew, ynew) && Get(xnew, ynew)) {
          x = xnew; y = ynew;
        } else
          fHunt = fTrue;
      }
    }
    if (!fHunt) {
      Set0(x-xoff[d], y-yoff[d]);
      Set0(x, y);
      pass = 0;
      if (y <= yhalf) {
        count--;

        // Done when all cells in the top section have been carved, and at
        // least one cell in the bottom section has been carved.
        if (count <= 0 && count2 > 0)
          break;
      } else {
        count2++;

        // If carving reaches bottom row of the bottom section, teleport to
        // top row of the top section, to give bias to finishing top section.
        if (y >= yh-1) {
          y = 1;
          fHunt = fTrue;
        }
      }
    } else {
      if (x + xInc > xl && x + xInc < xh)
        x += xInc;
      else {
        neg(xInc);
        if (y >= yhalf) {
          pass++;
          if (pass > 2)
            break;

          // If hunting reaches an edge in the bottom section, teleport to the
          // top row of top section, to give bias to finishing top section.
          y = 1;
          UpdateDisplay();
        } else if (y + yInc > yl && y + yInc < yh) {
          y += yInc;
        } else {
          neg(yInc);
          pass++;
          if (pass > 2)
            break;
          UpdateDisplay();
        }
      }
    }
  }

LAfterMaze:
  ms.nInfGen++;
  ms.fInfMiddle = phase != infEnd;
  ms.nInfCount = count2;
  if (ms.nInfFile && ms.fileInf != NULL) {

    // Write top section (or both sections if done with the Maze) to file.
    if (phase != infEnd)
      y = (m_y + 1) >> 1;
    else
      y = Odd(m_y);
    if (b2.FAllocate(m_x, y, this)) {
      b2.BlockMove(*this, 0, 0, m_x-1, y-1, 0, 0);
      if (ms.nInfFile == 3)
        b2.WriteBitmap(ms.fileInf, kvBlack, kvWhite);
      else if (ms.nInfFile == 2)
        b2.WriteTextDOS(ms.fileInf, 0, phase == infEnd ? 1 : 2);
      else
        b2.WriteText(ms.fileInf, fTrue, fFalse, fFalse);
    }

    // Close the file or the current file when done writing to it.
    if (phase == infEnd || ms.nInfFile == 3 ||
      (ms.nInfFileCutoff > 0 && ms.nInfGen % ms.nInfFileCutoff == 0)) {
      fclose(ms.fileInf);
      ms.fileInf = NULL;
    } else
      fflush(ms.fileInf);
  }
  return fTrue;
}


/*
******************************************************************************
** Virtual Standard Maze Routines
******************************************************************************
*/

// Create part of a Maze within a bitmap containing one section of a larger
// Maze, stopping when one of the four edges of the section is reached, using
// a version of the Hunt and Kill algorithm.

flag CMaz::PerfectPartial(int grfEdge, int xs, int ys,
  int *xe, int *ye, int *pcell)
{
  static int xInc = 0, yInc = 0;
  int dcell = 0, x, y, xnew, ynew, d, pass = 0;
  flag fHunt;

  x = xs | 1; y = ys | 1;
  fHunt = Get(x, y);
  if (xInc == 0) {
    xInc = Rnd(0, 1) ? 2 : -2; yInc = Rnd(0, 1) ? 2 : -2;
  }

  loop {
    if ((y <= yl+1 && (grfEdge & (1 << 0)) == 0) ||
        (x <= xl+1 && (grfEdge & (1 << 1)) == 0) ||
        (y >= yh-2 && (grfEdge & (1 << 2)) == 0) ||
        (x >= xh-2 && (grfEdge & (1 << 3)) == 0))
      break;
    if (!Get(x, y)) {
      // Check adjacent cells for available new cell to carve into.
      if (ms.fRiver || fHunt) {
        d = DirFindUncreated(&x, &y, fFalse);
        fHunt = d < 0;
      } else {
        d = RndDir();
        xnew = x + xoff2[d]; ynew = y + yoff2[d];
        if (FLegalMaze(xnew, ynew) && Get(xnew, ynew)) {
          x = xnew; y = ynew;
        } else
          fHunt = fTrue;
      }
    }
    if (!fHunt) {
      // Carve into new cell.
      Set0(x-xoff[d], y-yoff[d]);
      Set0(x, y);
      dcell++;
      pass = 0;
    } else {
      // Hunt mode: Move to a different cell.
      if (x + xInc > xl && x + xInc < xh)
        x += xInc;
      else {
        neg(xInc);
        if (y + yInc > yl && y + yInc < yh)
          y += yInc;
        else {
          neg(yInc);
          pass++;
          if (pass > 1)
            break;
        }
      }
    }
  }

  *xe = x; *ye = y;
  *pcell = dcell;
  return fTrue;
}


// Create part of a Maze within a bitmap containing one section of a larger
// Maze, stopping when one of the four edges of the section is reached, using
// a version of the Aldous-Broder algorithm.

flag CMaz::AldousBroderPartial(int grfEdge, int xs, int ys,
  int *xe, int *ye, int *pcell)
{
  int dcell = 0, x, y, xnew, ynew, d;

  x = xs | 1; y = ys | 1;

  loop {
    if ((y <= yl+1 && (grfEdge & (1 << 0)) == 0) ||
        (x <= xl+1 && (grfEdge & (1 << 1)) == 0) ||
        (y >= yh-2 && (grfEdge & (1 << 2)) == 0) ||
        (x >= xh-2 && (grfEdge & (1 << 3)) == 0))
      break;
    do {
      d = RndDir();
      xnew = x + xoff2[d]; ynew = y + yoff2[d];
    } while (!FLegalMaze(xnew, ynew));

    // Carve into this cell if it hasn't been visited yet.
    if (Get(xnew, ynew)) {
      Set0((x + xnew) >> 1, (y + ynew) >> 1);
      Set0(xnew, ynew);
      dcell++;
    }
    x = xnew; y = ynew;
  }

  *xe = x; *ye = y;
  *pcell = dcell;
  return fTrue;
}


// Create part of a Maze within a bitmap containing one section of a larger
// Maze, stopping when one of the four edges of the section is reached, or
// when either the low or high limit of the stack is reached, using a
// version of the recursive backtracking algorithm.

flag CMaz::RecursivePartial(int grfEdge, int xs, int ys, int ss,
  int *xe, int *ye, int *se, int *pcell, CMaz &bStack)
{
  int dcell = 0, x, y, s, sMax, xnew, ynew, d, id, e;

  x = xs | 1; y = ys | 1; s = ss;
  sMax = bStack.m_x >> 1;

  loop {
    if (s >= sMax ||
      (y <= yl+1 && (grfEdge & (1 << 0)) == 0) ||
      (x <= xl+1 && (grfEdge & (1 << 1)) == 0) ||
      (y >= yh-2 && (grfEdge & (1 << 2)) == 0) ||
      (x >= xh-2 && (grfEdge & (1 << 3)) == 0))
      break;

    // Check adjacent cells for available new cell to carve into.
    d = RndDir(); e = (Rnd(0, 1) << 1) - 1;
    for (id = 0; id < DIRS; id++) {
      xnew = x + xoff2[d]; ynew = y + yoff2[d];
      if (FLegalMaze(xnew, ynew) && Get(xnew, ynew))
        break;
      d = (d + e) & DIRS1;
    }

    // If no available new cell, pop stack and backtrack.
    if (id >= DIRS) {
      s--;
      d = (bStack.Get(s << 1, 0) << 1) + bStack.Get((s << 1) + 1, 0);
      x -= xoff2[d]; y -= yoff2[d];
      if (s <= 0)
        break;
      continue;
    }

    // Carve into new cell, and push direction taken here on the stack.
    Set0((x + xnew) >> 1, (y + ynew) >> 1);
    Set0(xnew, ynew);
    dcell++;
    x = xnew; y = ynew;
    bStack.Set(s << 1, 0, (d & 2) >> 1);
    bStack.Set((s << 1) + 1, 0, d & 1);
    s++;
  }

  *xe = x; *ye = y; *se = s;
  *pcell = dcell;
  return fTrue;
}

/* create2.cpp */
