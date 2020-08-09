/*
** Daedalus (Version 3.3) File: solve.cpp
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
** This file contains Maze solving algorithms.
**
** Created: 9/4/2000.
** Last code change: 11/29/2018.
*/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "util.h"
#include "graphics.h"
#include "color.h"
#include "threed.h"
#include "maze.h"


/*
******************************************************************************
** Maze Solving Routines
******************************************************************************
*/

// Solve a Maze by filling in all dead ends, including passages that become
// dead ends once other dead ends are filled.

long CMaz::SolveMazeFillDeadEnds(int xa, int ya, int xb, int yb)
{
  int x, y, x2, y2, zInc = 2 - ms.fSolveEveryPixel, d;
  long count = 0;
  flag fAny;

  fAny = ms.fSolveDotExit && (FLegalOff(xa, ya) || FLegalOff(xb, yb));
  for (y = xl + 1; y < yh; y += zInc)
    for (x = xl + 1; x < xh; x += zInc)
      if (!Get(x, y) && Count(x, y) >= DIRS1) {
        if (fAny && ((x == xa && y == ya) || (x == xb && y == yb)))
          continue;
        if (fCellMax)
          return count;
        count++;
        x2 = x; y2 = y;

        // For each dead end, fill back to a junction in the current Maze.
        do {
          Set1(x2, y2);
          for (d = 0; d < DIRS && Get(x2 + xoff[d], y2 + yoff[d]); d++)
            ;
          if (d >= DIRS)
            break;
          x2 += xoff[d]; y2 += yoff[d];
          if (!ms.fSolveEveryPixel) {
            Set1(x2, y2);
            x2 += xoff[d]; y2 += yoff[d];
          }
        } while (FLegalMaze(x2, y2) && Count(x2, y2) == DIRS1);
      }
  return count;
}


// Partially solve a Maze by filling in the current set of dead end passages.
// This does not fill passages that become dead ends once other dead ends are
// filled. Calling this until no dead ends are left is the same as calling
// SolveMazeFillDeadEnds() once.

long CMaz::DoFillDeadEnds(int xa, int ya, int xb, int yb)
{
  CMaz bCopy;
  int x, y, x2, y2, zInc = 2 - ms.fSolveEveryPixel, d;
  long count = 0;
  flag fAny;

  if (!bCopy.FBitmapCopy(*this))
    return -1;
  fAny = ms.fSolveDotExit && (FLegalOff(xa, ya) || FLegalOff(xb, yb));
  for (y = yl + zInc - 1; y <= yh; y += zInc)
    for (x = xl + zInc - 1; x <= xh; x += zInc)
      if (!Get(x, y) && bCopy.Count(x, y) >= DIRS1) {
        if (fAny && ((x == xa && y == ya) || (x == xb && y == yb)))
          continue;
        if (fCellMax)
          goto LDone;
        count++;
        x2 = x; y2 = y;

        // For each dead end, fill back to a junction in the original Maze.
        do {
          Set1(x2, y2);
          for (d = 0; d < DIRS && Get(x2 + xoff[d], y2 + yoff[d]); d++)
            ;
          if (d >= DIRS)
            break;
          x2 += xoff[d]; y2 += yoff[d];
          if (zInc >= 2) {
            Set1(x2, y2);
            x2 += xoff[d]; y2 += yoff[d];
          }
        } while (FLegalMaze2(x2, y2) && bCopy.Count(x2, y2) >= 2);
      }
LDone:
  return count;
}


// Partially solve a Maze by filling in cells at the ends of dead ends. Each
// time this is called, dead ends get slightly shorter. Calling this until
// all dead ends are gone is the same as calling SolveMazeFillDeadEnds() once.

long CMaz::DoMarkDeadEnds(int xa, int ya, int xb, int yb)
{
  int x, y, f, zInc = 2 - ms.fSolveEveryPixel;
  long count = 0;
  flag fAny;

  // Loop over each cell, but in two opposite checkerboard patterns, so as not
  // to check adjacent cells in sequence. This ensures gradual progress, and
  // avoids filling in long straight dead ends all at once.

  fAny = ms.fSolveDotExit && (FLegalOff(xa, ya) || FLegalOff(xb, yb));
  for (f = 0; f <= 1; f++)
    for (y = yl + 1; y < yh; y += zInc)
      for (x = xl + 1; x < xh; x += zInc)
        if ((((x >> 1) + (y >> 1)) & 1) == f &&
          !Get(x, y) && Count(x, y) >= DIRS1) {
          if (fAny && ((x == xa && y == ya) || (x == xb && y == yb)))
            continue;
          if (fCellMax)
            return count;
          count++;
          Set1(x, y);
          Set1(x-1, y); Set1(x, y-1); Set1(x+1, y); Set1(x, y+1);
        }
  return count;
}


// Convert all cul-de-sac or noose passages into dead ends.

long CMaz::DoMarkCulDeSacs(int xa, int ya, int xb, int yb)
{
  int x, y, x2, y2, d, dnew, zInc = 2 - ms.fSolveEveryPixel, i, j;
  long count = 0;
  flag fAny;

  fAny = ms.fSolveDotExit && (FLegalOff(xa, ya) || FLegalOff(xb, yb));
  for (y = yl + 1; y < yh; y += zInc)
    for (x = xl + 1; x < xh; x += zInc)
      if (!Get(x, y) && Count(x, y) <= 1) {

        // Check an individual junction and see if it's part of a cul-de-sac.
        for (i = 0; i < DIRS; i++) {
          if (!Get(x + xoff[i], y + yoff[i])) {

            // Follow an individual path and see if it reconnects.
            d = i;
            x2 = x + xoff[d]; y2 = y + yoff[d];
            if (!ms.fSolveEveryPixel) {
              x2 += xoff[d]; y2 += yoff[d];
            }
            while (FLegalMaze(x2, y2) && Count(x2, y2) == 2 &&
              !(fAny && ((x2 == xa && y2 == ya) || (x2 == xb && y2 == yb)))) {
              dnew = d ^ 2;
              for (j = 0; j < DIRS1; j++) {
                DirInc(dnew);
                if (!Get(x2 + xoff[dnew], y2 + yoff[dnew])) {
                  x2 += xoff[dnew]; y2 += yoff[dnew];
                  if (!ms.fSolveEveryPixel) {
                    x2 += xoff[dnew]; y2 += yoff[dnew];
                  }
                  break;
                }
              }
              d = dnew;
            }

            // Add a wall just down that path to make a new long dead end.
            if (x2 == x && y2 == y) {
              if (fCellMax)
                return count;
              count++;
              Set1(x + xoff[i], y + yoff[i]);
              break;
            }
          }
        }
      }
  return count;
}


// Solve a Maze by filling in all cul-de-sacs, including passages that become
// cul-de-sacs once others are filled.

long CMaz::SolveMazeFillCulDeSacs(int xa, int ya, int xb, int yb)
{
  long count = 0, i;

  // Convert all cul-de-sacs to dead ends, then fill all dead ends. Repeat the
  // process until none are left.
  do {
    count += (i = DoMarkCulDeSacs(xa, ya, xb, yb));
    if (i > 0)
      PrintSzL("Cul-de-sacs marked: %ld\n", i);
  } while (SolveMazeFillDeadEnds(xa, ya, xb, yb) > 0);
  return count;
}


// Delete all walls within blind alleys in a Maze, by removing walls that have
// the same blind alley on either side of them.

