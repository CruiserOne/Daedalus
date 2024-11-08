/*
** Daedalus (Version 3.5) File: maze.cpp
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
** This file contains Maze utilities, and all Maze operations outside of
** actual creation and solving.
**
** Created: 4/11/1991.
** Last code change: 10/30/2024.
*/

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include "util.h"
#include "graphics.h"
#include "color.h"
#include "threed.h"
#include "maze.h"


#define TRIES 250

CONST char *rgszDir[DIRS] = {"North", "West", "South", "East"};

// Order of directions to take when wall following in a 3D Maze.
CONST int dir3D[DIRS3] = {0, 4, 1, 2, 5, 3};

// DOS characters used by the DOS Text save commands.
CONST char rgchDOS[4] = {' ', '\334', '\337', '\333'};
CONST char rgchDOS2[16] =
  {' ', '\263', '\304', '\331', '\263', '\263', '\277', '\264',
  '\304', '\300', '\304', '\301', '\332', '\303', '\302', '\305'};
CONST char rgchDOS3[16] =
  {' ', '\272', '\315', '\274', '\272', '\272', '\273', '\271',
  '\315', '\310', '\315', '\312', '\311', '\314', '\313', '\316'};

MS ms = {
  // File settings
  0, 0, 0, fFalse, fFalse, 0, 1,
  // Random settings
  109, 0, 0,
  // Dot settings
  0,
  // Maze settings
  fFalse, fFalse, fFalse, fFalse, fFalse, fFalse, fFalse, 239, 0,
    3, 3, 1, epRandom,
  // Labyrinth settings
  5, lcsCircle, 1, fFalse, "3214765", fFalse, 2, 2, 2, 2, lccBalanced, 0,
  // Create settings
  fTrue, fTrue, fTrue, 13, 2, -1, -1, 0, "a2a3a0a1",
    fFalse, fTrue, 10, 1, -100, 15, 15, 0, 4, 4, 3, fFalse,
    fFalse, 1000, TRIES, 0, 0, 0, fFalse, fFalse, fFalse, 4,
  // Macro accessible only settings
  -1, 1, 10, 50, 0,
  // Internal settings
  1, 0, 1, 0, 0, 0, 0, 0, -1, NULL, fFalse, 0, NULL, 0};

int xl = 0, yl = 0, xh = Odd(xStart), yh = Odd(yStart);


/*
******************************************************************************
** Maze Utilities
******************************************************************************
*/

// Display an error message if the active rectangle section of the bitmap
// isn't at least x by y pixels.

flag FMazeSizeError(int x, int y)
{
  if (xh-xl+1 < x || yh-yl+1 < y) {
    PrintSzNN_W("Bitmap section needs to be at least %d by %d!\n", x, y);
    return fTrue;
  }
  return fFalse;
}


// Prepare the bitmap before generating a Maze. Resize the bitmap, and adjust
// the active rectangle within it appropriately.

flag CMaz::FEnsureMazeSize(int z, int grfems)
{
  int x = m_x, y = m_y, i;

  // Ensure the bitmap isn't too large for the Maze.
  if ((grfems & fems64K) > 0 && F64K()) {
    PrintSz_W("Bitmap needs to be smaller than 65536 by 65536!\n");
    return fFalse;
  }

  // Ensure the bitmap is large enough for the Maze.
  if (x < z || y < z) {
    if ((grfems & femsMinSize) == 0) {
      PrintSzNN_W("Bitmap needs to be at least %d by %d!\n", z, z);
      return fFalse;
    }
    if (x < z)
      x = z;
    if (y < z)
      y = z;
    if (!FBitmapSizeSet(x, y))
      return fFalse;
  }

  // If Apply Commands To Rectangle Section is off, or the Maze type in
  // question must fill the whole bitmap, make active rectangle whole bitmap.
  if (!ms.fSection || (grfems & femsNoSection) > 0) {
    if ((grfems & femsNoResize) == 0) {
      x = m_x; y = m_y;
      if (grfems & femsEvenSize) {
        x &= ~1; y &= ~1;
      } else if (grfems & femsOddSize) {
        x = Odd(x); y = Odd(y);
      }
      if (grfems & femsEvenSize2) {
        i = (grfems & femsOddSize) > 0;
        if (x + i & 2)
          x -= 2;
        if (y + i & 2)
          y -= 2;
      }
      if (!FBitmapSizeSet(x, y))
        return fFalse;
      SetXyh();
      return fTrue;
    }
    SetXyh();
  }

  // Adjust or resize the active rectangle.
  if (xh < xl)
    SwapN(xl, xh);
  if (yh < yl)
    SwapN(yl, yh);
  if (xl < 0)
    xl = 0;
  else if (xh >= m_x)
    xh = m_x - 1;
  if (yl < 0)
    yl = 0;
  else if (yh >= m_y)
    yh = m_y - 1;
  if (grfems & femsEvenStart) {
    if (FOdd(xl))
      xl++;
    if (FOdd(yl))
      yl++;
  }
  if (grfems & femsEvenSize) {
    if (!FOdd(xh - xl))
      xh--;
    if (!FOdd(yh - yl))
      yh--;
  } else if (grfems & femsOddSize) {
    if (FOdd(xh - xl))
      xh--;
    if (FOdd(yh - yl))
      yh--;
  }
  if (grfems & femsEvenSize2) {
    i = (grfems & femsOddSize) > 0;
    if (xh - xl + 1 + i & 2)
      xh -= 2;
    if (yh - yl + 1 + i & 2)
      yh -= 2;
  }
  if (xh - xl + 1 < z || yh - yl + 1 < z) {
    PrintSzNN_W("Section needs to be at least %d by %d!\n", z, z);
    return fFalse;
  }
  return fTrue;
}


// Turn off the bottom row and right column of the active rectangle.

void CMaz::BlockOuter(KV o)
{
  LineY(xh, yl, yh-1, o);
  LineX(xl, xh, yh, o);
}


// Set or clear all pixels in the active rectangle.

void CMaz::MazeClear(KV o)
{
  if (xl <= 0 && yl <= 0 && xh >= m_x-2 && yh >= m_y-2) {
    BitmapSet(o);
    if (o) {
      if (xh == m_x-2)
        LineY(xh+1, yl, yh+1, fOff);
      if (yh == m_y-2)
        LineX(xl, xh+1, yh+1, fOff);
    }
  } else
    Block(xl, yl, xh, yh, o);
  if (!o)
    Edge(xl, yl, xh, yh, fOn);
}


// Add an entrance (in top row) or exit (in bottom row) at a random location.
// Implements the Add Entrance and Add Exit commands.

int CMaz::AddEntranceExit(flag fExit)
{
  int x, y = fExit ? yh - (FOdd(yh - yl) && !Get(0, yh)) : yl, i;

  // Search for a location that doesn't already have an opening.
  do {
    for (i = 0; i < TRIES; i++) {
      switch (ms.nEntrancePos) {
      case 0:
        x = fExit ? (xh-2 | 1) - i*2 : (xl | 1) + i*2;
        break;
      case 1:
        x = (((xl + xh) >> 1) | 1) + (FOdd(i) ? i & ~1 : -i);
        break;
      default:
        x = Rnd(xl, xh-2) | 1;
      }
      if (!FBetween(x, xl | 1, xh-2 | 1))
        break;
      if (Get(x, y) && !Get(x, y - fExit*2 + 1)) {
        Set0(x, y);
        return x;
      }
    }
    y += fExit ? -2 : 2;
  } while (FBetween(y, yl, yh));
  return -1;
}


// Create the entrance and exit in the specified type of Maze, based on the
// Entrance Positioning maze setting.

void CMaz::MakeEntranceExit(int nType)
{
  int nEntrancePos = ms.nEntrancePos, xSpan,
    xEntrance, xExit, yEntrance, yExit;

  // Figure out how many possible entrance/exit positions there are.
  switch (nType)
    {
  case 0: // Passages every other pixel
  case 6:
  case 7:
    xSpan = (xh - xl) >> 1;
    break;
  case 1: // Passages every pixel
  case 2:
    xSpan = xh - xl - 1;
    break;
  case 3: // Passages every 4 pixels
    xSpan = (xh - xl) >> 2;
    break;
  case 4: // Passages every other pixel (3D Maze)
  case 5:
    xSpan = ((m_x3 + 1) >> 1) - 1;
    break;
  default:
    Assert(fFalse);
    }

  // Figure out which positions to use for the entrance and exit.
LRetry:
  switch (nEntrancePos)
    {
  case 0: // Corners
    xEntrance = 0;
    xExit     = xSpan - 1;
    break;
  case 1: // Middle
    xEntrance = (xSpan - 1) >> 1;
    xExit     = xEntrance + !FOdd(xSpan);
    break;
  case 2: // Balanced Random
    xEntrance = Rnd(0, xSpan-1);
    xExit     = xSpan - 1 - xEntrance;
    break;
  case 3: // True Random
    xEntrance = Rnd(0, xSpan - 1);
    xExit     = Rnd(0, xSpan - 1);
    break;
  default:
    Assert(fFalse);
    }

  // Actually create the entrance and exit at the chosen positions.
  switch (nType)
    {
  case 0:
  case 6:
  case 7:
    xEntrance = xl + (xEntrance << 1) + 1;
    xExit     = xl + (xExit << 1) + 1;
    yEntrance = yl;
    yExit     = yh-(nType == 7 && FOdd(yh));
    break;
  case 1:
  case 2:
    xEntrance = xl + 1 + xEntrance;
    xExit     = xl + 1 + xExit;
    yEntrance = yl;
    yExit     = yh;
    if (nType == 2 &&
      (Get(xEntrance, yEntrance + 1) || Get(xExit, yExit - 1))) {
      if (nEntrancePos < epRandom)
        nEntrancePos++;
      goto LRetry;
      }
    break;
  case 3:
    xEntrance = xl + (xEntrance << 2) + 3;
    xExit     = xl + (xExit << 2) + 3;
    yEntrance = yl;
    yExit     = yh;
    Set0(xEntrance, yEntrance + 1); Set0(xEntrance, yEntrance + 2);
    Set0(xExit, yExit - 1); Set0(xExit, yExit - 2);
    break;
  case 4:
    xEntrance = X2((xEntrance << 1) + 1, 0);
    xExit     = X2((xExit << 1) + 1, Even(m_z3) - 2);
    yEntrance = Y2(0, 0);
    yExit     = Y2(Even(m_y3) - 2, Even(m_z3) - 2);
    break;
  case 5:
    xEntrance = X4(0, (xEntrance << 1) + 1);
    xExit     = X4(Even(m_w3) - 2, (xExit << 1) + 1);
    yEntrance = Y2(0, 0);
    yExit     = Y4(Even(m_y3) - 2, Even(m_z3) - 2);
    break;
  default:
    Assert(fFalse);
    }
  if (nType != 7) {
    ms.xEntrance = xEntrance; ms.yEntrance = yEntrance;
    Set0(xEntrance, yEntrance);
  }
  if (nType != 6) {
    ms.xExit = xExit; ms.yExit = yExit;
    Set0(xExit, yExit);
  }
}


