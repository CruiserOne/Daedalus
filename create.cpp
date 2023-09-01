/*
** Daedalus (Version 3.4) File: create.cpp
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
** This file contains Maze creation algorithms, specifically algorithms that
** produce standard orthogonal 2D Mazes.
**
** Created: 9/4/2000.
** Last code change: 8/29/2023.
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
** Core Maze Creation Routines
******************************************************************************
*/

// Carve Maze passages adding on to any existing passages in the bitmap, using
// the Hunt and Kill algorithm.

flag CMaz::PerfectGenerate(flag fClear, int xs, int ys)
{
  int x, y, xnew, ynew, xInc, yInc, zInc, fHunt = fFalse, pass = 0,
    d, d0, d1, dd, i;
  long count;
  flag fClean = ms.fRiver && ms.fRiverEdge && ms.fRiverFlow;
  int dHunt, tHunt, iHunt, cHunt, cHuntMax;

  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize))
    return fFalse;
  xInc = Rnd(0, 1) ? 2 : -2; yInc = Rnd(0, 1) ? 2 : -2;
  zInc = ms.nHuntType != 1 || Rnd(0, 1);
  x = xs + !FOdd(xs ^ xl); y = ys + !FOdd(ys ^ yl);
  Set0(x, y);
  count = ((xh - xl) >> 1) * ((yh - yl) >> 1) - 1;
  d0 = ms.fRiver || ms.nHuntType >= 2 ? DIRS : 1;
  cHuntMax = NMax((xh - xl) >> 1, (yh - yl) >> 1) << 2;

  loop {
LNext:
    // Simple case: For Mazes covering the whole bitmap section, with no
    // special settings active, or for Hunt mode, simply carve into any
    // available uncreated cell from the current created cell.
    if (fClear && (fClean || fHunt)) {
      d = DirFindUncreated(&x, &y, fFalse);
      if (d < 0)
        goto LHunt;
      if (fCellMax)
        goto LDone;
      Set0(x - xoff[d], y - yoff[d]);
      Set0(x, y);
LSet:
      fHunt = fFalse;
      pass = 0;
      if (ms.nHuntType == 1)
        inv(zInc);

      // Simple termination case: When number of cells created is the number
      // in the whole bitmap section, there's clearly nothing left to do.
      count--;
      if (count <= 0)
        goto LDone;
      goto LNext;
    }

    // Complex cases: For Mazes that don't cover the whole bitmap, or when
    // some of the Perfect create settings are non-default values.
    d = RndDir();
    dd = (Rnd(0, 1) << 1) - 1;
    d1 = fHunt ? DIRS : d0;
    for (i = 0; i < d1; i++) {
      xnew = x + xoff2[d];
      ynew = y + yoff2[d];
      if (!(FLegalMaze2(xnew, ynew) &&
        (fClear || FOnMaze(xnew, ynew)))) {
        if (!ms.fRiverEdge && !fHunt)
          goto LHunt;
      } else if (!Get(xnew, ynew)) {
        if (!ms.fRiverFlow && !fHunt && i < 2 &&
          !Get((x + xnew) >> 1, (y + ynew) >> 1)) {
          x = xnew; y = ynew;
          goto LNext;
        }
      } else {
        if (!ms.fRiver && fHunt && i > 0) {
          pass = 0;
          goto LHunt;
        }
        Set0((x + xnew) >> 1, (y + ynew) >> 1);
        Set0(xnew, ynew);
        x = xnew; y = ynew;
        goto LSet;
      }
      d = (d + dd) & DIRS1;
    }

    // Hunt mode: Move to a different created cell.
LHunt:
    if (!fHunt) {
      fHunt = fTrue;
      if (ms.nHuntType >= 2) {
        dHunt = Rnd(0, DIRS1);
        tHunt = (Rnd(0, 1) << 1) - 1;
        cHunt = iHunt = 0;
      }
    }

    // Search in a square spiral pattern from the start cell.
    if (ms.nHuntType >= 2) {
      do {
        x += xoff2[dHunt]; y += yoff2[dHunt];
        iHunt++;
        if (iHunt > (cHunt >> 1)) {
          iHunt = 0;
          cHunt++;
          if (cHunt > cHuntMax)
            goto LDone;
          dHunt = (dHunt + tHunt) & DIRS1;
        }
      } while (!FLegalMaze2(x, y) || Get(x, y) || (!fClear && !FOnMaze(x, y)));
      continue;
    }

    // Search row by row in opposite directions for balance, and also search
    // column by column in opposite directions for balance too, to avoid bias.
    do {
      if (zInc) {
        if (x + xInc >= xl && x + xInc <= xh)
          x += xInc;
        else {
          neg(xInc);
          if (y + yInc >= yl && y + yInc <= yh)
            y += yInc;
          else {
            neg(yInc);

            // A complete pass over the Maze once in Hunt mode means finished.
            pass++;
            if (pass > 1)
              goto LDone;
            UpdateDisplay();
          }
        }
      } else {
        if (y + yInc >= yl && y + yInc <= yh)
          y += yInc;
        else {
          neg(yInc);
          if (x + xInc >= xl && x + xInc <= xh)
            x += xInc;
          else {
            neg(xInc);

            // A complete pass over the Maze once in Hunt mode means finished.
            pass++;
            if (pass > 1)
              goto LDone;
            UpdateDisplay();
          }
        }
      }
    } while (Get(x, y) || (!fClear && !FOnMaze(x, y)));
  }
LDone:
  return fTrue;
}


// Create a new perfect Maze in the bitmap using the Hunt and Kill algorithm,
// by carving passages.

flag CMaz::CreateMazePerfect()
{
  if (ms.fTreeWall)
    return CreateMazePerfect2();
  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize | femsMinSize))
    return fFalse;
  MazeClear(fOn);
  MakeEntranceExit(0);
  UpdateDisplay();
  PerfectGenerate(fTrue, Rnd(xl, xh-1), Rnd(yl, yh-1));
  return fTrue;
}


// Add all possible wall segments to the Maze, that won't cause a dead end to
// be created, or cause a section of the Maze to become inaccessible. Called
// from CreateMazeBraid() to do most of the work of creating braid Mazes.

long CMaz::BraidConnectWalls()
{
  int x, y, xnew, ynew, xspan, yspan, d;
  long count = 0, total, i, j, jInc;

  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize))
    return fFalse;
  xspan = (xh - xl) >> 1;
  yspan = (yh - yl) >> 1;
  total = xspan * yspan * 2;
  j = Rnd(1, total);
  jInc = LPrime(total);

  // Sweep over all potential wall segments in pseudo-random order.
  for (i = 0; i < total; i++) {
    j += jInc;
    while (j >= total)
      j -= total;
    y = (j >> 1) / xspan;
    x = xl + (((j >> 1) - y * xspan) << 1) + 2;
    y = yl + (y << 1) + 2;
    if (Get(x, y)) {
      d = j & 1;
      xnew = x + xoff2[d];
      ynew = y + yoff2[d];
      if (xnew < xh && ynew < yh && Get(xnew, ynew)) {
        xnew -= xoff[d];
        ynew -= yoff[d];

        // If there are two wall endpoints with a passage between, and adding
        // a wall there wouldn't cause a dead end or isolation, create it.
        if (!Get(xnew, ynew) && !FWouldMakeDeadEnd(xnew, ynew) &&
          !FWouldMakeIsolation(xnew, ynew)) {
          count++;
          Set1(xnew, ynew);
        }
      }
    }
  }
  return count;
}


// Create a new braid Maze in the bitmap, i.e. a Maze without any dead ends,
// using a wall adding algorithm.

flag CMaz::CreateMazeBraid()
{
  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize | femsMinSize))
    return fFalse;

  // Step 1: Start with the outer wall.
  MazeClear(fOff);

  // Step 2: Add a grid of wall vertices, then add wall segments touching each
  // vertex to ensure there are no open rooms or poles in the Maze.
  MazeNormalize(fOn);
  UpdateDisplay();
  MakeEntranceExit(0);
  DoConnectPoles(fFalse);

  // Step 2.5: There might still be a pole in the lower right corner.
  if (Count(xh-2, yh-2) == 0) {
    if (xh-xl > 4 && yh-yl > 2) {
      Set1(xh-3, yh-2);
      Set0(xh-4, yh-1); Set0(xh-4, yh-3);
    } else if (xh-xl > 2 && yh-yl > 4) {
      Set1(xh-2, yh-3);
      Set0(xh-1, yh-4); Set0(xh-3, yh-4);
    }
  }
  UpdateDisplay();

  // Step 3: Add as many wall segments that don't make dead ends as possible.
  BraidConnectWalls();
  return fTrue;
}


// Create a new braid Maze in the bitmap using a binary algorithm, i.e.
// starting with a template based on the Tilt Maze pattern.

flag CMaz::CreateMazeBraidTilt()
{
  int x, y, i, m, n, d, e;

  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize | femsMinSize))
    return fFalse;

  // Step 1: Start with the outer wall.
  MazeClear(fOff);
  UpdateDisplay();

  // Step 2: Create a tilt Maze pattern covering the Maze bitmap.
  for (y = yl; y <= yh; y += 4)
    for (x = xl; x <= xh; x += 4)
      for (i = 0; i <= 1; i++) {
        m = x + (i << 1); n = y + (i << 1);
        if (n <= yl || n >= yh - 1 || (ms.fTiltDiamond && !Get(m-2, n)))
          d = 1;
        else if (m <= xl || m >= xh - 1)
          d = 0;
        else
          d = Rnd(0, 1);
        d <<= 1; e = d ^ 2;
        if (d)
          LineX(NMax(m - d, xl), NMin(m + d, xh), n, fOn);
        else
          LineY(m, NMax(n - e, yl), NMin(n + e, yh), fOn);
      }

  // Step 2.5: Ensure all poles are set, if needed.
  UpdateDisplay();
  MakeEntranceExit(0);
  if (!ms.fTiltDiamond)
    MazeNormalize(fOn);

  // Step 3: Remove dead ends the tilt pattern may have placed in the corners.
  DoCrackDeadEnds();
  if (ms.fTiltDiamond) {
    for (i = 0; i < 10; i++) {
      if (DoConnectPoles(fTrue) <= 0)
        break;
      DoCrackDeadEnds();
    }
  } else
    DoConnectPoles(fFalse);

  // Step 4: Remove isolated sections to ensure the Maze is solvable.
  DoCrackIsolations();
  BraidConnectWalls();
  return fTrue;
}


