/*
** Daedalus (Version 3.3) File: create3.cpp
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
** produce non-orthogonal Mazes.
**
** Created: 9/4/2000.
** Last code change: 11/29/2018.
*/

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include "util.h"
#include "graphics.h"
#include "color.h"
#include "threed.h"
#include "maze.h"


/*
******************************************************************************
** Generic Maze Routines
******************************************************************************
*/

class Generic
{
public:
  CMaz *m_b;
  int ox0, oy0, ox1, oy1, ox2, oy2;

  void GLine(int, int, int, int);
  void MakePassage(long, int);
  flag FIsPassage(long, int);
  flag FIsOnMaze(long);
  void CreateMazeGeneral();
  void GenerateWireframe();

  // Creates an entrance or exit, returning the cell index next to it.
  virtual long LMakeOpening(int, flag) = 0;

  // Returns whether a cell has not been carved into yet. Cells not on the
  // Maze are considered to have been carved into.
  virtual flag FIsRoom(long) = 0;

  // Returns the number of possible passages leading from a cell.
  virtual int NCount(long) = 0;

  // Returns the new cell index when taking a passage from a cell.
  virtual long LEnum(long, int) = 0;

  // Returns the next cell after a given cell. Used in hunting mode when
  // systematically searching over all cells.
  virtual long LNext(long) = 0;

  // Return a random direction leading from a cell for binary tree Mazes.
  virtual int NDirBinary(long) = 0;

  // Returns the bitmap display coordinates of the wall segment next to a cell
  // in the given passage direction.
  virtual void MapDir(long, int, int *, int *, int *, int *) = 0;

  // Create a Maze in a bitmap.
  virtual flag FCreateMaze(CMaz &) = 0;
};


#define XYArea(x, y, area) x = ((area) & 0xFFFF); y = ((area) >> 16)
#define AreaXY(area, x, y) area = (((y) << 16) + (x))

// Draw a wall between two points. Used by generic Maze generators to draw the
// outlines of all cells before carving.

void Generic::GLine(int x1, int y1, int x2, int y2)
{
  // Don't draw the line if it's not in the active rectangle.
  if (ms.fSection && (!FLegalMaze2(x1, y1) || !FLegalMaze2(x2, y2)))
    return;
  m_b->Line(x1, y1, x2, y2, fOn);
}


// Carve a passage from a specified cell in a direction. Used to make
// entrances and by CreateMazeGeneral() to carve the Maze passages themselves.

void Generic::MakePassage(long area, int dir)
{
  int x1, y1, x2, y2;

  // Draw a line of off pixels, then turn the endpoints back on.
  MapDir(area, dir, &x1, &y1, &x2, &y2);
  m_b->Line(x1, y1, x2, y2, fFalse);
  m_b->Set1(x1, y1); m_b->Set1(x2, y2);
}


// Return whether there's a passage leading from the specified cell in a
// direction.

flag Generic::FIsPassage(long area, int dir)
{
  int x1, y1, x2, y2, x, y;

  // Check one or two pixels near the midpoint of the line segment.
  MapDir(area, dir, &x1, &y1, &x2, &y2);
  x = (x1 + x2) >> 1; y = (y1 + y2) >> 1;

  // If the difference in both the x and y coordinates is even, or the line
  // is straight, then the midpoint is guaranteed to be correct.
  if ((!FOdd(x2 - x1) && !FOdd(y2 - y1)) || x1 == x2 || y1 == y2)
    return !m_b->Get(x, y);

  // If the difference in both the x and y coordinates is odd, there's still
  // a guaranteed set pixel, however it may not be the rounded down midpoint.
  if (FOdd(x2 - x1) && FOdd(y2 - y1))
    return !m_b->Get(x + ((y2 > y1) != (x2 > x1)), y);

  // If one axis has an even difference and the other odd, it's ambiguous
  // which of two pixels is on, so need to check them both.
  if (m_b->Get(x, y))
    return fFalse;
  if (FOdd(x2 - x1))
    x++;
  else
    y++;
  return !m_b->Get(x, y);
}


// Return whether the specified cell is on the Maze. By default all cells are,
// but a clipping rectangle can remove some cells from the Maze.

flag Generic::FIsOnMaze(long area)
{
  int count, i, x1, y1, x2, y2;

  // Check whether each vertex surrounding the cell is set.
  count = NCount(area);
  for (i = 0; i < count; i++) {
    MapDir(area, i, &x1, &y1, &x2, &y2);
    if (!m_b->Get(x1, y1) || !m_b->Get(x2, y2))
      return fFalse;
  }
  return fTrue;
}


// Create a generic Maze, or a Maze that can have cells with any number of
// connections between them in any arrangement. The arrangement is defined by
// other methods in the class inheriting from Generic. Called from the
// FCreateMaze methods to create the various Omega style Mazes.

void Generic::CreateMazeGeneral()
{
  long area, test, hunt;
  int count, dir, d, pass = 0;
  flag fHunt = fFalse;

  UpdateDisplay();
  area = LMakeOpening(ms.omegas, fFalse);

  // Use the Binary Tree algorithm to create the generic Maze.
  if (ms.fInfEller) {
    hunt = area;
    do {
      dir = NDirBinary(area);
      if (dir != -1) {
        if (fCellMax)
          return;
        MakePassage(area, dir);
      }
      area = LNext(area);
    } while (area != hunt);

  // Use the Hunt and Kill algorithm to create the generic Maze.
  } else {
    loop {
  LNext:
      count = NCount(area);
      dir = Rnd(0, count-1);
      for (d = ms.fRiver || fHunt ? count : 1; d > 0; d--) {
        test = LEnum(area, dir);
        if (!FIsRoom(test)) {
          if (fCellMax)
            return;
          MakePassage(area, dir);
          area = test;
          fHunt = fFalse;
          pass = 0;
          goto LNext;
        }
        dir++;
        if (dir >= count)
          dir = 0;
      }
      if (!fHunt) {
        fHunt = fTrue;
        hunt = area;
      }
      do {
        area = LNext(area);
        if (area == hunt) {
          pass++;
          if (pass > (int)!ms.fRiver)
            break;
        }
      } while (!FIsRoom(area) || (ms.fSection && !FIsOnMaze(area)));
      if (pass > (int)!ms.fRiver)
        break;
    }
  }

  LMakeOpening(ms.omegaf, fTrue);
  GenerateWireframe();
}


// Create a wireframe or patch list in memory, using the wall edge coordinates
// of the generic Maze.

void Generic::GenerateWireframe()
{
  long area, test, hunt;
  int count, d, x1, y1, x2, y2, f, cEdge = 0;

  if (ms.nOmegaDraw <= 0)
    return;
  for (f = 0; f <= 1; f++) {
    for (area = 0; !FIsOnMaze(area) || !FIsRoom(area); area = LNext(area))
      ;
    hunt = area;
    loop {
      if (!FIsOnMaze(area) || !FIsRoom(area))
        continue;
      count = NCount(area);
      for (d = 0; d < count; d++) {
        test = LEnum(area, d);
        if (test < area && FIsOnMaze(test))
          continue;
        if (!FIsPassage(area, d)) {
          if (!f)
            cEdge++;
          else {
            MapDir(area, d, &x1, &y1, &x2, &y2);
            if (ms.nOmegaDraw == 1)
              FSetCoordinates(x1, y1, 0, x2, y2, 0, -1);
            else
              FSetPatch(x1, y1, 0, x2, y2, 10, -1);
          }
        }
      }
      area = LNext(area);
      if (area == hunt)
        break;
    }
    if (!f) {
      if (ms.nOmegaDraw == 1)
        FInitCoordinates(cEdge);
      else
        FInitPatch(cEdge);
    }
  }
  PrintSzN("Total number of edges generated: %d", cEdge);
}


// Create a Gamma Maze, or a standard orthogonal Maze formed of squares,
// forming one big square.

#define GMapX(x) (ox0 + (x) * ox1)
#define GMapY(y) (oy0 + (y) * oy1)

class Gamma : Generic
{
private:
  long LMakeOpening(int, flag);
  flag FIsRoom(long);
  int NCount(long);
  long LEnum(long, int);
  long LNext(long);
  int NDirBinary(long);
  void MapDir(long, int, int *, int *, int *, int *);

public:
  flag FCreateMaze(CMaz &);
};

void Gamma::MapDir(long area, int dir, int *x1, int *y1, int *x2, int *y2)
{
  int x, y;

  XYArea(x, y, area);
  *x1 = GMapX(x + (dir == 3));
  *y1 = GMapY(y + (dir == 2));
  *x2 = GMapX(x + (dir != 1));
  *y2 = GMapY(y + (dir != 0));
}

long Gamma::LMakeOpening(int hint, flag fExit)
{
  long area;
  int x, y, dir, z, i, x1, y1, x2, y2;

  if (hint < 0) {
    switch (ms.nEntrancePos) {
    case 0: hint = (fExit ? 2 : 0)*1000 + (fExit ? ms.omega-1 : 0);   break;
    case 1: hint = (fExit ? 2 : 0)*1000 + ((ms.omega - !fExit) >> 1); break;
    }
  }
  for (i = 0; i < 100; i++) {
    dir = hint / 1000;
    if (hint < 0 || !FBetween(dir, 0, 3))
      dir = Rnd(0, 3);
    z = hint % 1000;
    if (hint < 0 || !FBetween(z, 0, ms.omega-1))
      z = Rnd(0, ms.omega-1);
    switch (dir) {
    case 0: x = z; y = 0; break;
    case 1: x = 0; y = z; break;
    case 2: x = z; y = ms.omega-1; break;
    case 3: x = ms.omega-1; y = z; break;
    }
    AreaXY(area, x, y);
    if (!FIsPassage(area, dir))
      break;
    hint = -1;
  }
  MakePassage(area, dir);
  if (!fExit) {
    MapDir(area, dir, &x1, &y1, &x2, &y2);
    ms.xEntrance = (x1 + x2) >> 1; ms.yEntrance = (y1 + y2) >> 1;
  }
  return area;
}

flag Gamma::FIsRoom(long area)
{
  int x, y, d;

  XYArea(x, y, area);
  if (y < 0 || y >= ms.omega || x < 0 || x >= ms.omega)
    return fTrue;
  for (d = 0; d < 4; d++)
    if (FIsPassage(area, d))
      return fTrue;
  return fFalse;
}

int Gamma::NCount(long area)
{
  return 4;  // Squares have four sides.
}

long Gamma::LEnum(long area, int dir)
{
  int x, y;

  XYArea(x, y, area);
  switch (dir) {
  case 0: y--; break;
  case 1: x--; break;
  case 2: y++; break;
  case 3: x++; break;
  }
  AreaXY(area, x, y);
  return area;
}

long Gamma::LNext(long area)
{
  int x, y;

  XYArea(x, y, area);
  x++;
  if (x >= ms.omega) {
    x = 0;
    y++;
    if (y >= ms.omega)
      y = 0;
  }
  AreaXY(area, x, y);
  return area;
}

int Gamma::NDirBinary(long area)
{
  int x, y;

  XYArea(x, y, area);
  if (x <= 0 || y <= 0)
    return y > 0 ? 0 : (x > 0 ? 1 : -1);
  return Rnd(0, 1);
}

flag Gamma::FCreateMaze(CMaz &b)
{
  int x, y, z;

  m_b = &b;
  z = ms.omega*2 + 1;
  x = Max(b.m_x, z); y = Max(b.m_y, z);
  if (!b.FBitmapSizeSet(x, y))
    return fFalse;
  ox1 = (b.m_x - 1) / ms.omega;
  ox0 = (b.m_x - ox1 * ms.omega) >> 1;
  oy1 = (b.m_y - 1) / ms.omega;
  oy0 = (b.m_y - oy1 * ms.omega) >> 1;
  if (ox1 < 2 || oy1 < 2) {
    PrintSzN_W("Bitmap too small to make gamma Maze of size: %d", ms.omega);
    return fFalse;
  }
  b.BitmapOff();
  for (y = 0; y <= ms.omega; y++)
    for (x = 0; x <= ms.omega; x++) {
      if (x < ms.omega)
        GLine(GMapX(x), GMapY(y), GMapX(x+1), GMapY(y));
      if (y < ms.omega)
        GLine(GMapX(x), GMapY(y), GMapX(x), GMapY(y+1));
    }
  CreateMazeGeneral();
  return fTrue;
}