// Add or remove a random wall segment in a Maze. Implements the Add Passage
// and Add Wall commands.

flag CMaz::MakeIsolationDetachment(flag fDetach)
{
  int x, y, i;

  if (FMazeSizeError(6, 6))
    return fFalse;

  // Search for a passage or wall that can be changed.
  for (i = 0; i < TRIES; i++) {
    y = Rnd(yl+1, yh-1 - FOdd(yh-yl));
    x = xl + 1 + FOdd(y) + (Rnd(0, (xh >> 1) - FOdd(y) - 1) << 1);
    if (Get(x, y) == fDetach && FOnWall(x, y) && FOnPassage(x, y)) {
      Set(x, y, !fDetach);
      return fTrue;
    }
  }
  return fFalse;
}


// Would making a wall at the given coordinates create a new dead end? Used in
// the creation of Braid Mazes and when connecting poles, to avoid dead ends.

flag CMaz::FWouldMakeDeadEnd(int x, int y) CONST
{
  if ((x - xl) & 1) {
    if ((y - yl) & 1)
      return fTrue;  // Cell center
    else
      return Count(x, y - 1) >= 2 || Count(x, y + 1) >= 2;
  } else {
    if ((y - yl) & 1)
      return Count(x - 1, y) >= 2 || Count(x + 1, y) >= 2;
    else
      return fFalse;  // Wall vertex
  }
}


// Would making a passage at the given coordinates create a new pole? Called
// from DoCrackDeadEnds to avoid making poles.

flag CMaz::FWouldMakePole(int x, int y) CONST
{
  if ((x - xl) & 1) {
    if ((y - yl) & 1)
      return fFalse;  // Cell center
    else
      return (x > xl+1 && Count(x - 1, y) <= 1) ||
        (x < xh-1 && Count(x + 1, y) <= 1);
  } else {
    if ((y - yl) & 1)
      return (y > yl+1 && Count(x, y - 1) <= 1) ||
        (y < yh-1 && Count(x, y + 1) <= 1);
    else
      return fTrue;  // Wall vertex
  }
}


// Would making a wall at the given coordinates create a new inaccessible
// section? Used in the creation of Braid Mazes to avoid isolated sections.

flag CMaz::FWouldMakeIsolation(int x, int y) CONST
{
  int m[2], n[2], d[2], d0[2], i;

  // Two wall following robots start in opposite directions from the new wall.
  for (i = 0; i < 2; i++) {
    m[i] = x; n[i] = y;
    d[i] = d0[i] = (i << 1) + FOdd(y - yl);
  }

  // Keep wall following until one of the robots returns to where it started.
  loop {
    for (i = 0; i < 2; i++) {
      d[i] = FollowWall(&m[i], &n[i], d[i], fTrue);
      if (m[i] == x && n[i] == y)
        goto LDone;
      else if ((n[i] <= yl && d[i] == 0) || (n[i] >= yh && d[i] == 2))
        d[i] ^= 2;  // Treat entrances as dead ends.
    }
  }

  // If the robot returned from the same direction it was sent down (as
  // opposed to returning from a different direction) that passage forms a
  // blind alley or the sole path to an entrance, so can't put a wall here.
LDone:
  return d[i] != d0[i];
}


// Given a cell coordinate and a direction being faced, return the direction
// to move to follow the left or right wall. Used when wall following.

int CMaz::PeekWall(int x, int y, int z, int dir, flag f3D, int right) CONST
{
  int xnew, ynew, znew, i, j;

  if (right != 1)
    right = -1;
  if (!f3D) {

    // Search for the first available passage leading out of this cell.
    dir += 2 + right;
    for (i = 0; i < DIRS; i++) {
      dir &= DIRS1;
      xnew = x + xoff[dir]; ynew = y + yoff[dir];
      if (!Get(xnew, ynew))
        return dir;
      dir += right;
    }
  } else {

    // 3D wall following is implemented like 2D wall following, with the
    // assumption that up is northwest, and down is southeast.
    for (i = 0; i < DIRS3; i++)
      if (dir == dir3D[i])
        break;
    j = i + 3 + right;
    for (i = 0; i < DIRS3; i++) {
      if (j < 0)
        j += DIRS3;
      else if (j >= DIRS3)
        j -= DIRS3;
      dir = dir3D[j];
      xnew = x + xoff3[dir]; ynew = y + yoff3[dir]; znew = z + zoff3[dir];
      if (!Get3I(xnew, ynew, znew))
        return dir;
      j += right;
    }
  }
  return -1;
}


// Follow a wall. Like PeekWall() but also moves to a neighboring cell.

int CMaz::FollowWall(int *x, int *y, int dir, int right) CONST
{
  int xnew, ynew;

  // Move forward if possible, then return the new direction to face.
  xnew = *x + xoff[dir]; ynew = *y + yoff[dir];
  if (!Get(xnew, ynew)) {
    *x = xnew; *y = ynew;
  }
  return PeekWall(*x, *y, 0, dir, fFalse, right);
}


// Return whether the passage leading in the given direction from given
// coordinates forms a blind alley with a reasonably small circumference.

flag CMaz::FBlindAlley(int x, int y, int dir) CONST
{
  int x0, y0, d0, i, dOld, dd, dSum = 0;

  // Send a wall following robot down the passage. Follow for up to Radar
  // Length cells, or until the robot returns to where it started from.

  x0 = x; y0 = y; d0 = dir;
  for (i = ms.nRadar << 2; i > 0; i--) {
    dOld = d0; d0 = FollowWall(&x0, &y0, d0, fTrue); dd = (d0 - dOld);

    // If the robot returned from the same direction it was sent down, and it
    // turned around in the right direction once, this is a blind alley.

    if (x0 == x && y0 == y)
      return dOld == (dir + 2 & DIRS1) && dSum >= 0;
    dSum += (dd == 3 ? 1 : (dd == -3 ? -1 : (dd == -2 ? 2 : dd)));
  }
  return fFalse;
}


// Follow a passage one cell in the direction being faced, updating the passed
// in coordinates and returning the new direction. Return -1 when a junction
// is reached. Ignore junction passages forming blind alleys smaller than the
// Radar Length setting. Implements the Follow Passages dot motion mode.

int CMaz::FollowPassage(int *x, int *y, int *z, int dir, flag f3D) CONST
{
  int xnew, ynew, znew, dirMax, dirRev, d, cf = 0;
  flag f[DIRS3];

  // Figure out how many passages lead from this cell.
  dirMax = f3D ? DIRS3 : DIRS;
  for (d = 0; d < dirMax; d++) {
    xnew = *x + xoff3[d]; ynew = *y + yoff3[d];
    if (!f3D)
      f[d] = Get(xnew, ynew);
    else {
      znew = *z + zoff3[d];
      f[d] = Get3M(xnew, ynew, znew);
    }
    if (f[d])
      cf++;
  }

  // Ignore passages forming small blind alleys if Radar Length is > 0.
  if (cf <= 1 && !f3D) {
    for (d = 0; d < DIRS; d++) {
      if (!f[d] && ms.nRadar > 0 && FBlindAlley(*x, *y, d)) {
        f[d] = fTrue;
        cf++;
      }
    }
  }

  // If there are more than two passages left, stop here.
  if (cf <= (f3D ? 3 : 1))
    return -1;

  // Figure out which passage to take. Don't turn around unless only option.
  dirRev = dir < DIRS ? dir ^ 2 : 9-dir;
  for (d = 0; d < dirMax; d++) {
    if (!f[d] && d != dirRev)
      goto LFound;
  }
  if (!f[dirRev]) {
    d = dirRev;
    goto LFound;
  }
  return -1;

LFound:
  *x += xoff3[d]; *y += yoff3[d];
  if (f3D)
    *z += zoff3[d];
  return d;
}


// Given a cell coordinate and direction being faced, randomly return the
// direction of an available passage. Used by the Random dot move command.

int CMaz::PeekRandom(int x, int y, int z, int dir, flag f3D) CONST
{
  int xnew, ynew, znew, dirMax, dirRev, i, j, cdir = 0;
  flag f[DIRS3];

  dirMax = f3D ? DIRS3 : DIRS;
  dirRev = dir < DIRS ? dir ^ 2 : 9-dir;

  // Figure out how many passages lead from this cell.
  for (i = 0; i < dirMax; i++) {
    f[i] = fFalse;
    xnew = x + xoff3[i]; ynew = y + yoff3[i];
    if (!f3D) {
      if (FLegal(xnew, ynew) && !Get(xnew, ynew)) {

        // Don't allow movement away from a wall into the middle of a room.
        for (j = 0; j < DIRS2; j++)
          if (Get(xnew + xoff[j], ynew + yoff[j])) {
            f[i] = fTrue;
            cdir++;
            break;
          }
      }
    } else {
      znew = z + zoff3[i];
      if (FLegalCubeLevel(xnew, ynew) && znew >= 0 && znew < Odd(m_z3) &&
        !Get3(xnew, ynew, znew)) {
        f[i] = fTrue;
        cdir++;
      }
    }
  }

  // If no passages lead from this cell, return a random direction.
  if (cdir == 0)
    return Rnd(0, dirMax);

  // Don't turn around unless that's the only option.
  if (f[dirRev]) {
    f[dirRev] = fFalse;
    cdir--;
  }

  // Randomly pick one of the available directions.
  if (cdir > 0) {
    j = Rnd(1, cdir);
    for (i = 0; i < dirMax; i++)
      if (f[i]) {
        j--;
        if (j < 1)
          return i;
      }
  }
  return dirRev;
}