// Find and return the location of an uncreated wall vertex in the Maze.
// Called from SpiralMakeTemplate() to start a new spiral.

void CMaz::SpiralMakeNew(ushort *x0, ushort *y0)
{
  int xsize, ysize, x, y, count = 0;
  long total, i, iInc;

  xsize = ((xh - xl) >> 1) - 1;
  ysize = ((yh - yl) >> 1) - 1;
  total = xsize*ysize;
  iInc = LPrime(total);
  if (ms.iSpiralIndex < 0)
    ms.iSpiralIndex = Rnd(1, total);
  do {

    // Try a random location several times. If nothing found, then do a
    // pseudo-random order scan over all the vertices in the Maze.
    if (count < 5) {
      count++;
      i = Rnd(1, total) - 1;
    } else {
      ms.iSpiralIndex++;
      if (ms.iSpiralIndex >= total)
        ms.iSpiralIndex = 0;
      i = (ms.iSpiralIndex * iInc) % total;
    }
    y = i / xsize;
    x = xl + (((i - y * xsize) + 1) << 1);
    y = yl + ((y + 1) << 1);
  } while (Get(x, y));
  Set1(x, y);
  *x0 = x; *y0 = y;
}


// Create a set of spirals in the open spaces of the bitmap, by growing
// spirals from the center out. Called from CreateMazeSpiral() to create a
// spiral template, i.e. do most of the work of creating spiral Mazes.

int CMaz::SpiralMakeTemplate()
{
  PT rgpt[SpiralMax][SpiralWallMax];
  int numwalls[SpiralMax], hand[SpiralMax], dir[SpiralMax][SpiralWallMax];
  int spirals, wallsleft, x, y, xnew, ynew, count = 0, s, w, i;
  long cellstotal = (((xh - xl) >> 1) - 1)*(((yh - yl) >> 1) - 1),
    cellsleft = cellstotal;

  // Figure out how many spirals to have growing at once.
  i = cellsleft / 475;
  if (i > ms.cSpiral)
    spirals = ms.cSpiral;
  else if (i < 1 && cellsleft > 0)
    spirals = 1;
  else
    spirals = i;

  // Sprout the initial set of spirals.
  ms.iSpiralIndex = -1;
  for (s = 0; s < spirals; s++) {
    numwalls[s] = 1;
    dir[s][0] = Rnd(0, DIRS1);
    hand[s] = Rnd(0, 1) ? 1 : -1;
    SpiralMakeNew(&rgpt[s][0].x, &rgpt[s][0].y);
    cellsleft--;
    count++;
  }

  while (cellsleft > 0) {
    for (s = 0; s < spirals; s++) {
      wallsleft = numwalls[s];
      for (w = numwalls[s]-1; w >= 0; w--) {
        if (rgpt[s][w].x == SpiralDead) {
          wallsleft--;
          continue;
        }
LThisWall:
        x = rgpt[s][w].x; y = rgpt[s][w].y;
        xnew = x + (xoff[dir[s][w]] << 1);
        ynew = y + (yoff[dir[s][w]] << 1);
        if (!Get(xnew, ynew)) {
          Block(x, y, xnew, ynew, fOn);
          cellsleft--;
          rgpt[s][w].x = xnew;
          rgpt[s][w].y = ynew;

        // This spiral arm has another wall in front of it.
        } else {

          // 50% chance it turns and follows it.
          if (Rnd(1, 2) == 1) {
            dir[s][w] = dir[s][w] - hand[s] & DIRS1;
            goto LThisWall;
          }

          // Else 33% chance it connects with the wall.
          if (Rnd(1, 3) > 1)
            Block(x, y, xnew, ynew, fOn);

          // Otherwise just die here.
          rgpt[s][w].x = SpiralDead;
          continue;
        }

        // Take a peek to the side and check if it's time to turn yet.
        xnew = rgpt[s][w].x + (xoff[dir[s][w] + hand[s] & DIRS1] << 1);
        ynew = rgpt[s][w].y + (yoff[dir[s][w] + hand[s] & DIRS1] << 1);
        if (Get(xnew, ynew))
          goto LThisWall;

        // 25% chance to spawn another wall segment at the turn.
        if (Rnd(1, 4) > 1 || numwalls[s] >= ms.cSpiralWall)
          continue;
        numwalls[s]++;
        for (i = numwalls[s]-1; i > w+1; i--) {
          rgpt[s][i] = rgpt[s][i - 1];
          dir[s][i] = dir[s][i - 1];
        }
        rgpt[s][w + 1].x = xnew;
        rgpt[s][w + 1].y = ynew;
        dir[s][w + 1] = dir[s][w];
        Set1(xnew, ynew);
        cellsleft--;
        goto LThisWall;
      }

      // Turn all spiral arms in the direction of the spiral.
      for (w = 0; w < numwalls[s]; w++)
        dir[s][w] = dir[s][w] + hand[s] & DIRS1;

      // A spiral is dead when it has no arms left. Spawn a new one.
      if (!(wallsleft > 0 || cellsleft <= 0 || Rnd(1, 20) > 1)) {
        SpiralMakeNew(&rgpt[s][0].x, &rgpt[s][0].y);
        cellsleft--;
        count++;
        numwalls[s] = 1;
      }
    }
  }
  UpdateDisplay();

  // Toggle the state of random wall segments to make the Maze more random.
  if (cellstotal > 0)
    for (i = ((xh - xl) + (yh - yl)) / 3 + ms.cRandomAdd; i > 0; i--) {
      y = Rnd(yl + 1, yh - 1);
      w = 1 + FOdd(y - yl);
      x = RndSkip(xl + w, xh - w);
      Inv(x, y);
    }
  return count;
}


// Create a new spiral Maze in the bitmap, formed of interlocking spirals.

flag CMaz::CreateMazeSpiral()
{
  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize | femsMinSize))
    return fFalse;

  // Fill the bitmap with a random spiral template, which doesn't form a Maze.
  MazeClear(fOff);
  SpiralMakeTemplate();
  MakeEntranceExit(0);
  UpdateDisplay();

  // Run the isolation and loop removers, to make a perfect Maze out of it.
  DoCrackIsolations();
  UpdateDisplay();
  DoConnectDetachments();
  return fTrue;
}


// Create a Maze with a diagonal bias, where many walls look like stairs.

flag CMaz::CreateMazeDiagonal()
{
  int x, y, m, n, i;

  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize | femsMinSize))
    return fFalse;
  MazeClear(fOff);
  UpdateDisplay();

  // Create a template formed of a series of stairstep walls filling the
  // bitmap, going diagonally from lower left to upper right.
  for (n = yh; n > yl; n -= 4) {
    x = xl; y = n;
    while (y > yl && x < xh) {
      Set1(x, y); y--; Set1(x, y); y--;
      Set1(x, y); x++; Set1(x, y); x++;
      Set1(x, y);
    }
  }
  for (m = xl + 4; m < xh; m += 4) {
    x = m; y = yh;
    while (y > yl && x < xh) {
      Set1(x, y); y--; Set1(x, y); y--;
      Set1(x, y); x++; Set1(x, y); x++;
      Set1(x, y);
    }
  }
  UpdateDisplay();

  // Turn on random wall segments to make the Maze more random.
  m = xh - xl + 2; n = yh - yl + 2;
  for (i = (m + n) * 4 + ms.cRandomAdd; i > 0; i--) {
    x = xl + Rnd(0, (m >> 1) - 2) * 2; y = yl + Rnd(0, (n >> 1) - 2) * 2;
    Set1(x, y + 1);
    Set1(x + 1, y + 2);
  }
  MakeEntranceExit(0);
  UpdateDisplay();

  // Run the isolation remover, to make a perfect Maze out of the template.
  DoCrackIsolations();
  return fTrue;
}


/*
******************************************************************************
** More Perfect Maze Creation Routines
******************************************************************************
*/

CONST char rgdRecursive[64] = {
  0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3,
  0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3,
  0, 0, 0,          1, 1, 1,          2, 2, 2,          3, 3, 3,
  0,                1,                2,                3};
CONST char rgidRecursive[64] = {
  33,35,39,41,45,47,27,29,37,40,43,46,25,28,31,34,42,44,24,26,30,32,36,38,
  52,53,55,56,58,59,49,50,54,56,57,59,48,50,51,53,57,58,48,49,51,52,54,55,
  61,62,63,         60,62,63,         60,61,63,         60,61,62,
  -1,               -1,               -1,               -1};
CONST char rgidRecursiveBias[2][4] =
  {{2, 3, 12, 13}, {10, 11, 20, 21}};

// Carve Maze passages into a solid shape on the bitmap, using the Recursive
// Backtracking algorithm.

flag CMaz::RecursiveGenerate(int xs, int ys)
{
  byte *rgdir;
  long stack = 0, count;
  int cRunRnd = 0, dirRnd, x, y, xnew, ynew, id, d = -1;
  flag fFast = ms.nRndRun <= 0 && ms.nRndBias == 0;

  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize))
    return fFalse;
  count = ((xh - xl) >> 1) * ((yh - yl) >> 1);
  rgdir = RgAllocate(count, byte);
  if (rgdir == NULL)
    return fFalse;
  x = xl + ((xs - xl) | 1); y = yl + ((ys - yl) | 1);
  Set0(x, y);
  count--;
  loop {

    // Determine direction sequence to take from a newly visited cell.
    if (fFast && d >= 0)
      id = rgidRecursive[(d ^ 2)*6 + Rnd(0, 5)];
    else {
      if (cRunRnd > 0)
        cRunRnd--;
      else {
        if (ms.nRndRun > 0)
          cRunRnd = Rnd(0, ms.nRndRun);
        dirRnd = Rnd(0, 23 + NAbs(ms.nRndBias)*4);
      }
      if (dirRnd < 24)
        id = dirRnd;
      else
        id = rgidRecursiveBias[ms.nRndBias > 0][dirRnd & DIRS1];
    }

    // Move to an adjacent cell if any available, pushing the directions
    // still available from the current cell on the stack.
    loop {
      d = rgdRecursive[id];
      xnew = x + xoff2[d]; ynew = y + yoff2[d];
      if (FLegalMaze2(xnew, ynew) && Get(xnew, ynew)) {
        if (fCellMax)
          goto LDone;
        x = xnew; y = ynew;
        Set0(x - xoff[d], y - yoff[d]);
        Set0(x, y);
        count--;
        if (count <= 0)  // Done if all cells have been carved into.
          goto LDone;
        rgdir[stack] = id; stack++;
        break;
      }

      // Select next direction from the earlier chosen direction sequence.
      loop {
        id = rgidRecursive[id];
        if (id >= 0)
          break;

        // If no directions left, pop the stack and back up to previous cell.
        stack--;
        if (stack < 0)  // Done if stack becomes empty meaning no cells left.
          goto LDone;
        id = rgdir[stack]; d = rgdRecursive[id];
        x -= xoff2[d]; y -= yoff2[d];
      }
    }
  }