long CMaz::DoCrackBlindAlleys(CONST CMaz &bSol)
{
  CMaz bNew;
  CONST CMaz *pb;
  BFSS *rgpt;
  int zInc = 2 - ms.fSolveEveryPixel, x, y, x2, y2, xnew, ynew, d;
  long iset = 0, ipt;

  if (F64K())
    return -1;
  rgpt = RgAllocate(m_x*m_y, BFSS);
  if (rgpt == NULL)
    return -1;
  ClearPb(rgpt, m_y*m_x * sizeof(BFSS));

  // Potentially allow all off pixels in a 2nd bitmap to act as the solution.
  if (FBitmapSubset(bSol))
    pb = &bSol;
  else {
    if (!bNew.FBitmapCopy(*this))
      return -1;
    bNew.SolveMazeFillBlindAlleys(-1, -1, -1, -1);
    pb = &bNew;
  }
  BitmapXor(*pb);
  BitmapReverse();

  for (y = 1; y < m_y; y += zInc)
    for (x = 1; x < m_x; x += zInc)

      // For each section of passages that hasn't already been filled, flood
      // it and all passages that connect with it with a unique id number.
      if (!Get(x, y) && rgpt[(long)y * m_x + x].parent <= 0) {
        iset++;
        ipt = 0;
        x2 = x; y2 = y;
LSet:
        rgpt[(long)y2 * m_x + x2].parent = iset;
LNext:
        for (d = 0; d < DIRS; d++) {
          xnew = x2 + xoff[d]; ynew = y2 + yoff[d];
          if (FLegal(xnew, ynew) && !_Get(xnew, ynew) &&
            rgpt[(long)ynew * m_x + xnew].parent <= 0) {
            FillPush(x2, y2);
            x2 = xnew; y2 = ynew;
            goto LSet;
          }
        }
        if (ipt > 0) {
          FillPop(x2, y2);
          goto LNext;
        }
      }

  // For each pair of cells, remove the wall segment there if connected.
  for (y = 1; y < m_y; y += zInc)
    for (x = 1; x < m_x; x += zInc) {
      ipt = rgpt[(long)y * m_x + x].parent;
      if (ipt > 0) {
        if (x < m_x-2 && ipt == rgpt[(long)y * m_x + (x+2)].parent)
          Set0(x+1, y);
        if (y < m_y-2 && ipt == rgpt[(long)(y+2) * m_x + x].parent)
          Set0(x, y+1);
      }
    }

  BitmapXor(*pb);
  BitmapReverse();
  DeallocateP(rgpt);
  return iset;
}


// Seal off all blind alleys in a Maze, by filling in blind alley passages.
// This does not also fill in the section of passages that may be behind the
// stem, so this will create inaccessible sections out of cul-de-sacs and
// other blind alleys that are more than just dead ends.

long CMaz::DoMarkBlindAlleys()
{
  BFSS *rgpt;
  int zInc = 2 - ms.fSolveEveryPixel, x, y, x2, y2, xnew, ynew, d;
  long count = 0, iset = 0, ipt;

  if (F64K())
    return -1;
  rgpt = RgAllocate(m_x*m_y, BFSS);
  if (rgpt == NULL)
    return -1;
  ClearPb(rgpt, m_y*m_x * sizeof(BFSS));
  for (y = 0; y < m_y; y += zInc)
    for (x = 0; x < m_x; x += zInc)

      // For each section of walls that hasn't already been filled, flood it
      // and all walls that connect with it with a unique id number.
      if (Get(x, y) && rgpt[(long)y * m_x + x].parent <= 0) {
        iset++;
        ipt = 0;
        x2 = x; y2 = y;
LSet:
        rgpt[(long)y2 * m_x + x2].parent = iset;
LNext:
        for (d = 0; d < DIRS; d++) {
          xnew = x2 + xoff[d]; ynew = y2 + yoff[d];
          if (GetFast(xnew, ynew) &&
            rgpt[(long)ynew * m_x + xnew].parent <= 0) {
            FillPush(x2, y2);
            x2 = xnew; y2 = ynew;
            goto LSet;
          }
        }
        if (ipt > 0) {
          FillPop(x2, y2);
          goto LNext;
        }
      }

  // For each pair of wall endpoints, add a wall segment there if connected.
  for (y = 0; y < m_y; y += zInc)
    for (x = 0; x < m_x; x += zInc) {
      ipt = rgpt[(long)y * m_x + x].parent;
      if (ipt > 0) {
        if (x < m_x-2 && ipt == rgpt[(long)y * m_x + (x+2)].parent)
          Set1(x+1, y);
        if (y < m_y-2 && ipt == rgpt[(long)(y+2) * m_x + x].parent)
          Set1(x, y+1);
      }
    }

  // Fill in the centers of cells blocked with walls on all four sides.
  for (y = 1; y < m_y-1; y += zInc)
    for (x = 1; x < m_x-1; x += zInc)
      if (!Get(x, y) && Count(x, y) == DIRS) {
        count++;
        Set1(x, y);
      }
  DeallocateP(rgpt);
  return count;
}


// Solve a Maze by filling in all blind alleys. This fills in each blind alley
// stem as well as whatever section of passages may be behind it.

long CMaz::SolveMazeFillBlindAlleys(int xa, int ya, int xb, int yb)
{
  CMaz b2;
  int x, y, xdir, ydir, x2, y2, zInc = 2 - ms.fSolveEveryPixel,
    i, d, dold, dd, dsum;
  long count = 0;
  flag fAny, fAnyA, fAnyB;

  fAnyA = ms.fSolveDotExit && FLegalOff(xa, ya);
  fAnyB = ms.fSolveDotExit && FLegalOff(xb, yb);
  fAny = fAnyA || fAnyB;
  for (y = yl + 1; y < yh; y += zInc)
    for (x = xl + 1; x < xh; x += zInc)
      if (!Get(x, y) && Count(x, y) <= 1 &&
        (!ms.fSolveEveryPixel || FOnMaze(x, y))) {

        // Check a junction and see if any blind alleys lead from it.
        for (i = 0; i < DIRS; i++) {
          xdir = x + xoff[i];
          ydir = y + yoff[i];
          if (!Get(xdir, ydir)) {

            // Follow a path and see if wall following reconnects.
            d = i; x2 = xdir; y2 = ydir; dsum = 0;
            loop {
              dold = d; d = FollowWall(&x2, &y2, d, 1); dd = (d - dold);
              dsum += ((dd + 1) & DIRS1) - 1;
              if (!FLegalMaze(x2, y2) ||
                (x2 == x && y2 == y) ||
                (fAny && ((x2 == xa && y2 == ya) || (x2 == xb && y2 == yb))))
                break;
              if (x2 == xdir && y2 == ydir) {

                // Wall following has reconnected. Check for blind alley.
                if (dsum >= 0) {
                  if (fAny && !b2.FBitmapCopy(*this))
                    return -1;
                  Set1(x, y);
                  FFill(xdir, ydir, fOn);
                  Set0(x, y);

                  // Undo if flooding set either entrance point.
                  if (fAny &&
                    ((fAnyA && Get(xa, ya)) || (fAnyB && Get(xb, yb)))) {
                    FBitmapCopy(b2);
                    break;
                  }
                  count++;
                }
                break;
              }
            }
          }
        }
      }
  return count;
}


#define STREAMS 10000

typedef struct _collision {
  ushort x;
  ushort y;
  char mode;
  char unused;
} COLL;

// Remove many passage loops from a Maze by adding wall segments converting
// them to dead ends. Internally "flood" the Maze with "water", such that
// equal distances from the start are reached at the same time, where whenever
// two "columns of water" come down a passage from opposite sides (indicating
// a loop) add a wall segment where they collide.