flag CMaz::CreateMazeGamma()
{
  Gamma gamma;
  return gamma.FCreateMaze(*this);
}


// Create a Delta Maze, or a Maze formed of upward and downward pointing
// triangles, forming one big upward pointing triangle.

#define DMapX(x, y) (ox0 + (x) * (ox1 << 1) + (ms.omega - (y)) * ox1)
#define DMapY(y) (oy0 + (y) * oy1)

class Delta : Generic
{
private:
  void DLine(int, int, int, int, flag);

  long LMakeOpening(int, flag);
  flag FIsRoom(long);
  int NCount(long);
  long LEnum(long, int);
  long LNext(long);
  int NDirBinary(long);
  void MapDir(long, int, int *, int *, int *, int *);

public:
  flag FCreateMaze(CMaz &);
};

void Delta::MapDir(long area, int dir, int *x1, int *y1, int *x2, int *y2)
{
  int x, y;

  XYArea(x, y, area);
  *y2 = DMapY(y+1);
  if (dir == 2) {
    *y1 = DMapY(y+1-(x&1));
    *y2 = *y1;
    *x1 = DMapX(x >> 1, y+1-(x&1));
    *x2 = *x1 + (ox1 << 1);
  } else {
    *y1 = DMapY(y);
    *y2 = DMapY(y+1);
    *x1 = DMapX((x >> 1) + ((x & 1) && dir), y);
    *x2 = DMapX((x >> 1) + ((x & 1) || dir), y+1);
  }
}

long Delta::LMakeOpening(int hint, flag fExit)
{
  long area;
  int x, y, dir, z, i, x1, y1, x2, y2;

  if (hint < 0)
    switch (ms.nEntrancePos) {
    case 0: hint = (fExit ? 1 : 0)*1000 + (ms.omega-1); break;
    case 1: hint = (fExit ? 1 : 0)*1000 + (ms.omega * 71 / 100); break;
    }
  for (i = 0; i < 100; i++) {
    dir = hint / 1000;
    if (hint < 0 || !FBetween(dir, 0, 2))
      dir = Rnd(0, 2);
    z = hint % 1000;
    if (hint < 0 || !FBetween(z, 0, ms.omega-1))
      z = Rnd(0, ms.omega-1);
    switch (dir) {
    case 0:
      x = 0; y = z; break;
    case 1:
      x = z << 1; y = z; break;
    default:
      x = z << 1; y = ms.omega-1; break;
    }
    AreaXY(area, x, y);
    if (!FIsPassage(area, dir))
      break;
    hint = -1;
  }
  MakePassage(area, dir);
  if (!fExit) {
    MapDir(area, dir, &x1, &y1, &x2, &y2);
    ms.xEntrance = (x1 + x2) >> 1; ms.yEntrance = (y1 + y2) >> 1;
  }
  return area;
}

flag Delta::FIsRoom(long area)
{
  int x, y;

  XYArea(x, y, area);
  if (y < 0 || y >= ms.omega || x < 0 || x >= (y << 1) + 1)
    return fTrue;
  return FIsPassage(area, 0) || FIsPassage(area, 1) || FIsPassage(area, 2);
}

int Delta::NCount(long area)
{
  return 3;  // Triangles have three sides.
}

long Delta::LEnum(long area, int dir)
{
  int x, y;

  XYArea(x, y, area);
  switch (dir) {
  case 0: x--; break;
  case 1: x++; break;
  default:
    if (x & 1) {
      y--; x--;
    } else {
      y++; x++;
    }
    break;
  }
  AreaXY(area, x, y);
  return area;
}

long Delta::LNext(long area)
{
  int x, y;

  XYArea(x, y, area);
  x++;
  if (x > (y << 1)) {
    x = 0;
    y++;
    if (y >= ms.omega)
      y = 0;
  }
  AreaXY(area, x, y);
  return area;
}

int Delta::NDirBinary(long area)
{
  int x, y;

  XYArea(x, y, area);
  if (x & 1)
    return 2;
  if (x <= 0)
    return y > 0 ? 1 : -1;
  return x < (y << 1) ? Rnd(0, 1) : 0;
}

// If f is false, always draw a line. If f is true, redraw the line if one
// already exists. Called from Delta::FCreateMaze() to touch up walls that may
// have been partially erased by nearby walls getting erased.

void Delta::DLine(int x1, int y1, int x2, int y2, flag f)
{
  int x, y;

  if (f) {
    x = (x1 + x2) >> 1; y = (y1 + y2) >> 1;
    f = !m_b->Get(x, y) && !m_b->Get(x-1, y) && !m_b->Get(x+1, y);
  }
  if (!f)
    GLine(x1, y1, x2, y2);
}

flag Delta::FCreateMaze(CMaz &b)
{
  int x, y, m, n, f;

  m_b = &b;
  m = (ms.omega << 1)*2 + 1;
  n = ms.omega*2 + 1;
  x = Max(b.m_x, m); y = Max(b.m_y, n);
  if (!b.FBitmapSizeSet(x, y))
    return fFalse;
  ox1 = (b.m_x - 1) / (ms.omega << 1);
  ox0 = (b.m_x - ox1 * (ms.omega << 1)) >> 1;
  oy1 = (b.m_y - 1) / ms.omega;
  oy0 = (b.m_y - oy1 * ms.omega) >> 1;
  if (ox1 < 2 || oy1 < 2) {
    PrintSzN_W("Bitmap too small to make delta Maze of size: %d", ms.omega);
    return fFalse;
  }
  b.BitmapOff();
  for (f = fFalse; f <= fTrue; f++) {
    for (y = 0; y < ms.omega; y++)
      for (x = 0; x < y + 1; x++) {
        DLine(DMapX(x, y), DMapY(y), DMapX(x,   y+1), DMapY(y+1), f);
        DLine(DMapX(x, y), DMapY(y), DMapX(x+1, y+1), DMapY(y+1), f);
        DLine(DMapX(x, y+1), DMapY(y+1), DMapX(x+1, y+1), DMapY(y+1), f);
      }
    if (!f)
      CreateMazeGeneral();
  }
  return fTrue;
}

flag CMaz::CreateMazeDelta()
{
  Delta delta;
  return delta.FCreateMaze(*this);
}


// Create a Sigma Maze, or a Maze formed of hexagons (with points up and down)
// forming one big hexagon (with points left and right).

#define SXMax(y) (ms.omega + Min(y, SYMax - 1 - (y)))
#define SYMax ((ms.omega << 1) - 1)
#define SMapX(x, y) (ox0 + ox1 + (x) * (ox1 << 1) + (SYMax - SXMax(y)) * ox1)
#define SMapY(y) (oy0 + (y) * 3 * oy1)

class Sigma : Generic
{
private:
  long LMakeOpening(int, flag);
  flag FIsRoom(long);
  int NCount(long);
  long LEnum(long, int);
  long LNext(long);
  int NDirBinary(long);
  void MapDir(long, int, int *, int *, int *, int *);

public:
  flag FCreateMaze(CMaz &);
};

void Sigma::MapDir(long area, int dir, int *x1, int *y1, int *x2, int *y2)
{
  int x, y, m, n;

  XYArea(x, y, area);
  n = SMapY(y); m = SMapX(x, y);
  switch (dir) {
  case 0: *x1 = m; *y1 = n; *x2 = m-ox1; *y2 = n+oy1; break;
  case 1: *x1 = m; *y1 = n; *x2 = m+ox1; *y2 = n+oy1; break;
  case 2: *x1 = m-ox1; *y1 = n+oy1; *x2 = *x1; *y2 = n+oy1*3; break;
  case 3: *x1 = m+ox1; *y1 = n+oy1; *x2 = *x1; *y2 = n+oy1*3; break;
  case 4: *x1 = m-ox1; *y1 = n+oy1*3; *x2 = m; *y2 = n+oy1*4; break;
  case 5: *x1 = m+ox1; *y1 = n+oy1*3; *x2 = m; *y2 = n+oy1*4; break;
  }
}

long Sigma::LMakeOpening(int hint, flag fExit)
{
  long area;
  int x, y, sid, z, f, dir, i, x1, y1, x2, y2;

  if (hint < 0)
    switch (ms.nEntrancePos) {
    case 0:
      hint = (fExit ? 5 : 0)*1000 + (fExit ? 10000+ms.omega-1 : 0);
      break;
    case 1:
      hint = (fExit ? 5 : 0)*1000 + (ms.omega >> 1) -
        (!fExit && !FOdd(ms.omega)) + (fExit ^ !FOdd(ms.omega))*10000;
      break;
    }
  for (i = 0; i < 100; i++) {
    sid = (hint / 1000) % 10;
    if (hint < 0 || !FBetween(sid, 0, 5))
      sid = Rnd(0, 5);
    f = hint / 10000;
    if (hint < 0 || !FBetween(f, 0, 1))
      f = Rnd(0, 1);
    z = hint % 1000;
    if (hint < 0 || !FBetween(z, 0, ms.omega-1))
      z = Rnd(0, ms.omega-1);
    switch (sid) {
    case 0: y = 0; x = z; dir = f; break;
    case 1: y = z; x = SXMax(y)-1; dir = 1 + f*2; break;
    case 2: y = z; x = 0; dir = f*2; break;
    case 3: y = ms.omega-1+z; x = SXMax(y)-1; dir = 3 + f*2; break;
    case 4: y = ms.omega-1+z; x = 0; dir = 2 + f*2; break;
    case 5: y = SYMax-1; x = z; dir = 4+f; break;
    }
    AreaXY(area, x, y);
    if (!FIsPassage(area, dir))
      break;
    hint = -1;
  }
  MakePassage(area, dir);
  if (!fExit) {
    MapDir(area, dir, &x1, &y1, &x2, &y2);
    ms.xEntrance = (x1 + x2) >> 1; ms.yEntrance = (y1 + y2) >> 1;
  }
  return area;
}

flag Sigma::FIsRoom(long area)
{
  int x, y, d;

  XYArea(x, y, area);
  if (y < 0 || y >= SYMax || x < 0 || x >= SXMax(y))
    return fTrue;
  for (d = 0; d < 6; d++)
    if (FIsPassage(area, d))
      return fTrue;
  return fFalse;
}

int Sigma::NCount(long area)
{
  return 6;  // Hexagons have six sides.
}

long Sigma::LEnum(long area, int dir)
{
  int x, y;

  XYArea(x, y, area);
  switch (dir) {
  case 0: x -= (y < ms.omega); y--; break;
  case 1: x += (y >= ms.omega); y--; break;
  case 2: x--; break;
  case 3: x++; break;
  case 4: x -= (y >= ms.omega-1); y++; break;
  case 5: x += (y < ms.omega-1); y++; break;
  }
  AreaXY(area, x, y);
  return area;
}

long Sigma::LNext(long area)
{
  int x, y;

  XYArea(x, y, area);
  x++;
  if (x >= SXMax(y)) {
    x = 0;
    y++;
    if (y >= SYMax)
      y = 0;
  }
  AreaXY(area, x, y);
  return area;
}

int Sigma::NDirBinary(long area)
{
  int x, y;

  XYArea(x, y, area);
  if (x <= 0 || y <= 0)
    return y > 0 ? (y <= ms.omega ? 1 : Rnd(0, 1)) : (x > 0 ? 2 : -1);
  return x < SXMax(y)-1 ? Rnd(0, 2) : Rnd(0, 1)*2;
}