LDone:
  DeallocateP(rgdir);
  return fTrue;
}


// Create a new perfect Maze in the bitmap using the Recursive Backtracking
// algorithm. This carves passages.

flag CMaz::CreateMazeRecursive()
{
  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize | femsMinSize))
    return fFalse;
  MazeClear(fOn);
  MakeEntranceExit(0);
  UpdateDisplay();
  return RecursiveGenerate(Rnd(xl, xh-1), Rnd(yl, yh-1));
}


enum _primcell {
  primIn       = 0,
  primFrontier = 1,
  primOut      = 2,
  primNever    = 3,
};

typedef struct _prim {
  long zFrontier;      // Map cell index to cell coordinate
  long set;            // Map cell coordinate to cell type
} PRIM;

// Mark a cell as part of the set of cells that's part of the Maze. Called
// from PrimGenerate().

long PrimMakeIn(PRIM *prim, int x, int y, int xs, int ys, long cFrontier)
{
  long z;
  int xnew, ynew, d;

  z = y*xs + x;
  prim[z].set = primIn;

  // Update the status of the four adjacent cells.
  for (d = 0; d < DIRS; d++) {
    xnew = x + xoff[d]; ynew = y + yoff[d];
    if (xnew >= 0 && ynew >= 0 && xnew < xs && ynew < ys) {
      z = ynew*xs + xnew;
      if (prim[z].set == primOut) {
        prim[z].set = primFrontier;
        prim[cFrontier].zFrontier = z;
        cFrontier++;
      }
    }
  }
  return cFrontier;
}


// Carve Maze passages into a solid shape on the bitmap, or add walls assuming
// the bitmap is clear, using a modified version of Prim's algorithm.

flag CMaz::PrimGenerate(flag fWall, flag fClear, int xp, int yp)
{
  PRIM *prim;
  long cFrontier = 0, i;
  int xbase, ybase, x, y, xs, ys, j, xnew, ynew, d;

  Assert(FImplies(fWall, fClear));
  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize))
    return fFalse;
  xs = ((xh - xl) >> 1) + fWall; ys = ((yh - yl) >> 1) + fWall;
  prim = RgAllocate(xs*ys, PRIM);
  if (prim == NULL)
    return fFalse;
  xbase = xl + 1 - fWall; ybase = yl + 1 - fWall;

  // Figure out which cells are part of the Maze and which should be ignored
  // (because they're in the middle of solid blocks).
  if (fWall || fClear) {
    for (i = xs*ys-1; i >= 0; i--)
      prim[i].set = primOut;
  } else {
    for (y = 0; y < ys; y++)
      for (x = 0; x < xs; x++)
        prim[y*xs + x].set = primNever -
          Get(xbase + (x << 1), ybase + (y << 1));
  }

  // Figure out which cell or cells to start growing from.
  if (!fWall) {
    x = xp; y = yp;
    Set0(xbase + (x << 1), ybase + (y << 1));
    cFrontier = PrimMakeIn(prim, x, y, xs, ys, cFrontier);
  } else {
    for (x = 0; x < xs; x++) {
      prim[x].set = primIn;
      prim[(ys-1)*xs + x].set = primIn;
    }
    for (y = 1; y < ys-1; y++) {
      prim[y*xs].set = primIn;
      prim[y*xs + (xs-1)].set = primIn;
    }
    for (x = 0; x < xs; x++) {
      cFrontier = PrimMakeIn(prim, x, 0,    xs, ys, cFrontier);
      cFrontier = PrimMakeIn(prim, x, ys-1, xs, ys, cFrontier);
    }
    for (y = 1; y < ys-1; y++) {
      cFrontier = PrimMakeIn(prim, 0,    y, xs, ys, cFrontier);
      cFrontier = PrimMakeIn(prim, xs-1, y, xs, ys, cFrontier);
    }
  }

  // Grow into each reachable cell until there are none left.
  while (cFrontier > 0) {
    if (fCellMax)
      goto LDone;
    i = Rnd(0, cFrontier - 1);
    y = prim[i].zFrontier / xs; x = prim[i].zFrontier % xs;
    prim[i].zFrontier = prim[cFrontier-1].zFrontier;
    cFrontier--;
    Set(xbase + (x << 1), ybase + (y << 1), fWall);
    cFrontier = PrimMakeIn(prim, x, y, xs, ys, cFrontier);

    // Pick a random unreached cell adjacent to the set of current cells.
    d = RndDir();
    for (j = 0; j < DIRS; j++) {
      xnew = x + xoff[d]; ynew = y + yoff[d];
      if (xnew >= 0 && ynew >= 0 && xnew < xs && ynew < ys &&
        prim[ynew*xs + xnew].set == primIn) {
        Set(xbase + (x << 1) + xoff[d], ybase + (y << 1) + yoff[d], fWall);
        break;
      }
      DirInc(d);
    }
  }

LDone:
  DeallocateP(prim);
  return fTrue;
}


// Create a new perfect Maze in the bitmap using a modified version of Prim's
// algorithm. This can carve passages or add walls.

flag CMaz::CreateMazePrim()
{
  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize | femsMinSize))
    return fFalse;
  MazeClear(!ms.fTreeWall);
  MakeEntranceExit(0);
  UpdateDisplay();
  return PrimGenerate(ms.fTreeWall, fTrue,
    Rnd(0, ((xh - xl) >> 1) - 1), Rnd(0, ((yh - yl) >> 1) - 1));
}


typedef struct _prim2 {
  long x;        // Horizontal coordinate of edge
  long y;        // Vertical coordinate of edge
  long iEdge;    // Map cell coordinate to edge index/weight
  long iHeap;    // Map heap index to edge index/weight
} PRIM2;

#define ZPrimXY(x, y) ((y) * xs + ((x) >> 1))
#define FComparePrim(i1, i2) (prim[i1].iHeap < prim[i2].iHeap)

// Given a valid heap and an item at its end, push that item up through the
// heap so its valid. Called from CreateMazePrim2 to add edges to the set.

void PushupPrim(PRIM2 *prim, long i)
{
  int n = prim[i].iHeap, iParent;

  while (i > 0) {
    iParent = ((i + 1) >> 1) - 1;
    if (FComparePrim(iParent, i))
      break;
    prim[i].iHeap = prim[iParent].iHeap;
    prim[iParent].iHeap = n;
    i = iParent;
  }
}


// Create a new perfect Maze in the bitmap using full or simplified versions
// of Prim's algorithm. This can carve passages or add walls.

flag CMaz::CreateMazePrim2(int xp, int yp)
{
  PRIM2 *prim = NULL;
  long cedgeMax, cedge = 0, iFrontier = 0, i, j, jParent;
  int xs, ys, x, y, xnew, ynew, xnew2, ynew2, xInc, yInc, d;
  flag fWall = ms.fTreeWall, fRet = fFalse;

  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize | femsMinSize))
    return fFalse;
  MazeClear(!fWall);
  MakeEntranceExit(0);
  UpdateDisplay();

  xs = ((xh - xl) >> 1) + fWall; ys = ((yh - yl) >> 1) + fWall;
  cedgeMax = xs*ys << 1;
  prim = RgAllocate(cedgeMax, PRIM2);
  if (prim == NULL)
    goto LExit;
  for (i = 0; i < cedgeMax; i++)
    prim[i].x = prim[i].y = prim[i].iEdge = prim[i].iHeap = -1;

  // Create a list of all passage/wall segments, each with its own weight.
  j = fWall << 1;
  for (y = 1; y < yh-yl+1; y += 2)
    for (x = 2-j; x < xh-xl+j; x += 2) {
      prim[cedge].x = x; prim[cedge].y = y;
      prim[ZPrimXY(x, y)].iEdge = cedge;
      cedge++;
    }
  for (y = 2-j; y < yh-yl+j; y += 2)
    for (x = 1; x < xh-xl+1; x += 2) {
      prim[cedge].x = x; prim[cedge].y = y;
      prim[ZPrimXY(x, y)].iEdge = cedge;
      cedge++;
    }
  Assert(cedge < cedgeMax);
  if (ms.fTreeRandom) {
    for (i = 0; i < cedge; i++) {
      j = Rnd(0, cedge-1);
      // Don't use SwapN here because i may equal j.
      d = prim[i].x; prim[i].x = prim[j].x; prim[j].x = d;
      d = prim[i].y; prim[i].y = prim[j].y; prim[j].y = d;
    }
    for (i = 0; i < cedge; i++)
      prim[ZPrimXY(prim[i].x, prim[i].y)].iEdge = i;
  }

  // Start with an initial point or boundary wall of edges.
  if (!fWall) {
    if (!ms.fSolveDotExit) {
      x = RndSkip(1-fWall, xh-xl-1+fWall);
      y = RndSkip(1-fWall, yh-yl-1+fWall);
    } else {
      x = xp; y = yp;
      if (fWall) {
        x &= ~1; y &= ~1;
      } else {
        x |= 1; y |= 1;
      }
    }
  } else {
    for (i = 0; i <= 1; i++) {
      for (x = 2; x < xh-xl; x += 2) {
        j = prim[ZPrimXY(x, i ? 1 : yh-yl-1)].iEdge;
        if (j < 0)
          continue;
        prim[iFrontier].iHeap = j;
        if (ms.fTreeRandom)
          PushupPrim(prim, iFrontier);
        iFrontier++;
      }
      for (y = 2; y < yh-yl; y += 2) {
        j = prim[ZPrimXY(i ? 1 : xh-xl-1, y)].iEdge;
        if (j < 0)
          continue;
        prim[iFrontier].iHeap = j;
        if (ms.fTreeRandom)
          PushupPrim(prim, iFrontier);
        iFrontier++;
      }
    }
    goto LNext;
  }

  loop {
    Set(xl + x, yl + y, fWall);
    if (fCellMax)
      break;

    // Add edges surrounding new point to frontier set.
    for (d = 0; d < DIRS; d++) {
      xnew2 = xl + x + xoff2[d]; ynew2 = yl + y + yoff2[d];
      if (!FLegalMaze2(xnew2, ynew2) || GetFast(xnew2, ynew2) == fWall)
        continue;
      xnew = x + xoff[d]; ynew = y + yoff[d];
      i = prim[ZPrimXY(xnew, ynew)].iEdge;
      if (i < 0)
        continue;
      prim[iFrontier].iHeap = i;
      if (ms.fTreeRandom)
        PushupPrim(prim, iFrontier);
      iFrontier++;
    }

LNext:
    // Select next edge to consider.
    if (iFrontier <= 0)
      break;
    iFrontier--;
    if (ms.fTreeRandom) {

      // Full: All edge weights are different, so maintain heap.
      i = prim[0].iHeap;
      prim[0].iHeap = prim[iFrontier].iHeap;
      jParent = 0; j = 1;
      // Given a valid heap and an item that's replaced its top, push that
      // item down through the heap so the heap becomes valid again.
      while (j < iFrontier && (FComparePrim(j, jParent)) ||
        (j+1 < iFrontier && FComparePrim(j+1, jParent))) {
        j += (j+1 < iFrontier && FComparePrim(j+1, j));
        SwapN(prim[j].iHeap, prim[jParent].iHeap);
        jParent = j; j = (j << 1) + 1;
      }
    } else {

      // Simplified: All edges weights are same, so select randomly.
      j = Rnd(0, iFrontier);
      i = prim[j].iHeap;
      prim[j].iHeap = prim[iFrontier].iHeap;
    }

    // Carve edge and expand Maze to include new point.
    xnew = prim[i].x; ynew = prim[i].y;
    xInc = FOdd(ynew) ^ fWall; yInc = !xInc;
    if (GetFast(xl + xnew + xInc, yl + ynew + yInc) == fWall) {
      neg(xInc); neg(yInc);
    }
    x = xnew + xInc; y = ynew + yInc;
    if (GetFast(xl + x, yl + y) == fWall)
      goto LNext;
    Set(xl + xnew, yl + ynew, fWall);
  }

  fRet = fTrue;