// Update coordinates to the first available off pixel in a row or column.
// Implements the Entrance and Exit dot teleport commands.

flag CMaz::FFindPassage(int *x, int *y, flag fVertical) CONST
{
  int m = *x, n = *y;
  flag fRet = fFalse;

  while (fVertical ? ++n < yh : ++m < xh) {
    if (!Get(m, n)) {
      fRet = fTrue;
      break;
    }
  }
  *x = m; *y = n;
  return fRet;
}


// Randomly return the direction of a cell adjacent to the given coordinates
// that's not part of the Maze yet, evenly distributed among the available
// directions. Used by several Maze creation algorithms.

int CMaz::DirFindUncreated(int *x, int *y, flag fWall) CONST
{
  int rgdir[DIRS1], xnew, ynew, d, i, cdir;

  // Fast case: Try a random direction. If can move there, done already.
  d = RndDir();
  xnew = *x + xoff2[d]; ynew = *y + yoff2[d];
  if (FLegalMaze(xnew, ynew) && (Get(xnew, ynew) ^ fWall)) {
    *x = xnew; *y = ynew;
    return d;
  }

  // Standard case: Figure out how many other passages lead from this cell.
  cdir = 0;
  for (i = 0; i < DIRS1; i++) {
    DirInc(d);
    xnew = *x + xoff2[d]; ynew = *y + yoff2[d];
    if (FLegalMaze2(xnew, ynew) && (Get(xnew, ynew) ^ fWall)) {
      rgdir[cdir] = d;
      cdir++;
    }
  }

  // Randomly pick one of the available directions, if any.
  if (cdir < 1)
    return -1;
  d = rgdir[Rnd(0, cdir-1)];
  *x += xoff2[d]; *y += yoff2[d];
  return d;
}


// Return a random one of the four directions. Takes into account the Bias and
// Run random settings. Used by many creation and other Maze algorithms.

int RndDir()
{
  if (ms.cRunRnd > 0) {

    // If in the middle of a random run, return the previous direction again.
    ms.cRunRnd--;
  } else {

    // Randomly pick a new direction, and a new random run length if any.
    if (ms.nRndRun > 0)
      ms.cRunRnd = Rnd(0, ms.nRndRun);
    ms.dirRnd = Rnd(0, DIRS1 + NAbs(ms.nRndBias)*2);
  }

  // Return a standard direction.
  if (ms.dirRnd < DIRS)
    return ms.dirRnd;

  // Higher random numbers represent bias. Map them to a standard direction.
  return ((ms.dirRnd & 1) << 1) + (ms.nRndBias > 0);
}


/*
******************************************************************************
** Maze Operations
******************************************************************************
*/

// Implements the Add Passages and Add Walls normalize commands.

long CMaz::MazeNormalize(flag fWall)
{
  int x, y;
  long count = 0;

  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize))
    return fFalse;
  if (!fWall) {

    // Ensure all odd coordinate pixels in the middle of cells are off.
    for (y = yl + 1; y < yh; y += 2)
      for (x = xl + 1; x < xh; x += 2) {
        if (Get(x, y)) {
          count++;
          Set0(x, y);
        }
      }
  } else {

    // Ensure all even coordinate pixels at wall endpoints are on.
    for (y = yl; y <= yh; y += 2)
      for (x = xl; x <= xh; x += 2) {
        if (!Get(x, y)) {
          count++;
          Set1(x, y);
        }
      }
  }
  return count;
}


// Zoom the bitmap by a factor of two, doubling its size. Unlike normal zoom,
// here each set pixel becomes a 3x3 instead of a 2x2 section in the new
// bitmap. Implements the Expand Set Maze command.

long CMaz::MazeZoomAndExpandSetCells()
{
  CMaz bNew;
  int x, y;
  long count = 0;

  // Quickly zoom the bitmap by 200%.
  if (!bNew.FAllocate((m_x << 1) + FOdd(m_x), (m_y << 1) + FOdd(m_y), this))
    return -1;
  bNew.SetXyh();
  bNew.BitmapOff();
  for (y = 0; y < (bNew.m_y - 1 & ~1); y++)
    for (x = 0; x < (bNew.m_x - 1 & ~1); x++)
      if (Get(x >> 1, y >> 1))
        bNew.Set1(x, y);

  // For each set 2x2 block, turn on extra pixels making it a 3x3 block.
  for (y = yl + 1; y < yh; y += 2)
    for (x = xl + 1; x < xh; x += 2)
      if (bNew.Get(x, y)) {
        count++;
        bNew.Set1(x + 1, y - 1);
        bNew.Set1(x + 1, y    );
        bNew.Set1(x + 1, y + 1);
        bNew.Set1(x    , y + 1);
        bNew.Set1(x - 1, y + 1);
      }

  CopyFrom(bNew);
  return count;
}


// Make each cell in a Maze that's been bias zoomed such that walls are one
// pixel and passages three pixels thick, look like a small room with one
// pixel wide doors to adjacent cells, or undo such edits by changing modified
// pixels back to the way they started. Implements the Room Thinned command.

long CMaz::MazeRoomifyThinnedCells()
{
  int x, y;
  long count = 0;

  if (!FEnsureMazeSize(5, femsOddSize | femsNoResize))
    return fFalse;
  for (y = yl + 2; y < yh - 1; y += 4)
    for (x = xl + 2; x < xh - 1; x += 4)
      if (FOnMaze2(x, y)) {
        count++;
        Inv(x, y);
        if (!Get(x - 2, y)) {
          Inv(x - 2, y - 1);
          Inv(x - 2, y + 1);
        }
        if (!Get(x, y - 2)) {
          Inv(x - 1, y - 2);
          Inv(x + 1, y - 2);
        }
        if (!Get(x + 2, y) && (x + 6 > xh || !FOnMaze2(x + 4, y))) {
          Inv(x + 2, y - 1);
          Inv(x + 2, y + 1);
        }
        if (!Get(x, y + 2) && (y + 6 > yh || !FOnMaze2(x, y + 4))) {
          Inv(x - 1, y + 2);
          Inv(x + 1, y + 2);
        }
      }
  return count;
}


// Make all wall endpoints in a Maze point in a new random direction.
// Implements the Tweak Endpoints command.

long CMaz::MazeTweakEndpoints()
{
  int x, y, dir;
  long count = 0;

  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize))
    return fFalse;
  for (y = yl + 2; y < yh - 1; y += 2)
    for (x = xl + 2; x < xh - 1; x += 2)
      if (FOnMaze2(x, y) && Count(x, y) == 1) {
        count++;
        Set0(x, y-1); Set0(x-1, y); Set0(x, y+1); Set0(x+1, y);
        dir = Rnd(0, DIRS1);
        Set1(x + xoff[dir], y + yoff[dir]);
      }
  return count;
}


#define NBinary2(f0, f1, f2, f3, f4, f5, f6, f7) \
  ((f0 << 3) | (f1 << 6) | (f2 << 2) | (f3 << 5) | \
  (f4 << 1) | (f5 << 4) | (f6 << 0) | (f7 << 7))

// Slightly modify or shake passages in a Maze that are next to solid cells.
// Implements the Tweak Passages command.

long CMaz::MazeTweakPassages()
{
  int nCorner, nEdge, nUturn, xspan, yspan, x, y, grf, grfToggle, i;
  flag fCorner, fEdge, fUturn, f1, f2, f3, f4;
  long total, z, zInc, j, count = 0;

  if (!FEnsureMazeSize(3, femsOddSize | femsNoResize))
    return fFalse;
  nCorner = ms.nTweakPassage / 10 % 10;
  nEdge = ms.nTweakPassage % 10;
  nUturn = ms.nTweakPassage / 100;
  xspan = (xh - xl - 2) >> 1;
  yspan = (yh - yl - 2) >> 1;
  total = xspan * yspan;
  z = Rnd(1, total);
  zInc = LPrime(total);

  for (j = 0; j < total; j++) {
    z += zInc;
    while (z >= total)
      z -= total;
    y = z / xspan;
    x = xl + ((z - y * xspan) << 1) + 2;
    y = yl + (y << 1) + 2;
    if (Get(x, y) && FOnMaze2(x, y) && Count2(x, y) == 4) {
      fCorner = nCorner > 0 && Rnd(1, nCorner) == 1;
      fEdge   = nEdge   > 0 && Rnd(1, nEdge)   == 1;
      fUturn  = nUturn  > 0 && Rnd(1, nUturn)  == 1;
      if (!(fCorner || fEdge || fUturn))
        continue;

      // Figure out the status of pixels surrounding this wall endpoint.
      grf = 0;
      for (i = 0; i < DIRS2; i++)
        grf = (grf << 1) | Get(x + xoff[i], y + yoff[i]);
      f1 = Get(x-1, y-2) && Get(x-2, y-1);
      f2 = Get(x-1, y+2) && Get(x-2, y+1);
      f3 = Get(x+1, y+2) && Get(x+2, y+1);
      f4 = Get(x+1, y-2) && Get(x+2, y-1);

      // Toggle between types of corner.
      if (fCorner) {
        if (grf == NBinary2(0,0,0,1,1,1,0,0) && f1) {
          grfToggle = NBinary2(1,1,0,1,1,1,0,1);
          goto LTweak;
        }
        if (grf == NBinary2(0,0,0,0,0,1,1,1) && f2) {
          grfToggle = NBinary2(0,1,1,1,0,1,1,1);
          goto LTweak;
        }
        if (grf == NBinary2(1,1,0,0,0,0,0,1) && f3) {
          grfToggle = NBinary2(1,1,0,1,1,1,0,1);
          goto LTweak;
        }
        if (grf == NBinary2(0,1,1,1,0,0,0,0) && f4) {
          grfToggle = NBinary2(0,1,1,1,0,1,1,1);
          goto LTweak;
        }
      }

      // Turn straight edges into U-turns.
      if (fEdge) {
        if (grf == NBinary2(0,1,1,1,1,1,0,0)) {
          grfToggle = NBinary2(0,1,1,1,1,1,0,1);
          goto LTweak;
        }
        if (grf == NBinary2(0,0,0,1,1,1,1,1)) {
          grfToggle = NBinary2(0,1,0,1,1,1,1,1);
          goto LTweak;
        }
        if (grf == NBinary2(1,1,0,0,0,1,1,1)) {
          grfToggle = NBinary2(1,1,0,1,0,1,1,1);
          goto LTweak;
        }
        if (grf == NBinary2(1,1,1,1,0,0,0,1)) {
          grfToggle = NBinary2(1,1,1,1,0,1,0,1);
          goto LTweak;
        }
      }

      // Turn U-turns into straight edges.
      if (fUturn) {
        if (grf == NBinary2(0,0,0,0,0,0,0,1) && f2 && f3) {
          grfToggle = NBinary2(0,1,1,1,1,1,0,1);
          goto LTweak;
        }
        if (grf == NBinary2(0,1,0,0,0,0,0,0) && f3 && f4) {
          grfToggle = NBinary2(0,1,0,1,1,1,1,1);
          goto LTweak;
        }
        if (grf == NBinary2(0,0,0,1,0,0,0,0) && f4 && f1) {
          grfToggle = NBinary2(1,1,0,1,0,1,1,1);
          goto LTweak;
        }
        if (grf == NBinary2(0,0,0,0,0,1,0,0) && f1 && f2) {
          grfToggle = NBinary2(1,1,1,1,0,1,0,1);
          goto LTweak;
        }
      }
      continue;

LTweak:
      // Invert pixels next to the wall endpoint to perform the toggle.
      count++;
      for (i = 0; i < DIRS2; i++)
        if (grfToggle & (1 << (DIRS2-1 - i)))
          Inv(x + xoff[i], y + yoff[i]);
    }
  }
  return count;
}