flag Sigma::FCreateMaze(CMaz &b)
{
  int x, y, m, n;

  m_b = &b;
  m = (SYMax << 1)*2 + 1;
  n = (SYMax * 3 + 1)*2 + 1;
  x = Max(b.m_x, m); y = Max(b.m_y, n);
  if (!b.FBitmapSizeSet(x, y))
    return fFalse;
  ox1 = (b.m_x - 1) / (SYMax << 1);
  ox0 = (b.m_x - ox1 * (SYMax << 1)) >> 1;
  oy1 = (b.m_y - 1) / (SYMax * 3 + 1);
  oy0 = (b.m_y - oy1 * (SYMax * 3 + 1)) >> 1;
  if (ox1 < 2 || oy1 < 2) {
    PrintSzN_W("Bitmap too small to make sigma Maze of size: %d", ms.omega);
    return fFalse;
  }
  b.BitmapOff();
  for (y = 0; y < SYMax; y++) {
    for (x = 0; x < SXMax(y); x++) {
      m = SMapX(x, y); n = SMapY(y);
      GLine(m, n, m-ox1, n+oy1);
      GLine(m, n, m+ox1, n+oy1);
      GLine(m-ox1, n+oy1, m-ox1, n+oy1*3);
      if (x >= SXMax(y) - 1)
        GLine(m+ox1, n+oy1, m+ox1, n+oy1*3);
      if ((x == 0 && y >= ms.omega-1) || y >= SYMax-1)
        GLine(m-ox1, n+oy1*3, m, n+oy1*4);
      if ((x >= SXMax(y)-1 && y >= ms.omega-1) || y >= SYMax-1)
        GLine(m+ox1, n+oy1*3, m, n+oy1*4);
    }
  }
  CreateMazeGeneral();
  return fTrue;
}

flag CMaz::CreateMazeSigma()
{
  Sigma sigma;
  return sigma.FCreateMaze(*this);
}


// Create a Theta Maze, or a Maze formed of concentric circles, expanding from
// an inner to an outer radius.

#define TList 26
#define TPoint(y) theta[Min((y)+1, TList-1)]
#define TMapX(x, y) \
  (ox0 + NCosRD((real)(ox1*(y) + ox1/2), (x) * (rDegMax / TPoint(y)) - 45.0))
#define TMapY(x, y) \
  (oy0 + NSinRD((real)(oy1*(y) + oy1/2), (x) * (rDegMax / TPoint(y)) - 45.0))

CONST int theta[TList] =
  {0, 4, 8, 16, 16, 32, 32, 32, 64, 64, 64, 64,
  128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 256};

class Theta : Generic
{
private:
  long LMakeOpening(int, flag);
  flag FIsRoom(long);
  int NCount(long);
  long LEnum(long, int);
  long LNext(long);
  int NDirBinary(long);
  void MapDir(long, int, int *, int *, int *, int *);

public:
  flag FCreateMaze(CMaz &);
};

void Theta::MapDir(long area, int dir, int *x1, int *y1, int *x2, int *y2)
{
  int x, y, m, m1, m2;

  XYArea(x, y, area);
  m = (x + 1) & (TPoint(y) - 1);
  m1 = x << (int)(TPoint(y) < TPoint(y+1));
  m2 = m << (int)(TPoint(y) < TPoint(y+1));
  switch (dir) {
  case 0:
    *x1 = TMapX(x, y); *y1 = TMapY(x, y);
    *x2 = TMapX(m, y); *y2 = TMapY(m, y);
    break;
  case 1:
    *x1 = TMapX(x, y); *y1 = TMapY(x, y);
    *x2 = TMapX(m1, y+1); *y2 = TMapY(m1, y+1);
    break;
  case 2:
    *x1 = TMapX(m, y); *y1 = TMapY(m, y);
    *x2 = TMapX(m2, y+1); *y2 = TMapY(m2, y+1);
    break;
  case 3:
    *x1 = TMapX(m1, y+1); *y1 = TMapY(m1, y+1);
    *x2 = TMapX(m1+1, y+1); *y2 = TMapY(m1+1, y+1);
    break;
  case 4:
    *x1 = TMapX(m1+1, y+1); *y1 = TMapY(m1+1, y+1);
    *x2 = TMapX(m2, y+1); *y2 = TMapY(m2, y+1);
    break;
  }
}

long Theta::LMakeOpening(int hint, flag fExit)
{
  long area;
  int x, y, dir, i, x1, y1, x2, y2;

  for (i = 0; i < 100; i++) {
    if (fExit) {
      y = ms.omega-1; dir = 3 + Rnd(0, TPoint(y) < TPoint(y+1));
    } else {
      y = ms.omega2; dir = 0;
    }
    if (hint < 0)
      switch (ms.nEntrancePos) {
      case 0: hint = TPoint(y)*(fExit ? 7 : 3)/8 - (y > 0); break;
      case 1: hint = TPoint(y)*7/8 - fExit; break;
      }
    x = hint;
    if (hint < 0 || !FBetween(x, 0, TPoint(y)-1))
      x = Rnd(0, TPoint(y)-1);
    AreaXY(area, x, y);
    if (!FIsPassage(area, dir))
      break;
    hint = -1;
  }
  MakePassage(area, dir);
  if (!fExit) {
    MapDir(area, dir, &x1, &y1, &x2, &y2);
    ms.xEntrance = (x1 + x2) >> 1; ms.yEntrance = (y1 + y2) >> 1;
  }
  return area;
}

flag Theta::FIsRoom(long area)
{
  int x, y, d;

  XYArea(x, y, area);
  if (y < 0 || y >= ms.omega)
    return fTrue;
  for (d = 0; d < NCount(area); d++)
    if (FIsPassage(area, d))
      return fTrue;
  return fFalse;
}

int Theta::NCount(long area)
{
  int x, y;

  // Cells in a ring have four sides, except when the number of cells in a
  // ring doubles, in which case the inner ring has five sides.
  XYArea(x, y, area);
  return 4 + (TPoint(y) < TPoint(y+1));
}

long Theta::LEnum(long area, int dir)
{
  int x, y;

  XYArea(x, y, area);
  switch (dir) {
  case 0: x = x >> (int)(TPoint(y) > TPoint(y-1)); y--; break;
  case 1: x = (x - 1) & (TPoint(y) - 1); break;
  case 2: x = (x + 1) & (TPoint(y) - 1); break;
  case 3: x = x << (int)(TPoint(y) < TPoint(y+1)); y++; break;
  case 4: x = (x << (int)(TPoint(y) < TPoint(y+1))) + 1; y++; break;
  }
  AreaXY(area, x, y);
  return area;
}

long Theta::LNext(long area)
{
  int x, y;

  XYArea(x, y, area);
  x++;
  if (x >= TPoint(y)) {
    x = 0;
    y++;
    if (y >= ms.omega)
      y = ms.omega2;
  }
  AreaXY(area, x, y);
  return area;
}

int Theta::NDirBinary(long area)
{
  int x, y;

  XYArea(x, y, area);
  if (y <= ms.omega2)
    return 1;
  return Rnd(0, 1);
}

flag Theta::FCreateMaze(CMaz &b)
{
  int x, y, m;

  m_b = &b;
  m = ((ms.omega << 1) + 1) * 5;
  x = Max(b.m_x, m); y = Max(b.m_y, m);
  if (!b.FBitmapSizeSet(x, y))
    return fFalse;
  ox1 = b.m_x / ((ms.omega << 1) + 1);
  ox0 = (b.m_x >> 1) - 1;
  oy1 = b.m_y / ((ms.omega << 1) + 1);
  oy0 = (b.m_y >> 1) - 1;
  if (ox1 < 5 || oy1 < 5) {
    PrintSzN_W("Bitmap too small to make theta Maze of size: %d", ms.omega);
    return fFalse;
  }
  b.BitmapOff();
  for (y = ms.omega2; y <= ms.omega; y++) {
    for (x = 0; x < TPoint(y); x++) {
      m = (x + 1) & (TPoint(y) - 1);
      GLine(TMapX(x, y), TMapY(x, y), TMapX(m, y), TMapY(m, y));
      if (y < ms.omega) {
        m = x << (int)(TPoint(y) < TPoint(y+1));
        GLine(TMapX(x, y), TMapY(x, y), TMapX(m, y+1), TMapY(m, y+1));
      }
    }
  }
  CreateMazeGeneral();
  return fTrue;
}

flag CMaz::CreateMazeTheta()
{
  Theta theta;
  return theta.FCreateMaze(*this);
}


// Create an Upsilon Maze, or a Maze formed of octagons and squares in a grid.

#define USquare(x, y) (((x) ^ (y)) & 1)
#define UMapX(x) (ox0 + ox2 + (x)*(ox1 + ox2))
#define UMapY(y) (oy0 + oy2 + (y)*(oy1 + oy2))

class Upsilon : Generic
{
private:
  long LMakeOpening(int, flag);
  flag FIsRoom(long);
  int NCount(long);
  long LEnum(long, int);
  long LNext(long);
  int NDirBinary(long);
  void MapDir(long, int, int *, int *, int *, int *);

public:
  flag FCreateMaze(CMaz &);
};

void Upsilon::MapDir(long area, int dir, int *x1, int *y1, int *x2, int *y2)
{
  int x, y, m, n;

  XYArea(x, y, area);
  m = UMapX(x); n = UMapY(y);
  *x1 = *x2 = m; *y1 = *y2 = n;
  if (USquare(x, y)) {
    switch (dir) {
    case 0: *x2 += ox1; break;
    case 1: *y2 += oy1; break;
    case 2: *x2 += ox1; *y1 += oy1; *y2 += oy1; break;
    case 3: *x1 += ox1; *x2 += ox1; *y2 += oy1; break;
    }
  } else {
    switch (dir) {
    case 0: *x2 += ox1; *y1 -= oy2; *y2 -= oy2; break;
    case 1: *x1 -= ox2; *x2 -= ox2; *y2 += oy1; break;
    case 2: *x2 += ox1; *y1 += oy1+oy2; *y2 += oy1+oy2; break;
    case 3: *x1 += ox1+ox2; *x2 += ox1+ox2; *y2 += oy1; break;
    case 4: *x1 -= ox2; *y2 -= oy2; break;
    case 5: *x1 -= ox2; *y1 += oy1; *y2 += oy1+oy2; break;
    case 6: *x1 += ox1; *x2 += ox1+ox2; *y1 += oy1+oy2; *y2 += oy1; break;
    case 7: *x1 += ox1; *x2 += ox1+ox2; *y1 -= oy2; break;
    }
  }
}

long Upsilon::LMakeOpening(int hint, flag fExit)
{
  long area;
  int x, y, z, dir, i, x1, y1, x2, y2;

  if (hint < 0)
    switch (ms.nEntrancePos) {
    case 0: hint = (fExit ? 2 : 0)*1000 + (fExit ? ms.omega-1 : 0);   break;
    case 1: hint = (fExit ? 2 : 0)*1000 + ((ms.omega - !fExit) >> 1); break;
    }
  for (i = 0; i < 100; i++) {
    dir = hint / 1000;
    if (hint < 0 || !FBetween(dir, 0, 3))
      dir = Rnd(0, 3);
    z = hint % 1000;
    if (hint < 0 || !FBetween(z, 0, ms.omega-1))
      z = Rnd(0, ms.omega-1);
    switch (dir) {
    case 0: x = z; y = 0; break;
    case 1: x = 0; y = z; break;
    case 2: x = z; y = ms.omega-1; break;
    case 3: x = ms.omega-1; y = z; break;
    }
    AreaXY(area, x, y);
    if (!FIsPassage(area, dir))
      break;
    hint = -1;
  }
  MakePassage(area, dir);
  if (!fExit) {
    MapDir(area, dir, &x1, &y1, &x2, &y2);
    ms.xEntrance = (x1 + x2) >> 1; ms.yEntrance = (y1 + y2) >> 1;
  }
  return area;
}

flag Upsilon::FIsRoom(long area)
{
  int x, y, d;

  XYArea(x, y, area);
  if (y < 0 || y >= ms.omega || x < 0 || x >= ms.omega)
    return fTrue;
  for (d = 0; d < 8 - 4*USquare(x, y); d++)
    if (FIsPassage(area, d))
      return fTrue;
  return fFalse;
}

int Upsilon::NCount(long area)
{
  int x, y;

  XYArea(x, y, area);
  return 8 - 4*USquare(x, y);  // Octagons/squares have eight/four sides.
}