LExit:
  if (prim != NULL)
    DeallocateP(prim);
  return fRet;
}


// Find out what set a node is part of. Nodes are arranged in a tree, where
// the set id is just a pointer to the tree's root node.

KRUS *KruskalFind(KRUS *krus)
{
  KRUS *krusRet, *krusT;

  // Find the root node to return.
  for (krusRet = krus; krusRet != krusRet->next; krusRet = krusRet->next)
    ;

  // Performance: Make each node along the path point directly to the root.
  while (krus != krusRet) {
    krusT = krus;
    krus = krus->next;
    krusT->next = krusRet;
  }
  return krusRet;
}


// Union two sets together, so they have the same root node.

void KruskalUnion(KRUS *krus1, KRUS *krus2)
{
  krus1 = krus1->next;
  krus2 = krus2->next;

  // Make whichever node has fewer children, a child of the larger tree.
  if (krus1->count > krus2->count) {
    krus2->next = krus1;
    krus1->count += krus2->count;
  } else {
    krus1->next = krus2;
    krus2->count += krus1->count;
  }
}


#define FCompareKrus(pt1, pt2) \
  (rgs[(pt1).y * z + (pt1).x] < rgs[(pt2).y * z + (pt2).x])

// Given a list of contrast numbers forming a heap style of binary tree, where
// a parent is greater than its two children, and the index of a random value
// number in it, push it down through the tree until the heap condition is met
// again. Called from CreateMazeKruskal() to implement heap sort.

void PushdownKrus(short *rgs, int z, PT *hedge, long i, long chedge)
{
  PT ptT;

  while (i < chedge && (FCompareKrus(hedge[i >> 1], hedge[i]) ||
    (i < chedge-1 && FCompareKrus(hedge[i >> 1], hedge[i + 1])))) {
    i += (i < chedge-1 && FCompareKrus(hedge[i], hedge[i + 1]));
    ptT = hedge[i]; hedge[i] = hedge[i >> 1]; hedge[i >> 1] = ptT;
    i <<= 1;
  }
}


// Create a new perfect Maze in the bitmap using Kruskal's algorithm. This can
// carve passages or add walls.

flag CMaz::CreateMazeKruskal(flag fClear, CCol *c2, CCol *c3)
{
  CCol cCopy;
  KRUS *cell = NULL;
  PT *hedge = NULL, pt;
  short *rgsContrast = NULL;
  long ccell, chedge, icell, ihedge = 0, i, c;
  int xs, ys, x, y, x2, y2, j;
  flag fRet = fFalse, fWall = ms.fTreeWall && fClear;
  KV kv, kv2;

  if (fClear) {
    if (ms.fKruskalPic && c3 != NULL)
      FBitmapSizeSet(c3->m_x & ~1, c3->m_y & ~1);
    if (!FEnsureMazeSize(3, femsOddSize | femsNoResize | femsMinSize |
      fems64K))
      goto LExit;
  } else {
    if (!FEnsureMazeSize(3, femsOddSize | femsNoResize | fems64K))
      goto LExit;
  }

  // If a color bitmap is specified, check whether it's a valid pre-connection
  // map, to indicate cells that are already linked.
  if (!fClear && c2 != NULL) {
    if (m_x != c2->m_x || m_y != c2->m_y) {
      c2 = NULL;
      goto LAfterCheck;
    }
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++)
        if (Get(x, y) != (c2->Get(x, y) != kvBlack)) {
          c2 = NULL;
          goto LAfterCheck;
        }
  }
LAfterCheck:

  xs = ((xh - xl) >> 1) + fWall; ys = ((yh - yl) >> 1) + fWall;
  ccell = xs*ys;
  chedge = (xs-1)*ys + (ys-1)*xs;
  cell = RgAllocate(ccell, KRUS);
  if (cell == NULL)
    goto LExit;
  hedge = RgAllocate(chedge, PT);
  if (hedge == NULL)
    goto LExit;
  if (fClear) {
    MazeClear(!fWall);
    MazeNormalize(fWall);
    MakeEntranceExit(0);
    UpdateDisplay();
  }

  // Start with each cell in a set by itself.
  for (icell = 0; icell < ccell; icell++) {
    cell[icell].next = &cell[icell];
    cell[icell].count = 1;
  }

  // Create a list of all passage/wall segments, then randomize their order.
  j = fWall << 1;
  for (y = 1; y < yh-yl+1; y += 2)
    for (x = 2-j; x < xh-xl+j; x += 2) {
      if (!fClear && (!Get(x-1, y) || !Get(x+1, y) ||
        !FOnMaze(x-1, y) || !FOnMaze(x+1, y)))
        continue;
      hedge[ihedge].x = x; hedge[ihedge].y = y;
      ihedge++;
    }
  for (y = 2-j; y < yh-yl+j; y += 2)
    for (x = 1; x < xh-xl+1; x += 2) {
      if (!fClear && (!Get(x, y-1) || !Get(x, y+1) ||
        !FOnMaze(x, y-1) || !FOnMaze(x, y+1)))
        continue;
      hedge[ihedge].x = x; hedge[ihedge].y = y;
      ihedge++;
    }
  if (fClear)
    Assert(ihedge == chedge);
  else {
    chedge = ihedge;
    MazeNormalize(fWall);
  }
  for (ihedge = 0; ihedge < chedge; ihedge++) {
    i = Rnd(0, chedge-1);
    pt = hedge[ihedge]; hedge[ihedge] = hedge[i]; hedge[i] = pt;
  }

  // If a color bitmap is specified, create a list of contrast numbers between
  // adjacent cells, and order the hedges based on it.
  if (c3 != NULL) {
    rgsContrast = RgAllocate(c3->m_x * c3->m_y, short);
    if (rgsContrast == NULL)
      goto LExit;

    // Determine the contrast between the cells on either side of each hedge.
    for (ihedge = 0; ihedge < chedge; ihedge++) {
      pt = hedge[ihedge];
      if (!fWall) {
        x2 = pt.x & ~1; y2 = pt.y & ~1;
      } else {
        x2 = pt.x | 1; y2 = pt.y | 1;
      }
      kv = c3->Get(x2, y2);
      kv2 = c3->Get(x2 + (pt.x - x2), y2 + (pt.y - y2));
      j = NAbs(RgbR(kv) - RgbR(kv2)) + NAbs(RgbG(kv) - RgbG(kv2)) +
        NAbs(RgbB(kv) - RgbB(kv2));
      rgsContrast[pt.y * c3->m_x + pt.x] = j;
    }

    // Heap sort the list of hedges.
    for (i = (chedge - 1) >> 1; i >= 0; i--)
      PushdownKrus(rgsContrast, c3->m_x, hedge, i << 1, chedge);
    for (i = chedge - 1; i > 0; i--) {
      pt = hedge[i]; hedge[i] = hedge[0]; hedge[0] = pt;
      PushdownKrus(rgsContrast, c3->m_x, hedge, 0, i);
    }
  }

  // For passage carving, start with a single cell. For wall adding, start
  // with the boundary wall in one large set.
  c = ccell;
  if (!fWall) {
    c--;

    // Process a pre-connection map, which indicates cells that should be
    // considered in the same set to start with. Merge any cells with pixels
    // having the same (non-white) color in the color bitmap, into same set.
    if (c2 != NULL && cCopy.FBitmapCopy(*c2)) {
      for (y = 0; y < ys; y++)
        for (x = 0; x < xs; x++) {
          kv = cCopy.Get(xl + (x << 1) + 1, yl + (y << 1) + 1);
          if (kv != kvBlack && kv != kvWhite) {
            for (y2 = y; y2 < ys; y2++)
              for (x2 = (y2 == y ? x+1 : 0); x2 < xs; x2++)
                if (cCopy.Get(xl + (x2 << 1) + 1, yl + (y2 << 1) + 1) == kv) {
                  cCopy.Set(xl + (x2 << 1) + 1, yl + (y2 << 1) + 1, kvBlack);
                  KruskalUnion(&cell[y*xs + x], &cell[y2*xs + x2]);
                  c--;
                }
          }
        }
    }
  } else {
    for (x = 0; x < xs-1; x++) {
      i = (ys-1)*xs;
      KruskalUnion(&cell[x], &cell[x+1]);
      KruskalUnion(&cell[i+x], &cell[i+x+1]);
      c -= 2;
    }
    for (y = 0; y < ys-1; y++) {
      i = y*xs;
      KruskalUnion(&cell[i], &cell[i+xs]);
      KruskalUnion(&cell[i+xs-1], &cell[i+xs-1+xs]);
      c -= 2;
    }
  }

  // Loop over each passage/wall segment, unifying them if in different sets.
  j = fWall ^ 1;
  for (ihedge = 0; ihedge < chedge && c > 0; ihedge++) {
    x = (hedge[ihedge].x - j) >> 1; y = (hedge[ihedge].y - j) >> 1;
    x2 = x; y2 = y;
    if (FOdd(hedge[ihedge].x) ^ fWall)
      y2++;
    else
      x2++;
    icell = y*xs + x; i = y2*xs + x2;
    if (KruskalFind(&cell[icell]) != KruskalFind(&cell[i])) {
      if (fCellMax)
        goto LExit;
      Set(xl + hedge[ihedge].x, yl + hedge[ihedge].y, fWall);
      KruskalUnion(&cell[icell], &cell[i]);
      c--;
    }
  }

  fRet = fTrue;