// Remove all internal walls within a Maze, leaving only the boundary wall and
// a field of poles in the interior. Implements the Crack Cells command.

long CMaz::DoSetAllCellsToPoles()
{
  int x, y;
  long count = 0;

  for (y = yl + 1; y < yh; y += 2)
    for (x = xl + 1; x < xh; x += 2)
      if (FOnMaze(x, y)) {
        count++;
        if (x - 2 >= xl && FOnMaze(x - 2, y))
          Set0(x - 1, y);
        if (x + 2 <  xh && FOnMaze(x + 2, y))
          Set0(x + 1, y);
        if (y - 2 >= yl && FOnMaze(x, y - 2))
          Set0(x, y - 1);
        if (y + 2 <  yh && FOnMaze(x, y + 2))
          Set0(x, y + 1);
      }
  return count;
}


enum _isolationdetachmentcell {
  idIn            = 0,
  idEffectivelyIn = 1,
  idFrontier      = 2,
  idOut           = 3,
  idNever         = 4,
};

#define AssignId(iTo, iFrom) \
  id[iTo].zList = id[iFrom].zList; \
  id[id[iTo].zList].iBack = iTo

// Mark a cell as part of the set of cells known to not be part of an isolated
// section or detached wall. Called from DoRemoveIsolationDetachment().

void CMaz::RemoveIdIn(ID *id, int x, int y, int xs, int ys,
  long *cEffectivelyIn, long *cFrontier, flag fDetach)
{
  long z;
  int xnew, ynew, xp, yp, d;
  flag fIsolate = !fDetach;

  z = (long)y*xs + x;
  id[z].set = idIn;

  // Update the status of the four adjacent cells.
  for (d = 0; d < DIRS; d++) {
    xnew = x + xoff[d]; ynew = y + yoff[d];
    if (xnew >= 0 && ynew >= 0 && xnew < xs && ynew < ys) {
      z = (long)ynew*xs + xnew;
      if (id[z].set == idFrontier || id[z].set == idOut) {
        xp = xl + (x << 1) + fIsolate + xoff[d];
        yp = yl + (y << 1) + fIsolate + yoff[d];
        if (fIsolate ? Get(xp, yp) : !Get(xp, yp)) {

          // If no direct connection to the adjacent cell, make it FRONTIER,
          // where a future iteration may carve into the adjacent cell.
          if (id[z].set == idOut) {
            id[z].set = idFrontier;
            id[*cEffectivelyIn + *cFrontier].zList = z;
            id[z].iBack = *cEffectivelyIn + *cFrontier;
            (*cFrontier)++;
          }
        } else {

          // If a direct connection to the adjacent cell, make it
          // EFFECTIVELYIN, to be made IN before the next iteration.
          if (id[z].set == idOut) {
            AssignId(*cEffectivelyIn + *cFrontier, *cEffectivelyIn);
          } else {
            AssignId(id[z].iBack, *cEffectivelyIn);
            (*cFrontier)--;
          }
          id[z].set = idEffectivelyIn;
          id[*cEffectivelyIn].zList = z;
          (*cEffectivelyIn)++;
        }
      }
    }
  }
}


// Remove all isolated sections in a Maze by adding passages connecting them
// to the rest of the Maze, or remove all detached walls or loops by adding
// walls connecting them.

long CMaz::DoRemoveIsolationDetachment(flag fDetach)
{
  ID *id;
  long count = 0, cEffectivelyIn = 0, cFrontier = 0, i;
  int x, y, xs, ys, j, xnew, ynew, d;
  flag fIsolate = !fDetach;

  if (FMazeSizeError(3, 3))
    return fFalse;
  xs = ((xh - xl | 1) + fDetach) >> 1; ys = ((yh - yl | 1) + fDetach) >> 1;
  id = RgAllocate(xs*ys, ID);
  if (id == NULL)
    return -1;
  for (i = (long)xs*ys-1L; i >= 0L; i--) {
    id[i].set = idOut;
    id[i].zList = 0;
  }

  // Figure out which cells are part of the Maze and which should be ignored
  // (because they're in the middle of rooms or solid blocks). Also figure out
  // which cell to start flooding from.
  j = fTrue;
  for (y = 0; y < ys; y++)
    for (x = 0; x < xs; x++) {
      if (Get(xl + (x << 1) + fIsolate,
        yl + (y << 1) + fIsolate) != fDetach)
        id[(long)y*xs + x].set = idNever;
      else if (j) {
        j = fFalse;
        RemoveIdIn(id, x, y, xs, ys, &cEffectivelyIn, &cFrontier, fDetach);
      } else {
        xnew = x; ynew = y;
      }
    }
  // For detached wall remover, start from two locations if possible, to
  // account for the two detached halves of a standard Maze.
  if (fDetach && !j)
    RemoveIdIn(id, xnew, ynew, xs, ys, &cEffectivelyIn, &cFrontier, fTrue);

  // Use a modified version of Prim's algorithm to cover each reachable cell.
  loop {

    // Flood any new cells reachable from the current set of cells.
    while (cEffectivelyIn > 0) {
      i = cEffectivelyIn - 1;
      y = (int)(id[i].zList / xs); x = (int)(id[i].zList % xs);
      AssignId(i, cEffectivelyIn+cFrontier-1);
      cEffectivelyIn--;
      RemoveIdIn(id, x, y, xs, ys, &cEffectivelyIn, &cFrontier, fDetach);
    }

    // Done when there are no more cells to carve into.
    if (cFrontier <= 0)
      break;

    // Pick a random unreached cell adjacent to the set of current cells.
    count++;
    i = Rnd(cEffectivelyIn, cEffectivelyIn + cFrontier - 1);
    y = (int)(id[i].zList / xs); x = (int)(id[i].zList % xs);
    d = RndDir();
    for (j = 0; j < DIRS; j++) {
      xnew = x + xoff[d]; ynew = y + yoff[d];
      if (xnew >= 0 && ynew >= 0 && xnew < xs && ynew < ys &&
        id[(long)ynew*xs + xnew].set == idIn) {
        Set(xl + (x << 1) + fIsolate + xoff[d],
          yl + (y << 1) + fIsolate + yoff[d], fDetach);
        break;
      }
      DirInc(d);
    }
    AssignId(i, cEffectivelyIn+cFrontier-1);
    cFrontier--;
    RemoveIdIn(id, x, y, xs, ys, &cEffectivelyIn, &cFrontier, fDetach);
  }
  DeallocateP(id);
  return count;
}


// Remove all poles from the Maze, or single pixel wall segments surrounded by
// a small loop, by adding wall segments to connect them with the rest of the
// Maze. Avoids creating dead ends with the new wall segment if possible. Used
// in the creation of Braid Mazes and implements the Connect Poles command.

long CMaz::DoConnectPoles(flag fForce)
{
  int x, y, xnew, ynew, d, i;
  long count = 0;

  for (y = yl + 2; y <= yh - 2; y += 2)
    for (x = xl + 2; x <= xh - 2; x += 2)
      if (Get(x, y) && Count(x, y) == 0) {
        d = RndDir();
        for (i = 0; i < DIRS; i++) {
          xnew = x + xoff[d];
          ynew = y + yoff[d];
          if (Get(xnew + xoff[d], ynew + yoff[d]) &&
            !FWouldMakeDeadEnd(xnew, ynew))
            break;
          DirInc(d);
        }
        if (fForce || i < DIRS) {
          count++;
          Set1(xnew, ynew);
        }
      }
  return count;
}


// Remove all poles from the Maze, or single pixel wall segments surrounded by
// a small loop, by turning off the pole pixel, leaving a small room.
// Implements the Remove Poles command.

long CMaz::DoDeletePoles(flag fAll)
{
  int x, y, zInc, d;
  long count = 0;

  zInc = 2 - fAll;
  for (y = yl; y <= yh; y += zInc)
    for (x = xl; x <= xh; x += zInc) {
      if (Get(x, y)) {
        for (d = 0; d < DIRS; d++) {
          if (Get(x + xoff[d], y + yoff[d]))
            goto LNext;
        }
        count++;
        Set0(x, y);
      }
LNext:
      ;
    }
  return count;
}