long Upsilon::LEnum(long area, int dir)
{
  int x, y;

  XYArea(x, y, area);
  x += xoff[dir]; y += yoff[dir];
  AreaXY(area, x, y);
  return area;
}

long Upsilon::LNext(long area)
{
  int x, y;

  XYArea(x, y, area);
  x++;
  if (x >= ms.omega) {
    x = 0;
    y++;
    if (y >= ms.omega)
      y = 0;
  }
  AreaXY(area, x, y);
  return area;
}

int Upsilon::NDirBinary(long area)
{
  int x, y, d;

  XYArea(x, y, area);
  if (USquare(x, y)) {
    if (x <= 0 || y <= 0)
      return x > 0;
    return Rnd(0, 1);
  }
  if (x <= 0 || y <= 0)
    return y > 0 ? Rnd(0, 1)*7 : (x > 0 ? 1 : -1);
  d = Rnd(0, 3 - (x >= ms.omega-1));
  return d < 2 ? d : (d == 2 ? 4 : 7);
}

flag Upsilon::FCreateMaze(CMaz &b)
{
  int x, y, m, n;
  real r;

  m_b = &b;
  m = (int)((real)ms.omega*(4.0 + 4.0/rSqr2)) + 5;
  x = Max(b.m_x, m); y = Max(b.m_y, m);
  if (!b.FBitmapSizeSet(x, y))
    return fFalse;
  r = 1.0/rSqr2 + (real)ms.omega*(1.0 + 1.0/rSqr2);
  ox1 = (int)((real)(b.m_x-1) / r);
  ox2 = (int)((real)ox1 / rSqr2);
  ox0 = (b.m_x - (ox1+ox2)*ms.omega - ox2) >> 1;
  oy1 = (int)((real)(b.m_y-1) / r);
  oy2 = (int)((real)oy1 / rSqr2);
  oy0 = (b.m_y - (oy1+oy2)*ms.omega - oy2) >> 1;
  if (ox1 < 4 || oy1 < 4 || ox2 < 2 || oy2 < 2) {
    PrintSzN_W("Bitmap too small to make upsilon Maze of size: %d", ms.omega);
    return fFalse;
  }
  b.BitmapOff();
  for (y = 0; y <= ms.omega; y++) {
    for (x = 0; x <= ms.omega; x++) {
      m = UMapX(x); n = UMapY(y);
      if (((x | y) & 1) == 0) {
        if ((x < ms.omega && y < ms.omega) ||
          (x == ms.omega && y > 1) || (y == ms.omega && x > 1))
          GLine(m-ox2, n, m, n-oy2);
        if (x < ms.omega)
          GLine(m+ox1, n-oy2, m+ox1+ox2, n);
        if (x < ms.omega && y < ms.omega)
          GLine(m+ox1+ox2, n+oy1, m+ox1, n+oy1+oy2);
        if (y < ms.omega)
          GLine(m, n+oy1+oy2, m-ox2, n+oy1);
      }
      if (x < ms.omega)
        GLine(m, n - oy2*(!USquare(x, y)), m+ox1, n - oy2*(!USquare(x, y)));
      if (y < ms.omega)
        GLine(m - ox2*(!USquare(x, y)), n, m - ox2*(!USquare(x, y)), n+oy1);
    }
  }
  CreateMazeGeneral();
  return fTrue;
}

flag CMaz::CreateMazeUpsilon()
{
  Upsilon upsilon;
  return upsilon.FCreateMaze(*this);
}


/*
******************************************************************************
** Recursive Fractal Maze Routines
******************************************************************************
*/

CONST char *rgszChip[7] = {
  "4", "26", "246", "1357", "13457", "123567", "1234567"};
CONST char *rgszEnd[9] = {
  "BL2R4", "BH2R2D4NL2R2", "BH2R4D2L4D2R4", "BH2R4D2NL3D2L4", "BH2D2R4NU2D2",
  "BE2L4D2R4D2L4", "BE2L4D4R4U2L4", "BH2R4D4", "NU2NL2NR2D2"};


// Set a pixel in a recursive fractal Maze to a specified color. This edits
// both the monochrome and optional color versions of the Maze.

void CMaz::FractalSet(CMazK *c, KV kv, int x, int y)
{
  Set0(x, y);
  if (c != NULL)
    c->Set(x, y, kv);
}


// Add a passage to a recursive fractal Maze, by drawing a line between two
// specified locations on it. If the ending coordinates are invalid, this
// instead draws from starting coordinates to a specified existing passage.

flag CMaz::FFractalLine(CMaz *b2, CMazK *c, KV kv,
  int x1, int y1, int x2, int y2)
{
  BFSS *bfss;
  CMaz bT;
  int x = x1, y = y1, xnew, ynew, d0, cd, d;
  long iLo = 0, iHi = 1, iMax = 1, i, j;
  flag fLine = (x2 < 0), fRet = fFalse;

  bfss = RgAllocate((m_x >> 2)*(m_y >> 2), BFSS);
  if (bfss == NULL)
    return fFalse;
  if (!bT.FBitmapCopy(*this)) {
    DeallocateP(bfss);
    return fFalse;
  }

  // Determine initial direction to move in.
  if (y == yh + 1)
    d0 = 0;
  else if (x == xh + 1)
    d0 = 1;
  else if (y == yl - 1)
    d0 = 2;
  else if (x == xl - 1)
    d0 = 3;
  else
    d0 = RndDir();
  bfss[0].x = x; bfss[0].y = y; bfss[0].parent = -1;

  // Do a breadth first search to find a shortest path to the destination.
  while (iLo < iHi) {
    for (i = iLo; i < iHi; i++) {
      x = bfss[i].x; y = bfss[i].y;
      for (cd = 0, d = d0; cd < DIRS; cd++, DirInc(d)) {
        xnew = x + (xoff[d] << 2); ynew = y + (yoff[d] << 2);
        if (!fLine ? xnew == x2 && ynew == y2 && i > 0 :
          b2->Get(xnew, ynew)) {

          // Reached the destination! First draw the ending point. Make it
          // thicker if ending point is intersection with an existing passage.
          j = fLine && !ms.fTiltDiamond;
          for (y = ynew - j; y <= ynew + j; y++)
            for (x = xnew - j; x <= xnew + j; x++)
              FractalSet(c, kv, x, y);

          // Draw passage backwards from ending point to starting point.
          j = i;
          do {
            x = bfss[i].x; y = bfss[i].y;
            if (Count(xnew, ynew) == 3 || i == j)
              FractalSet(c, kv,
                xnew + ((x - xnew) >> 2), ynew + ((y - ynew) >> 2));
            FractalSet(c, kv, (xnew + x) >> 1, (ynew + y) >> 1);
            i = bfss[i].parent;
            if (Get(x, y) || i < 0) {
              FractalSet(c, kv,
                x - ((x - xnew) >> 2), y - ((y - ynew) >> 2));
              FractalSet(c, kv, x, y);
              if (b2 != NULL && i >= 0)
                b2->Set1(x, y);
            }
            xnew = x; ynew = y;
          } while (i >= 0);

          // Randomly toggle underpasses formed by passage into overpasses.
          do {
            x = bfss[j].x; y = bfss[j].y;
            j = bfss[j].parent;
            if (j >= 0 && Count2(x, y) == 0 && Rnd(0, 1)) {
              for (d = 0; d < DIRS; d++) {
                xnew = x + xoff[d]; ynew = y + yoff[d];
                Inv(xnew, ynew);
                if (c != NULL) {
                  if (c->Get(xnew, ynew) == kvWhite) {
                    kv = c->Get(xnew + xoff[d], ynew + yoff[d]);
                    c->Set(xnew, ynew, kv);
                    c->Set(x, y, kv);
                  } else
                    c->Set(xnew, ynew, kvWhite);
                }
              }
            }
          } while (j >= 0);

          fRet = fTrue;
          goto LDone;
        } else if (FLegalMaze2(xnew, ynew)) {

          // Check if a cell is not already occupied by an existing passage.
          // If occupied, might still be able to go under it for an underpass.
          if (bT.Get(xnew, ynew)) {
            bT.Set0(xnew, ynew);
            BfssPush(iMax, xnew, ynew, i);
          } else if (bT.Get((x + xnew) >> 1, (y + ynew) >> 1) &&
            bT.Count(xnew, ynew) == 2) {
            if (!ms.fWeaveCorner ? Get(xnew + xoff2[d], ynew + yoff2[d]) :
              FOnMaze2(xnew, ynew)) {
              bT.Set0((x + xnew) >> 1, (y + ynew) >> 1);
              bT.Set0(xnew, ynew+1);
              BfssPush(iMax, xnew, ynew, i);
            }
          }
        }
      }
      if (ms.fRandomPath)
        d0 = Rnd(0, 3);
    }
    iLo = iHi; iHi = iMax;
  }
LDone:
  DeallocateP(bfss);
  return fRet;
}


// Create a new infinite recursive fractal Maze, where subsections of the Maze
// are defined to be copies of the Maze. The fractal Maze's size and number of
// copies is defined by the Fractal fields in the Create Settings dialog.