LExit:
  if (cell != NULL)
    DeallocateP(cell);
  if (hedge != NULL)
    DeallocateP(hedge);
  if (rgsContrast != NULL)
    DeallocateP(rgsContrast);
  return fRet;
}


// Create a new perfect Maze in the bitmap using the Hunt and Kill algorithm,
// by adding walls.

flag CMaz::CreateMazePerfect2()
{
  int x, y, xnew, ynew, xInc, yInc, zInc, fHunt = fTrue, pass = 0, d, d0,
    i, j;
  long count;

  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize | femsMinSize))
    return fFalse;
  MazeClear(fOff);
  MakeEntranceExit(0);
  xInc = Rnd(0, 1) ? 2 : -2; yInc = Rnd(0, 1) ? 2 : -2; zInc = Rnd(0, 1);
  count = ((xh - xl - 2) >> 1) * ((yh - yl - 2) >> 1);
  d0 = ms.fRiver ? DIRS : 1;
  x = RndSkip(xl, xh); y = RndSkip(yl, yh);
  UpdateDisplay();
  loop {

    // Hunt mode: Move to a different created wall vertex. Search row by row
    // in opposite directions from top to bottom or reverse, and also search
    // column by column from left to right or reverse, for total balance.
    do {
      if (zInc) {
        if (x + xInc >= xl && x + xInc < xh)
          x += xInc;
        else {
          neg(xInc);
          if (y + yInc >= yl && y + yInc < yh)
            y += yInc;
          else {
            neg(yInc);
            pass++;
            UpdateDisplay();
            if (pass > 1)
              goto LDone;
            if (yInc > 0) {
              inv(zInc);
              x = xl;
            }
          }
        }
      } else {
        if (y + xInc >= yl && y + xInc < yh)
          y += xInc;
        else {
          neg(xInc);
          if (x + yInc >= xl && x + yInc < xh)
            x += yInc;
          else {
            neg(yInc);
            pass++;
            UpdateDisplay();
            if (pass > 1)
              goto LDone;
            if (yInc > 0) {
              inv(zInc);
              y = yl;
            }
          }
        }
      }
    } while (!Get(x, y));

    // Move into any available uncreated vertex from the current vertex.
LNext:
    d = RndDir();
    j = fHunt ? DIRS : d0;
    for (i = 0; i < j; i++) {
      xnew = x + xoff2[d]; ynew = y + yoff2[d];
      if (FLegalMaze2(xnew, ynew) && !Get(xnew, ynew)) {
        if (fCellMax)
          goto LDone;
        fHunt = fFalse;
        x = xnew; y = ynew;
        Set1(xnew - xoff[d], ynew - yoff[d]);
        Set1(xnew, ynew);
        pass = 0;
        count--;
        if (count <= 0)
          goto LDone;
        goto LNext;
      }
      DirInc(d);
    }

    // When entering hunting mode, teleport to one of the edges. This helps
    // increase the number of walls that get attached to edges.
    if (!fHunt) {
      fHunt = fTrue;
      j = Rnd(0, 1);
      if (zInc) {
        if (j)
          x = xl;
        else
          x = xh;
      } else {
        if (j)
          y = yl;
        else
          y = yh;
      }
      xInc = j ? -2 : 2;
    }
  }
LDone:
  return fTrue;
}


// Carve Maze passages into a solid shape on the bitmap, or add walls assuming
// the bitmap is clear, using the Growing Tree algorithm.

flag CMaz::TreeGenerate(flag fWall, int xs, int ys)
{
  PT *rgpt;
  long count, cpt = 1, ipt = 0, iptLo, iptHi;
  int x, y, d;

  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize | fems64K))
    return fFalse;
  count = (((xh - xl) >> 1) - fWall) * (((yh - yl) >> 1) - fWall);
  if (count <= 0)
    return fTrue;
  rgpt = RgAllocate(count, PT);
  if (rgpt == NULL)
    return fFalse;

  // For passage carved Mazes, start with a single cell in the list of cells.
  // For wall added Mazes, start with the outer boundary wall in the list.
  if (!fWall) {
    x = xl + ((xs - xl) | 1); y = yl + ((ys - yl) | 1);
    Set0(x, y);
    PutPt(ipt, x, y);
    count--;
  } else {
    for (x = xl + 2; x < xh; x += 2) {
      PutPt(ipt, x, yl); PutPt(ipt+1, x, yh);
      ipt += 2;
    }
    for (y = yl + 2; y < yh; y += 2) {
      PutPt(ipt, xl, y); PutPt(ipt+1, xh, y);
      ipt += 2;
    }
    cpt = ipt;
    ipt = Rnd(0, cpt-1);
    GetPt(ipt, x, y);
  }

  // Grow into each cell, visiting them in an appropriate random order.
  loop {
    d = DirFindUncreated(&x, &y, fWall);

    // When extending the Maze into a new cell, add the new cell to the list.
    // When nothing can be created from a cell, remove it from the list.
    if (d >= 0) {
      if (fCellMax)
        break;
      Set(x - xoff[d], y - yoff[d], fWall);
      Set(x, y, fWall);
      count--;
      if (count <= 0)  // Done if all cells have been extended into.
        break;
      PutPt(cpt, x, y);
      cpt++;
    } else {
      cpt--;
      if (cpt < 1)  // Done if list becomes empty meaning no cells left.
        break;
      rgpt[ipt] = rgpt[cpt];
    }

    // Figure out which cell to look at next.
    if (ms.fTreeRandom) {
      if (Rnd(0, ms.nTreeRiver) == 0)
        ipt = Rnd(0, cpt-1);           // Use a random cell.
      else
        ipt = cpt-1;                   // Use the most recent cell.
    } else {
      if (ms.nTreeRiver >= 0) {
        iptLo = cpt-1 - ms.nTreeRiver;
        if (iptLo < 0)
          iptLo = 0;
        iptHi = cpt-1;                 // Use one of the most recent cells.
      } else {
        iptLo = 0;
        iptHi = -(ms.nTreeRiver + 1);  // Use one of the oldest cells.
        if (iptHi >= cpt)
          iptHi = cpt-1;
      }
      ipt = Rnd(iptLo, iptHi);
    }
    GetPt(ipt, x, y);
  }
  DeallocateP(rgpt);
  return fTrue;
}


// Create a new perfect Maze in the bitmap using the Growing Tree algorithm.
// This can carve passages or add walls.

flag CMaz::CreateMazeTree()
{
  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize | femsMinSize | fems64K))
    return fFalse;
  MazeClear(!ms.fTreeWall);
  MakeEntranceExit(0);
  UpdateDisplay();
  return TreeGenerate(ms.fTreeWall, Rnd(xl, xh-1), Rnd(yl, yh-1));
}


typedef struct _fors { // Forest Struct
  PT pt;
  long ipt;
  KRUS krus;
} FORS;

#define FZ(x, y) ((y) * xs + (x))
#define ForsSwap(i1, i2) \
  ptT = fors[i1].pt; fors[i1].pt = fors[i2].pt; fors[i2].pt = ptT; \
  fors[FZ(fors[i1].pt.x, fors[i1].pt.y)].ipt = i1; \
  fors[FZ(fors[i2].pt.x, fors[i2].pt.y)].ipt = i2;
#define ForsUnion(x1, y1, x2, y2) \
  KruskalUnion(&fors[FZ(x1, y1)].krus, &fors[FZ(x2, y2)].krus); cIsland--;
#define ForsFind(x1, y1, x2, y2) \
  (KruskalFind(&fors[FZ(x1, y1)].krus) == KruskalFind(&fors[FZ(x2, y2)].krus))

// Create a new perfect Maze in the bitmap using the Growing Forest algorithm.
// This can carve passages or add walls.