// Remove all dead ends from the Maze, by randomly removing a wall segment at
// the end of each dead end. Implements the Crack Dead Ends command.

long CMaz::DoCrackDeadEnds()
{
  int x, y, xnew, ynew, xsav, ysav, d, i;
  long count = 0;

  for (y = yl + 1; y < yh; y += 2)
    for (x = xl + 1; x < xh; x += 2)
      if (FOnMaze(x, y) && Count(x, y) > 2) {
        d = RndDir();
        xsav = -1;

        // For each dead end, look in the four directions to see if removing
        // a wall there would avoid making a new entrance and avoid a pole.
        for (i = 0; i < DIRS; i++) {
          xnew = x + xoff2[d];
          ynew = y + yoff2[d];
          if (FLegalMaze2(xnew, ynew) && FOnMaze(xnew, ynew)) {
            xnew = (x + xnew) >> 1;
            ynew = (y + ynew) >> 1;
            if (Get(xnew, ynew)) {
              xsav = xnew; ysav = ynew;
              if (!FWouldMakePole(xnew, ynew))
                goto LCrack;
            }
          }
          DirInc(d);
        }
        if (xsav >= 0) {
LCrack:
          count++;
          Set0(xsav, ysav);
        }
      }
  return count;
}


// Remove all entrances from the Maze, by adding wall segments to seal them
// off. An entrance is considered to be any passage that has part of the Maze
// on one side of it, and an open room or the edge of the bitmap on the other.
// Implements the Seal Entrances command.

long CMaz::DoCrackEntrances()
{
  int x, y;
  long count = 0;

  for (y = yl; y <= yh; y++)
    for (x = xl + 1 - FOdd(y); x <= xh; x += 2)
      if (!Get(x, y))
        if (FOnMaze(x | 1, y | 1) !=
          FOnMaze(x - FOdd(y), y - FOdd(x))) {
          count++;
          Set1(x, y);
        }
  return count;
}


// Fill in all passages or cells in the Maze, making them a solid area. Leaves
// rooms or open spaces in the Maze alone. Implements Fill Passages command.

long CMaz::DoCrackPassages()
{
  int x, y;
  long count = 0;

  // For each cell within the Maze, convert it into a 3x3 block of on pixels.
  for (y = yl + 1; y < yh; y += 2)
    for (x = xl + 1; x < xh; x += 2)
      if (FOnMaze(x, y)) {
        count++;
        Set1(x, y);
        Set1(x, y-1);
        Set1(x-1, y);
        Set1(x, y+1);
        Set1(x+1, y);
      }
  return count;
}


// Fill in the specified cell, replacing it with a 3x3 block, if doing so
// won't create a detached wall or isolation. Called from DoCrackRooms().

flag CMaz::FCrackRoom(int x, int y)
{
  int grf, i;

  // Figure out the status of pixels around the circumference of this cell.
  grf = 0;
  for (i = 0; i < DIRS2; i++)
    if (Get(x + xoff[i], y + yoff[i]))
      grf |= (1 << i);

  // Fill in the cell if it consists of a solid edge or a solid corner.
  if (grf == NBinary(1,1,1,0,0,0,1,1) ||
    grf == NBinary(1,1,1,1,1,0,0,0) ||
    grf == NBinary(0,0,1,1,1,1,1,0) ||
    grf == NBinary(1,0,0,0,1,1,1,1) ||
    grf == NBinary(1,0,0,0,0,0,1,1) ||
    grf == NBinary(1,1,1,0,0,0,0,0) ||
    grf == NBinary(0,0,1,1,1,0,0,0) ||
    grf == NBinary(0,0,0,0,1,1,1,0)) {
    Block(x-1, y-1, x+1, y+1, fOn);
    return fTrue;
    }
  return fFalse;
}


// Fill in all rooms or open spaces in the Maze, by thickening the walls next
// to open spaces, converting most everything into one pixel wide passages.
// Implements the Fill Open Cells command.

long CMaz::DoCrackRooms()
{
  int x, y;
  long count = 0;

  // Do a pass from upper left to lower right.
  for (y = yl + 1; y < yh; y += 2)
    for (x = xl + 1; x < xh; x += 2)
      if (!FOnMaze(x, y))
        count += FCrackRoom(x, y);

  // Do a pass in reverse order to catch cells in upper left not filled above.
  for (y = yh - 1 - FOdd(yh - yl); y > yl; y -= 2)
    for (x = xh - 1 - FOdd(xh - xl); x > xl; x -= 2)
      if (!FOnMaze(x, y))
        count += FCrackRoom(x, y);
  return count;
}


// Get a pixel value, given a mode for how to handle out of range
// coordinates. Options are treat as off, treat as on, or wrap around.

KV CMaz::GetB(int x, int y, int nMode)
{
  if (nMode >= 2)
    Legalize2(&x, &y);
  else if (!FLegal(x, y))
    return nMode;
  return Get(x, y);
}


// Remove cells or pixels adjacent to open spaces or the edge of the bitmap,
// i.e. remove all outer and inner boundary walls in the Maze, or walls with
// open space on one or both sides. Implements the Remove Boundary command.

long CMaz::DoCrackBoundaryWall(flag fCorner, int nMode)
{
  CMaz bCopy;
  int x, y, zInc = 2 - ms.fSolveEveryPixel, n;
  long count = 0;

  if (!bCopy.FBitmapCopy(*this))
    return -1;
  for (y = yl - 1; y <= yh + 1; y += zInc)
    for (x = xl - 1; x <= xh + 1; x += zInc) {
      if (!ms.fSolveEveryPixel) {

        // Clear cells that aren't entirely within the Maze.
        n = bCopy.Get(x-1, y-1) + bCopy.Get(x-1, y+1) + bCopy.Get(x+1, y+1) +
          bCopy.Get(x+1, y-1);
        if (n < DIRS) {
          if (n > 0)
            count++;
          Block(x-1, y-1, x+1, y+1, fOff);
        }
      } else {

        // Delete or turn off pixels adjacent to other off pixels.
        if (Get(x, y)) {
          n = !bCopy.GetB(x, y-1, nMode) + !bCopy.GetB(x-1, y, nMode) +
            !bCopy.GetB(x, y+1, nMode) + !bCopy.GetB(x+1, y, nMode);
          if (fCorner)
            n += !bCopy.GetB(x-1, y-1, nMode) + !bCopy.GetB(x-1, y+1, nMode) +
              !bCopy.GetB(x+1, y+1, nMode) + !bCopy.GetB(x+1, y-1, nMode);
          if (n > 0) {
            count++;
            Set0(x, y);
          }
        }
      }
    }
  return count;
}


// Remove all passage tube cells from the Maze, or horizontal or vertical
// passage segments with open space on both sides of the walls forming the
// passage. Implements the Remove Tubes command.

long CMaz::DoCrackTubes()
{
  CMaz bCopy;
  int x, y;
  long count = 0;
  flag fx1, fx2, fy1, fy2;

  if (!bCopy.FBitmapCopy(*this))
    return -1;
  for (y = yl + 1; y < yh; y += 2)
    for (x = xl + 1; x < xh; x += 2)
      if (bCopy.FOnMaze(x, y)) {
        fx1 = !bCopy.Get(x-3, y-1) && !bCopy.Get(x+3, y-1);
        fx2 = !bCopy.Get(x-3, y+1) && !bCopy.Get(x+3, y+1);
        fy1 = !bCopy.Get(x-1, y-3) && !bCopy.Get(x-1, y+3);
        fy2 = !bCopy.Get(x+1, y-3) && !bCopy.Get(x+1, y+3);
        if (fx1 || fx2 || fy1 || fy2) {
          count++;
          Set0(x, y);
          if (fx1 || fx2) {
            Set0(x-1, y); Set0(x+1, y);
            if (fx1)
              LineX(x-1, x+1, y-1, fOff);
            if (fx2)
              LineX(x-1, x+1, y+1, fOff);
          }
          if (fy1 || fy2) {
            Set0(x, y-1); Set0(x, y+1);
            if (fy1)
              LineY(x-1, y-1, y+1, fOff);
            if (fy2)
              LineY(x+1, y-1, y+1, fOff);
          }
        }
      }
  return count;
}


// Connect a region of on pixels with the nearest region disconnected from it,
// by drawing a shortest possible line of on pixels between them. This can be
// used to fuse detached walls in a Maze with each other. Implements the
// NoIsland operation.

long CMaz::DoCrackIsland(int x0, int y0, flag fCorner)
{
  BFSS *bfss = NULL, bfssT;
  CMaz bT;
  long lRet = -1, iLo = 0, iHi, iMax = 0, i, j, k;
  int x, y, xnew, ynew, d, dMax = DIRS + fCorner*DIRS;

  if (F64K())
    goto LDone;
  if (!FLegal(x0, y0) || !Get(x0, y0)) {
    if (!FBitmapFind(&x0, &y0, fOn))
      return 0;
  }
  bfss = RgAllocate(m_x*m_y, BFSS);
  if (bfss == NULL)
    goto LDone;
  if (!bT.FBitmapSizeSet(m_x, m_y))
    goto LDone;
  bT.BitmapOff();
  lRet = 0;

  // Flood the original region, so only new regions will be encountered.
  j = iMax;
  bT.Set1(x0, y0);
  BfssPush(iMax, x0, y0, -1);
  k = iMax;
  while (j < k) {
    for (i = j; i < k; i++) {
      x = bfss[i].x; y = bfss[i].y;
      for (d = 0; d < dMax; d++) {
        xnew = x + xoff[d]; ynew = y + yoff[d];
        if (Get(xnew, ynew) && !bT.Get(xnew, ynew)) {
          bT.Set1(xnew, ynew);
          BfssPush(iMax, xnew, ynew, -1);
        }
      }
    }
    j = k; k = iMax;
  }
  iHi = iMax;

  // Randomize the order of pixels in the input region, so the shortest
  // connecting line randomly starts from among the set of possible points.
  if (ms.fRandomPath)
    for (i = iLo; i < iHi; i++) {
      j = Rnd(iLo, iHi-1);
      bfssT = bfss[i]; bfss[i] = bfss[j]; bfss[j] = bfssT;
    }

  // Do a breadth first search to flood in all directions from the input
  // region, where each pixel remembers which pixel filled it.
  while (iLo < iHi) {
    for (i = iLo; i < iHi; i++) {
      x = bfss[i].x; y = bfss[i].y;
      for (d = 0; d < dMax; d++) {
        xnew = x + xoff[d]; ynew = y + yoff[d];
        if (!bT.Get(xnew, ynew)) {
          if (Get(xnew, ynew)) {

            // Found a new region! Draw a line backwards to original region.
            j = i;
            do {
              Set1(bfss[j].x, bfss[j].y);
              j = bfss[j].parent;
              lRet++;
            } while (j >= 0);
            goto LDone;
          } else if (bT.FLegal(xnew, ynew)) {
            bT.Set1(xnew, ynew);
            BfssPush(iMax, xnew, ynew, i);
          }
        }
      }
    }
    iLo = iHi; iHi = iMax;
  }

LDone:
  if (bfss != NULL)
    DeallocateP(bfss);
  return lRet;
}