int CMaz::CreateMazeFractal2(CMazK *c)
{
  CMaz b2;
  int nRet = 0, *rgSet, iset, rgcConnect[8], dxChip = (ms.xFractal + 1) << 2,
    dyChip = (ms.yFractal + 1) << 2, cChip = Min(ms.zFractal, 7),
    cPinChip, cPinMaze, cMaze = 0, cTry, iline, ilineMax, ichip, ipt, cpt,
    ipath, ipathSav, x, y, z, x1, y1, x2, y2, xT, yT, d, i, ichipSav, zStart;
  KV kv;
  flag rgfChip[9];

  if (!FBitmapSizeSet(7 * dxChip + 11, 7 * dyChip + 11))
    return -1;
  if (c != NULL && !c->FBitmapSizeSet(m_x, m_y))
    return -1;
  xl = yl = 6; xh = m_x-7; yh = m_y-7;
  if (!b2.FAllocate(m_x, m_y, this))
    return fFalse;
  cPinChip = (ms.xFractal + ms.yFractal) << 1;
  cPinMaze = 2 + (cChip + 1) * cPinChip;
  rgSet = RgAllocate(cPinMaze, int);
  if (rgSet == NULL)
    return -1;
  for (ichip = 0; ichip < 9; ichip++)
    rgfChip[ichip] = ichip == 0 || ichip == 8;
  for (ichip = 0; rgszChip[cChip - 1][ichip]; ichip++)
    rgfChip[rgszChip[cChip - 1][ichip] - '0'] = fTrue;

LCreate:
  // Make a number of attempts to create the Maze.

  if (++cMaze > ms.nCrackPass)
    goto LDone;
  for (ichip = 0; ichip <= cChip; ichip++)
    rgcConnect[ichip] = 0;
  ClearPb(rgSet, cPinMaze * sizeof(int));
  iset = 0;
  BitmapOn();
  BoxAll(6, 6, fOff);
  if (c != NULL) {
    c->BitmapSet(kvWhite);
    c->BoxAll(6, 6, kvBlack);
  }

  // Start with solid boxes for all chips, so paths don't cross them.

  for (ichip = 0; ichip < 9; ichip++) {
    if (!rgfChip[ichip])
      continue;
    y = ichip / 3; x = ichip - y*3;
    xT = xl + ((x << 1) + 1) * dxChip - 1;
    yT = yl + ((y << 1) + 1) * dyChip - 1;
    Block(xT, yT, xT + dxChip, yT + dyChip, fOff);
  }

  // Draw the paths of the Maze.

  ilineMax = (cChip + 1) * cPinChip >> 2;
  if (ilineMax < cPinChip)
    ilineMax = cPinChip;
  for (iline = -2; iline < ilineMax; iline++) {
    if (iline < 0)
      cpt = 4;
    else
      cpt = 2 + Rnd(0, 2);
    for (ipt = 0; ipt < cpt; ipt++) {
      UpdateDisplay();
      cTry = 0;

LPickPt:
      // Make a number of attempts to draw this passage.
      if (++cTry > 100) {
        if (iline < cPinChip)
          goto LCreate;
        break;
      }

      // Determine chip to connect to.
      if (iline < 0 && ipt == 0)
        ichip = iline;
      else if (iline < 0)
        ichip = Rnd(1, cChip);
      else if (iline < cPinChip) {
        if (ipt == 0)
          ichip = 0;
        else {
          ichip = Rnd(0, cChip);
          if (ms.nRndBias < 0) {
            if (ichip > 0 && Rnd(0, 49) < -ms.nRndBias)
              ichip = 0;
          } else if (ms.nRndBias > 0) {
            if (ichip == 0 && Rnd(0, 49) < ms.nRndBias)
              ichip = Rnd(1, cChip);
          }
        }
      } else
        ichip = Rnd(1, cChip);
      ipath = 2 + (ichip < 0 ? ichip : ichip * cPinChip);

      // Determine pin on the chip.
      if (ichip == 0 && ipt == 0)
        z = iline;
      else if (iline < 0 && ipt == 0 && ms.nEntrancePos < epRandom) {
        if (ms.nEntrancePos == epCorner)
          z = iline == -2 ? 0 : cPinChip - ms.yFractal - 1;
        else if (ms.nEntrancePos == epMiddle)
          z = (iline == -2 ? cPinChip + ms.xFractal : ms.xFractal - 1) >> 1;
        else {
          if (iline == -2)
            z = zStart = Rnd(0, cPinChip-1);
          else
            z = cPinChip-1 + (zStart < ms.xFractal ||
              FBetween(zStart, cPinChip >> 1, cPinChip - ms.yFractal - 1) ?
              -ms.yFractal : ms.xFractal) - zStart;
        }
      } else
        z = Rnd(0, cPinChip-1);
      if (ichip >= 0)
        ipath += z;
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

      // Get coordinates of endpoint.
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
        y = i / 3; x = i - y*3;
        xT = xl + ((x << 1) + 1) * dxChip - 1;
        yT = yl + ((y << 1) + 1) * dyChip - 1;
        if (!FOdd(d)) {
          x = xT + (z << 2);
          y = d == 0 ? yT : yT + dyChip;
        } else {
          x = d == 1 ? xT : xT + dxChip;
          y = yT + (z << 2);
        }
      }

      // Check if endpoint already has a path to it.
      d = Count(x, y);
      if (d == 0) {
        if (ichip == 0 && ipt == 0)
          break;
        goto LPickPt;
      }

      // Have a valid endpoint: Now do something with it.
      if (ipt == 0) {
        x1 = x; y1 = y;
        ichipSav = ichip;
        ipathSav = ipath;
      } else if (ipt == 1) {
        if (x == x1 && y == y1)
          goto LPickPt;
        x2 = x; y2 = y;
        if (cpt >= 3)
          b2.BitmapOff();
        kv = KvShade(kvRandomRainbow, -0.15);
        if (!FFractalLine(cpt < 3 ? NULL : &b2, c, kv, x1, y1, x2, y2))
          goto LPickPt;
        if (ichipSav >= 0)
          rgcConnect[ichipSav]++;
        if (ichip >= 0)
          rgcConnect[ichip]++;
        iset++;
        rgSet[ipathSav] = rgSet[ipath] = iset;
      } else {
        if ((x == x1 && y == y1) || (x == x2 && y == y2))
          goto LPickPt;
        if (!FFractalLine(&b2, c, kv, x, y, -1, -1))
          goto LPickPt;
        if (ichip >= 0)
          rgcConnect[ichip]++;
        rgSet[ipath] = iset;
      }
    }
  }

  // Reject Maze if any chip has less than two paths connected to it.

  for (ichip = 0; ichip <= cChip; ichip++)
    if (rgcConnect[ichip] < 2)
      goto LCreate;

  // Reject Maze if it doesn't have a solution. Detecting whether a recursive
  // fractal Maze has some solution is simpler than actually solving one. All
  // passages have been put into sets. Merge sets based on known crossings of
  // internal chips. Maze solvable if start and end passages end in same set.

  z = zStart = 0;
  loop {
    y = 0;

    // Loop over all internal chips.
    for (ichip = 1; ichip <= cChip; ichip++) {

      // Loop over each pair of pins within that chip.
      x = 2 + ichip * cPinChip;
      for (x1 = 0; x1 < cPinChip; x1++) {
        y1 = rgSet[x + x1];
        if (y1 < 1)
          continue;
        for (x2 = 0; x2 < cPinChip; x2++) {
          y2 = rgSet[x + x2];
          if (y2 < 1 || y1 == y2)
            continue;

          // Merge the sets of the passages linking that pair of pins, if
          // the corresponding outer pins are in the same set, i.e. if there
          // exists some path that crosses the chip between those two pins.
          if (rgSet[2 + x1] == rgSet[2 + x2]) {
            y++;
            for (i = 0; i < cPinMaze; i++)
              if (rgSet[i] == y2)
                rgSet[i] = y1;
          }
        }
      }
    }

    // Done when a pass over all chips produced no new set mergings.
    if (y < 1)
      break;
    iset -= y;
    z++;
    if (zStart < 1 && rgSet[0] == rgSet[1])
      zStart = z;
  }
  if (zStart == 0 || (!ms.fFractalI && iset > 1))
    goto LCreate;
  nRet = zStart;

LDone:
  // Decorate the interior chips.

  b2.BitmapOn();
  gs.turtlec = fOff;
  gs.turtles = 1 + ((Min(ms.xFractal, ms.yFractal) - 1) >> 1);
  for (ichip = i = 0; ichip < 9; ichip++) {
    if (!rgfChip[ichip])
      continue;
    y = ichip / 3; x = ichip - y*3;
    xT = xl + ((x << 1) + 1) * dxChip - 1;
    yT = yl + ((y << 1) + 1) * dyChip - 1;
    Block(xT + 1, yT + 1, xT + dxChip - 1, yT + dyChip - 1, fOn);
    if (c != NULL) {
      c->Edge(xT, yT, xT + dxChip, yT + dyChip, kvBlack);
      c->Block(xT + 1, yT + 1, xT + dxChip - 1, yT + dyChip - 1,
        KvShade(kvRandomRainbow, 0.85));
    }
    if (ichip > 0 && ichip < 8) {
      for (x = 1; x <= ms.xFractal; x++) {
        b2.LineY(xT + (x << 2), yT + 1, yT + 2, fOff);
        b2.LineY(xT + (x << 2), yT + dyChip - 1, yT + dyChip - 2, fOff);
      }
      for (y = 1; y <= ms.yFractal; y++) {
        b2.LineX(xT + 1, xT + 2, yT + (y << 2), fOff);
        b2.LineX(xT + dxChip - 1, xT + dxChip - 2, yT + (y << 2), fOff);
      }
    }
    if (ichip >= 8)
      i = ichip;
    gs.turtlex = xT + (dxChip >> 1); gs.turtley = yT + (dyChip >> 1);
    b2.Turtle(rgszEnd[i]);
    i++;
  }

  // Draw the thicker pins on the outer chip.

  Box(xl-4, yl-4, xh+4, yh+4, 4, 4, fOn);
  for (x = 1; x <= ms.xFractal; x++) {
    b2.Block(xl + (x * 7 << 2) - 2, yl-1, xl + (x * 7 << 2), yl-4, fOff);
    b2.Block(xl + (x * 7 << 2) - 2, yh+1, xl + (x * 7 << 2), yh+4, fOff);
  }
  for (y = 1; y <= ms.yFractal; y++) {
    b2.Block(xl-1, yl + (y * 7 << 2) - 2, xl-4, yl + (y * 7 << 2), fOff);
    b2.Block(xh+1, yl + (y * 7 << 2) - 2, xh+4, yl + (y * 7 << 2), fOff);
  }

  BitmapAnd(b2);
  if (c != NULL) {
    c->Box(xl-4, yl-4, xh+4, yh+4, 4, 4, kvWhite);
    c->ColmapOrAndFromBitmap(b2, kvBlack, kvWhite, 0);
  }
  DeallocateP(rgSet);
  if (nRet < 1) {
    UpdateDisplay();
    PrintSz_W("Failed to create recursive fractal Maze within parameters.");
  }
  return nRet;
}


/*
******************************************************************************
** Other Maze Creation Routines
******************************************************************************
*/

// Carve weave Maze passages adding on to any existing passages in the bitmap.
// Passages are one pixel thick, however walls are three pixels thick, to give
// room to make it clear when a passage is a dead end and when it goes
// underneath another.

flag CMaz::WeaveGenerate(flag fClear, int x, int y)
{
  int xnew, ynew, xnew2, ynew2, xInc = 4, yInc = 4,
    fHunt = fFalse, pass = 0, d, dInc = 1, d2, i, iMax, j, k;
  long count;

  if (!FEnsureMazeSize(7, femsOddSize | femsEvenSize2 | femsNoResize))
    return fFalse;
  while ((x - xl & 3) != 3)
    x--;
  while ((y - yl & 3) != 3)
    y--;
  count = (long)((xh - xl) >> 2) * ((yh - yl) >> 2) - 1;
  iMax = ms.fRiver ? DIRS : 1;
  Set0(x, y);

  // Use the Hunt and Kill algorithm to carve passages for the weave Maze.
  loop {
    if (!Get(x, y) && (fClear || FOnMaze(x, y))) {
LNextLoop:
      d = RndDir();
      neg(dInc);
      for (i = fHunt ? DIRS : iMax; i > 0; i--) {
        xnew = x + (xoff[d] << 2);
        ynew = y + (yoff[d] << 2);
        if (!FLegalMaze2(xnew, ynew))
          goto LNextDir;
        if (Get(xnew, ynew)) {
          if (fCellMax)
            goto LDone;
          Set0(x + xoff[d], y + yoff[d]);
          Set0(x + xoff2[d], y + yoff2[d]);
          Set0(xnew - xoff[d], ynew - yoff[d]);
          Set0(xnew, ynew);
          x = xnew; y = ynew;
LSet:
          pass = 0;
          fHunt = fFalse;
          count--;
          if (count <= 0)
            goto LDone;
          goto LNextLoop;
        }

        // Even if a cell is already part of the Maze, can still move in that
        // direction if can carve under that cell to a truly new cell. Only
        // reject this direction if the cell can't be used as an overpass.

        if (!Get((x + xnew) >> 1, (y + ynew) >> 1) ||
          CountWeave(xnew, ynew) != 1 ||
          Count(xnew, ynew) > 2)
          goto LNextDir;

        // Each valid underpass has only one direction it can go once under
        // the cell. Figure out which direction it is, and carve it. Only
        // straight underpasses allowed if Weave Crossings May Corner is off.

        for (j = -ms.fWeaveCorner; j <= ms.fWeaveCorner; j++) {
          d2 = (d + j) & DIRS1;
          xnew2 = xnew + (xoff[d2] << 2);
          ynew2 = ynew + (yoff[d2] << 2);
          if (Get(xnew2, ynew2)) {
            Set0(x + xoff[d], y + yoff[d]);
            Set0((x + xnew) >> 1, (y + ynew) >> 1);
            Set0((xnew + xnew2) >> 1, (ynew + ynew2) >> 1);
            Set0(xnew2 - xoff[d2], ynew2 - yoff[d2]);
            Set0(xnew2, ynew2);

            // Toggle pixels in all four directions to make the new passage be
            // an overpass instead of an underpass.
            if (Rnd(0, 1)) {
              for (k = 0; k < DIRS; k++)
                Inv(xnew + xoff[k], ynew + yoff[k]);
            }
            x = xnew2; y = ynew2;
            goto LSet;
          }
        }
        Assert(!ms.fWeaveCorner);
LNextDir:
        d = (d + dInc) & DIRS1;
      }
      fHunt = fTrue;
    }
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
          goto LDone;
        UpdateDisplay();
      }
    }
  }
LDone:
  return fTrue;
}


// Create a new weave Maze in the bitmap, or a 2D Maze where passages can go
// over and under each other. The Mazes created are orthogonal and perfect.

flag CMaz::CreateMazeWeave()
{
  if (!FEnsureMazeSize(7, femsOddSize | femsEvenSize2 | femsNoResize |
    femsMinSize))
    return fFalse;
  MazeClear(fOn);
  MakeEntranceExit(3);
  UpdateDisplay();
  return WeaveGenerate(fTrue, Rnd(xl + 3, xh), Rnd(yl + 3, yh));
}