flag CMaz::CreateMazeForest()
{
  FORS *fors;
  PT ptT;
  long count, cpt, ipt, iptT, iptLo, iptHi, iptDun = 0, cIsland;
  int rgdir[DIRS], rgfNew[DIRS], xs, ys, xb, yb, xp, yp, x, y, d, id, cdir;
  flag fWall = ms.fTreeWall, fNew;

  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize | femsMinSize | fems64K))
    return fFalse;
  MazeClear(!ms.fTreeWall);
  MakeEntranceExit(0);
  UpdateDisplay();

  xs = ((xh - xl) >> 1) + fWall; ys = ((yh - yl) >> 1) + fWall;
  count = cIsland = xs * ys;
  if (count <= 0)
    return fTrue;
  fors = RgAllocate(count, FORS);
  if (fors == NULL)
    return fFalse;

  // Compose list of cells, then shuffle them randomly.
  ipt = 0;
  for (y = 0; y < ys; y++)
    for (x = 0; x < xs; x++) {
      fors[ipt].pt.x = x; fors[ipt].pt.y = y;
      fors[ipt].ipt = ipt++;
    }
  for (ipt = 0; ipt < count; ipt++) {
    iptT = Rnd(0, count-1);
    ForsSwap(ipt, iptT);
    fors[ipt].krus.next = &fors[ipt].krus;
    fors[ipt].krus.count = 1;
  }

  // For passage carved Mazes, start with a single cell in the list of cells.
  // For wall added Mazes, start with the outer boundary wall in the list.
  xb = xl + !fWall; yb = yl + !fWall;
  if (!fWall) {
    cpt = 1;
    xp = Rnd(xl, xh-1); yp = Rnd(yl, yh-1);
    x = (xp - xl) >> 1; y = (yp - yl) >> 1;
    Set0(xb + (x << 1), yb + (y << 1));
    ForsSwap(fors[FZ(x, y)].ipt, 0);
  } else {
    cpt = 0;
    for (x = 0; x < xs; x++) {
      ForsSwap(fors[FZ(x, 0)].ipt, cpt);
      ForsSwap(fors[FZ(x, ys-1)].ipt, cpt+1);
      cpt += 2;
      if (x > 0) {
        ForsUnion(0, 0, x, 0);
      }
      ForsUnion(0, 0, x, ys-1);
    }
    for (y = 1; y < ys-1; y++) {
      ForsSwap(fors[FZ(0, y)].ipt, cpt);
      ForsSwap(fors[FZ(xs-1, y)].ipt, cpt+1);
      cpt += 2;
      ForsUnion(0, 0, 0, y);
      ForsUnion(0, 0, xs-1, y);
    }
    ipt = Rnd(0, cpt-1);
    x = fors[ipt].pt.x; y = fors[ipt].pt.y;
  }

  // Start with a certain number of additional cell sets.
  iptHi = NMin(ms.nForsInit >= 0 ? ms.nForsInit-1 : count / -ms.nForsInit,
    count - cpt);
  for (ipt = 0; ipt < iptHi; ipt++) {
    iptT = Rnd(cpt, count-1);
    x = fors[iptT].pt.x; y = fors[iptT].pt.y;
    Set(xb + (x << 1), yb + (y << 1), fWall);
    ForsSwap(iptT, cpt);
    cpt++;
  }
  UpdateDisplay();

  // Expand into each cell, visiting them in an appropriate random order.
  loop {

    // Figure out how many passages lead from this cell.
    cdir = 0;
    for (d = 0; d < DIRS; d++) {
      xp = x + xoff[d]; yp = y + yoff[d];
      if (xp < 0 || yp < 0 || xp >= xs || yp >= ys)
        continue;
      fNew = Get(xb + (xp << 1), yb + (yp << 1)) ^ fWall;
      if (fNew || (ms.fRiverFlow && !ForsFind(x, y, xp, yp))) {
        rgdir[cdir] = d;
        rgfNew[cdir] = fNew;
        cdir++;
      }
    }

    // When extending the Maze into a new cell, add the new cell to the list.
    if (cdir > 0) {
      if (fCellMax)
        goto LExit;
      id = Rnd(0, cdir-1);
      d = rgdir[id]; fNew = rgfNew[id];
      xp = x + xoff[d]; yp = y + yoff[d];
      ForsUnion(x, y, xp, yp);
      Set(xb + (x << 1) + xoff[d], yb + (y << 1) + yoff[d], fWall);
      x = xp; y = yp;
      if (fNew) {
        Set(xb + (x << 1), yb + (y << 1), fWall);
        ForsSwap(fors[FZ(x, y)].ipt, cpt);
        cpt++;
        if (cpt >= count)  // Done if all cells have been extended into.
          break;

        // Periodically add new cell island(s) to the list.
        iptHi = ms.nForsAdd >= 0 ? ms.nForsAdd : cpt % -ms.nForsAdd == 0;
        for (ipt = 0; ipt < iptHi; ipt++) {
          iptT = Rnd(cpt, count-1);
          x = fors[iptT].pt.x; y = fors[iptT].pt.y;
          Set(xb + (x << 1), yb + (y << 1), fWall);
          ForsSwap(iptT, cpt);
          cpt++;
        }
      }

    // When nothing can be created from a cell, remove it from the list.
    } else {
      ForsSwap(ipt, iptDun);
      iptDun++;
      if (cpt <= iptDun)  // Done if list becomes empty meaning no cells left.
        break;
    }

    // Figure out which cell to look at next.
    if (ms.fTreeRandom) {
      if (Rnd(0, ms.nTreeRiver) == 0)
        ipt = Rnd(iptDun, cpt-1);      // Use a random cell.
      else
        ipt = cpt-1;                   // Use the most recent cell.
    } else {
      if (ms.nTreeRiver >= 0) {
        iptLo = cpt-1 - ms.nTreeRiver;
        if (iptLo < iptDun)
          iptLo = iptDun;
        iptHi = cpt-1;                 // Use one of the most recent cells.
      } else {
        iptLo = iptDun;
        iptHi = iptDun - (ms.nTreeRiver + 1);  // Use one of the oldest cells.
        if (iptHi >= cpt)
          iptHi = cpt-1;
      }
      ipt = Rnd(iptLo, iptHi);
    }
    x = fors[ipt].pt.x; y = fors[ipt].pt.y;
  }
  UpdateDisplay();

  // Merge remaining cell sets together. This process is effectively Kruskal's
  // algorithm, except the sets unioned are of cells instead of edges.
  iptT = Rnd(0, count-1);
  cpt = LPrime(count);
  for (ipt = 0; ipt < count; ipt++) {
    iptT += cpt;
    while (iptT >= count)
      iptT -= count;
    x = fors[iptT].pt.x; y = fors[iptT].pt.y;
    d = Rnd(0, DIRS1);
    for (id = 0; id < DIRS; id++) {
      DirInc(d);
      xp = x + xoff[d]; yp = y + yoff[d];
      if (xp < 0 || yp < 0 || xp >= xs || yp >= ys)
        continue;
      if (!ForsFind(x, y, xp, yp)) {
        if (fCellMax)
          goto LExit;
        Set(xb + (x << 1) + xoff[d], yb + (y << 1) + yoff[d], fWall);
        ForsUnion(x, y, xp, yp);
        if (cIsland <= 1)
          goto LExit;
      }
    }
  }

LExit:
  DeallocateP(fors);
  return fTrue;
}


// Create a new perfect Maze in the bitmap using the Aldous-Broder algorithm.
// This can carve passages or add walls. This is the simplest unbiased
// algorithm for creating perfect Mazes. Here's an obfuscated passage carving
// smaller version that only takes up 206 bytes when placed all on one line:
//
// int p[1841],z=82,c=428;main(n){for(srand(time(0));(p[2]=p[1838]=p[z]=1)&&
// c;)if((n=z+((n=rand())&1?2:160)*((n&2)-1))>1&&n<1841&&n%80)
// p[n+z>>1]|=!p[n],c-=!p[z=n];for(z=0;++z<1841;)putchar(z%80?35-3*p[z]:10);}

flag CMaz::CreateMazeAldousBroder()
{
  int x, y, xnew, ynew, d;
  long count;
  flag fWall = ms.fTreeWall;

  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize | femsMinSize))
    return fFalse;
  MazeClear(!fWall);
  MakeEntranceExit(0);
  if (!fWall) {
    x = ms.fTreeRandom ? RndSkip(xl + 1, xh - 1) : xl + 1;
    y = ms.fTreeRandom ? RndSkip(yl + 1, yh - 1) : yl + 1;
    Set0(x, y);
  } else
    x = y = 0;
  count = (((xh - xl) >> 1) - fWall) * (((yh - yl) >> 1) - fWall) - !fWall;
  if (count <= 0)
    return fTrue;
  UpdateDisplay();

  // Randomly walk around on the Maze until all cells have been visited.
  loop {
    d = RndDir();
    xnew = x + xoff2[d]; ynew = y + yoff2[d];
    if (!fWall) {
      if (!FLegalMaze2(xnew, ynew))
        continue;
    } else {

      // For wall added Mazes, when run into the boundary wall, teleport to a
      // random boundary wall vertex. The boundary wall should be treated like
      // one big vertex to create all Mazes with equal probability.
      if (xnew <= xl || xnew >= xh || ynew <= yl || ynew >= yh) {
        if (Rnd(0, (xh - xl) + (yh - yl) - 4) < (xh - xl) - 2) {
          x = RndSkip(xl + 2, xh - 2);
          y = Rnd(0, 1) ? yl : yh;
        } else {
          x = Rnd(0, 1) ? xl : xh;
          y = RndSkip(yl + 2, yh - 2);
        }
        continue;
      }
    }

    // When moving to an unvisited cell, extend the Maze into it.
    if (Get(xnew, ynew) != fWall) {
      if (fCellMax)
        break;
      Set((x + xnew) >> 1, (y + ynew) >> 1, fWall);
      Set(xnew, ynew, fWall);
      count--;
      if (count <= 0)
        break;
    }
    x = xnew; y = ynew;
  }
  return fTrue;
}


typedef struct _wilson {
  long zList;
  long iBack;
  long dir;
} WILS;

#define AssignWils(iTo, iFrom) \
  wils[iTo].zList = wils[iFrom].zList; \
  wils[wils[iTo].zList].iBack = iTo

// Create a new perfect Maze in the bitmap using Wilson's algorithm. This can
// carve passages or add walls. Like the Aldous-Broder algorithm, this
// generates all possible Mazes with equal probability, however this runs
// about five times faster on average.