#define KrusFind(x, y) KruskalFind(&cell[y * m_x + x]);

// Connect all regions of on pixels with the nearest region disconnected from
// them, by drawing shortest possible lines of on pixels between them. This
// can be used to fuse all detached walls in a Maze with each other. This is
// much faster for multiple regions than calling DoCrackIsland multiple times,
// at the cost of extra memory. Implements the NoIslands operation.

long CMaz::DoCrackIslands(flag fCorner)
{
  CMazK c;
  BFSS *bfss = NULL, bfssT;
  KRUS *cell = NULL, *krus, *krusT;
  int x, y, x2, y2, xnew, ynew, d, dMax = DIRS + fCorner*DIRS;
  long lRet = -1, iset = 0, iLo = 0, iHi, iMax = 0, i, j, k, ccell, icell;

  if (F64K())
    goto LDone;
  ccell = m_x*m_y;
  bfss = RgAllocate(ccell, BFSS);
  if (bfss == NULL)
    goto LDone;
  cell = RgAllocate(ccell, KRUS);
  if (cell == NULL)
    goto LDone;
  if (!c.FBitmapSizeSet(m_x, m_y))
    goto LDone;
  c.BitmapOff();
  lRet = 0;

  // Start with each cell in a set by itself.
  for (icell = 0; icell < ccell; icell++) {
    cell[icell].next = &cell[icell];
    cell[icell].count = 1;
  }

  // For each section of walls that hasn't already been filled, flood it and
  // all walls that connect with it with a unique id number.
  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x; x++) {
      krus = KrusFind(x, y);
      if (Get(x, y) && krus->count <= 1) {
        iset++;
        j = iMax;
        c.Set(x, y, iMax);
        BfssPush(iMax, x, y, -1);
        k = iMax;
        while (j < k) {
          for (i = j; i < k; i++) {
            x2 = bfss[i].x; y2 = bfss[i].y;
            for (d = 0; d < dMax; d++) {
              xnew = x2 + xoff[d]; ynew = y2 + yoff[d];
              if (!FLegal(xnew, ynew))
                continue;
              krusT = KrusFind(xnew, ynew);
              if (Get(xnew, ynew) && krusT->count <= 1) {
                KruskalUnion(krus, krusT);
                c.Set(xnew, ynew, iMax);
                BfssPush(iMax, xnew, ynew, -1);
              }
            }
          }
          j = k; k = iMax;
        }
      }
    }

  // Randomize the order of pixels in the set of regions, so the shortest
  // connecting lines randomly start from among the set of possible points.
  if (ms.fRandomPath)
    for (i = 0; i < iMax; i++) {
      j = Rnd(0, iMax-1);
      bfssT = bfss[i]; bfss[i] = bfss[j]; bfss[j] = bfssT;
    }

  // Do a breadth first search to flood in all directions from regions, where
  // each pixel remembers which pixel filled it.
  iHi = iMax;
  while (iLo < iHi) {
    for (i = iLo; i < iHi; i++) {
      x = bfss[i].x; y = bfss[i].y;
      krus = KrusFind(x, y);
      for (d = 0; d < dMax; d++) {
        xnew = x + xoff[d]; ynew = y + yoff[d];
        if (!FLegal(xnew, ynew))
          continue;
        krusT = KrusFind(xnew, ynew);
        if (krusT == krus)
          continue;
        if (krusT->count <= 1) {
          KruskalUnion(krus, krusT);
          c.Set(xnew, ynew, iMax);
          BfssPush(iMax, xnew, ynew, i);
          continue;
        }

        // Found another new region! Draw lines backwards to start regions.
        lRet++;
        j = i;
        do {
          Set1(bfss[j].x, bfss[j].y);
          j = bfss[j].parent;
        } while (j >= 0);
        j = c.Get(xnew, ynew);
        if (bfss[j].x == xnew && bfss[j].y == ynew)
          do {
            Set1(bfss[j].x, bfss[j].y);
            j = bfss[j].parent;
          } while (j >= 0);

        // Union the two regions quickly.
        KruskalUnion(krus, krusT);
        krus = KrusFind(x, y);
      }
    }
    iLo = iHi; iHi = iMax;
  }

LDone:
  if (bfss != NULL)
    DeallocateP(bfss);
  if (cell != NULL)
    DeallocateP(cell);
  return lRet;
}


// Modify a Maze to make it symmetric. This means replacing one half with the
// other half flipped or rotated appropriately, then touching up the result as
// needed. Implements the Make Symmetric command and Symmetric operations.

long CMaz::DoMakeSymmetric(int nOp, flag fBraid)
{
  CMaz b2;
  long l;
  int x2, y2, x, i;
  flag fOdd = FOdd(Even(m_x) >> 1) && FOdd(Even(m_y) >> 1), f = 0;

  switch (nOp) {
  case 0: x2 = Odd(m_x); y2 = Odd(m_y) >> 1; break;
  case 1: x2 = Odd(m_x) >> 1; y2 = m_y; break;
  case 2: x2 = m_x; y2 = Odd(m_y) >> 1; break;
  case 3: x2 = Odd(m_x) >> 1; y2 = Odd(m_y) >> 1; f = 1;
    if (x2 != y2)
      return -2;
    break;
  default:
    Assert(fFalse);
  }
  if (!b2.FBitmapSizeSet(x2+f, y2))
    return -1;

  for (i = 0; i < ms.nCrackPass; i++) {
    b2.BlockMove(*this, 0, 0, b2.m_x-1, b2.m_y-1, 0, 0);
    switch (nOp) {

    // Make a Maze symmetric, rotated 2x about the center.
    case 0:
      b2.BitmapRotateA();
      BlockMove(b2, 0, 0, b2.m_x-1, b2.m_y-1, 0, y2+1);
      for (x = 0; x < x2 >> 1; x++)
        Set(x2 - 1 - x, y2, Get(x, y2));

    // Make a Maze symmetric, reflected from left to right.
    case 1:
      b2.BitmapFlipX();
      BlockMove(b2, 0, 0, b2.m_x-1, b2.m_y-1, x2+1, 0);
      break;

    // Make a Maze symmetric, reflected from top to bottom.
    case 2:
      b2.BitmapFlipY();
      BlockMove(b2, 0, 0, b2.m_x-1, b2.m_y-1, 0, y2+1);
      break;

    // Make a Maze symmetric, rotated 4x about the center.
    case 3:
      b2.BitmapRotateA();
      BlockMove(b2, 0, 0, b2.m_x-1, b2.m_y-1, x2+1-f, y2+1);
      b2.BitmapRotateL();
      BlockMove(b2, 0, 0, b2.m_x-1, b2.m_y-1, x2+1, 0);
      b2.BitmapRotateA();
      BlockMove(b2, 0, 0, b2.m_x-1, b2.m_y-1, 0, y2+1-f);
      b2.FBitmapSizeSet(x2+f, y2);
      break;
    }

    // Ensure the Maze is still solvable and without blemishes. Doing this can
    // make the Maze asymmetric again, so repeat the process if needed.
    l = DoCrackIsolations() + (!fBraid ? DoConnectDetachments() :
      DoConnectPoles(fTrue) + DoCrackDeadEnds() + BraidConnectWalls());
    if (l <= fOdd)
      break;
    UpdateDisplay();
  }
  return l;
}


// Zoom an orthogonal Maze so passages and walls become 1 or 2 pixels wide,
// based on the brightness of corresponding pixels in a color bitmap.

flag CMaz::MazeZoomWithPicture(CONST CCol *c1)
{
  int x, y, n;
  flag f1, f2;

  if (!FBitmapBias(1, 2, 1, 2))
    return fFalse;
  for (y = 0; y < m_y; y += 3)
    for (x = 0; x < m_x; x += 3) {
      n = c1->Get(x/3, y/3);
      n = (RgbR(n) + RgbG(n) + RgbB(n)) / 192;
      f1 = Get(x, y+1); f2 = Get(x+1, y);
      if (n > 0) {
        Set1(x+1, y+1-(!f1 && !f2));
        if (n > 1) {
          Set1(x+f1, y+1+f1);
          if (n > 2)
            Set1(x+1+f2, y+f2);
        }
      }
    }
  return fTrue;
}


// Render a section of Maze on a bitmap. Walls are always one pixel thick and
// it's assumed no cells are solid blocks, however passages can be of greater
// sizes and displayed in different orientations. Used in the rendering of
// upper levels of fractal Mazes, and implements the DrawLevel operation.