long CMaz::DoMarkCollisions(int x, int y)
{
  CMaz bNew;
  COLL *coll;
  int x0, y0, xnew, ynew, streams = 1, i, j, d;
  long count = 0;

  if (F64K())
    return -1;
  if (FLegalOff(x, y)) {
    x0 = x; y0 = y;
  } else {
    x0 = xl, y0 = yl;
    if (!FFindPassage(&x0, &y0, fFalse))
      return -1;
  }
  if (!bNew.FBitmapCopy(*this))
    return -1;
  coll = RgAllocate(STREAMS, COLL);
  if (coll == NULL)
    return -1;
  coll[0].x = x0; coll[0].y = y0;
  coll[0].mode = 'a';
  while (streams > 0) {
    for (i = 0; i < streams; i++) {

      // For each active (a) stream in the list, mark it dead (d).
      if (coll[i].mode == 'a') {
        x0 = coll[i].x; y0 = coll[i].y;
        coll[i].mode = 'd';
        if (Get(x0, y0)) {

          // If the pixel under this stream has already been set, then some
          // other stream did it, where this is a collision. Let this stream
          // die and add a wall if the collision happened in a narrow passage.
          if (Count(x0, y0) == DIRS) {
            count++;
            for (d = 0; d < DIRS; d++) {
              xnew = x0 + xoff[d]; ynew = y0 + yoff[d];
              if (!bNew.Get(xnew, ynew)) {
                bNew.Set1(xnew, ynew);
                break;
              }
            }
          }
        } else {

          // This stream is in new territory that hasn't been flooded yet.
          // Create new (n) streams for any off pixels adjacent to it.
          Set1(x0, y0);
          for (d = 0; d < DIRS; d++) {
            xnew = x0 + xoff[d]; ynew = y0 + yoff[d];
            if (FLegalMaze(xnew, ynew) && !Get(xnew, ynew)) {
              for (j = 0; j < streams && coll[j].mode != 'd'; j++)
                ;
              if (j >= streams)
                streams++;
              if (streams >= STREAMS) {
                count = -1;
                goto LDone;
              }
              coll[j].x = xnew; coll[j].y = ynew;
              coll[j].mode = 'n';
            }
          }
        }
      }
    }

    // Convert all new (n) streams to active (a) streams for next iteration.
    for (i = 0; i < streams; i++)
      if (coll[i].mode == 'n')
        coll[i].mode = 'a';

    // Ignore dead (d) streams at the end of the list.
    while (streams > 0 && coll[streams - 1].mode == 'd')
      streams--;
  }
LDone:
  CopyFrom(bNew);
  DeallocateP(coll);
  return count;
}


// Solve a Maze by filling in all passages that aren't on a shortest solution
// path.

long CMaz::SolveMazeFillCollisions(int xa, int ya, int xb, int yb)
{
  long count = 0, i;

  // Convert all collision passages to dead ends, then fill all dead ends.
  // Repeat the process until none are left.
  do {
    count += (i = DoMarkCollisions(xa, ya));
    if (i > 0) {
      UpdateDisplay();
      PrintSzL("Collisions marked: %ld\n", i);
    }
  } while (SolveMazeFillDeadEnds(xa, ya, xb, yb) > 0);
  return count;
}


// Solve a Maze with a recursive backtracking algorithm. This is implemented
// with a stack formed by a line of pixels in another bitmap.

long CMaz::SolveMazeRecursive(int x, int y, int x2, int y2, flag fCorner)
{
  CMaz b2;
  int xnew, ynew, d, dMax = DIRS + fCorner*DIRS, dInc = 1, dir, i, cf;
  long count = 0;
  flag fAny, fAny2, fDotsOnly;

  fAny2 = FLegalOff(x2, y2) && (x2 != 0 || y2 != 0);
  fAny = FLegalOff(x, y);
  fDotsOnly = fAny2 && ms.fSolveDotExit;
  if (!fAny && !FBitmapFind(&x, &y, fOff))
    return -2;
  if (!b2.FAllocate(m_x, m_y, this))
    return -1;
  b2.BitmapOff();
  loop {

    // Set the current location, and figure out which direction to try first.
LPush:
    count++;
    Set1(x, y); b2.Set1(x, y);
    if (!ms.fRandomPath)
      dir = 0;
    else {
      dir = Rnd(0, dMax-1);
      dInc = (Rnd(0, 1) << 1) - 1;
    }

    // Move in each direction that hasn't been tried yet.
    for (d = 0; d < dMax; d++) {
      xnew = x + xoff[dir]; ynew = y + yoff[dir];
      if (!FLegal(xnew, ynew) || (fAny2 && xnew == x2 && ynew == y2)) {
        if (!fDotsOnly ?
          fAny || fAny2 || ynew >= m_y : xnew == x2 && ynew == y2) {
          // Reached a goal point! Return the path taken to get here.
LFinish:
          UpdateDisplay();
          FBitmapCopy(b2);
          BitmapReverse();
          goto LDone;
        }
      } else if (!Get(xnew, ynew)) {
        if (y >= m_y-1 && !fDotsOnly)
          goto LFinish;

        // Don't move adjacent to earlier parts of the path, to avoid solid
        // areas in rooms and ensure the path is always easily followable.
        cf = 0;
        for (i = 0; i < dMax; i++)
          cf += b2.Get(xnew + xoff[i], ynew + yoff[i]);
        if (cf == 1) {
          x = xnew; y = ynew;
          goto LPush;
        }
      }
LPop:
      dir = (dir + dInc) & (dMax-1);
    }

    // Backtrack: Undo the move just taken, then look for the next path.
    count--;
    if (count <= 0)
      break;
    b2.Set0(x, y);
    for (d = 0; d < dMax; d++) {
      xnew = x + xoff[d]; ynew = y + yoff[d];
      if (b2.Get(xnew, ynew))
        break;
    }
    x = xnew; y = ynew;
    dir = d ^ 2; d = ms.fRandomPath ? 0 : dir;
    goto LPop;
  }
LDone:
  return count;
}


// Solve a Maze by finding a shortest solution, making the bitmap be one
// solution path leading from start to end.

long CMaz::SolveMazeShortest(int x, int y, int x2, int y2, flag fCorner)
{
  BFSS *bfss;
  int xnew, ynew, d, dMax = DIRS + fCorner*DIRS, dInc = 1, dir;
  long count = 0, iLo = 0, iHi = 1, iMax = 1, i;
  flag fAny, fAny2, fDotsOnly;

  bfss = RgAllocate(m_x*m_y, BFSS);
  if (bfss == NULL)
    return -1;
  fAny2 = FLegalOff(x2, y2) && (x2 != 0 || y2 != 0);
  fAny = FLegalOff(x, y);
  fDotsOnly = fAny2 && ms.fSolveDotExit;
  if (!fAny)
    if (!FBitmapFind(&x, &y, fOff)) {
      DeallocateP(bfss);
      return -2;
    }
  Set1(x, y);
  bfss[0].x = x; bfss[0].y = y; bfss[0].parent = -1;

  // Flood the Maze, where each pixel remembers which pixel filled it.
  while (iLo < iHi) {
    for (i = iLo; i < iHi; i++) {
      x = bfss[i].x; y = bfss[i].y;
      if (!ms.fRandomPath)
        dir = 0;
      else {
        dir = Rnd(0, dMax-1);
        dInc = (Rnd(0, 1) << 1) - 1;
      }
      for (d = 0; d < dMax; d++) {
        xnew = x + xoff[dir]; ynew = y + yoff[dir];
        if (!FLegal(xnew, ynew) || (fAny2 && xnew == x2 && ynew == y2)) {
          if (!fDotsOnly ?
            fAny || fAny2 || ynew >= m_y : xnew == x2 && ynew == y2) {

            // Reached a goal point! Draw a path backwards to the start.
            UpdateDisplay();
            BitmapOn();
            do {
              Set0(bfss[i].x, bfss[i].y);
              i = bfss[i].parent;
              count++;
            } while (i >= 0);
            goto LDone;
          }
        } else if (!Get(xnew, ynew)) {
          Set1(xnew, ynew);
          BfssPush(iMax, xnew, ynew, i);
        }
        dir = (dir + dInc) & (dMax-1);
      }
    }
    iLo = iHi; iHi = iMax;
  }

LDone:
  DeallocateP(bfss);
  return count;
}


// Solve a Maze by finding all shortest solutions, making the bitmap be all
// the solution paths leading from start to end.