flag CMaz::CreateMazeWilson()
{
  WILS *wils;
  int xbase, ybase, x, y, xs, ys, x0, y0, xnew, ynew, d;
  long count, i;
  flag fWall = ms.fTreeWall;

  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize | femsMinSize))
    return fFalse;
  xs = ((xh - xl) >> 1) + fWall; ys = ((yh - yl) >> 1) + fWall;
  wils = RgAllocate(xs*ys, WILS);
  if (wils == NULL)
    return fFalse;
  for (i = xs*ys-1; i >= 0; i--) {
    wils[i].zList = wils[i].iBack = i;
    wils[i].dir = -2;
  }
  MazeClear(!fWall);
  MakeEntranceExit(0);
  count = xs * ys;
  xbase = xl + !fWall; ybase = yl + !fWall;

  // For passage carved Mazes, start with a single cell. For wall added Mazes,
  // start with the outer boundary wall.
  if (!fWall) {
    x = Rnd(0, xs-1); y = Rnd(0, ys-1);
    i = y * xs + x;
    AssignWils(i, --count);
    wils[i].dir = -1;
    Set0(xbase + (x << 1), ybase + (y << 1));
  } else {
    for (x = 0; x < xs; x++) {
      i = x;
      wils[i].dir = -1; i = wils[i].iBack; AssignWils(i, --count);
      i = (ys - 1) * xs + x;
      wils[i].dir = -1; i = wils[i].iBack; AssignWils(i, --count);
    }
    for (y = 1; y < ys-1; y++) {
      i = y * xs;
      wils[i].dir = -1; i = wils[i].iBack; AssignWils(i, --count);
      i = y * xs + (xs - 1);
      wils[i].dir = -1; i = wils[i].iBack; AssignWils(i, --count);
    }
  }
  UpdateDisplay();

  while (count > 0) {
    i = ms.fTreeRandom ? Rnd(0, count-1) : count-1;
    x = x0 = wils[i].zList % xs; y = y0 = wils[i].zList / xs;

    // From a random uncreated location, do a random walk until run into part
    // of the Maze that's already been created, remembering the path taken.
    loop {
      d = RndDir();
      xnew = x + xoff[d]; ynew = y + yoff[d];
      if (xnew < 0 || xnew >= xs || ynew < 0 || ynew >= ys)
        continue;
      wils[y * xs + x].dir = d;
      if (wils[ynew * xs + xnew].dir == -1)
        break;
      x = xnew; y = ynew;
    }

    // Draw along the path taken, skipping any loops so a single line gets
    // attached to the Maze, that starts from the uncreated location above.
    if (fCellMax)
      break;
    x = x0; y = y0;
    loop {
      i = y * xs + x;
      d = wils[i].dir;
      if (d == -1)
        break;
      Set(xbase + (x << 1), ybase + (y << 1), fWall);
      Set(xbase + (x << 1) + xoff[d], ybase + (y << 1) + yoff[d], fWall);
      wils[i].dir = -1; i = wils[i].iBack; AssignWils(i, --count);
      x += xoff[d]; y += yoff[d];
    }
  }
  DeallocateP(wils);
  return fTrue;
}


// Carve one row of a Maze using Eller's algorithm, adding onto the Maze and
// updating the sets the cells in the current row are within appropriately.

void CMaz::EllerMakeRow(long *nL, long *nR,
  int xs, int xinc1, int xinc2, int yp, int yinc, flag fFinal)
{
  int x, xold;

  // Carve horizontal passage segments at current row.
  if (xs > 1) {
    xold = x = Rnd(0, xs-2);
    do {
      Set0(xl + (x << 1) + 1, yp);
      if ((fFinal || Rnd(1, 5) > 2) && nR[x] != x+1) {

        // Merge two sets together.
        nL[nR[x]] = nL[x+1]; nR[nL[x+1]] = nR[x];
        nR[x] = x+1; nL[x+1] = x;
        Set0(xl + (x << 1) + 2, yp);
      }
      x += xinc1;
      while (x >= xs-1)
        x -= (xs-1);
    } while (x != xold);
  }
  Set0(xh-1-FOdd(xh), yp);
  if (fFinal)
    return;

  // Carve vertical passage segments from current row.
  xold = x = Rnd(0, xs-1);
  do {
    if (nR[x] != x && Rnd(1, 5) > 2) {

      // Put element in set by itself, by removing it from linked list.
      nL[nR[x]] = nL[x]; nR[nL[x]] = nR[x];
      nL[x] = nR[x] = x;
    } else
      Set0(xl + (x << 1) + 1, yp + yinc);
    x += xinc2;
    while (x >= xs)
      x -= xs;
  } while (x != xold);
}


// Create a new perfect Maze in the bitmap using Eller's algorithm. This can
// carve passages or add walls. This is the fastest algorithm for creating
// general perfect Mazes, and runs over twice as fast as any of the others.

flag CMaz::CreateMazeEller()
{
  long rgn[zBitmapStack], *pn = NULL, *nL, *nR;
  int xs, ys, x, yp, xold, xinc1, xinc2, xloop;
  flag fFinal;

  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize | femsMinSize))
    return fFalse;
  MazeClear(!ms.fTreeWall);
  MakeEntranceExit(0);
  UpdateDisplay();
  xs = (xh - xl) >> 1; ys = (yh - yl) >> 1;
  if (ms.fTreeWall) {
    xs++; ys++; yp = yl+2;
  } else
    yp = yh-1;

  // Use sets to determine whether cells are linked with each other. Each set
  // is defined by a doubly linked circular list, where each cell has left and
  // right pointers to the cells on either side of it in the set. Eller's
  // algorithm doesn't require arbitrary set comparison, but rather only needs
  // to check adjacent cells for whether they're in the same set. Hence linked
  // lists are the fastest method possible, because comparisons, insertions,
  // and deletions all take O(1) constant time, no matter the size of the set.

  if (xs*2 <= zBitmapStack)
    nL = rgn;
  else {
    pn = RgAllocate(xs*2, long);
    if (pn == NULL)
      return fFalse;
    nL = pn;
  }
  nR = nL + xs;
  x = xs-1;              xinc1 = LPrime(x);
  x = xs-2*ms.fTreeWall; xinc2 = LPrime(x);

  // Create one row of the Maze at a time in sequence.
  if (!ms.fTreeWall) {

    // Passage carve case: Start with each element in its own set.
    for (x = 0; x < xs; x++)
      nL[x] = nR[x] = x;
    loop {
      fFinal = (yp <= yl+1);
      EllerMakeRow(nL, nR, xs, xinc1, xinc2, yp, -1, fFinal);
      if (fFinal)
        break;
      yp -= 2;
    }
    goto LDone;
  }

  // Wall add case: Start with vertical wall segments from top row.
  if (yp >= yh-2)
    goto LDone;
  xold = 0;
  for (x = 1; x < xs-1; x++) {
    if (Rnd(1, 3) > 1)
      nL[x] = nR[x] = x;
    else {
      nR[xold] = x; nL[x] = xold;
      xold = x;
      Set1(xl + (x << 1), yl + 1);
    }
    Set1(xl + (x << 1), yl + 2);
  }
  nR[xold] = xs-1; nL[xs-1] = xold;
  nR[xs-1] = 0; nL[0] = xs-1;

  loop {
    // Add horizontal wall segments in current row.
    xold = x = Rnd(0, xs-2);
    neg(xinc1);
    do {
      if (Rnd(1, 2) > 1 && nR[x] != x+1) {

        // Merge two sets together.
        nL[nR[x]] = nL[x+1]; nR[nL[x+1]] = nR[x];
        nR[x] = x+1; nL[x+1] = x;
        Set1(xl + (x << 1) + 1, yp);
      }
      x += xinc1;
      while (x >= xs-1)
        x -= (xs-1);
      while (x < 0)
        x += (xs-1);
    } while (x != xold);
    if (yp >= yh-3)
      break;

    // Add vertical wall segments from current row.
    yp += 2;
    if (xs <= 2)
      continue;
    xold = x = Rnd(1, xs-2);
    neg(xinc2);
    do {
      if (nR[x] != x && Rnd(1, 5) > 1) {

        // Put element in set by itself, by removing it from linked list.
        nL[nR[x]] = nL[x]; nR[nL[x]] = nR[x];
        nL[x] = nR[x] = x;
      } else
        Set1(xl + (x << 1), yp-1);
      Set1(xl + (x << 1), yp);
      x += xinc2;
      while (x >= xs-1)
        x -= (xs-2);
      while (x <= 0)
        x += (xs-2);
    } while (x != xold);
  }

  // Add vertical wall segments from bottom row.
  for (xloop = 0; (xold = nR[xloop]) != 0; xloop = xold)
    nL[xloop] = -1; // nR[xloop] = -1;
  nL[xloop] = -1; // nR[xloop] = -1;
  for (x = 1; x < xs-1; x++) {
    if (nL[x] < 0)
      continue;
    if (nR[x] == x || Rnd(1, 5) == 1) {

      // Erase rest of set so it doesn't get connected to bottom edge twice.
      for (xloop = x; (xold = nR[xloop]) != x; xloop = xold)
        nL[xloop] = -2; // nR[xloop] = -2;
      nL[xloop] = -2; // nR[xloop] = -2;
      Set1(xl + (x << 1), yp+1);
    } else {

      // Remove element from linked list, decreasing size of rest of set.
      nL[nR[x]] = nL[x]; nR[nL[x]] = nR[x];
      // nL[x] = nR[x] = x;
    }
  }

LDone:
  if (pn != NULL)
    DeallocateP(pn);
  return fTrue;
}


// Carve one row of a braid Maze using a variation of Eller's algorithm,
// adding onto the Maze and updating the sets of the cells in the current row.