void CMaz::BlockMoveMaze(CONST CMaz &b1, int x1, int y1, int x2, int y2,
  int x0, int y0, int xx, int xy, int yx, int yy)
{
  int x, y, xT, yT;

  if (xy == 0 && yx == 0) {

    // Simplest case: The Maze bitmap is basically just being copied over.
    if (xx == 2 && yy == 2) {
      for (y = y1; y < y2; y += 2)
        for (x = x1; x < x2; x += 2) {
          xT = x0 + x - x1; yT = y0 + y - y1;
          Set1(xT, yT);
          if (b1.Get(x+1, y))
            Set1(xT+1, yT);
          if (b1.Get(x, y+1))
            Set1(xT, yT+1);
        }
      return;
    }

    // Simple case: Maze being rendered larger, but in standard orientation.
    for (y = y1; y < y2; y += 2)
      for (x = x1; x < x2; x += 2) {
        xT = x0 + ((x - x1) >> 1) * xx; yT = y0 + ((y - y1) >> 1) * yy;
        Set1(xT, yT);
        if (b1.Get(x+1, y))
          LineX(xT, xT + xx, yT, fOn);
        if (b1.Get(x, y+1))
          LineY(xT, yT, yT + yy, fOn);
      }
    return;
  }

  // Complex case: The Maze is being rendered in a non-standard orientation.
  for (y = y1; y < y2; y += 2)
    for (x = x1; x < x2; x += 2) {
      xT = x0 + ((x - x1) >> 1) * xx + ((y - y1) >> 1) * yx;
      yT = y0 + ((y - y1) >> 1) * yy + ((x - x1) >> 1) * xy;
      Set1(xT, yT);
      if (b1.Get(x+1, y))
        Line(xT, yT, xT + xx, yT + xy, fOn);
      if (b1.Get(x, y+1))
        Line(xT, yT, xT + yx, yT + yy, fOn);
    }
}


// Render a section of 3D Maze on a bitmap. Walls are always one pixel thick
// and it's assumed no cells are solid blocks, however passages can be of
// greater sizes. Used in the rendering of upper levels of fractal Mazes.

void CMaz::BlockMoveMaze3(CONST CMaz &b1, int x1, int y1, int z1,
  int x2, int y2, int z2, int x0, int y0, int z0, int xx, int yy, int zz)
{
  int x, y, z, xT, yT, zT;

  // Simple case: The Maze bitmap is basically just being copied over.
  if (xx == 2 && yy == 2 && zz == 2) {
    for (z = z1; z < z2; z += 2)
      for (y = y1; y < y2; y += 2)
        for (x = x1; x < x2; x += 2) {
          xT = x0 + x - x1; yT = y0 + y - y1; zT = z0 + z - z1;
          Set31(xT, yT, zT);
          Set31(xT+1, yT, zT);
          Set31(xT, yT+1, zT);
          Set31(xT, yT, zT+1);
          if (b1.Get3(x+1, y+1, z))
            Set31(xT+1, yT+1, zT);
          if (b1.Get3(x+1, y, z+1))
            Set31(xT+1, yT, zT+1);
          if (b1.Get3(x, y+1, z+1))
            Set31(xT, yT+1, zT+1);
         }
    return;
  }

  // Normal case: Maze being rendered larger, but in standard orientation.
  for (z = z1; z < z2; z += 2)
    for (y = y1; y < y2; y += 2)
      for (x = x1; x < x2; x += 2) {
        xT = x0 + ((x - x1) >> 1) * xx;
        yT = y0 + ((y - y1) >> 1) * yy;
        zT = z0 + ((z - z1) >> 1) * zz;
        Set31(xT, yT, zT);
        CubeBlock(xT+1, yT, zT, xT+xx-1, yT, zT, fOn);
        CubeBlock(xT, yT+1, zT, xT, yT+yy-1, zT, fOn);
        CubeBlock(xT, yT, zT+1, xT, yT, zT+zz-1, fOn);
        if (b1.Get3(x+1, y+1, z))
          CubeBlock(xT+1, yT+1, zT, xT+xx-1, yT+yy-1, zT, fOn);
        if (b1.Get3(x+1, y, z+1))
          CubeBlock(xT+1, yT, zT+1, xT+xx-1, yT, zT+zz-1, fOn);
        if (b1.Get3(x, y+1, z+1))
          CubeBlock(xT, yT+1, zT+1, xT, yT+yy-1, zT+zz-1, fOn);
      }
}


#define BLOCKS 40

// Display information about passages in the Maze, specifically counts and
// percentage information about the types of cells, lengths of passages, and
// lengths of dead ends. Implements the Analyze Passages command.

void CMaz::MazeAnalyze() CONST
{
  long c[16], e[BLOCKS], tot = 0, m, n;
  int x, y, i, j, k, l, xm = 0, ym = 0, xnew, ynew;
  char sz[cchSzMax*6], sz1[cchSzMax], sz2[cchSzMax], sz3[cchSzMax],
    sz4[cchSzMax], sz5[cchSzMax*2], *pch;

  for (i = 0; i < 16; i++)
    c[i] = 0;
  for (i = 0; i < BLOCKS; i++)
    e[i] = 0;
  l = ms.nRadar; ms.nRadar = 0;
  for (y = yl+1; y < yh; y += 2)
    for (x = xl+1; x < xh; x += 2) {

      // See what arrangement of passages this cell has.
      k = 0;
      for (j = 0; j < DIRS; j++)
        k = (k << 1) + Get(x + xoff[j], y + yoff[j]);
      c[k]++, tot++;

      // See how long straight passages are after this point.
      if (Get(x-1, y)) {
        for (i = 0; x+i < xh-1 && !Get(x+i, y); i++)
          ;
        if (i > xm)
          xm = i;
      }
      if (Get(x, y-1)) {
        for (i = 0; y+i < yh-1 && !Get(x, y+i); i++)
          ;
        if (i > ym)
          ym = i;
      }

      // If a dead end cell, see how many cells long the dead end is.
      if (k == 14 || k == 13 || k == 11 || k == 7) {
        xnew = x; ynew = y; j = 0;
        for (i = 0; i < BLOCKS*2; i++) {
          j = FollowPassage(&xnew, &ynew, NULL, j, fFalse);
          if (j < 0)
            break;
        }
        i >>= 1;
        if (i < 1)
          i = 1;
        else if (i > BLOCKS)
          i = BLOCKS;
        e[i-1]++;
      }
    }
  ms.nRadar = l;

  if (tot < 1)
    return;
  m = c[14]+c[13]+c[11]+c[7];
  n = c[10]+c[5];
  sprintf(S(sz1),
    "Holes: %ld (%.2f%%)\n"
    "Dead ends: %ld (%.2f%%) [U: %ld, L: %ld, D: %ld, R: %ld]\n"
    "Straightaways: %ld (%.2f%%) [Y: %ld, X: %ld]\n",
    c[15], (real)c[15]/(real)tot*100.0,
    m, (real)m/(real)tot*100.0, c[7], c[11], c[13], c[14],
    n, (real)n/(real)tot*100.0, c[5], c[10]);
  m = c[12]+c[9]+c[3]+c[6];
  n = c[1]+c[2]+c[4]+c[8];
  sprintf(S(sz2),
    "Turnings: %ld (%.2f%%) [UL: %ld, LL: %ld, LR: %ld, UR: %ld]\n"
    "Junctions: %ld (%.2f%%) [U: %ld, L: %ld, D: %ld, R: %ld]\n",
    m, (real)m/(real)tot*100.0, c[12], c[6], c[3], c[9],
    n, (real)n/(real)tot*100.0, c[2], c[1], c[8], c[4]);
  m += c[10]+c[5];
  sprintf(S(sz3),
    "Crossroads: %ld (%.2f%%)\n\n"
    "Total cells: %ld, Total passage cells: %ld (%.2f%%)\n",
    c[0], (real)c[0]/(real)tot*100.0,
    tot, m, (real)m/(real)tot*100.0);
  sprintf(S(sz4),
    "Longest horizontal passage: %d (%d), Longest vertical passage %d (%d)\n",
    (xm + 1) >> 1, xm, (ym + 1) >> 1, ym);
  sprintf(S(sz5), "\nDead end lengths:\n");
  pch = sz5;
  for (j = BLOCKS-1; j > 0 && e[j] == 0; j--)
    ;
  for (i = 0; i <= j; i++) {
    while (*pch)
      pch++;
    sprintf(SO(pch, sz5), "%d%s: %ld%s%c", i+1, i < BLOCKS-1 ? "" : "+", e[i],
      i < j ? "," : "", (i&7) == 7 || i == j ? '\n' : ' ');
  }
  sprintf(S(sz), "%s%s%s%s%s", sz1, sz2, sz3, sz4, sz5);
  PrintSz_N(sz);
}


// Display information about walls in the Maze, specifically counts and
// percentage information about the types of wall intersection points, and
// lengths of walls. Implements the Analyze Walls command.