// Convert a weave Maze to a two level 3D Maze, replacing one with the other.
// The top level of the 3D bitmap will contain most of the Maze, while the
// bottom stores the underpasses.

flag CMaz::ConvertWeaveTo3D(CMaz *bTrans)
{
  CMaz t;
  int x, y, x1, y1, x2, y2, d;

  if (!t.FAllocateCube(m_x, m_y, 3, m_w3))
    return fFalse;
  if (bTrans != NULL) {
    if (!bTrans->FBitmapSizeSet(t.m_x, t.m_y))
      return fFalse;
    bTrans->Copy3(t);
    bTrans->BitmapOff();
  }
  t.BitmapOff();

  // Make top level be the visible weave Maze, and start with bottom solid.
  t.BlockMove(*this, 0, 0, m_x-1, m_y-1, 0, 0);
  t.CubeBlock(0, 0, 1, Odd(t.m_x3) - 1, Odd(t.m_y3) - 1, 2, fOn);

  // Carve the underpasses through the bottom level.
  for (y = 7; y < t.m_y3-3; y += 4)
    for (x = 7; x < t.m_x3-3; x += 4)
      if (t.Count(x, y) == 2 && t.Count2(x, y) == 0) {
        t.Set30(x, y, 2);
        for (d = 0; d < DIRS; d++) {
          x1 = x + xoff[d]; y1 = y + yoff[d];
          if (t.Get(x1, y1)) {
            x2 = x + xoff2[d]; y2 = y + yoff2[d];
            if (t.Count(x2, y2) < 4) {

              // Don't allow access between the levels if the top level is
              // just one pixel above two underpasses, as that would appear
              // like a vertical dead end. Technically makes an isolation.
              t.Set30(x2, y2, 1);
            }
            t.Set30(x2, y2, 2);
            t.Set30(x1, y1, 2);

            // If a second bitmap is specified, give it on pixels in the top
            // level on either side of an overpass. If that bitmap is treated
            // as semitransparent walls for the perspective inside view, it
            // will allow visibility between overpasses and underpasses.
            if (bTrans != NULL)
              bTrans->Set1(x1, y1);
          }
        }
      }

  CopyFrom(t);
  return fTrue;
}


#define CPV(x, y, kv1, kv2) \
  cVar.Set(x, y, kv1); c.Set(x, y, kv2)
#define BlockCV(x1, y1, x2, y2, kv1, kv2) \
  cVar.Block(x1, y1, x2, y2, kv1); c.Block(x1, y1, x2, y2, kv2)

// Convert a weave Maze to a scene to be explored in the perspective inside
// view. This is a more advanced version of ConvertWeaveTo3D, where the scene
// will contain stairs, bridges from which one can look over the Maze, etc.

flag CMaz::ConvertWeaveToInside(CMazK &c, CMazK &cVar, CMaz &bVar,
  int zBias, KV kv1, KV kv2, KV kv3)
{
  CMaz b2;
  int x, y, z, xnew, ynew, m, n, m1, n1, m2, n2, d, u,
    zStep = iVar / (zBias+1), zRail = zStep*3;
  flag fOdd;

  if (!FBitmapCollapse())
    goto LFail;

  if (!b2.FBitmapCopy(*this) || !b2.FBitmapBias(1, zBias, 1, zBias) ||
    !cVar.FBitmapSizeSet(b2.m_x, b2.m_y) || !b2.FBitmapCopy(*this))
    goto LFail;
  cVar.BitmapSet(kvBlack);
  if (!c.FBitmapSizeSet(cVar.m_x, cVar.m_y))
    goto LFail;
  c.BitmapSet(kv1);

  // Loop over each cell in the weave Maze.
  for (y = 3; y < m_y; y += 4)
    for (x = 3; x < m_x; x += 4)

      // Is the current cell an overpass?
      if (!Get(x, y) && Count(x, y) == 2 && Count2(x, y) == 0) {
        m = (x >> 1) * (1 + zBias) + 1;
        n = (y >> 1) * (1 + zBias) + 1;
        b2.Set1(x, y);
        BlockCV(m, n, m + zBias - 1, n + zBias - 1,
          UD(iVar, iVar - zStep), kv3);

        // Loop over each direction facing away from the overpass cell.
        for (d = 0; d < DIRS; d++) {
          xnew = x + xoff[d]; ynew = y + yoff[d];
          fOdd = FOdd(d);
          if (!fOdd) {
            m1 = m; m2 = m + zBias - 1;
            n1 = n2 = d < 2 ? n - 1 : n + zBias;
          } else {
            m1 = m2 = d < 2 ? m - 1 : m + zBias;
            n1 = n; n2 = n + zBias - 1;
          }
          if (!Get(xnew, ynew)) {
            b2.Set1(xnew, ynew);
            if (!b2.Get(xnew + xoff[d], ynew + yoff[d])) {

              // Draw the staircase and its handrails
              b2.Set1(xnew + xoff[d], ynew + yoff[d]);
              for (z = 0; z <= zBias; z++) {
                u = (zBias+1 - z) * iVar / (zBias+1);
                BlockCV(m1, n1, m2, n2, UD(u,
                  z == 0 ? 0 : Max(u - zStep*2, 0)), kv3);
                u = (zBias+4 - z) * iVar / (zBias+1);
                u = UD(Min(u, iVar + zRail), 0);
                CPV(m1 - !fOdd, n1 - fOdd, u, kv2);
                CPV(m2 + !fOdd, n2 + fOdd, u, kv2);
                m1 += xoff[d]; n1 += yoff[d]; m2 += xoff[d]; n2 += yoff[d];
              }

              // Draw bottom step handrail if not up against another bridge.
              if (!b2.Get(xnew + xoff2[d], ynew + yoff2[d])) {
                u = UD(zRail, 0);
                CPV(m1 - !fOdd, n1 - fOdd, u, kv2);
                CPV(m2 + !fOdd, n2 + fOdd, u, kv2);
              }
            } else {

              // Extending a bridge. Make another platform instead of stairs.
              cVar.Block(m1 + xoff[d], n1 + yoff[d], m2 + xoff[d]*zBias,
                n2 + yoff[d]*zBias, UD(iVar, iVar - zStep));
              c.Block(m1, n1, m2 + xoff[d]*zBias, n2 + yoff[d]*zBias, kv3);
              u = UD(iVar + zRail, 0);
              BlockCV(m1 - !fOdd, n1 - fOdd, m1 - !fOdd + xoff[d]*zBias,
                n1 - fOdd + yoff[d]*zBias, u, kv2);
              BlockCV(m2 + !fOdd, n2 + fOdd, m2 + !fOdd + xoff[d]*zBias,
                n2 + fOdd + yoff[d]*zBias, u, kv2);
            }
          } else {

            // Draw handrail for bridge over passage
            BlockCV(m1, n1, m2, n2, UD(iVar + zRail, iVar - zStep), kv2);
            u = UD(iVar + zRail, 0);
            CPV(m1 - !fOdd, n1 - fOdd, u, kv2);
            CPV(m2 + !fOdd, n2 + fOdd, u, kv2);

            // Mark the walls leading to the underpass as variable height
            // so drawing will continue and allow seeing handrail over them.
            if (!b2.Get(xnew + xoff[d], ynew + yoff[d])) {
              cVar.Block(m1 - !fOdd + xoff[d], n1 - fOdd + yoff[d],
                m1 - (!fOdd)*3 + xoff[d]*zBias, n1 - fOdd*3 + yoff[d]*zBias,
                UD(iVar, 0));
              cVar.Block(m2 + !fOdd + xoff[d], n2 + fOdd + yoff[d],
                m2 + (!fOdd)*3 + xoff[d]*zBias, n2 + fOdd*3 + yoff[d]*zBias,
                UD(iVar, 0));
            }
          }
        }
      }

  if (!b2.FBitmapBias(1, zBias, 1, zBias) ||
    !cVar.FColmapPutToBitmap(bVar, 0))
    goto LFail;
  c.ColmapOrAndFromBitmap(b2, kvBlack, kvWhite, 0);
  CopyFrom(b2);
  return fTrue;

LFail:
  return fFalse;
}


// Replace a Weave Maze in a bitmap with a clarified version of itself. Walls
// and passages expand to the specified number of pixels. Railings next to
// overpasses take up the specified number of pixels within a wall.

flag CMaz::FClarifyWeave(int zWall, int zPass, int zRail)
{
  CMaz bNew;
  int zCell = zWall + zPass, xs = (m_x - 1) >> 2, ys = (m_y - 1) >> 2,
    x, y, x2, y2, xT, yT;

  if (!bNew.FAllocate(xs * zCell + zWall, ys * zCell + zWall, this))
    return fFalse;
  bNew.BitmapOn();
  if (zRail <= 0 || zRail*2 >= zWall)
    zRail = 0;
  for (y = y2 = 0; y < (ys << 2) + 3; y++, y2 += yT) {
    yT = FOdd(y) ? ((y & 3) == 3 ? zPass : zWall - zRail*2) : zRail;
    if (yT == 0)
      continue;
    for (x = x2 = 0; x < (xs << 2) + 3; x++, x2 += xT) {
      xT = FOdd(x) ? ((x & 3) == 3 ? zPass : zWall - zRail*2) : zRail;
      if (xT == 0)
        continue;
      if (!Get(x, y)) {
        if (zRail <= 0 && (x & 3) == 3 && (y & 3) == 3 &&
          Count(x, y) == 2 && Count2(x, y) == 0)
          continue;
        bNew.Block(x2, y2, x2 + xT - 1, y2 + yT - 1, fOff);
      }
    }
  }

  CopyFrom(bNew);
  return fTrue;
}


#define FCavernSet(x, y) (!FLegalMaze2(x, y) || (Get(x, y) ^ fWall))

// Randomly return the direction of a pixel adjacent to the given coordinates
// that's not part of the Maze yet, evenly distributed among the available
// directions. Like DirFindUncreated() but for cavern Mazes.

int CMaz::DirFindUncreatedCavern(int *x, int *y, flag fWall)
{
  int rgdir[DIRS], xnew, ynew, xnew2, ynew2, x1, y1, x2, y2, xT, yT,
    d, d1, d2, i, count;

  d = RndDir();
  count = 0;
  for (i = 0; i < DIRS; i++) {
    DirInc(d);
    xnew = *x + xoff[d]; ynew = *y + yoff[d];
    if (xnew > xl && ynew > yl && xnew < xh && ynew < yh &&
      (Get(xnew, ynew) ^ fWall)) {

      // Not only does the pixel in an available direction have to not be part
      // of the Maze, but same with the five pixels surrounding it in that
      // direction, to ensure creating here leaves the Maze perfect.

      d1 = d + 1 & DIRS1; d2 = d1 ^ 2;
      if (ms.nSparse < 1) {
        if (FCavernSet(xnew + xoff[d1], ynew + yoff[d1]) &&
          FCavernSet(xnew + xoff[d2], ynew + yoff[d2])) {
          xnew2 = xnew + xoff[d]; ynew2 = ynew + yoff[d];
          if (FCavernSet(xnew2, ynew2) &&
            FCavernSet(xnew2 + xoff[d1], ynew2 + yoff[d1]) &&
            FCavernSet(xnew2 + xoff[d2], ynew2 + yoff[d2])) {
            rgdir[count] = d;
            count++;
          }
        }
      } else {
        x1 = xnew + xoff[d1]*ms.nSparse; y1 = ynew + yoff[d1]*ms.nSparse;
        xnew2 = xnew + xoff[d]*ms.nSparse; ynew2 = ynew + yoff[d]*ms.nSparse;
        x2 = xnew2 + xoff[d2]*ms.nSparse; y2 = ynew2 + yoff[d2]*ms.nSparse;
        SortN(&x1, &x2);
        SortN(&y1, &y2);
        for (yT = y1; yT <= y2; yT++)
          for (xT = x1; xT <= x2; xT++)
            if (!FCavernSet(xT, yT))
              goto LNext;
        rgdir[count] = d;
        count++;
LNext:
        ;
      }
    }
  }
  if (count < 1)
    return -1;
  d = rgdir[Rnd(0, count-1)];
  *x += xoff[d]; *y += yoff[d];
  return d;
}