long CMaz::SolveMazeShortest2(int x, int y, int x2, int y2, flag fCorner)
{
  PT *rgpt = NULL;
  int xnew, ynew, d, d2, dMax = DIRS + fCorner*DIRS;
  long *rgl = NULL, count = 0, iLo = 0, iHi = 1, iMax = 1, i;
  flag fCount = ms.fCountShortest, fAny, fAny2, fDotsOnly;

  if (!FEnsureMazeSize(1, femsNoResize | fems64K))
    return fFalse;
  rgpt = RgAllocate(m_x*m_y, PT);
  if (rgpt == NULL) {
    count = -1;
    goto LDone;
  }
  if (fCount) {
    rgl = RgAllocate(m_x*m_y, long);
    if (rgl == NULL) {
      count = -1;
      goto LDone;
    }
    ClearPb(rgl, (long)m_x*m_y*sizeof(PT));
  }
  fAny2 = FLegalOff(x2, y2) && (x2 != 0 || y2 != 0);
  fAny = FLegalOff(x, y);
  fDotsOnly = fAny2 && ms.fSolveDotExit;
  if (!fAny)
    if (!FBitmapFind(&x, &y, fOff)) {
      count = -2;
      goto LDone;
    }
  Set1(x, y);
  rgpt[0].x = x; rgpt[0].y = y;

  // Flood the Maze, where each pixel is put in a list in order it's reached.
  while (iLo < iHi) {
    count++;
    for (i = iLo; i < iHi; i++) {
      x = rgpt[i].x; y = rgpt[i].y;
      for (d = 0; d < dMax; d++) {
        xnew = x + xoff[d]; ynew = y + yoff[d];
        if (!FLegal(xnew, ynew) || (fAny2 && xnew == x2 && ynew == y2)) {
          if (!fDotsOnly ?
            fAny || fAny2 || ynew >= m_y : xnew == x2 && ynew == y2) {

            // Reached a goal point! Draw paths backwards to the start.
            UpdateDisplay();
            BitmapOn();
            Set0(x, y);
            if (fCount) {
              count = 1;
              rgl[y * m_x + x] = count;
            }
            while (i >= 0) {

              // Each pixel in the list gets set only if adjacent to a pixel
              // already on a solution. Since looping backwards, and pixels
              // are in the list in forwards order, this avoids going down
              // non-shortest paths that were flooded from opposite ends.

              x = rgpt[i].x; y = rgpt[i].y;
              for (d = 0; d < dMax; d++) {
                xnew = x + xoff[d]; ynew = y + yoff[d];
                if (FLegal(xnew, ynew) && !Get(xnew, ynew)) {
                  Set0(x, y);

                  // Count the number of ways to reach the current pixel. Each
                  // pixel is the sum of all the ways leading to it. The
                  // number of shortest solutions is the sum at the end pixel.

                  if (fCount) {
                    count = 0;
                    for (d2 = 0; d2 < dMax; d2++) {
                      xnew = x + xoff[d2]; ynew = y + yoff[d2];
                      if (FLegal(xnew, ynew)) {
                        count += rgl[ynew * m_x + xnew];
                        if (count < 0) {
                          count = 0;
                          fCount = fFalse;
                        }
                      }
                    }
                    rgl[y * m_x + x] = count;
                  }
                  break;
                }
              }
              i--;
            }
            goto LDone;
          }
        } else if (!Get(xnew, ynew)) {
          Set1(xnew, ynew);
          rgpt[iMax].x = xnew; rgpt[iMax].y = ynew;
          iMax++;
        }
      }
    }
    iLo = iHi; iHi = iMax;
  }

LDone:
  if (rgpt != NULL)
    DeallocateP(rgpt);
  if (rgl != NULL)
    DeallocateP(rgl);
  return count;
}


// Solve a Maze by following a wall, always taking left or right turns. This
// makes the bitmap be the cells visited from start to end, or just the cells
// visited from start back to start if wall following can't solve this Maze.
// Return the number of cells visited if the Maze was successfully solved.

long CMaz::SolveMazeFollowWall(int x, int y, int dir, int x2, int y2,
  flag fRight)
{
  CMaz b2;
  long count = 0;
  int x0, y0;
  flag fAny, fAny2;

  fAny2 = FLegalOff(x2, y2) && (x2 != 0 || y2 != 0);
  fAny = FLegalOff(x, y);
  if (!fAny) {
    if (!FBitmapFind(&x, &y, fOff))
      return -2;
    dir = 2;
  }
  if (!b2.FBitmapCopy(*this))
    return -1;
  BitmapOn();
  x0 = x; y0 = y;

  // Keep on taking left turns or right turns until end or start is reached.
  while (FLegalMaze(x, y) && (!fAny2 || x != x2 || y != y2)) {
    count++;
    Set0(x, y);
    dir = b2.FollowWall(&x, &y, dir, fRight);
    if (x == x0 && y == y0) {
      count = 0;
      break;
    }
  }
  if (FLegalMaze2(x, y))
    Set0(x, y);
  return count;
}


// Solve a Maze with the Pledge algorithm. This is implemented by following a
// wall, so is similar to SolveMazeFollowWall, however this has the ability to
// jump between islands when pointed in the starting direction.

long CMaz::SolveMazePledge(int x, int y, int dir, int x2, int y2,
  flag fRight)
{
  CMaz b2;
  long count = 0;
  int x0, y0, d, dold, dsum = 0, dd, dInc = fRight ? 1 : -1;
  flag fStraight = fFalse, fAny, fAny2;

  fAny2 = FLegalOff(x2, y2) && (x2 != 0 || y2 != 0);
  fAny = FLegalOff(x, y);
  if (!b2.FBitmapCopy(*this))
    return -1;
  BitmapOn();
  if (!fAny) {
    if (!b2.FBitmapFind(&x, &y, fOff))
      return -2;
    dir = 2;
    if (!b2.Get(x, y+1)) {
      Set0(x, y);
      b2.Set1(x, y);
      y++;
    }
  }
  x0 = x; y0 = y; d = dir;

  // Keep on following wall or going straight, until outer edge is reached.
  while (FLegalMaze(x, y) && (!fAny2 || x != x2 || y != y2)) {
    count++;
    Set0(x, y);
    if (!fStraight && dsum == 0 && !b2.Get(x + xoff[dir], y + yoff[dir]))
      fStraight = fTrue;
    dold = d;
    if (fStraight) {
      Assert(d == dir);
      x += xoff[d]; y += yoff[d];
      if (b2.Get(x + xoff[dir], y + yoff[dir])) {
        count++;
        fStraight = fFalse;
        d = d + dInc & DIRS1;
      }
    } else
      d = b2.FollowWall(&x, &y, d, fRight);
    dd = (d - dold);
    dsum += (dd == 3 ? -1 : (dd == -3 ? 1 : (dd == -2*dInc ? 2*dInc : dd)));
    if (NAbs(dsum) > ms.nCrackPass) {
      count = 0;
      break;
    }
  }
  if (FLegalMaze2(x, y))
    Set0(x, y);
  return count;
}


// Solve a Maze with the Chain algorithm. This finds a reasonably short path
// between two specific points, by following a direct line between them when
// possible, and following a wall to get farther when that line is blocked.