void CMaz::BraidMakeRow(long *nL, long *nR,
  int xs, int xinc1, int xinc2, int yp, int yinc, int yLeft)
{
  int x, xold, xp;

  // Carve horizontal passage segments at current row.
  if (xs > 1) {
    xold = x = Rnd(0, xs-2);
    do {
      xp = xl + (x << 1) + 1;
      Set0(xp, yp);
      if ((nR[x] == x && Count(xp, yp) > 3) ||
        (yLeft < 1 && (nR[x] != x+1 || x == xs-2)) ||
        (x == xs-2 && nR[x+1] == x+1 && Count(xp+2, yp) > 2) ||
        (Rnd(1, 5) > 3 && Count(xp+1, yp - yinc) > 1)) {

        // Carve horizontal if:
        // 1: Passage is isolated cell, to start connecting it.
        // 2: If on last row and cells are disconnected, to avoid isolation.
        // 3: If last column would create an isolation.
        // 4: If doing so won't create a pole.
        if (nR[x] != x+1) {

          // Merge two sets together.
          nL[nR[x]] = nL[x+1]; nR[nL[x+1]] = nR[x];
          nR[x] = x+1; nL[x+1] = x;
        }
        Set0(xp+1, yp);
      }
      x += xinc1;
      while (x >= xs-1)
        x -= (xs-1);
    } while (x != xold);
  }
  Set0(xh-1, yp);

  // Remove dead ends on last row.
  if (yLeft < 1) {
    for (x = 0; x < xs-1; x++) {
      xp = xl + (x << 1) + 1;
      if (Count(xp, yp) > 2)
        Set0(xp + (Get(xp+1, yp) || x < 1 ? 1 : -1), yp);
    }
    return;
  }

  // Carve vertical passage segments from current row.
  xold = x = Rnd(0, xs-1);
  do {
    xp = xl + (x << 1) + 1;
    if (nR[x] == x ||
      Count(xp, yp) >= 3 ||
      (yLeft == 1 && (x == 0 || x == xs-1)) /*|| Rnd(1, 5) > 5*/) {

      // Carve vertical if:
      // 1: Passage is sole outlet, to avoid creating isolation.
      // 2: Passage has no junctions, to avoid creating dead end.
      // 3: If next to last row and at sides of Maze, to avoid dead ends.
      Set0(xp, yp + yinc);
    } else {

      // Put element in set by itself, by removing it from linked list.
      nL[nR[x]] = nL[x]; nR[nL[x]] = nR[x];
      nL[x] = nR[x] = x;
    }
    x += xinc2;
    while (x >= xs)
      x -= xs;
  } while (x != xold);
}


// Create a new braid Maze in a bitmap using a variation of Eller's Algorithm.

flag CMaz::CreateMazeBraidEller()
{
  long rgn[zBitmapStack], *pn = NULL, *nL, *nR;
  int xs, ys, x, yp, xinc1, xinc2, yLeft;

  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize | femsMinSize))
    return fFalse;
  MazeClear(fOn);
  MakeEntranceExit(0);
  UpdateDisplay();
  xs = (xh - xl) >> 1; ys = (yh - yl) >> 1; yp = yh-1;

  if (xs*2 <= zBitmapStack)
    nL = rgn;
  else {
    pn = RgAllocate(xs*2, long);
    if (pn == NULL)
      return fFalse;
    nL = pn;
  }
  nR = nL + xs;
  x = xs-1; xinc1 = LPrime(x);
  x = xs;   xinc2 = LPrime(x);

  // Start with each element in its own set.
  for (x = 0; x < xs; x++)
    nL[x] = nR[x] = x;

  // Create one row of the Maze at a time in sequence.
  loop {
    yLeft = (yp - (yl+1)) >> 1;
    BraidMakeRow(nL, nR, xs, xinc1, xinc2, yp, -1, yLeft);
    if (yLeft <= 0)
      break;
    yp -= 2;
  }

  if (pn != NULL)
    DeallocateP(pn);
  return fTrue;
}


// Create a new perfect Maze in the bitmap using recursive division. This
// always adds walls, recursively dividing the Maze into smaller rectangles.

flag CMaz::CreateMazeDivision()
{
  RC *rgrect, rect, *pr;
  int x, y;
  flag f;

  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize | femsMinSize))
    return fFalse;
  rgrect = RgAllocate(((xh - xl) + (yh - yl)) >> 1, RC);
  if (rgrect == NULL)
    return fFalse;

  // Start with one rectangle in stack covering the entire Maze.
  rect.x1 = xl; rect.y1 = yl; rect.x2 = xh; rect.y2 = yh;
  pr = rgrect;
  *pr = rect;
  MazeClear(fOff);
  MakeEntranceExit(0);
  UpdateDisplay();

  // Continue while there's at least one rectangle on the stack.
  while (pr >= rgrect) {
    rect = *pr;
    if (rect.x2 - rect.x1 < 4 || rect.y2 - rect.y1 < 4) {
      pr--;
      continue;
    }
    if (fCellMax)
      break;

    // Determine whether to divide current rectangle horizontal or vertical.
    if (ms.nRndRun)
      f = Rnd(-50, 49) >= ms.nRndBias;
    else
      f = Rnd(0, (rect.x2 - rect.x1) + (rect.y2 - rect.y1)) + ms.nRndBias <=
        (rect.x2 - rect.x1);

    // Divide current rectangle and push two subrectangles on stack.
    if (f) {
      x = RndSkip(rect.x1 + 2, rect.x2 - 2);
      LineY(x, rect.y1+1, rect.y2-1, fOn);
      Set0(x, RndSkip(rect.y1 + 1, rect.y2 - 1));
      pr->x1 = x; pr->y1 = rect.y1; pr->x2 = rect.x2; pr->y2 = rect.y2;
      pr++;
      pr->x1 = rect.x1; pr->y1 = rect.y1; pr->x2 = x; pr->y2 = rect.y2;
    } else {
      y = RndSkip(rect.y1 + 2, rect.y2 - 2);
      LineX(rect.x1+1, rect.x2-1, y, fOn);
      Set0(RndSkip(rect.x1 + 1, rect.x2 - 1), y);
      pr->x1 = rect.x1; pr->y1 = y; pr->x2 = rect.x2; pr->y2 = rect.y2;
      pr++;
      pr->x1 = rect.x1; pr->y1 = rect.y1; pr->x2 = rect.x2; pr->y2 = y;
    }
  }
  DeallocateP(rgrect);
  return fTrue;
}


// Create a new perfect Maze in the bitmap using the Binary Tree algorithm.
// This can carve passages or add walls. This is the simplest algorithm of any
// type for creating perfect Mazes. Here's an obfuscated smaller version that
// only takes up 115 bytes when placed all on one line:
//
// main(z){for(srand(time(z=0));++z<598;)printf(z%25?((z+1)%25<3||rand()&1&
// z>25)&z<575?"# ":"~~":"\n");printf("~ ~");}

flag CMaz::CreateMazeBinary()
{
  int nPercent = 50 + ms.nRndBias, zInc = NAbs(ms.cRandomAdd),
    nDir, xInc, iMax, x, y, i, d, xT, yT;
  flag fWall = ms.fTreeWall, fFade = !FBetween(ms.nRndBias, -50, 50);

  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize | femsMinSize))
    return fFalse;
  MazeClear(!fWall);
  if (!fWall)
    LineX(xl+1, xh-1, yl+1, fOff);
  MakeEntranceExit(0);
  UpdateDisplay();
  nDir = ms.nRndRun <= 0 ? -1 : ms.nRndRun >= 100;
  iMax = (xh - xl) >> 1;

  if (!fWall) {
    for (y = yh - 1; y >= yl + 2; y -= 2) {
      xInc = (nDir < 0 || (nDir == 0 && Rnd(0, 99) < ms.nRndRun)) ? -1 : 1;
      x = xInc < 0 ? xh - 1 : xl + 1;
      for (i = 1; i < iMax; i++) {

        // For each cell, carve a passage up or left/right, but not both.
        if (fCellMax)
          break;
        Set0(x, y);
        if (fFade) {
          xT = x - xl; yT = y - yl;
          if (ms.nRndBias < 0) {
            xT = xh - xl - xT + zInc; yT = yh - yl - yT + zInc;
          }
          if (xT > yT)
            nPercent = 100 - NMultDiv(yT, 50, xT);
          else
            nPercent = NMultDiv(xT, 50, yT);
        }
        d = Rnd(0, 99) < nPercent;
        Set0(x + d*xInc, y - (d ^ 1));
        x += (xInc << 1);
      }
      Set0(x, y); Set0(x, y-1);
    }
  } else {
    for (y = yl + 2; y < yh - 1; y += 2) {
      xInc = (nDir < 0 || (nDir == 0 && Rnd(0, 99) < ms.nRndRun)) ? -1 : 1;
      x = xInc < 0 ? xh - 2 : xl + 2;
      for (i = 1; i < iMax; i++) {

        // For each wall vertex, add segment down or right/left, but not both.
        if (fCellMax)
          break;
        Set1(x, y);
        if (fFade) {
          xT = x - xl; yT = y - yl;
          if (ms.nRndBias < 0) {
            xT = xh - xl - xT + zInc; yT = yh - yl - yT + zInc;
          }
          if (xT > yT)
            nPercent = 100 - NMultDiv(yT, 50, xT);
          else
            nPercent = NMultDiv(xT, 50, yT);
        }
        d = Rnd(0, 99) < nPercent;
        Set1(x - d*xInc, y + (d ^ 1));
        x += (xInc << 1);
      }
    }
  }
  return fTrue;
}


// Create a new perfect Maze in the bitmap using the Sidewinder algorithm.
// This can carve passages or add walls.

flag CMaz::CreateMazeSidewinder()
{
  int nPercent = 50 + ms.nRndBias, x, y, dx;
  flag fWall = ms.fTreeWall;

  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize | femsMinSize))
    return fFalse;
  MazeClear(!fWall);
  if (!fWall)
    LineX(xl+1, xh-1, yl+1, fOff);
  MakeEntranceExit(0);
  UpdateDisplay();

  if (!fWall) {
    for (y = yl + 3; y < yh; y += 2)
      for (x = xl + 1; x < xh; x += 2) {

        // For each cell, decide whether to carve a passage right.
        dx = 0;
        while (x <= xh - 3 && Rnd(0, 99) < nPercent)
          dx++, x += 2;
        if (fCellMax)
          break;

        // If no passage right, then carve a randomly positioned passage up.
        if (dx == 0)
          LineY(x, y-1, y, fOff);
        else {
          Set0(x - (Rnd(0, dx) << 1), y - 1);
          LineX(x - (dx << 1), x, y, fOff);
        }
      }
  } else {
    for (y = yh - 2; y > yl; y -= 2)
      for (x = xl; x < xh; x += 2) {

        // For each wall vertex, decide whether to add a segment right.
        dx = 0;
        while (x < xh && Rnd(0, 99) < nPercent)
          dx++, x += 2;
        if (fCellMax)
          break;

        // If no segment right, then add a randomly positioned segment down.
        if (dx == 0) {
          if (x > xl)
            LineY(x, y+1, y, fOn);
        } else {
          if (x > xl + (dx << 1) && x < xh)
            Set1(x - (Rnd(0, dx) << 1), y + 1);
          LineX(x - (dx << 1), x, y, fOn);
          if (x >= xh && dx >= (xh - xl) >> 1)
            Set0(RndSkip(xl + 1, xh - 1), y);
        }
      }
  }
  return fTrue;
}

/* create.cpp */