// Carve cavern Maze passages into a solid shape on the bitmap, or add walls
// assuming the bitmap is clear.

flag CMaz::CavernGenerate(flag fWall, int xs, int ys)
{
  PT *rgpt;
  long cpt = 1, ipt = 0, iptLo, iptHi;
  int x, y, d;

  if (!FEnsureMazeSize(3, femsNoResize | fems64K))
    return fFalse;
  rgpt = RgAllocate((xh-xl+1)*(yh-yl+1), PT);
  if (rgpt == NULL)
    return fFalse;

  // Figure out which pixel or pixels to start growing from.
  if (!fWall) {
    x = xs; y = ys;
    Set0(x, y);
    PutPt(ipt, x, y);
  } else {
    for (x = xl; x <= xh; x++) {
      PutPt(ipt, x, yl); PutPt(ipt + 1, x, yh);
      ipt += 2;
    }
    for (y = yl + 1; y < yh; y++) {
      PutPt(ipt, xl, y); PutPt(ipt + 1, xh, y);
      ipt += 2;
    }
    cpt = ipt;
    ipt = Rnd(0, cpt-1);
    GetPt(ipt, x, y);
  }

  // Use the Growing Tree algorithm to create the cavern maze.
  loop {
    d = DirFindUncreatedCavern(&x, &y, fWall);
    if (d >= 0) {
      if (fCellMax)
        break;
      Set(x, y, fWall);
      PutPt(cpt, x, y);
      cpt++;
    } else {
      cpt--;
      if (cpt < 1)
        break;
      rgpt[ipt] = rgpt[cpt];
    }
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
        iptHi = cpt - 1;
      } else {
        iptHi = -(ms.nTreeRiver + 1);
        if (iptHi >= cpt)
          iptHi = cpt - 1;
        iptLo = 0;
      }
      ipt = Rnd(iptLo, iptHi);
    }
    GetPt(ipt, x, y);
  }
  DeallocateP(rgpt);
  return fTrue;
}


// Create a new cavern style Maze in the bitmap, or a Maze not on any grid
// where each passage or wall segment is a single pixel. The result looks like
// natural passages in a cave, with small solid areas or rooms. The Maze will
// be perfect, where this can carve passages or add walls.

flag CMaz::CreateMazeCavern()
{
  int x1, x2, x, y, i, d;
  flag fRet;

  if (!FEnsureMazeSize(3, femsNoResize | fems64K))
    return fFalse;
  BitmapSet(!ms.fTreeWall);
  if (ms.fTreeWall)
    Edge(xl, yl, xh, yh, fOn);
  UpdateDisplay();
  fRet = CavernGenerate(ms.fTreeWall, Rnd(xl+1, xh-1), Rnd(yl+1, yh-1));
  MakeEntranceExit(2);

  // Sparse means wide passages, so widen entrance and exit opening too.
  if (ms.nSparse > 0 && ms.fTreeWall)
    for (i = 0; i <= 1; i++) {
      x1 = (!i ? ms.xEntrance : ms.xExit) - 1; x2 = x1 + 2;
      y = !i ? ms.yEntrance : ms.yExit;
      d = 1 - (i << 1);
      for (x = 1; x < ms.nSparse; x++) {
        if (FOdd(x) && !Get(x1, y + d))
          Set0(x1--, y);
        else if (!Get(x2, y + d))
          Set0(x2++, y);
        else if (!Get(x1, y + d))
          Set0(x1--, y);
      }
    }
  return fRet;
}


// Create a crack Maze, or a Maze formed of lines at random angles not on any
// grid. This creates a perfect Maze by adding walls.

long CMaz::CreateMazeCrack(flag fClear)
{
  int mpgrff[256], xm, ym, pass = 0, x1, y1, x2, y2, i, k,
    x, y, dx, dy, xInc, yInc, xInc2, yInc2, d, dInc, z, zMax;
  flag fAllLegal, f;
  long count = 0, l;

  if (!FEnsureMazeSize(6, femsNoResize))
    return -1;
  ClearPb(mpgrff, sizeof(mpgrff));
  for (i = 0; i < 25; i++)
    mpgrff[rggrfNeighbor[i]] = fTrue;
  xm = (xh - xl + 1) >> 1; ym = (yh - yl + 1) >> 1;
  if (xm > ms.nCrackLength)
    xm = ms.nCrackLength;
  if (ym > ms.nCrackLength)
    ym = ms.nCrackLength;

  // Start with the outer boundary wall.
  if (fClear) {
    MazeClear(fOff);
    MakeEntranceExit(1);
    UpdateDisplay();
  }

  // Draw lines appending onto what's already present.
  while (pass < ms.nCrackPass) {

    // Find an on pixel, i.e. an existing wall, to start drawing a line from.
    for (l = ms.nCrackPass * 100L; l > 0; l--) {
      x1 = Rnd(xl, xh); y1 = Rnd(yl, yh);
      if (Get(x1, y1))
        break;
    }
    if (l <= 0)
      break;

    // Find a destination point to try to draw the line to.
    if (!ms.nCrackSector) {
      do {
        y2 = y1 + Rnd(-ym, ym);
      } while (!ms.fCrackOff && (y2 < yl || y2 > yh));
      do {
        x2 = x1 + Rnd(-xm, xm);
      } while (!ms.fCrackOff && (x2 < xl || x2 > xh));
    } else {
      d = NAbs(ms.nCrackSector) << 1;
      k = Rnd(0, d-1);
      i = ms.nCrackSector >= 0 ? 0 : 180 / d;
      do {
        z = Rnd(0, xm);
        y2 = y1 + NSinRD((real)z, k * 360 / d + i);
        x2 = x1 + NCosRD((real)z, k * 360 / d + i);
      } while (!ms.fCrackOff && (y2 < yl || y2 > yh || x2 < xl || x2 > xh));
    }

    x = x1; y = y1; dx = x2 - x1; dy = y2 - y1;
    fAllLegal = FLegalMaze2(x2, y2);
    f = fFalse;

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

    // Draw the line a pixel at a time. Stop before hitting any other wall.
    for (z = 0; z <= zMax; z++) {
      x += xInc; y += yInc; d += dInc;
      if (d >= zMax) {
        x += xInc2; y += yInc2; d -= zMax;
      }
      if ((!fAllLegal && !FLegalMaze2(x, y)) || Get(x, y))
        break;
      k = 0;
      for (i = 0; i < DIRS2; i++)
        k = (k << 1) + Get(x + xoff[i], y + yoff[i]);
      if (mpgrff[k]) {
        Set1(x, y);
        f = fTrue;
      } else
        break;
    }

    // If at least one pixel was drawn in the line, the line was a success.
    // Stop the Maze after too many attempts in a row fail to add anything.
    if (f) {
      pass = 0;
      count++;
      if (fCellMax)
        break;
    } else
      pass++;
  }
  return count;
}


// Carve Zeta passages adding on to any existing passages in the bitmap.

flag CMaz::ZetaGenerate(flag fClear, int x, int y)
{
  int xnew, ynew, xInc = 4, yInc = 4, fHunt = fFalse, pass = 0, count,
    i, iMax, d, dInc = 1;

  if (!FEnsureMazeSize(7, femsOddSize | femsEvenSize2 | femsNoResize))
    return fFalse;
  while ((x - xl & 3) != 3)
    x--;
  while ((y - yl & 3) != 3)
    y--;
  count = ((xh - xl) >> 2)*((yh - yl) >> 2) - 1;
  iMax = ms.fRiver ? DIRS2 : 1;
  Set0(x, y);
  UpdateDisplay();

  // Use the Hunt and Kill algorithm to carve passages for the Zeta Maze.
  loop {
    if (!Get(x, y) && (fClear || FOnMazeZeta(x, y))) {
LNext:
      d = Rnd(0, DIRS2-1);
      neg(dInc);
      for (i = fHunt ? DIRS2 : iMax; i > 0; i--) {
        xnew = x + (xoff[d] << 2);
        ynew = y + (yoff[d] << 2);

        // Not only must the cell moved to be uncarved, but if moving
        // diagonally make sure not to cross any opposite diagonal passage.
        if (FLegalMaze2(xnew, ynew) && Get(xnew, ynew) &&
          (d < DIRS || (Get(x + xoff2[d], y + yoff2[d]) && (fClear ||
          (Get(x + xoff[d], y + yoff[d]*3) &&
          Get(x + xoff[d]*3, y + yoff[d])))))) {
          if (fCellMax)
            goto LDone;
          Set0(x + xoff[d], y + yoff[d]);
          Set0(x + xoff2[d], y + yoff2[d]);
          Set0(xnew - xoff[d], ynew - yoff[d]);
          Set0(xnew, ynew);
          x = xnew; y = ynew;
          pass = 0;
          fHunt = fFalse;
          count--;
          if (count <= 0)
            goto LDone;
          goto LNext;
        }
        d = (d + dInc) & (DIRS2-1);
      }
      fHunt = fTrue;
    }
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
          goto LDone;
        UpdateDisplay();
      }
    }
  }
LDone:
  return fTrue;
}


// Create a new Zeta style Maze in the bitmap, or a Maze on an orthogonal grid
// where diagonal passages are allowed too. Each cell can have up to eight
// passages connecting to it.

flag CMaz::CreateMazeZeta()
{
  if (!FEnsureMazeSize(7, femsOddSize | femsEvenSize2 | femsNoResize |
    femsMinSize))
    return fFalse;
  if (xl <= 0 && yl <= 0 &&
    FBetween(xh, m_x-4, m_x-2) && FBetween(yh, m_y-4, m_y-2))
    BitmapOff();
  MazeClear(fOn);
  MakeEntranceExit(3);
  UpdateDisplay();
  return ZetaGenerate(fTrue, Rnd(xl + 3, xh), Rnd(yl + 3, yh));
}


#define ALEN 12
#define AWID 2

// Draw an arrow on the bitmap, with its tail at the given coordinates and
// facing in one of the four directions.

void CMaz::ArrowDraw(CCol *c, int x, int y, int d, KV kv)
{
  int e = d, i, j, k, m, n;

  x = (x >> 1)*ALEN + AWID + 1; y = ((y + 1) >> 1)*ALEN - 1;
  x += xoff2[d]; y += yoff2[d];
  DirInc(e);
  for (i = 0; i <= ALEN-4; i++) {

    // Draw the staff of the arrow
    j = i < ALEN-4-AWID ? 0 : ALEN-4-i;
    for (k = -j; k <= j; k++) {

      // When within the arrowhead, make the staff more than one pixel thick
      m = x + xoff[e]*k; n = y + yoff[e]*k;
      Set1(m, n);
      if (c != NULL)
        c->Set(m, n, kv);
    }
    x += xoff[d]; y += yoff[d];
  }
}


// Create a new arrow Maze in the bitmap, or a Maze where passages can only be
// traversed in one direction. The Maze is composed of a bunch of arrows,
// where one may only travel over an arrow in the direction it's pointing.