long CMaz::SolveMazeChain(int x, int y, int x2, int y2)
{
  CMaz bCopy, bLine;
  long count = 0;
  int m[2], n[2], e0[2], e[2], len, lenT, xT, yT, d, xnew, ynew, i;
  flag fAny, fAny2, f0, f1, f2, f3;

  fAny2 = FLegalOff(x2, y2) && (x2 != 0 || y2 != 0);
  fAny = FLegalOff(x, y);
  if (!fAny && !FBitmapFind(&x, &y, fOff))
    return -2;
  if (!fAny2) {
    x2 = xl; y2 = yh - !Get(xl, yh);
    if (!FFindPassage(&x2, &y2, fFalse))
      return -2;
  }
  if (!bCopy.FBitmapCopy(*this))
    return -1;
  len = NAbs(x2 - x) + NAbs(y2 - y);

  // Draw a line between the start and end points.
  if (!bLine.FBitmapCopy(*this))
    return -1;
  bLine.BitmapOff();
  bLine.Line(x, y, x2, y2, fOn);

  // Expand the line so it can be followed with orthogonal moves.
  for (yT = 0; yT < bLine.m_y-1; yT++)
    for (xT = 0; xT < bLine.m_x-1; xT++) {
      f0 = bLine.Get(xT, yT); f1 = bLine.Get(xT+1, yT);
      f2 = bLine.Get(xT, yT+1); f3 = bLine.Get(xT+1, yT+1);
      if (f0 != f1 && f0 == f3 && f1 == f2)
        bLine.Set1(xT+f2, yT+1);
    }
  BitmapOn();

  loop {
    Set0(x, y);
    if (x == x2 && y == y2)
      break;
    count++;

    // Find the next position along the direct line that's closer to the end.
    for (d = 0; d < DIRS; d++) {
      xnew = x + xoff[d]; ynew = y + yoff[d];
      lenT = NAbs(x2 - xnew) + NAbs(y2 - ynew);
      if (lenT < len && bLine.Get(xnew, ynew)) {
        len = lenT;
        break;
      }
    }
    Assert(d < DIRS);

    // If next position along the line is not blocked by a wall, move to it.
    if (!bCopy.Get(xnew, ynew)) {
      x = xnew; y = ynew;
      continue;
    }

    // Send out two wall following robots to find the next spot on the line.
    for (i = 0; i < 2; i++) {
      m[i] = x; n[i] = y;
      e[i] = e0[i] = (d + (i << 1) - 1) & DIRS1;
    }
    loop {
      for (i = 0; i < 2; i++) {
        if (e0[i] < 0)
          continue;
        e[i] = bCopy.FollowWall(&m[i], &n[i], e[i], i);
        if (m[i] == x && n[i] == y && e[i] == e0[i]) {
          e0[i] = -1;
          if (e0[1 - i] < 0)
            return 0;
          continue;
        }
        if (bLine.Get(m[i], n[i])) {
          lenT = NAbs(x2 - m[i]) + NAbs(y2 - n[i]);
          if (lenT < len) {
            len = lenT;
            goto LNext1;
          }
        } else if (!FLegalMaze(m[i], n[i])) {
          e[i] ^= 2;
          continue;
        }
      }
    }

    // When found, draw passage from current to the next spot on the line.
LNext1:
    m[i] = x; n[i] = y;
    e[i] = e0[i];
    loop {
      e[i] = bCopy.FollowWall(&m[i], &n[i], e[i], i);
      if (bLine.Get(m[i], n[i])) {
        lenT = NAbs(x2 - m[i]) + NAbs(y2 - n[i]);
        if (lenT <= len) {
          x = m[i]; y = n[i];
          goto LNext2;
        }
      } else if (!FLegalMaze(m[i], n[i])) {
        e[i] ^= 2;
        continue;
      }
      Set0(m[i], n[i]);
      count++;
    }
LNext2:
    ;
  }
  if (y == yh-1 && !bCopy.Get(xl, yh) && !bCopy.Get(x, yh))
    Set0(x, yh);

  return count;
}


// Solve a Maze with Tremaux's algorithm. The Maze is in a monochrome bitmap
// and the solution is drawn in a color bitmap. This is effectively the same
// as the recursive backtracking algorithm, except it only looks at breadcrumb
// markings made locally, making it implementable by a person inside the Maze.

long CMaz::SolveMazeTremaux(CMazK &c, int x, int y, int x2, int y2,
  KV kv0, KV kv9, KV kv1, KV kv2) CONST
{
  long count = 0;
  int xnew, ynew, xT, yT, d, dInc, dir, dUsed, zInc, nDone = 0;
  KV kv;
  flag fAny, fAny2, fDotsOnly, fOld = fFalse, fT;

  // kv0: Passage, kv9: Wall, kv1: Marked once, kv2: Marked twice
  if (kv0 == kv9 || kv0 == kv1 || kv0 == kv2 ||
    kv9 == kv1 || kv9 == kv2 || kv1 == kv2)
    return -3;
  fAny2 = FLegalOff(x2, y2) && (x2 != 0 || y2 != 0);
  fAny = FLegalOff(x, y);
  fDotsOnly = fAny2 && ms.fSolveDotExit;
  if (!fAny && !FBitmapFind(&x, &y, fOff))
    return -2;

  // Tremaux's algorithm considers markings between cells, hence there are two
  // modes: (1) Assume the Maze is on a grid where passages are on odd number
  // pixels. (2) Zoom the Maze by a factor of 3 in order to draw a line down
  // the center of each passage, while allowing pixels between adjacent cells.

  if (!ms.fSolveEveryPixel) {
    if (!c.FColmapGetFromBitmap(*this, kv0, kv9))
      return -1;
    UpdateDisplay();
    if (y == 0)
      c.Set(x, 0, kv1);
    x |= 1; y |= 1;
    zInc = 2;
  } else {
    if (!c.FBitmapSizeSet(m_x*3, m_y*3))
      return -1;
    UpdateDisplay();
    c.BitmapSet(kv0);
    fT = gs.fTraceDot; gs.fTraceDot = fFalse;
    for (yT = 0; yT < m_y; yT++)
      for (xT = 0; xT < m_x; xT++)
        if (Get(xT, yT))
          c.Block(xT*3, yT*3, xT*3 + 2, yT*3 + 2, kv9);
    gs.fTraceDot = fT;
    zInc = 1;
  }
  UpdateDisplay();
  dInc = !ms.fRandomPath ? 1 : (Rnd(0, 1) << 1) - 1;

  loop {
    count++;
    if (!ms.fRandomPath)
      dir = 0;
    else
      dir = Rnd(0, DIRS1);

    dUsed = -1;
    for (d = 0; d < DIRS; d++, dir = (dir + dInc) & DIRS1) {
      xnew = x + xoff[dir]*zInc; ynew = y + yoff[dir]*zInc;
      if (Get(xnew, ynew) ||
        (!ms.fSolveEveryPixel && Get((x + xnew) >> 1, (y + ynew) >> 1)))
        continue;

      // Check for the finish.
      if (!FLegal(xnew, ynew)) {
        if (!fDotsOnly && (fAny || fAny2 || ynew >= m_y)) {
          nDone = 1;
          break;
        }
        continue;
      }
      if ((!fDotsOnly && ynew >= m_y-1) ||
        (fAny2 && xnew == x2 && ynew == y2)) {
        nDone = 2;
        break;
      }

      // Check an adjacent cell to see if should move there.
      kv = !ms.fSolveEveryPixel ? c.Get(x + xoff[dir], y + yoff[dir]) :
        c.Get(x*3+1 + xoff[dir], y*3+1 + yoff[dir]);
      if (kv == kv0) {
        kv = !ms.fSolveEveryPixel ? c.Get(xnew, ynew) :
          c.Get(xnew*3 + 1, ynew*3 + 1);
        if (kv == kv0)
          break;
      } else if (kv == kv1 && dUsed < 0)
        dUsed = dir;
    }

    // No new cell to visit. Check if a once visited cell is available.
    fOld = d >= DIRS;
    if (fOld) {
      if (dUsed >= 0) {
        dir = dUsed;
        xnew = x + xoff[dir]*zInc; ynew = y + yoff[dir]*zInc;
      } else {
        count = 0;
        goto LDone;
      }
    }

    // Move into another cell, drawing a once or twice visited line behind.
    kv = !fOld ? kv1 : kv2;
    if (!ms.fSolveEveryPixel) {
      c.Set(x, y, kv);
      c.Set(x + xoff[dir], y + yoff[dir], kv);
    } else {
      c.Set(x*3+1, y*3+1, kv);
      c.Set(x*3+1 + xoff[dir], y*3+1 + yoff[dir], kv);
    }
    if (nDone == 1)
      goto LDone;
    x = xnew; y = ynew;
    if (!ms.fSolveEveryPixel)
      c.Set(x, y, kv);
    else {
      c.Set(x*3+1 + xoff[dir^2], y*3+1 + yoff[dir^2], kv);
      c.Set(x*3+1, y*3+1, kv);
    }
    if (nDone == 2)
      goto LDone;
  }
LDone:
  return count;
}