void CMaz::MazeAnalyze2() CONST
{
  long c[16], tot = 0, m, n, xs, ys;
  int x, y, i, j, k, xm = 0, ym = 0;
  char sz[cchSzMax*3], sz1[cchSzMax], sz2[cchSzMax], sz3[cchSzMax],
    sz4[cchSzMax];

  for (i = 0; i < 16; i++)
    c[i] = 0;
  for (y = yl; y <= yh; y += 2)
    for (x = xl; x <= xh; x += 2) {

      // See what arrangement of wall segments this intersection point has.
      k = 0;
      for (j = 0; j < DIRS; j++)
        k = (k << 1) + Get(x + xoff[j], y + yoff[j]);
      c[k]++, tot++;

      // See how long walls are after this point (excluding boundary walls).
      if (y > yl && y < yh-1 && !Get(x-1, y)) {
        for (i = 0; x+i < xh && Get(x+i, y); i++)
          ;
        if (i > xm)
          xm = i;
      }
      if (x > xl && x < xh-1 && !Get(x, y-1)) {
        for (i = 0; y+i < yh && Get(x, y+i); i++)
          ;
        if (i > ym)
          ym = i;
      }
    }

  if (tot < 1)
    return;
  m = c[8]+c[4]+c[2]+c[1];
  n = c[10]+c[5];
  sprintf(S(sz1),
    "Poles: %ld (%.2f%%)\n"
    "Endpoints: %ld (%.2f%%) [U: %ld, L: %ld, D: %ld, R: %ld]\n"
    "Straight walls: %ld (%.2f%%) [Y: %ld, X: %ld]\n",
    c[0], (real)c[0]/(real)tot*100.0,
    m, (real)m/(real)tot*100.0, c[8], c[4], c[2], c[1],
    n, (real)n/(real)tot*100.0, c[10], c[5]);
  m = c[3]+c[6]+c[12]+c[9];
  n = c[14]+c[13]+c[11]+c[7];
  xs = ys = m;
  sprintf(S(sz2),
    "Corners: %ld (%.2f%%) [UL: %ld, LL: %ld, LR: %ld, UR: %ld]\n"
    "T-Sections: %ld (%.2f%%) [U: %ld, L: %ld, D: %ld, R: %ld]\n",
    m, (real)m/(real)tot*100.0, c[3], c[9], c[12], c[6],
    n, (real)n/(real)tot*100.0, c[13], c[14], c[7], c[11]);
  m += c[5]+c[10];
  sprintf(S(sz3),
    "Crosspieces: %ld (%.2f%%)\n\n"
    "Total points: %ld, Total wall points: %ld (%.2f%%)\n",
    c[15], (real)c[15]/(real)tot*100.0,
    tot, m, (real)m/(real)tot*100.0);
  xs = (xs + c[1]+c[4] + c[11]+c[14] + (c[5]  + c[7] +c[13] + c[15])*2) >> 1;
  ys = (ys + c[2]+c[8] + c[7] +c[13] + (c[10] + c[11]+c[14] + c[15])*2) >> 1;
  sprintf(S(sz4),
    "Total wall segments: %d [X: %d, Y: %d]\n"
    "Longest horizontal wall: %d (%d), Longest vertical wall %d (%d)\n",
    xs + ys, xs, ys, (xm + 1) >> 1, xm, (ym + 1) >> 1, ym);
  sprintf(S(sz), "%s%s%s%s", sz1, sz2, sz3, sz4);
  PrintSz_N(sz);
}


// Display information about passages in a 3D Maze, specifically counts and
// percentage information about the types of cells. Implements the Analyze
// Passages command for 3D bitmaps.

void CMaz::MazeAnalyze3D() CONST
{
  long c[DIRS3+1], e[BLOCKS], tot = 0;
  int x, y, z, i, j, k, l, xnew, ynew, znew;
  char sz[cchSzMax*3], sz1[cchSzMax], sz2[cchSzMax*2], *pch;

  for (i = 0; i <= DIRS3; i++)
    c[i] = 0;
  for (i = 0; i < BLOCKS; i++)
    e[i] = 0;
  l = ms.nRadar; ms.nRadar = 0;
  for (z = 0; z < m_z3-1; z += 2)
    for (y = 1; y < m_y3-1; y += 2)
      for (x = 1; x < m_x3-1; x += 2) {

        // See what arrangement of passages this cell has.
        k = 0;
        for (j = 0; j < DIRS3; j++)
          k += Get3M(x + xoff3[j], y + yoff3[j], z + zoff3[j]);
        c[k]++, tot++;

        // If a dead end cell, see how many cells long the dead end is.
        if (k == DIRS3-1) {
          xnew = x; ynew = y; znew = z; j = 0;
          for (i = 0; i < BLOCKS*2; i++) {
            j = FollowPassage(&xnew, &ynew, &znew, j, fTrue);
            if (j < 0)
              break;
          }
          i >>= 1;
          if (i < 1)
            i = 1;
          else if (i > BLOCKS)
            i = BLOCKS;
          e[i-1]++;
        }
      }
  ms.nRadar = l;

  if (tot < 1)
    return;
  sprintf(S(sz1),
    "Cavities: %ld (%.2f%%)\n"
    "Dead ends: %ld (%.2f%%)\n"
    "Passages: %ld (%.2f%%)\n"
    "Junctions: %ld (%.2f%%)\n"
    "Crossroads: %ld (%.2f%%)\n"
    "5-way intersections: %ld (%.2f%%)\n"
    "6-way spaces: %ld (%.2f%%)\n",
    c[6], (real)c[6]/(real)tot*100.0,
    c[5], (real)c[5]/(real)tot*100.0,
    c[4], (real)c[4]/(real)tot*100.0,
    c[3], (real)c[3]/(real)tot*100.0,
    c[2], (real)c[2]/(real)tot*100.0,
    c[1], (real)c[1]/(real)tot*100.0,
    c[0], (real)c[0]/(real)tot*100.0);
  sprintf(S(sz2), "\nTotal cells: %ld\n\nDead end lengths:\n", tot);
  pch = sz2;
  for (j = BLOCKS-1; j > 0 && e[j] == 0; j--)
    ;
  for (i = 0; i <= j; i++) {
    while (*pch)
      pch++;
    sprintf(SO(pch, sz2), "%d%s: %ld%s%c", i+1, i < BLOCKS-1 ? "" : "+", e[i],
      i < j ? "," : "", (i&7) == 7 || i == j ? '\n' : ' ');
  }
  sprintf(S(sz), "%s%s", sz1, sz2);
  PrintSz_N(sz);
}


// Return how many possible different perfect Mazes or spanning trees there
// are of the given dimensions. This ignores entrances and just considers
// internal passages, and also counts rotations and reflections as different
// Mazes. Used by the Count Possible command.

ulong MazeCountPossible(int x, int y)
{
  int i, j;
  real r = 1.0, rx = (real)x, ry = (real)y, rMax, s, t;

  if (x < 1 || y < 1)
    return 0L;
  rMax = (real)(0xFFFFFFFFL) * (rx*ry);

  // Use eigenvalues to mathematically compute the number.
  for (i = 0; i < x; i++)
    for (j = 0; j < y; j++) {
      if (i > 0 || j > 0) {
        s = RSin(((real)i*rPi) / (2.0*rx));
        t = RSin(((real)j*rPi) / (2.0*ry));
        r *= (4.0 * (s*s + t*t));
        if (r > rMax)
          return dwSet;
      }
    }
  return (ulong)(r / (rx*ry) + rRound);
}


/*
******************************************************************************
** Maze File Operations
******************************************************************************
*/

// Save a monochrome bitmap to a plain text file. Use a compact form suitable
// for Mazes, where one character covers two pixels. Implements the Save
// Blocks command when Line Chars In Text Bitmaps is on.

void CMaz::WriteText2(FILE *file, flag fClip) CONST
{
  int x, y, x2 = m_x-1;
  char ch;

  for (y = -1; y < m_y; y += 2) {
    if (fClip)
      for (x2 = m_x-1; x2 >= 0 && !Get(x2, y) && !Get(x2, y+1); x2--)
        ;
    for (x = 0; x <= x2; x++) {
      if (Get(x, y)) {
        if (Get(x, y+1))
          ch = Get(x-1, y) && Get(x+1, y) &&
            Get(x-1, y+1) && Get(x+1, y+1) ? '#' : '|';
        else
          ch = Get(x-1, y) && Get(x+1, y) ? '~' : '\'';
      } else {
        if (Get(x, y+1))
          ch = Get(x-1, y+1) && Get(x+1, y+1) ? '_' : '.';
        else
          ch = ' ';
      }
      putbyte(ch);
    }
    if (y < m_y-2 || !fClip)
      putbyte('\n');
  }
}


// Save a monochrome bitmap to a text file, using DOS graphics characters.

void CMaz::WriteTextDOS(FILE *file, int nMode, int nClip) CONST
{
  int x, y, x2 = m_x-1;
  CONST char *pch;

  pch = (nMode == 1 ? rgchDOS2 : rgchDOS3);
  for (y = 0; y < m_y; y += 2) {
    if (nMode < 1) {

      // Block graphics: One character covers two pixels.
      if (nClip != 0)
        for (x2 = m_x-1; x2 >= 0 && !Get(x2, y) && !Get(x2, y+1); x2--)
          ;
      for (x = 0; x <= x2; x++)
        putbyte(rgchDOS[(Get(x, y) << 1) + Get(x, y+1)]);
    } else {

      // Line graphics: One character represents four pixels.
      for (x = 0; x <= x2; x += 2)
        putbyte(pch[(Get(x+1, y) << 3) + (Get(x, y+1) << 2) +
          (Get(x-1, y) << 1) + Get(x, y-1)]);
    }
    if (y < m_y-2 || nClip != 1)
      putbyte('\n');
  }
}


// Save the 3D Maze in a monochrome bitmap to a plain text file, formatted so
// the different levels and passages between them are easily seen. The levels
// are arranged in a grid, and special characters indicate up and/or down
// staircases. Implements the Save Text command for 3D bitmaps.

void CMaz::WriteText3D(FILE *file, flag fThin, flag fTab) CONST
{
  int zy, zx, zx1, x, y, z, grf;

  for (zy = 0; zy < (m_z3 + 1) >> 1; zy += ms.xCutoff) {
    for (y = 0; y < Odd(m_y3); y++) {
      zx1 = Min(ms.xCutoff, ((m_z3 + 1) >> 1) - zy);
      for (zx = 0; zx < zx1; zx++) {
        z = (zy + zx) << 1;
        for (x = 0; x < Odd(m_x3); x++) {
          if (Get3(x, y, z)) {
            if (fThin) {
              grf = Get3(x, y-1, z) | Get3(x-1, y, z) << 1 |
                Get3(x, y+1, z) << 2 | Get3(x+1, y, z) << 3;
              if (grf == 10)
                putbyte(chX);
              else if (grf == 5)
                putbyte(chY);
              else
                putbyte(chXY);
            } else
              putbyte(chOn);
          } else {
            if (Get3M(x, y, z-1)) {
              if (Get3M(x, y, z+1))
                putbyte(chOff);
              else
                putbyte(chD);
            } else {
              if (Get3M(x, y, z+1))
                putbyte(chU);
              else
                putbyte(chUD);
            }
          }
          if (fTab)
            putbyte(chTab);
        }
        if (zx < zx1-1) {
          putbyte(chOff);
          if (fTab)
            putbyte(chTab);
        }
      }
      putbyte('\n');
    }
    if (zy < ((m_z3 + 1) >> 1) - ms.xCutoff)
      putbyte('\n');
  }
}

/* maze.cpp */