flag CMaz::CreateMazeArrow(CCol *c)
{
  CMaz b2, b3;
  int xc, yc, x0, y0, x, y, xnew, ynew, d, i, j;

  xc = (m_x - AWID*2 - 3) / ALEN + 1; yc = (m_y + 1) / ALEN - 1;
  if (xc < 1)
    xc = 1;
  if (yc < 1)
    yc = 1;
  if (!FBitmapSizeSet((xc << 1) + 1, (yc << 1) + 1))
    return fFalse;
  SetXyh();
  MazeClear(fOn);
  MakeEntranceExit(0);

  // An arrow Maze is based on a standard perfect Maze.

  PerfectGenerate(fTrue, Rnd(xl, xh-1), Rnd(yl, yh-1));
  UpdateDisplay();
  b2.CopyFrom(*this);
  if (!FAllocate((xc-1)*ALEN + AWID*2 + 3, (yc+1)*ALEN - 1, this))
    return fFalse;
  BitmapOff();
  if (!b3.FBitmapCopy(b2))
    return fFalse;
  UpdateDisplay();
  b2.SetXyh();
  if (c != NULL) {
    if (!c->FBitmapSizeSet(m_x, m_y))
      return fFalse;
    c->BitmapOff();
  }

  // Draw arrows down all dead ends of the perfect Maze.

  for (y0 = yl + 1; y0 < yh; y0 += 2)
    for (x0 = xl + 1; x0 < xh; x0 += 2)
      if (!b2.Get(x0, y0) && b2.Count(x0, y0) >= DIRS1) {
        x = x0; y = y0;
        do {
          b2.Set1(x, y);
          for (d = 0; d < DIRS &&
            b2.Get(xnew = x + xoff[d], ynew = y + yoff[d]); d++)
            ;
          b2.Set1(xnew, ynew);
          x += (xnew - x) << 1; y += (ynew - y) << 1;
          ArrowDraw(c, x, y, d ^ 2, kvWhite);
        } while (FLegalMaze2(x, y) && b2.Count(x, y) == DIRS1);
      }

  // Draw extra arrows between dead end paths in the perfect Maze.

  for (y = yl + 1; y < yh; y += 2)
    for (x = xl + 1; x < xh; x += 2)
      if (b2.Get(x, y))
        for (d = 2; d < DIRS; d++) {
          xnew = x + xoff[d]; ynew = y + yoff[d];
          if (xnew < xh && ynew < yh && b3.Get(xnew, ynew) &&
            b2.Get(xnew += xoff[d], ynew += yoff[d])) {
            if (Rnd(0, 1))
              ArrowDraw(c, x, y, d, kvGray);
            else
              ArrowDraw(c, xnew, ynew, d ^ 2, kvGray);
          }
        }

  // Draw arrows down solution path and extra arrows leading off solution.

  x = ms.xEntrance; y = yl-1; d = 2;
  loop {
    ArrowDraw(c, x, y, d, kvRed);
    j = b2.FollowWall(&x, &y, d, fTrue);
    d = b2.FollowWall(&x, &y, j, fTrue);
    for (i = 0; i < DIRS; i++)
      if (i != d && i != (j ^ 2)) {
        xnew = x + xoff2[i]; ynew = y + yoff2[i];
        if (!b2.FLegal(xnew, ynew))
          continue;
        if (b2.Get(xnew, ynew))
          ArrowDraw(c, x, y, i, kvBlue);
        else if (b3.Get(xnew, ynew))
          ArrowDraw(c, x, y, i, kvGreen);
      }
    if (y > yh)
      break;
    b3.Set1(x, y);
  }

  ms.xEntrance = (ms.xEntrance >> 1)*ALEN + AWID + 1;
  return fTrue;
}


#define ROOMS 100

// Create a new dungeon style Maze in the bitmap, and draw a colorized version
// of it in a color bitmap if present. The dungeon is formed of random rooms
// with connecting passages, similar to dungeon levels seen in the computer
// games Rogue and Nethack. Implements the Dungeon operation.

flag CMaz::CreateMazeDungeon(CCol &c, CMon &bDoor,
  int cRoom, int cPassage, int zRoom, int nDoorPct, flag f3D,
  int nEntranceExit, int *pxStart, int *pyStart)
{
  int xr1[ROOMS], yr1[ROOMS], xr2[ROOMS], yr2[ROOMS], nRoomSet[ROOMS],
    x3D, y3D, x1, y1, x2, y2, i, j, k, l;
  flag fEntrance = nEntranceExit / 10, fExit = nEntranceExit % 10, fX;

  SetXyh();
  if (f3D) {
    x3D = m_x >> 1; y3D = m_y / 3;
    xh = x3D - 1;
    if (!fEntrance)
      yl = y3D;
    yh = yl + y3D - 1;
  }
  if (FMazeSizeError(3, 3))
    return fFalse;
  BitmapOn();

  // Create up to the specified number of rooms.

  for (i = 0; i < cRoom; i++) {
    for (k = 0; k < 100; k++) {
      if (cRoom > 1) {
        x1 = Rnd(xl, xh-zRoom); y1 = Rnd(yl, yh-zRoom);
        x2 = x1 + Rnd(4, zRoom); y2 = y1 + Rnd(4, zRoom);
      } else {
        // If creating just one room, make it fill the whole level.
        x1 = xl; y1 = yl; x2 = xh; y2 = yh;
      }

      // Make sure the room isn't too close to any other rooms.
      for (j = 0; j < i; j++)
        if (x1 < xr2[j]+2 && x2 > xr1[j]-2 &&
          y1 < yr2[j]+2 && y2 > yr1[j]-2)
          break;
      if (j >= i) {
        xr1[i] = x1; yr1[i] = y1; xr2[i] = x2; yr2[i] = y2;
        Block(x1+1, y1+1, x2-1, y2-1, fOff);
        break;
      }
    }

    // Stop after enough tries to randomly place a room fail.
    if (k >= 100) {
      cRoom = i;
      break;
    }
  }

  // If creating zero rooms, fill the level with passages, i.e. a Maze.

  if (cRoom <= 0) {
    fX = ms.fSection; ms.fSection = fTrue;
    PrimGenerate(fFalse, fTrue,
      Rnd(0, ((xh - xl) >> 1) - 1), Rnd(0, ((yh - yl) >> 1) - 1));
    ms.fSection = fX;
  }
  UpdateDisplay();

  // Create passages connecting rooms with each other.

  for (i = 0; i < cRoom; i++)
    nRoomSet[i] = i;
  if (cRoom <= 1)
    cPassage = 0;
  for (k = 1; k < cRoom + cPassage; k++) {
    loop {

      // Figure out which rooms to try to connect with each other. The first
      // cRoom-1 passages will form a minimum spanning tree between the rooms
      // to ensure it's possible to get from any room to any other room.
      do {
        i = Rnd(0, cRoom-1);
        j = Rnd(0, cRoom-1);
      } while (i == j || (k < cRoom && nRoomSet[i] == nRoomSet[j]));

      // Figure out whether the passage should be horizontal or vertical.
      if (xr1[i] < xr2[j]+2 && xr2[i] > xr1[j]-2)
        fX = 0;
      else if (yr1[i] < yr2[j]+2 && yr2[i] > yr1[j]-2)
        fX = 1;
      else
        fX = Rnd(0, 1);

      // Figure out at what points the passage should enter the two rooms.
      if (fX) {
        y1 = Rnd(yr1[i]+1, yr2[i]-1); x1 = xr1[i] > xr1[j] ? xr1[i] : xr2[i];
        y2 = Rnd(yr1[j]+1, yr2[j]-1); x2 = xr1[j] > xr1[i] ? xr1[j] : xr2[j];
      } else {
        x1 = Rnd(xr1[i]+1, xr2[i]-1); y1 = yr1[i] > yr1[j] ? yr1[i] : yr2[i];
        x2 = Rnd(xr1[j]+1, xr2[j]-1); y2 = yr1[j] > yr1[i] ? yr1[j] : yr2[j];
      }

      // Make sure the passage rectangle doesn't intersect any other rooms.
      for (l = 0; l < cRoom; l++) {
        if (l == i || l == j)
          continue;
        if (Min(x1, x2) < xr2[l]+2 && Max(x1, x2) > xr1[l]-2 &&
          Min(y1, y2) < yr2[l]+2 && Max(y1, y2) > yr1[l]-2)
          break;
      }
      if (l < cRoom)
        continue;

      // Draw the passage, usually containing two right angle turns along it.
      if (fX) {
        l = x1 < x2 ? Rnd(x1+1, x2-1) : Rnd(x2+1, x1-1);
        LineX(x1, l, y1, fOff);
        LineY(l, y1, y2, fOff);
        LineX(l, x2, y2, fOff);
      } else {
        l = y1 < y2 ? Rnd(y1+1, y2-1) : Rnd(y2+1, y1-1);
        LineY(x1, y1, l, fOff);
        LineX(x1, x2, l, fOff);
        LineY(x2, l, y2, fOff);
      }

      // Unify the sets the two rooms are in, since they're connected now.
      i = nRoomSet[i]; j = nRoomSet[j];
      if (k < cRoom && i != j)
        for (l = 0; l < cRoom; l++)
          if (nRoomSet[l] == j)
            nRoomSet[l] = i;
      break;
    }
  }

  // Draw the entrance to the dungeon.

  if (fEntrance) {

    // Draw a vertical entrance passage from the top of the dungeon, to run
    // into the top of the first room with no other room wholly above it.
    for (i = 0; i < cRoom; i++) {
      for (j = 0; j < cRoom; j++)
        if (j != i && yr1[i] > yr2[j])
          break;
      if (j >= cRoom) {
        k = Rnd(xr1[i]+1, xr2[i]-1);
        LineY(k, yl, yr1[i], fOff);
        x1 = k; y1 = yl;
        break;
      }
    }
  } else {

    // Pick a random spot in a random room for the entry point.
    if (cRoom > 0) {
      i = Rnd(0, cRoom-1);
      x1 = Rnd(xr1[i]+1, xr2[i]-1); y1 = Rnd(yr1[i]+1, yr2[i]-1);
    } else {
      // Or pick a random passage if this is a Maze with no rooms.
      x1 = xl + (Rnd(0, ((xh - xl) >> 1) - 1) << 1) + 1;
      y1 = yl + (Rnd(0, ((yh - yl) >> 1) - 1) << 1) + 1;
    }
    // If the bitmap is 3D, drill a hole in the ceiling at the entry point.
    if (f3D) {
      Set0(x1, y1-y3D); Set0(x1+x3D, y1-y3D);
    }
  }

  // Draw the exit to the dungeon (if specified and for 3D bitmaps only).

  if (!fExit && f3D) {

    // Pick a random spot in a random room for the exit pit. Can't be same
    // spot as entrance, and can't be next to a wall as might block a door.
    do {
      if (cRoom > 0) {
        i = Rnd(0, cRoom-1);
        x2 = Rnd(xr1[i]+2, xr2[i]-2); y2 = Rnd(yr1[i]+2, yr2[i]-2);
      } else {
        // The exit pit in a Maze has to be a dead end to not block a passage.
        do {
          x2 = xl + (Rnd(0, ((xh - xl) >> 1) - 1) << 1) + 1;
          y2 = yl + (Rnd(0, ((yh - yl) >> 1) - 1) << 1) + 1;
        } while (Count(x2, y2) < DIRS1);
      }
    } while (x2 == x1 && y2 == y1);
    // Drill a hole in the floor at the exit point.
    Set0(x2, y2+y3D); Set0(x2+x3D, y2);
  }
  *pxStart = x1; *pyStart = y1 - yl;

  // If a color bitmap specified, make it a colorized version of the dungeon.

  if (!c.FNull()) {
    if (!c.FColmapGetFromBitmap(*this, kvDkGray, kvLtGray))
      return fFalse;
    if (!bDoor.FNull() && !bDoor.FBitmapSizeSet(m_x, m_y))
      return fFalse;
    if (!bDoor.FNull())
      bDoor.BitmapOff();
    for (k = 0; k < cRoom; k++) {
      c.Block(xr1[k], yr1[k], xr2[k], yr2[k], kvGray);
      for (j = yr1[k]; j <= yr2[k]; j++)
        for (i = xr1[k]; i <= xr2[k]; i++)
          if (i == xr1[k] || i == xr2[k] || j == yr1[k] || j == yr2[k])
            if (!Get(i, j)) {

              // Randomly put doors in some entrances to rooms. If a door
              // bitmap is specified, it will have on pixels where doors are.
              if (Rnd(1, 100) <= nDoorPct) {
                c.Set(i, j, kvGreen);
                Set1(i, j);
                if (!bDoor.FNull())
                  bDoor.Set1(i, j);
              }
            } else
              c.Set(i, j, kvRed);
    }
    if (f3D) {
      if (!fEntrance)
        c.Set(x1, y1, kvBlack);
      if (!fExit)
        c.Set(x2, y2, kvBlack);
    }
  }
  return fTrue;
}

/* create3.cpp */