// Show all bottlenecks in a Maze, or parts of the Maze such that every
// solution to the Maze goes through them.

flag CMaz::DoShowBottlenecks(int x, int y, int dir, int x2, int y2)
{
  CMaz bCopy, bEdit;

  if (!bCopy.FBitmapCopy(*this) || !bEdit.FBitmapCopy(*this))
    return fFalse;
  if (SolveMazeFollowWall(x, y, dir, x2, y2, fFalse) > 0) {
    bEdit.SolveMazeFollowWall(x, y, dir, x2, y2, fTrue);

    // Pixels common to the left hand wall follow and right hand wall follow
    // are all bottleneck passages and intersections.

    BitmapOr(bEdit);
  } else {

    // If wall following doesn't work to solve the Maze, then places where any
    // solution intersects with passages sealed by the blind alley sealer are
    // all bottleneck passages.

    FBitmapCopy(bCopy);
    DoMarkBlindAlleys();
    BitmapXor(bCopy);
    BitmapReverse();
    bEdit.FBitmapCopy(bCopy);
    bEdit.SolveMazeShortest(x, y, x2, y2, fFalse);
    BitmapOr(bEdit);
  }
  return fTrue;
}


// Solve a Weave Maze created with CreateMazeWeave(), by filling in all dead
// ends.

long CMaz::SolveMazeWeave()
{
  int x0, y0, x, y, xnew, ynew, d, dnew, f, i;
  long count = 0;

  for (y0 = yl + 3; y0 < yh; y0 += 4)
    for (x0 = xl + 3; x0 < xh; x0 += 4)
      if (!Get(x0, y0) && Count(x0, y0) >= DIRS1) {
        count++;
        x = x0; y = y0;
        do {
          Set1(x, y);
          for (d = 0; d < DIRS && Get(x + xoff2[d], y + yoff2[d]); d++)
            ;
          if (d >= DIRS)
            break;
          loop {
            f = Get(xnew = x + xoff[d]*3, ynew = y + yoff[d]*3);
            Block(x + xoff[d], y + yoff[d], xnew, ynew, fOn);
            x = xnew + xoff[d]; y = ynew + yoff[d];
            if (!FLegalMaze(x, y) ||
              !(Count(x, y) > 1 && Count2(x, y) == 1))
              break;
            dnew = d - 1 & DIRS1;
            for (i = 0; i < DIRS1; i++) {
              if (Get(xnew = x + xoff[dnew], ynew = y + yoff[dnew]) == f)
                d = dnew;
              Set0(xnew, ynew);
              DirInc(dnew);
            }
          }
        } while (FLegalMaze(x, y) && Count(x, y) >= DIRS1);
      }
  return count;
}


// Solve a 3D Maze created with CreateMaze3D(), by filling in all dead ends.

long CMaz::SolveMaze3D()
{
  int x0, y0, z0, x, y, z, xnew, ynew, znew, d;
  long count = 0;

  for (z0 = 0; z0 <= m_z3-1; z0 += 2)
    for (y0 = 1; y0 <= m_y3-2; y0 += 2)
      for (x0 = 1; x0 <= m_x3-2; x0 += 2)
        if (!Get3(x0, y0, z0) && Count3(x0, y0, z0) >= DIRS3-1) {
          count++;
          x = x0; y = y0; z = z0;
          do {
            Set31(x, y, z);
            for (d = 0; d < DIRS3 && Get3M(xnew = x + xoff3[d],
              ynew = y + yoff3[d], znew = z + zoff3[d]); d++)
              ;
            if (d >= DIRS3)
              break;
            Set31(xnew, ynew, znew);
            x += (xnew - x) << 1; y += (ynew - y) << 1; z += (znew - z) << 1;
          } while (FLegalMaze3(x, y, z) && Count3(x, y, z) == DIRS3-1);
        }
  return count;
}


// Solve a 4D Maze created with CreateMaze4D(), by filling in all dead ends.

long CMaz::SolveMaze4D()
{
  int w0, x0, y0, z0, w, x, y, z, wnew, xnew, ynew, znew, d;
  long count = 0;

  for (w0 = 0; w0 <= m_w3-1; w0 += 2)
    for (z0 = 0; z0 <= m_z3-1; z0 += 2)
      for (y0 = 1; y0 <= m_y3-2; y0 += 2)
        for (x0 = 1; x0 <= m_x3-2; x0 += 2)
          if (!Get4(w0, x0, y0, z0) &&
            Count4(w0, x0, y0, z0) >= DIRS4-1) {
            count++;
            w = w0; x = x0; y = y0; z = z0;
            do {
              Set41(w, x, y, z);
              for (d = 0; d < DIRS4 && Get4M(wnew = w + woff3[d],
                xnew = x + xoff3[d], ynew = y + yoff3[d],
                znew = z + zoff3[d]); d++)
                ;
              if (d >= DIRS4)
                break;
              Set41(wnew, xnew, ynew, znew);
              w += (wnew - w) << 1; x += (xnew - x) << 1;
              y += (ynew - y) << 1; z += (znew - z) << 1;
            } while (FLegalMaze4(w, x, y, z) &&
              Count4(w, x, y, z) == DIRS4-1);
          }
  return count;
}


// Solve an n-Dimensional Maze created with CreateMazeND(), by filling in all
// dead ends. Implements the SolveND operation.

long CMaz::SolveMazeND()
{
  int n[dimMax], n2[dimMax], nNew[dimMax], nMax[dimMax],
    cDim = m_w3, tx, ty, tz, cDir, cd, x, y, i, i2, j;
  long count = 0;
  flag fFound;

  tx = Even(m_x3); ty = Even(m_y3); tz = Even(m_z3);
  cDir = cDim << 1;
  nMax[0] = tx; nMax[1] = ty;
  n[0] = n[1] = 1;
  for (i = 2; i < cDim; i++) {
    nMax[i] = tz;
    n[i] = 0;
  }

  loop {
    MapND(n, &x, &y);
    if (Get(x, y))
      goto LNext;
    for (i = 0; i < cDim; i++)
      n2[i] = n[i];
    fFound = fFalse;

LCheck:
    // Count the number of dead ends at this cell.
    cd = 0;
    for (i = 0; i < cDir; i++) {
      for (j = 0; j < cDim; j++)
        nNew[j] = n2[j];
      j = i >> 1;
      nNew[j] += (FOdd(i) << 1) - 1;
      if (!(nNew[j] < 0 || nNew[j] > nMax[j]-2)) {
        MapND(nNew, &x, &y);
        if (!Get(x, y)) {
          cd++;
          if (cd > 1)
            break;
          i2 = i;
        }
      }
    }

    // Fill in this cell if it's a dead end.
    if (cd <= 1) {
      fFound = fTrue;
      MapND(n2, &x, &y);
      Set1(x, y);
      if (cd == 1) {
        j = i2 >> 1;
        n2[j] += (FOdd(i2) << 1) - 1;
        MapND(n2, &x, &y);
        Set1(x, y);
        n2[j] += (FOdd(i2) << 1) - 1;
        goto LCheck;
      }
    }
    count += fFound;

LNext:
    // Adjust the ND point to the next cell.
    for (i = 0; i < cDim; i++) {
      n[i] += 2;
      if (n[i] <= nMax[i]-2)
        break;
      n[i] = i < 2;
    }
    if (i >= cDim)
      break;
  }
  return count;
}


// Solve a Hypermaze created with CreateMazeHyper(), by filling in all space
// that isn't part of the solution surface. This is the Hypermaze equivalent
// to filling in all dead ends, and works similar to DoMarkBlindAlleys().

long CMaz::SolveMazeHyper()
{
  CMaz tLoop, tSec;
  int x0, y0, z0, x, y, z;
  long count = 0;

  if (!tLoop.FBitmapCopy(*this))
    return -1;
  for (z0 = 0; z0 < m_z3; z0 += 2)
    for (y0 = 0; y0 < m_y3; y0 += 2)
      for (x0 = 0; x0 < m_x3; x0 += 2)
        if (tLoop.Get3(x0, y0, z0)) {

          // For each section of walls that hasn't already been filled, flood
          // it and all walls that connect with it.
          if (!tSec.FBitmapCopy(tLoop))
            goto LDone;
          if (!tLoop.FCubeFill(x0, y0, z0, fOff))
            goto LDone;
          tSec.BitmapXor(tLoop);

          // For each pair of wall endpoints, add a wall beam there if they're
          // both part of the wall section that was just highlighted.
          for (z = 0; z < m_z3; z += 2)
            for (y = 0; y < m_y3; y += 2)
              for (x = 0; x < m_x3; x += 2)
                if (tSec.Get3(x, y, z)) {
                  if (x < m_x3-2 && tSec.Get3(x+2, y, z))
                    Set31(x+1, y, z);
                  if (y < m_y3-2 && tSec.Get3(x, y+2, z))
                    Set31(x, y+1, z);
                  if (z < m_z3-2 && tSec.Get3(x, y, z+2))
                    Set31(x, y, z+1);

                  // Fill in the centers of faces blocked with wall beams on
                  // all four sides.
                  if (y < m_y3-2 && z < m_z3-2 && tSec.Get3(x, y+2, z) &&
                    tSec.Get3(x, y, z+2) && tSec.Get3(x, y+2, z+2))
                    CubeBlock(x, y, z, x, y+2, z+2, fOn);
                  if (x < m_x3-2 && z < m_z3-2 && tSec.Get3(x+2, y, z) &&
                    tSec.Get3(x, y, z+2) && tSec.Get3(x+2, y, z+2))
                    CubeBlock(x, y, z, x+2, y, z+2, fOn);
                  if (x < m_x3-2 && y < m_y3-2 && tSec.Get3(x+2, y, z) &&
                    tSec.Get3(x, y+2, z) && tSec.Get3(x+2, y+2, z))
                    CubeBlock(x, y, z, x+2, y+2, z, fOn);
                }
        }

  // Fill in the centers of cells blocked with wall faces on all six sides.
  for (z0 = 1; z0 < m_z3-1; z0 += 2)
    for (y0 = 1; y0 < m_y3-1; y0 += 2)
      for (x0 = 1; x0 < m_x3-1; x0 += 2)
        if (!Get3(x0, y0, z0) && Count3(x0, y0, z0) >= DIRS3) {
          count++;
          Set31(x0, y0, z0);
        }
LDone:
  return count;
}


/*
******************************************************************************
** Recursive Fractal Maze Solving Routines
******************************************************************************
*/

#define MAXLENGTH 100

// Print a given point on a recursive fractal Maze, appending it to a string.
// A point means a particular pin on a particular chip. Used by PrintMoves.

void PrintMove(char **ppch, int cchMax, int i, int cPinChip)
{
  char *pch = *ppch;
  int x, y, d;

  // Start and End chips are unnumbered, and don't need pins.
  if (i < 2) {
    sprintf(SS(pch, cchMax - (pch - *ppch)), i ? "+++" : "---");
    goto LExit;
  }

  // Print the chip number, unless referring to the unnumbered outer rim.
  x = (i - 2) / cPinChip;
  y = (i - 2) - x * cPinChip;
  if (x > 0) {
    sprintf(SS(pch, cchMax - (pch - *ppch)), "%d", x);
    while (*pch)
      pch++;
  }

  // Translate pin number to direction + pin number along that direction.
  if (y < ms.xFractal)
    d = 0;
  else if (y < ms.xFractal + ms.yFractal) {
    d = 1;
    y -= ms.xFractal;
  } else if (y < (ms.xFractal << 1) + ms.yFractal) {
    d = 2;
    y -= ms.xFractal + ms.yFractal;
  } else {
    d = 3;
    y -= (ms.xFractal << 1) + ms.yFractal;
  }
  sprintf(SS(pch, cchMax - (pch - *ppch)), "%c%d", rgszDir[d][0], y + 1);

LExit:
  while (*pch)
    pch++;
  *ppch = pch;
}


// Print a sequence of moves to navigate through a recursive fractal Maze.
// Used by SolveMazeFractal2.

void PrintMoves(char **ppch, int cchMax, CONST int *rgPath, int cPath,
  int cPinChip)
{
  char *pch = *ppch;
  int rgStack[MAXLENGTH], iStack = 0, i, j, k, l = 0;

  for (i = 0; i <= cPath; i++) {
    sprintf(SS(pch, cchMax - (pch - *ppch)), "From ");
    pch += 5;
    PrintMove(&pch, cchMax - PD(pch - *ppch), l, cPinChip);
    *pch++ = ' ';

    // Print move
    k = rgPath[i];
    sprintf(SS(pch, cchMax - (pch - *ppch)),
      k < 2 ? "Goto" : (k < 2 + cPinChip ? "Leave" : "Enter"));
    while (*pch)
      pch++;
    *pch++ = ' ';
    PrintMove(&pch, cchMax - PD(pch - *ppch), k, cPinChip);

    // Print current stack
    sprintf(SS(pch, cchMax - (pch - *ppch)), ", Stack:");
    pch += 8;
    if (iStack > 0) {
      *pch++ = ' ';
      for (j = 0; j < iStack; j++)
        *pch++ = '0' + rgStack[j];
    }
    *pch++ = '\n';

    // Update stack
    if (k >= 2) {
      if (k < 2 + cPinChip) {
        iStack--;
        l = rgStack[iStack] * cPinChip + k;
      } else {
        rgStack[iStack++] = (k - 2) / cPinChip;
        l = 2 + (k - 2) % cPinChip;
      }
    }
  }
  *pch = '\0';
  *ppch = pch;
}


// Solve a recursive fractal Maze created with CreateMazeFractal2(), by
// finding all paths from start "-" to finish "+" (within a maximum move
// length and chip depth) and printing each sequence of moves found.

int CMaz::SolveMazeFractal2() CONST
{
  char sz[cchSzMax*12], *pch;
  int *rgSet, iset, dxChip = (ms.xFractal + 1) << 2,
    dyChip = (ms.yFractal + 1) << 2, cChip = Min(ms.zFractal, 7), cPinChip,
    cPinMaze, ichip, ipin, x, y, z, d, x0, y0, d0, dMax, x1, y1, xT, yT, i, j;
  flag rgfChip[9], fUnder;
  int rgPath[MAXLENGTH], rgLast[MAXLENGTH], rgStack[MAXLENGTH],
    rgSav[MAXLENGTH], cSolve, iPath, iStack, cLimit;

  // Set up initial variables and tables. The fractal Maze is assumed to have
  // the size and number of chips specified in the Maze Settings dialog.

  x = 7 * dxChip + 11; y = 7 * dyChip + 11;
  if (m_x != x || m_y != y) {
    PrintSzNN_W("Bitmap needs to be %d by %d!\n", x, y);
    return -2;
  }
  xl = yl = 6; xh = m_x-7; yh = m_y-7;
  cPinChip = (ms.xFractal + ms.yFractal) << 1;
  cPinMaze = 2 + (cChip + 1) * cPinChip;
  rgSet = RgAllocate(cPinMaze, int);
  if (rgSet == NULL)
    return -1;
  ClearPb(rgSet, cPinMaze * sizeof(int));
  for (ichip = 0; ichip < 9; ichip++)
    rgfChip[ichip] = ichip == 0 || ichip == 8;
  for (ichip = 0; rgszChip[cChip - 1][ichip]; ichip++)
    rgfChip[rgszChip[cChip - 1][ichip] - '0'] = fTrue;

  // Analyze the Maze to determine which pins connect with each other.

  for (ichip = -2; ichip <= cChip; ichip++)
    for (ipin = 0; ipin < cPinChip; ipin++) {
      if (ichip < 0)
        iset = ichip + 2;
      else
        iset++;

      // Loop over each pin on each chip, including start/end and outer rim.
      z = ipin;
      if (z < ms.xFractal)
        d = 0;
      else if (z < ms.xFractal + ms.yFractal) {
        d = 1;
        z -= ms.xFractal;
      } else if (z < (ms.xFractal << 1) + ms.yFractal) {
        d = 2;
        z -= ms.xFractal + ms.yFractal;
      } else {
        d = 3;
        z -= (ms.xFractal << 1) + ms.yFractal;
      }
      z++;

      // Figure out the pixel coordinates of this pin.
      if (ichip == 0) {
        if (!FOdd(d)) {
          x = xl + (z * 7 << 2) - 1;
          y = d == 0 ? xl - 1 : yh + 1;
        } else {
          x = d == 1 ? xl - 1 : xh + 1;
          y = yl + (z * 7 << 2) - 1;
        }
      } else {
        i = ichip == -2 ? 0 : (ichip == -1 ? 8 :
          rgszChip[cChip - 1][ichip - 1] - '0');
        y1 = i / 3; x1 = i - y1*3;
        xT = xl + ((x1 << 1) + 1) * dxChip - 1;
        yT = yl + ((y1 << 1) + 1) * dyChip - 1;
        if (!FOdd(d)) {
          x = xT + (z << 2);
          y = d == 0 ? yT : yT + dyChip;
        } else {
          x = d == 1 ? xT : xT + dxChip;
          y = yT + (z << 2);
        }
      }

      // Ignore pin if no path connects, or it's already part of a set.
      x0 = x; y0 = y;
      if (ichip == 0)
        d ^= 2;
      if (Get(x + xoff[d], y + yoff[d]) || rgSet[iset] > 0)
        continue;
      Assert(iset < cPinMaze);
      rgSet[iset] = iset + 1;

      // Wall follow around from this pin, to see what pins it connects with.
      for (cLimit = m_x*m_y; cLimit > 0; cLimit--) {
        x += xoff[d] << 2; y += yoff[d] << 2;

        // Stop when wall following returns back to start pin.
        if (x == x0 && y == y0)
          break;

        if (!FLegalMaze(x, y)) {

          // Wall following reached the edge of Maze, i.e. an outer rim pin.
          if (y < yl || y > yh) {
            z = x / (7 << 2) - 1;
            if (y > yh)
              z += ms.yFractal + ms.xFractal;
          } else {
            z = y / (7 << 2) - 1 + ms.xFractal;
            if (x > xh)
              z += ms.yFractal + ms.xFractal;
          }
          j = 2 + z;
          Assert(j < cPinMaze && rgSet[j] == 0);
          rgSet[j] = iset + 1;
          d ^= 2;
          x += xoff[d] << 2; y += yoff[d] << 2;
        } else for (i = 0; i < cChip; i++) {

          // Check if wall following reached a pin on an inner chip.
          z = rgszChip[cChip - 1][i] - '0';
          y1 = z / 3; x1 = z - y1*3;
          xT = xl + ((x1 << 1) + 1) * dxChip - 1;
          yT = yl + ((y1 << 1) + 1) * dyChip - 1;
          if (FBetween(x, xT, xT + dxChip) && FBetween(y, yT, yT + dyChip)) {
            if (y <= yT || y >= yT + dyChip) {
              z = (x - xT) / 4 - 1;
              if (y >= yT + dyChip)
                z += ms.yFractal + ms.xFractal;
            } else {
              z = (y - yT) / 4 - 1 + ms.xFractal;
              if (x >= xT + dxChip)
                z += ms.yFractal + ms.xFractal;
            }
            j = 2 + (i + 1) * cPinChip + z;
            Assert(j < cPinMaze && rgSet[j] == 0);
            rgSet[j] = iset + 1;
            d ^= 2;
            x += xoff[d] << 2; y += yoff[d] << 2;
            break;
          }
        }

        // Wall follow to the next cell, which may be under an underpass.
        d0 = d;
        fUnder = (Count(x, y) == 2 && Count2(x, y) == 0 &&
          Get(x - xoff[d], y - yoff[d]));
        d = d0 - 1 & DIRS1;
        for (i = 0; i < DIRS && (Get(x + xoff2[d], y + yoff2[d]) ||
          Get(x + xoff[d], y + yoff[d]) != fUnder); i++)
          d = d + 1 & DIRS1;
      }
    }

  // The set map of the Maze, indicating which pins connect with each other at
  // the top level, has been formed. Now solve the Maze by jumping between
  // pins based on this array. Do a depth first search with an increasing
  // cutoff limit to find solutions from shortest to longest.

  cSolve = 0;
  for (dMax = 3; dMax < ms.nFractalL; dMax += 2 - ms.fFractalI) {
    iPath = iStack = 0;
    d0 = 0; rgPath[iPath] = rgLast[iPath] = d0; iset = rgSet[d0];

    // Search a depth for a long time (but not forever) before giving up.
    for (cLimit = 2000000000; cLimit > 0; cLimit--) {
      for (d = rgPath[iPath] + 1;
        d < cPinMaze && (rgSet[d] != iset || d == d0); d++)
        ;

      // Undo a move on the move list
      if (d >= cPinMaze || iPath >= dMax) {
        // Tried all choices before, or path would be too long
        iPath--;
        if (iPath < 0)  // No solution
          break;
        z = rgPath[iPath];
        d0 = rgLast[iPath];
        iset = rgSet[z];
        if (z < 2 + cPinChip) {

          // Undoing a pop out, means re-entering the chip
          iStack++;
        } else {

          // Undoing a chip entry, means popping out of it
          iStack--;
          rgStack[iStack] = rgSav[iPath];
        }
        continue;
      }

      // Make a move
      rgPath[iPath] = d;
      rgLast[iPath] = d0;
      if (d < 2) {
        if (iPath < dMax - 1)  // Found this solution earlier
          continue;
        if (iStack > 0 && !ms.fFractalI)  // Found solution at lower level
          continue;

        x = y = 0;
        for (i = 0; i < iPath; i++) {
          x += rgPath[i] < cPinChip + 2 ? -1 : 1;
          if (x > y)
            y = x;
        }
        pch = sz;
        sprintf(SO(pch, sz), "Solution #%d of length %d, Max depth %d",
          ++cSolve, dMax, y);
        while (*pch)
          pch++;
        if (ms.fFractalI) {
          sprintf(SO(pch, sz), ", End depth %d", iStack);
          while (*pch)
            pch++;
        }
        sprintf(SO(pch, sz), ":\n");
        while (*pch)
          pch++;

        PrintMoves(&pch, sizeof(sz) - PD(pch - sz), rgPath, iPath, cPinChip);
        if (PrintSzCore(sz, -1/*nAskOkCancel*/) == 1)
          continue;
        goto LExit;
      }

      z = (d - 2) % cPinChip;
      if (d < 2 + cPinChip) {

        // Pop out of the current chip
        if (iStack <= 0)  // Can't go out from top level
          continue;
        z += rgStack[iStack - 1] * cPinChip;
        if (rgSet[z + 2] == 0)  // Can't go out when no connecting wire
          continue;
        iStack--;
      } else {

        // Enter into some chip
        if (iStack >= ms.nFractalD)  // Would go too deep
          continue;
        rgSav[iPath] = rgStack[iStack];
        rgStack[iStack] = (d - 2) / cPinChip;
        iStack++;
      }
      d0 = z + 2;
      iset = rgSet[d0];
      iPath++;
      rgPath[iPath] = 0;
      rgLast[iPath] = d0;
    }
    if (cLimit <= 0) {
      PrintSzNN("%d solutions found up to too complex length %d\n", cSolve,
        dMax);
      goto LExit;
    }
  }
  PrintSzNN("All %d solutions found up to limit length %d\n", cSolve, dMax);

LExit:
  DeallocateP(rgSet);
  return cSolve;
}

/* solve.cpp */
