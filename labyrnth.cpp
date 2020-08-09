/*
** Daedalus (Version 3.3) File: labyrnth.cpp
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
** This file contains unicursal Labyrinth creation algorithms.
**
** Created: 7/12/2009.
** Last code change: 11/29/2018.
*/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include "util.h"
#include "graphics.h"
#include "color.h"
#include "threed.h"
#include "maze.h"


// Convert a Maze to a unicursal Maze, by adding walls bisecting each passage,
// where each old dead end becomes a new U-turn passage. This should only be
// called on a bitmap that's been bias zoomed such that walls are one pixel
// and passages three pixels thick. Called from CreateMazeUnicursal() to do
// most of the work of creating unicursal Mazes.

long CMaz::UnicursalApply(flag fClear)
{
  int x, y;
  int count = 0;

  for (y = yl + 2; y <= yh - 2; y += 4)
    for (x = xl + 2; x <= xh - 2; x += 4)
      if (fClear || FOnMaze2(x, y)) {
        count++;
        Set1(x, y);
        if (!Get(x, y - 2)) {
          Set1(x, y - 1);
          Set1(x, y - 2);
        }
        if (!Get(x - 2, y)) {
          Set1(x - 1, y);
          Set1(x - 2, y);
        }
        if (!Get(x, y + 2))
          Set1(x, y + 1);
        if (!Get(x + 2, y))
          Set1(x + 1, y);
      }
  return count;
}


// Create a new Unicursal Maze in the bitmap, i.e. a Maze without junctions.

flag CMaz::CreateMazeUnicursal()
{
  CMaz b2;
  flag fOddX, fOddY, fX, fY, fSav;
  int nSav1, nSav2;

  if (!FEnsureMazeSize(3, femsNoResize | femsNoSection | femsMinSize))
    return fFalse;
  fOddX = FOdd(m_x); fOddY = FOdd(m_y);
  fX = (m_x + fOddX & 2) == 0; fY = (m_y + fOddY & 2) == 0;

  // Start with a bitmap slightly more than one half the size of the final.
  if (!FBitmapSizeSet(
    ((m_x + fOddX) >> 1)+2 & ~1, ((m_y + fOddY) >> 1)+2 & ~1))
    return fFalse;
  SetXyh();
  if (!(ms.fInfEller && fX && fY)) {
    BitmapOn();
    BlockOuter(fOff);

    // Add passages along 0 to 2 edges to allow different entrance locations.
    if (fX || fY) {
      if (fX)
        LineY(fY ? xh - 2 : 1, 1, yh - 2, fOff);
      if (fY)
        LineX(1, xh - 2, 1, fOff);
      Set0(1, 0);
      if (fX) {
        if (fY)
          Set0(xh-2, yh-1);
        else
          Set0(1, yh-1);
      } else
        Set0(xh-2, 1);
    } else
      MakeEntranceExit(6);

    // Create a new perfect Maze by adding onto the initial lines.
    fSav = ms.fRiver; ms.fRiver = fFalse;
    nSav1 = ms.nHuntType; ms.nHuntType = 1;
    PerfectGenerate(fFalse, 1, 1);
    ms.fRiver = fSav; ms.nHuntType = nSav1;
  } else {

    // Create binary tree Maze which already has passages along 2 edges.
    nSav1 = ms.nRndRun; ms.nRndRun = 100;
    nSav2 = ms.nEntrancePos; ms.nEntrancePos = epCorner;
    CreateMazeBinary();
    ms.nRndRun = nSav1; ms.nEntrancePos = nSav2;
  }

  // Zoom this standard Maze 200% and convert it to a Unicursal Maze.
  if (!FBitmapBias(1, 3, 1, 3))
    return fFalse;
  SetXyh();
  UpdateDisplay();
  UnicursalApply(fTrue);

  // Resize the bitmap to get rid of the starting lines and do touch ups.
  if (!b2.FAllocate(
    m_x - (1 + fX)*2 - fOddX, m_y - (1 + fY)*2 - fOddY, this))
    return fFalse;
  b2.BlockMove(*this, (fX && !fY)*2, fY*2, m_x + 1, m_y + 1, 0, 0);
  CopyFrom(b2);
  if (fX || fY) {
    if (fX)
      Set1(fY ? m_x-2+fOddX : 0, m_y-2+fOddY);
    ms.xEntrance = 1;
  } else
    ms.xEntrance = (ms.xEntrance << 1) - 1;
  ms.yEntrance = 0;
  return fTrue;
}


// Draw a Hilbert curve, a type of fractal unicursal Labyrinth.

flag CMaz::CreateMazeHilbert(flag f3D)
{
  CMaz b2;
  int i, w, x, y, z;

  if (!f3D) {

    // Create base case generation 1
    if (!FBitmapSizeSet(5, 5))
      return fFalse;
    BitmapOn();
    for (x = 1; x <= 3; x += 2)
      for (y = 1; y <= 3; y++)
        Set0(x, y);
    Set0(2, 3);

    // Create each new generation from four copies of the original
    for (i = 1; i < ms.zFractal; i++) {
      b2.CopyFrom(*this);
      w = b2.m_x - 1;
      if (!FBitmapSizeSet((m_x << 1) - 1, (m_y << 1) - 1))
        return fFalse;
      BitmapOn();
      for (y = 1; y < w; y++)
        for (x = 1; x < w; x++)
          if (!b2.Get(x, y)) {
            Set0(y, x);       // 1 transpose
            Set0(x, w+y);     // 2 normal
            Set0(w+x, w+y);   // 3 normal
            Set0(m_x-1-y, x); // 4 transpose flipx
          }
      Set0(1, w); Set0(m_x-2, w);
      Set0(w, w+1);
      UpdateDisplay();
    }
    Set0(1, 0); Set0(m_x-2, 0);
  } else {

    // Create base case generation 1
    if (!FCubeSizeSet(5, 5, 3, m_w3))
      return fFalse;
    BitmapOn();
    for (x = 1; x <= 3; x += 2)
      for (y = 1; y <= 3; y += 2) {
        for (z = 0; z <= 2; z++)
          Set30(x, y, z);
        Set30(x, 2, 2);
      }
    Set30(2, 3, 0);

    // Create each new generation from eight copies of the original
    for (i = 1; i < ms.zFractal; i++) {
      b2.CopyFrom(*this);
      w = b2.m_x3 - 1;
      if (!FCubeSizeSet((m_x3 << 1) - 1, (m_y3 << 1) - 1,
        (m_z3 << 1) + 1, m_w3))
        return fFalse;
      BitmapOn();
      for (z = 1; z < w; z++)
        for (y = 1; y < w; y++)
          for (x = 1; x < w; x++)
            if (!b2.Get3(x, y, z-1)) {
              Set30(z, y, x-1);          // 1 transpose xz
              Set30(m_x3-1-z, y, x-1);   // 8 transpose xz flipx
              Set30(w-z, w+y, x-1);      // 4 transpose xz flipx
              Set30(w+z, w+y, x-1);      // 5 transpose xz
              Set30(y, x, w+z-1);        // 2 transpose xy
              Set30(m_x3-1-y, x, w+z-1); // 7 transpose xy flipx
              Set30(x, w+y, w+z-1);      // 3 normal
              Set30(w+x, w+y, w+z-1);    // 6 normal
            }
      Set30(1, 1, w-1); Set30(m_x3-2, 1, w-1);
      Set30(1, w, w); Set30(m_x3-2, w, w);
      Set30(w-1, w+1, w-1); Set30(w+1, w+1, w-1);
      Set30(w, w+1, 0);
      UpdateDisplay();
    }
    Set30(1, 0, 0); Set30(m_x3-2, 0, 0);
  }
  return fTrue;
}


// Draw a tilt style Maze pattern in the bitmap, or a design formed by a grid
// of diagonal line segments, which randomly face one way or the other. This
// results in a bunch of unicursal passages, which either start and end on the
// edges, or are isolated passage loops within the pattern.

void CMaz::CreateMazeTilt()
{
  int x, y, d, n = Max(NAbs(ms.nTiltSize), 1), f = ms.nTiltSize > 0;

  BitmapOff();
  for (y = yl; y <= yh-n+f; y += n)
    for (x = xl; x <= xh-n+f; x += n) {
      d = Rnd(0, 1);
      if (ms.fTiltDiamond && d && x > xl && y > yl &&
        Get(x-1, y-n) && Get(x, y-n) && Get(x-n, y))
        d = fFalse;
      LineZ(x + (d^1)*(n-f), y+n-f, y, d*2-1, fOn);
    }
}


// Create a subsection of a virtual Labyrinth of the classical style, in which
// only part of the Labyrinth is rendered.

void CMaz::LabyrinthClassicalPartial(int zs, int m1, int n1)
{
  int z2, z4, z8, m2, n2, x, y, x1, y1, x2, y2, i;

  BitmapOff();
  zs = (((zs << 2) + 3) << 1) + 1;
  z2 = (zs + 1) >> 1; z4 = (zs + 1) >> 2; z8 = (zs + 1) >> 3;
  m2 = m1 + (m_x >> 1); n2 = n1 + (m_y >> 1);
  for (i = 0; i <= 1; i++) {

    // Draw horizontal lines
    for (y = n1; y <= n2; y++) {
      if (!FBetween(y, 0, zs))
        continue;
      x1 = x2 = 0;
      if (!i) {
        if (y >= z2) {
          x1 = zs - y; x2 = y;
        } else if (y < z2-z8) {
          x1 = y; x2 = z2;
          if (y < z4)
            x2 -= (z8 - NAbs(z8 - y) + 1);
          else
            x2 -= (y - z4);
        }
      } else {
        if (y < z2-z8) {
          x1 = z2; x2 = zs - y;
          if (y < z4)
            x1 += (z8 - NAbs(z8 - y) - (y >= z8)*2);
          else
            x1 += (y - z4 + 1);
        }
      }
      x1 = NMax(x1 - m1, 0); x2 = NMin(x2 - m1, m_x);
      if (x1 < x2)
        LineX(x1 << 1, x2 << 1, (y - n1) << 1, fOn);
    }

    // Draw vertical lines
    for (x = m1; x <= m2; x++) {
      if (!FBetween(x, 0, zs))
        continue;
      y1 = y2 = 0;
      if (!i) {
        if (x >= z2) {
          y1 = zs - x; y2 = x;
        } else {
          y1 = x; y2 = zs - x;
        }
        if (x >= z2-z8 && x <= z2+z8)
          y1 -= (z8 - NAbs(z2 - x)) << 1;
      } else {
        if (x >= z2-z8 && x <= z2+z8) {
          y1 = NAbs(z2 - x) - (x < z2);
          y2 = z4 - NAbs(z2 - x) + 1 - (x >= z2)*3;
        }
      }
      y1 = NMax(y1 - n1, 0); y2 = NMin(y2 - n1, m_y);
      if (y1 < y2)
        LineY((x - m1) << 1, y1 << 1, y2 << 1, fOn);
    }
  }
}


#define iStackMax (32 * 4)
#define VX2(x) (((x) < m1 ? -1 : ((x) > m2 ? m_x : (x)-m1))*2)
#define VY2(y) (((y) < n1 ? -1 : ((y) > n2 ? m_y : (y)-n1))*2)
#define SetRC2(m1, n1, m2, n2) \
  rgrc[id].x1 = m1; rgrc[id].y1 = n1; rgrc[id].x2 = m2; rgrc[id].y2 = n2;

// Create a subsection of a virtual Labyrinth of the Hilbert curve style, in
// which only part of the Labyrinth is rendered.

void CMaz::LabyrinthHilbertPartial(int zFactor, int m1, int n1)
{
  int rgd[iStackMax], id = 0, zs, m2, n2, x1, y1, x2, y2, x12, y12, d;
  RC rgrc[iStackMax], rc;

  zs = 1 << zFactor;
  m2 = m1 + (m_x >> 1); n2 = n1 + (m_y >> 1);
  BitmapOff();

  SetRC2(0, 0, zs, zs);
  rgd[0] = 0;

  // Do Edge(-m1*2, -n1*2, (zs-m1)*2, (zs-n1)*2) without overflowing 32 bits.
  if (FBetween(0,  n1, n2)) LineX(VX2(0),  VX2(zs), VY2(0),  fOn);
  if (FBetween(0,  m1, m2)) LineY(VX2(0),  VY2(0),  VY2(zs), fOn);
  if (FBetween(zs, n1, n2)) LineX(VX2(0),  VX2(zs), VY2(zs), fOn);
  if (FBetween(zs, m1, m2)) LineY(VX2(zs), VY2(0),  VY2(zs), fOn);
  Set0(1-m1*2, -n1*2); Set0((zs-m1)*2-1, -n1*2);

  // Continue while there's at least one square on the stack.
  while (id >= 0) {
    rc = rgrc[id];
    if (rc.x2 - rc.x1 < 2 || rc.y2 - rc.y1 < 2 ||
      rc.x1 > m2 || rc.x2 < m1 || rc.y1 > n2 || rc.y2 < n1 ||
      id >= iStackMax) {
      // If square is too small, or is outside visible viewport, skip it.
      id--;
      continue;
    }
    if (fCellMax)
      break;

    // Draw lines quadsecting the square into four subsquares.
    x1 = rc.x1; y1 = rc.y1; x2 = rc.x2; y2 = rc.y2;
    d = rgd[id];
    x12 = (x1 + x2) >> 1; y12 = (y1 + y2) >> 1;
    if (!FOdd(d)) {
      LineX(VX2(x1 + 1), VX2(x2 - 1), VY2(y12), fOn);
      LineY(VX2(x12), d < 1 ? VY2(y1) + 1 : VY2(y2) - 1, VY2(y12), fOn);
      if (y2 - y1 > 2)
        LineY(VX2(x12), VY2(y12 + (d < 1 ? 1 : -1)), d < 1 ? VY2(y2) : VY2(y1),
          fOn);
    } else {
      LineY(VX2(x12), VY2(y1 + 1), VY2(y2 - 1), fOn);
      LineX(d < 2 ? VX2(x1) + 1 : VX2(x2) - 1, VX2(x12), VY2(y12), fOn);
      if (x2 - x1 > 2)
        LineX(VX2(x12 + (d < 2 ? 1 : -1)), d < 2 ? VX2(x2) : VX2(x1), VY2(y12),
          fOn);
    }

    // Divide current square's coordinates and push four subsquares on stack.
    SetRC2(x1, y1, x12, y12);
    rgd[id++] = d + (d <= 0) - (d == 1) & DIRS1;
    SetRC2(x12, y1, x2, y12);
    rgd[id++] = d + (d >= 3) - (d <= 0) & DIRS1;
    SetRC2(x1, y12, x12, y2);
    rgd[id++] = d + (d == 1) - (d == 2) & DIRS1;
    SetRC2(x12, y12, x2, y2);
    rgd[id]   = d + (d == 2) - (d >= 3) & DIRS1;
  }
}


#define iStackMax3 (32 * 8)
#define VX3(x) (((x) < m1 ? -1 : ((x) > m2 ? m_x3 : (x)-m1))*2)
#define VY3(y) (((y) < n1 ? -1 : ((y) > n2 ? m_y3 : (y)-n1))*2)
#define VZ3(z) (((z) < o1 ? -1 : ((z) > o2 ? m_z3 : (z)-o1))*2 - 1)
#define SetRC3(m1, n1, o1, m2, n2, o2) \
  rgrc[id].x1 = m1; rgrc[id].y1 = n1; rgrc[id].z1 = o1; \
  rgrc[id].x2 = m2; rgrc[id].y2 = n2; rgrc[id].z2 = o2;
#define SetRGD(xx, xy, xz, yx, yy, yz, zx, zy, zz) \
  rgd[id][0] = xx; rgd[id][1] = xy; rgd[id][2] = xz; \
  rgd[id][3] = yx; rgd[id][4] = yy; rgd[id][5] = yz; \
  rgd[id][6] = zx; rgd[id][7] = zy; rgd[id][8] = zz;

// Create a subsection of a virtual 3D Labyrinth of the 3D Hilbert curve style,
// in which only part of the Labyrinth is rendered.

void CMaz::LabyrinthHilbertPartial3D(int zFactor, int m1, int n1, int o1)
{
  int rgd[iStackMax3][9], id = 0, zs, m2, n2, o2, x1, y1, z1, x2, y2, z2,
    x12, y12, z12, i, dw;
  int xx, xy, xz, yx, yy, yz, zx, zy, zz;
  RC3 rgrc[iStackMax3], rc;

  zs = 1 << zFactor;
  m2 = m1 + (m_x3 >> 1); n2 = n1 + (m_y3 >> 1); o2 = o1 + (m_z3 >> 1);
  BitmapOff();

  SetRC3(0, 0, 0, zs, zs, zs);
  SetRGD(1, 0, 0, 0, 1, 0, 0, 0, 1);

  // Draw boundary faces without overflowing 32 bits.
  if (FBetween(0,  o1, o2))
    for (i = -o1*2-1; i <= -o1*2+1; i += 2)
      CubeBlock(0, 0, i, m_x3-1, m_y3-1, i, fOn);
  if (FBetween(zs, o1, o2))
    CubeBlock(0, 0, (zs-o1)*2-1, m_x3-1, m_y3-1, (zs-o1)*2-1, fOn);
  if (FBetween(0,  n1, n2)) CubeBlock(VX3(0), VY3(0), VZ3(0),
    VX3(zs), VY3(0), VZ3(zs), fOn);
  if (FBetween(0,  m1, m2)) CubeBlock(VX3(0), VY3(0), VZ3(0),
    VX3(0), VY3(zs), VZ3(zs), fOn);
  if (FBetween(zs, n1, n2)) CubeBlock(VX3(0), VY3(zs), VZ3(0),
    VX3(zs), VY3(zs), VZ3(zs), fOn);
  if (FBetween(zs, m1, m2)) CubeBlock(VX3(zs), VY3(0), VZ3(0),
    VX3(zs), VY3(zs), VZ3(zs), fOn);
  Set30(1-m1*2, -n1*2, -o1*2); Set30((zs-m1)*2-1, -n1*2, -o1*2);

  // Continue while there's at least one cube on the stack.
  while (id >= 0) {
    rc = rgrc[id];
    SortN((int *)&rc.x1, (int *)&rc.x2);
    SortN((int *)&rc.y1, (int *)&rc.y2);
    SortN((int *)&rc.z1, (int *)&rc.z2);
    if (rc.x2 - rc.x1 < 2 || rc.y2 - rc.y1 < 2 || rc.z2 - rc.z1 < 2 ||
      rc.x1 > m2 || rc.x2 < m1 || rc.y1 > n2 || rc.y2 < n1 ||
      rc.z1 > o2 || rc.z2 < o1 || id >= iStackMax3) {
      // If cube is too small, or is outside visible viewport, skip it.
      id--;
      continue;
    }
    if (fCellMax)
      break;

    // Draw planes dividing the cube into eight subcubes.
    x1 = rc.x1; y1 = rc.y1; z1 = rc.z1; x2 = rc.x2; y2 = rc.y2; z2 = rc.z2;
    xx = rgd[id][0]; xy = rgd[id][1]; xz = rgd[id][2];
    yx = rgd[id][3]; yy = rgd[id][4]; yz = rgd[id][5];
    zx = rgd[id][6]; zy = rgd[id][7]; zz = rgd[id][8];
    x12 = (x1 + x2) >> 1; y12 = (y1 + y2) >> 1; z12 = (z1 + z2) >> 1;
    dw = (x2 - x1) >> 1;
    CubeBlock(VX3(x12), VY3(y1), VZ3(z1),
      VX3(x12), VY3(y2), VZ3(z2), fOn);
    CubeBlock(VX3(x1), VY3(y12), VZ3(z1),
      VX3(x2), VY3(y12), VZ3(z2), fOn);
    CubeBlock(VX3(x1), VY3(y1), VZ3(z12),
      VX3(x2), VY3(y2), VZ3(z12), fOn);
    for (i = -1; i <= 1; i += 2) {
      //Set30(VX3(x12-i*dw)+i, VY3(y12-dw)+1, VZ3(z12));
      Set30(VX3(x12-(xx*i+xy)*dw)+xx*i+xy, VY3(y12-(yx*i+yy)*dw)+yx*i+yy,
        VZ3(z12-(zx*i+zy)*dw)+zx*i+zy);
      //Set30(VX3(x12-i*dw)+i, VY3(y12), VZ3(z12)+1);
      Set30(VX3(x12-xx*i*dw)+xx*i+xz, VY3(y12-yx*i*dw)+yx*i+yz,
        VZ3(z12-zx*i*dw)+zx*i+zz);
      //Set30(VX3(x12)-i, VY3(y12)+1, VZ3(z12));
      Set30(VX3(x12)-xx*i+xy, VY3(y12)-yx*i+yy, VZ3(z12)-zx*i+zy);
    }
    //Set30(VX3(x12), VY3(y12)+1, VZ3(z12-dw)+1);
    Set30(VX3(x12-xz*dw)+xy+xz, VY3(y12-yz*dw)+yy+yz, VZ3(z12-zz*dw)+zy+zz);

    // Divide current cube's coordinates and push eight subcubes on stack.
    for (i = -1; i <= 1; i += 2) {
      //SetRC3(x12, y12, z12, x12+i*dw, y12-dw, z12-dw);
      SetRC3(x12, y12, z12,
        x12+(xx*i-xy-xz)*dw, y12+(yx*i-yy-yz)*dw, z12+(zx*i-zy-zz)*dw);
      SetRGD(xz, xy, xx*-i, yz, yy, yx*-i, zz, zy, zx*-i); // transpose xz
      id++;
      //SetRC3(x12, y12, z12, x12+i*dw, y12-dw, z12+dw);
      SetRC3(x12, y12, z12,
        x12+(xx*i-xy+xz)*dw, y12+(yx*i-yy+yz)*dw, z12+(zx*i-zy+zz)*dw);
      SetRGD(xy, xx*-i, xz, yy, yx*-i, yz, zy, zx*-i, zz); // transpose xy
      id++;
      //SetRC3(x12, y12, z12, x12+i*dw, y12+dw, z12+dw);
      SetRC3(x12, y12, z12,
        x12+(xx*i+xy+xz)*dw, y12+(yx*i+yy+yz)*dw, z12+(zx*i+zy+zz)*dw);
      SetRGD(xx*-i, xy, xz, yx*-i, yy, yz, zx*-i, zy, zz); // normal
      id++;
      //SetRC3(x12, y12, z12, x12+i*dw, y12+dw, z12-dw);
      SetRC3(x12, y12, z12,
        x12+(xx*i+xy-xz)*dw, y12+(yx*i+yy-yz)*dw, z12+(zx*i+zy-zz)*dw);
      SetRGD(xz, xy, -xx*-i, yz, yy, -yx*-i, zz, zy, -zx*-i); // trans xz flipx
      id++;
    }
    id--;
  }
}


// Center a grid of cells within a bitmap. Return the largest pixel size of a
// cell able to fit within the grid, along with the pixel coordinates of the
// upper left corner of the grid.

flag CMaz::FCenterCells(int xn, int yn, int *px0, int *py0, int *pzs)
{
  char sz[cchSzMax];
  int x0, y0, xs, ys, zs, x, y;

  // If bitmap is too small to fit 2x2 pixel cells, resize it large enough.
  xs = xn*2 + 1; ys = yn*2 + 1;
  x = Max(m_x, xs); y = Max(m_y, ys);
  if (!FBitmapSizeSet(x, y))
    return fFalse;

  // Compute pixel size of cells, and upper left corner of grid.
  xs = (m_x - 1) / xn; ys = (m_y - 1) / yn;
  zs = Min(xs, ys);
  x0 = (m_x - zs * xn) >> 1; y0 = (m_y - zs * yn) >> 1;
  *px0 = x0; *py0 = y0; *pzs = zs;

  // Also clear the bitmap and prepare for drawing on it.
  BitmapOff();
  sprintf(S(sz), "_BM%d,%dS%dO%d", x0, y0, zs, ms.lcs < lcsSquare);
  Turtle(sz);
  return fTrue;
}


// Draw one of several different styles of unicursal Maze or Labyrinth
// patterns in the bitmap.

flag CMaz::CreateLabyrinth()
{
  char sz[cchSzMax];
  int nLaby = ms.nLabyrinth, c = ms.nClassical,
    x0, y0, zs, xn, yn, x1, y1, x, y, x2, y2, i, j, k, l, m, n;

  if (nLaby == ltClassical && ms.lcs == lcsSquare && ms.lcc == lccMerged)
    nLaby = ltPartial;
  else if (nLaby == ltClassical && (ms.lcs == lcsRound || ms.lcs == lcsSquare))
    nLaby = ltClassicalF;
  else if ((nLaby == ltChartres || nLaby == ltChartresR) && ms.lcs == lcsFlat)
    nLaby = ltChartresF;

  switch (nLaby) {
  case ltClassical:  xn = 14 + c*16; yn = 13 + c*14; break; // Classical
  case ltChartres:   xn = yn = 28;                   break; // Chartres
  case ltCretan:     xn = yn = 58;                   break; // Cretan
  case ltManInMaze:  xn = yn = 16 + c*2 + c*8;       break; // Man in the Maze
  case ltClassicalF: xn = 30; yn = 27;               break; // Flat Classical
  case ltChartresF:  xn = 18; yn = 11;               break; // Flat Chartres
  case ltChartresR:  return CreateLabyrinthChartres();
  case ltCustom:     return CreateLabyrinthCustom();
  case ltPartial:    LabyrinthClassicalPartial(ms.nClassical, 0, 0);
    return fTrue;
  default:
    Assert(fFalse);
  }

  if (!FCenterCells(xn, yn, &x0, &y0, &zs))
    return fFalse;
  switch (nLaby) {

  case ltClassical:  // Classical seven circuit Labyrinth.
    if (c == 1) {
      Turtle(
        "BR10BD15OA5OC5OE2L2U2OHOFD8OD4OB12OH15OF15OD10OB2U2L2OG6OA11OC11OE8"
        "BU8OH3OF3D2L2OG2OA7OC7OE4L8OG4OA9OC9OE6L2D2OD2OB10OH13OF13OD8");
      ms.xEntrance = x0 + zs*13; ms.yEntrance = y0 + zs*24;
    } else {
      j = (c + 1) * 2;
      sprintf(S(sz), "BR%dBD%dNU%dNL%dND%dNR%d",
        6 + c*8, 9 + c*10, j, j, j, j);
      Turtle(sz);
      for (i = 1; i <= c; i++) {
        j = i*2; k = (c + 1) * 2 - j;
        sprintf(S(sz), "BE%dNU%dNR%dBD%dNR%dND%dBL%dND%dNL%dBU%dNL%dNU%dBF%d",
          k, j, j, k*2, j, j, k*2, j, j, k*2, j, j, k); Turtle(sz);
      }
      sprintf(S(sz), "BRBU%d", (c + 1) * 2); Turtle(sz);
      for (i = 0; i < 4 + c*4; i++) {
        j = i*2 + 1;
        sprintf(S(sz), "BL%dOA%dOC%dBL%d", j, j, j, j); Turtle(sz);
      }
      sprintf(S(sz), "BR%d", 1 + c*2); Turtle(sz);
      for (i = 0; i < 3 + c*3; i++) {
        j = i*2 + 2;
        sprintf(S(sz), "BR%dOE%dBU%d", j, j, j); Turtle(sz);
      }
      sprintf(S(sz), "BL%d", 4 + c*4); Turtle(sz);
      for (i = 0; i < 2 + c*3; i++) {
        j = i*2 + 2;
        sprintf(S(sz), "BD%dOG%dBR%d", j, j, j); Turtle(sz);
      }
      sprintf(S(sz), "BF%d", 4 + c*4); Turtle(sz);
      for (i = 0; i < 1 + c; i++) {
        j = i*2 + 2;
        sprintf(S(sz), "BD%dOG%dBR%d", j, j, j); Turtle(sz);
      }
      sprintf(S(sz), "BL%d", 4 + c*4); Turtle(sz);
      for (i = 0; i < c; i++) {
        j = i*2 + 2;
        sprintf(S(sz), "BR%dOE%dBU%d", j, j, j); Turtle(sz);
      }
      ms.xEntrance = x0 + zs*(5 + c*8); ms.yEntrance = y0 + zs*(11 + c*13);
    }
    break;

  case ltChartres:  // Chartres Cathedral Labyrinth.
    Turtle("BR14BD17ND11L2OGU4OAR2NU2R2OCD4OELND4BLBG");
    Turtle("ND4LOG2U4OA2RBR2ROC2D2NR2D2OE2BL4BD");
    Turtle("OG3U2NL2U2OA3R4OC3DBD2DOE3LBLBG");
    Turtle("LOG4UBU2UOA4R2NU2R2OC4D4OE4BL4BD");
    Turtle("OG5U4OA5RBR2ROC5D2NR2D2OE5LBLBG");
    Turtle("LOG6U2NL2U2OA6R4OC6DBD2DOE6L2BD");
    Turtle("L2OG7UBU2UOA7R2NU2R2OC7D4OE7LND4BLBG");
    Turtle("ND4LOG8U4OA8RBR2ROC8D2NR2D2OE8BL4BD");
    Turtle("OG9U2NL2U2OA9R4OC9DBD2DOE9LBLBG");
    Turtle("LOG10UBU2UOA10R2NU2R2OC10D4OE10BL4BD");
    Turtle("OG11U4OA11RBR2ROC11D4OE11LBLBG");
    Turtle("LOG12U4OA12R4OC12D4OE12L2");
    ms.xEntrance = x0 + (zs*27 >> 1); ms.yEntrance = y0 + zs*28;
    break;

  case ltCretan:  // Cretan Labyrinth.
    Turtle("BR28BD38ND4L6OG2U14OA2R14OC2D14OE2L6ND20BL4BD2");
    Turtle("L4OG4U7NL4U7OA4R14OC4D7NR4D7OE4L4BL4BD2");
    Turtle("L6OG6U5BU4U5OA6R7NU4R7OC6D5BD4D5OE6L6BD2");
    Turtle("L8OG8U14OA8R5BR4R5OC8D14OE8L4ND8BL4BD2");
    Turtle("ND12L6OG10U7NL4U7OA10R14OC10D7NR4D7OE10L2BL8BD2");
    Turtle("ND8L4OG12U5BU4U5OA12R7NU4R7OC12D5BD4D5OE12L4BL8BD2");
    Turtle("ND4L2OG14U14OA14R5BR4R5OC14D14OE14L2BL12BD2");
    Turtle("OG16U7NL4U7OA16R14OC16D7NR4D7OE16L4BL8BD2");
    Turtle("L2OG18U5BU4U5OA18R7NU4R7OC18D5BD4D5OE18L6BL4BD2");
    Turtle("L4OG20U14OA20R5BR4R5OC20D14OE20L4BL4BD2");
    Turtle("L6OG22U14OA22R14OC22D14OE22L6");
    ms.xEntrance = x0 + zs*29; ms.yEntrance = y0 + zs*58;
    break;

  case ltManInMaze:  // Man in the Maze Labyrinth.
    l = 8 + c*8; m = (64 / l + ((64 % l) > 0)) << 1;
    if (m < 1 || ms.lcs >= lcsSquare)
      m = 1;
    if (FOdd(m))
      m++;
    n = l * m;
    x1 = x0 + (xn >> 1) * zs; y1 = y0 + (yn >> 1) * zs;
    j = zs*(1+c);
    ArcPolygon(x1, y1, j, j, 1, n+1, ms.lcs < lcsFlat ? 2 : n/4, n, fOn);
    for (i = 0; i < c; i++)
      for (j = 0; j < 4; j++) {
        ArcPolygon(x1, y1, zs*(2+c + i), zs*(2+c + i),
          n/8 + m/2 - m*(c-i) + j*(n/4), n/8 + m/2 + m*(c-i) + j*(n/4),
          ms.lcs < lcsFlat ? 2 : m*(c-i)*2, n, fOn);
      }
    for (i = 0; i < 4 + c*4; i++) {
      j = m/2 + i*m;
      ArcPolygon(x1, y1, zs*(5+c + i), zs*(5+c + i),
        n/4 - j, n/4 + j, 2, n, fOn);
    }
    for (i = 0; i < l; i++) {
      j = i*m - n/4 + m/2;
      k = i % (l/8);
      if (i / (l/8) & 1)
        k = l/8 - k;
      k = zs*(1+c + k);
      AngleR(&x, &y, x1, y1, (real)k, (real)k, (real)j * rDegMax / (real)n);
      k = zs*(5+c + NAbs(i - l/2) - (i < l/2));
      AngleR(&x2, &y2, x1, y1, (real)k, (real)k, (real)j * rDegMax / (real)n);
      Line(x, y, x2, y2, fOn);
    }
    ms.xEntrance = x1; ms.yEntrance = y1 - zs*(8 + c*5);
    break;

  case ltClassicalF:  // Flat Classical seven circuit Labyrinth.
    Turtle(
      "BR10BD17U2OA5OC5D2L4U2OHOFD12R16U12OH15OF15D12R12U4L8U8OA11OC11D8L8");
    Turtle(
      "BU8OH3OF3D4L4U4OA7OC7D4L8BD2L8U6OA9OC9D6L8D4R12U10OH13OF13D10R8");
    ms.xEntrance = x0 + zs*13; ms.yEntrance = y0 + zs*27;
    break;

  case ltChartresF:  // Flat Chartres Cathedral Labyrinth.
    Turtle("D11BEU4BU2U4BR4BDD2BDD2BDD2BR4ND2BUU2BUU2BUU2");
    Turtle("BR4BD2D2BDD2BDD2BR4BDU4BU2U4BED11");
    Turtle("BLL17BER7BR2R6BEL3BL2L10BHR3BR2R10BEL7BL2L6BHR11BR2R4");
    Turtle("BHL11BL2L4BER7BR2R6BEL3BL2L10BHR3BR2R10BEL7BL2L6BHR17");
    ms.xEntrance = x0 + (zs >> 1); ms.yEntrance = y0;
    break;
  }
  return fTrue;
}


// Draw the Chartres Cathedral Labyrinth pattern, in the right proportions and
// with extra decorations making it a true replica of the Chartres Cathedral
// Labyrinth. Draws a more accurate version than in CreateLabyrinth().

flag CMaz::CreateLabyrinthChartres()
{
  int x0, y0, xs, ys, zs, zn, i, j, e1, e2;
  real r, d1, d2, r1, r2, d, rc, x, y, x1, y1, x2, y2, x3, y3, x4, y4;

  zn = 62; xs = ys = zn * 2;
  x0 = Max(m_x, xs); y0 = Max(m_y, ys);
  if (!FBitmapSizeSet(x0, y0))
    return fFalse;
  xs = m_x / zn; ys = m_y / zn; zs = Min(xs, ys);
  x0 = m_x >> 1; y0 = m_y >> 1;
  BitmapOff();

  // Draw concentric rings.

  for (i = 0; i < 12; i++)
    for (j = 0; j < 4; j++) {
      e1 = e2 = 0;
      switch (j) {
      case 0:
        e1++;
        if (i > 0 && i != 6)
          e1 += (2 - (i & 1)) << 1;
        if (i > 0 && i % 3 == 0)
          e2 += 2;
        break;
      case 1:
        if (i > 0 && i % 3 == 0)
          e1 += 2;
        if ((i + 2) % 3 == 0)
          e2 += 2;
        break;
      case 2:
        if ((i + 2) % 3 == 0)
          e1 += 2;
        if (i < 11 && (i + 1) % 3 == 0)
          e2 += 2;
        break;
      case 3:
        e2--;
        if (i < 11 && (i + 1) % 3 == 0)
          e1 += 2;
        if (i != 5 && i < 11)
          e2 += (1 + (i & 1)) << 1;
        break;
      }
      r = (real)((7 + i*2) * zs);
      d1 = ((real)j / 4.0 + 0.25) * rDegMax;
      if (d1 >= rDegMax)
        d1 -= rDegMax;
      d2 = d1 + rDegQuad;
      x = (real)(zs * e1); y = (real)(zs * e2);
      if (e1 > (j == 0) << 2)
        x += (real)zs / rPhi;
      if (e2 > (int)(j == 3))
        y += (real)zs / rPhi;
      d1 += RAsinD(x / r); d2 -= RAsinD(y / r);
      ArcReal((real)x0, (real)y0, r, d1, d2, fOn);
      if (ms.lcs >= lcsSquare)
        continue;

      // Draw curves around U-turns and corners.
      if ((e1 > (j == 0) << 2) || (j == 0 && (i == 1 || i == 5 || i == 7))) {
        PlotAngle(x1, y1, x0, y0, r, d1);
        d = d1 - rDegHalf;
        if (j == 0) {
          if (i == 5)
            d = 0.0;
          else if (i == 1 || i == 7)
            d1 = 0.0;
        }
        ArcReal(x1, y1, (real)(zs*2), d, d1, fOn);
      }
      if ((e2 > (int)(j == 3)) || (j == 3 && (i == 4 || i == 6 || i == 10))) {
        PlotAngle(x1, y1, x0, y0, r, d2);
        d = d2 + rDegHalf;
        if (j == 3) {
          if (i == 6)
            d2 = rDegHalf;
          else if (i == 4 || i == 10)
            d = rDegHalf;
        }
        ArcReal(x1, y1, (real)(zs*2), d2, d, fOn);
      }
    }

  // Draw straight lines.

  for (i = 0; i < 3; i++) {
    LineX(x0 - (27 - i*6)*zs, x0 - (23 - i*6)*zs, y0, fOn);
    LineX(x0 + (9 + i*6)*zs, x0 + (13 + i*6)*zs, y0, fOn);
  }
  for (i = 0; i < 4; i++)
    LineY(x0, y0 - (29 - i*6)*zs, y0 - (25 - i*6)*zs, fOn);
  LineY(x0 - zs, y0 + NSqr(Sq(7*zs)-Sq(zs)),
    y0 + NSqr(Sq(29*zs)-Sq(zs)), fOn);
  for (i = 9; i <= 21; i += 12)
    LineY(x0 - zs*3, y0 + NSqr(Sq(i*zs)-Sq(zs*3)),
      y0 + NSqr(Sq((i + 8)*zs)-Sq(zs*3)), fOn);
  for (i = 7; i <= 19; i += 12)
    LineY(x0 + zs, y0 + NSqr(Sq(i*zs)-Sq(zs)),
      y0 + NSqr(Sq((i + 8)*zs)-Sq(zs)), fOn);
  if (ms.lcs >= lcsRound)
    return fTrue;

  // Draw lunations around the circumference.

  r1 = (real)(29*zs) + 1.0;
  r2 = (real)zs * 0.67;
  for (i = 1; i <= 114; i++) {
    d = rDegQuad + ((real)i  - 0.5) * rDegMax / 114.0 + RAsinD(2.0 / 29.0);
    if (d > rDegMax)
      d -= rDegMax;
    PlotAngle(x1, y1, x0, y0, r1+r2, d);
    d1 = d + 60.0; d2 = d + 300.0;
    if (i == 1)
      d2 = rDegHalf + RAcosD((x1 - (x0 - (real)(zs*3))) / r2);
    else if (i == 114)
      d1 = rDegMax - RAcosD(((x0 - (real)zs) - x1) / r2);
    ArcReal(x1, y1, r2, d1, d2, fOn);
    PlotAngle(x, y, x1, y1, r2, d2);
    if (i > 1)
      Line((int)x2, (int)y2, (int)x, (int)y, fOn);
    PlotAngle(x2, y2, x1, y1, r2, d1);
    if (i == 1)
      LineY(x0 - zs*3, y0 + zs*28, (int)y, fOn);
    else if (i == 114)
      LineY(x0 - zs, y0 + zs*28, (int)y2, fOn);
  }

  // Draw rosette in the center.

  d = rDegMax / 13.0;
  r1 = (real)(7*zs);
  d1 = r1 * RTanD(d);
  r2 = RSqr(Sq(r1) + Sq(d1));
  d2 = r2 - d1;
  rc = d2 * RTanD(d);
  r = r1 - rc;
  r2 = (real)zs * 0.33;
  for (i = -5; i <= 5; i += 2) {
    d1 = rDeg34 + (real)i / 13.0 * rDegMax;
    PlotAngle(x1, y1, x0, y0, r, d1);
    d2 = d1 - rDegQuad - d;
    ArcReal(x1, y1, rc, d2, d1 + rDegQuad + d, fOn);
    PlotAngle(x3, y3, x1, y1, rc, d2);
    if (i == -5) {
      LineY(x0 - zs, (int)(y3 - r2), y0 + 7*zs, fOn);
      LineY(x0 + zs, (int)(y3 - r2), y0 + 7*zs, fOn);
      LineX(x0 - zs - (int)r2, x0 - zs, (int)y3, fOn);
      LineX(x0 + zs, x0 + zs + (int)r2, (int)y3, fOn);
    } else {
      d2 = d1 - d;
      for (j = 90; j <= nDegMax; j += 90) {
        PlotAngle(x4, y4, x3, y3, r2, d2 + (real)j);
        Line((int)x3, (int)y3, (int)x4, (int)y4, fOn);
      }
    }
  }
  ms.xEntrance = x0 - zs*2; ms.yEntrance = y0 + zs*30;
  return fTrue;
}


CONST char *szClassical = "3214765";

// Copy a range of characters to a string that's valid for the Custom Paths
// Labyrinth field, translating if needed.

flag FCopyRgchToSzCustom(CONST char *pch, int cch, char *sz)
{
  int n, ich, nT, nMode, iBase = 0;
  char ch;

  // Simple strings that don't start with "=" are directly copied
  if (pch[0] != '=') {
    if (sz != pch)
      CopyRgchToSz(pch, cch, sz, cchSzDef);
    return fTrue;
  }

  // For complex strings, search for a sequence of macros to translate
  pch++;
  loop {

    // Translate "=Classical" to the classical 7 circuit Labyrinth
    // Translate "'X" to simple circuit sequence (offset to current position)
    // Translate "=X" to "123...X" for the first/simplest X circuit Labyrinth
    // Translate "=-X" to "X...321" for the last X circuit Labyrinth
    // Translate "=@X" to single spiral e.g. "7254361" for X circuit Labyrinth

    if (FCompareSzRgchI("Classical", pch, 9)) {
      nMode = 3;
      n = 7;
      pch += 9;
    } else if (*pch == '\'') {
      nMode = 2;
      for (n = 0, pch++; FDigitCh(*pch); n++, pch++)
        ;
    } else {
      switch (*pch) {
      case '-': nMode = -1; break;
      case '@': nMode = 1;  break;
      default:  nMode = 0;  break;
      }
      if (nMode != 0)
        pch++;
      n = atoi(pch);
      while (FDigitCh(*pch))
        pch++;
    }
    if ((n <= 0 && *pch != '0') || iBase + n > iCircuitMax)
      return fFalse;
    for (ich = 0; ich < n; ich++) {
      if (nMode == 3)
        nT = szClassical[ich] - '1';
      else if (nMode == 2)
        nT = pch[ich-n] - '1';
      else if (nMode == 0 || (nMode == 1 && FOdd(ich)))
        nT = ich;
      else {
        if (FOdd(n))
          nT = n-1 - ich;
        else
          nT = ich < n-1 ? n-2 - ich : n-1;
      }
      nT += iBase;
      ch = nT < 9 ? '1' + nT : 'A' + nT - 9;
      sz[iBase + ich] = ch;
    }
    iBase += n;
    if (*pch == chNull)
      break;
    if (*pch++ != ',')
      return fFalse;
  }
  sz[iBase] = chNull;
  return fTrue;
}


// Return whether a legal array of circuits forms a valid custom Labyrinth,
// where if rendered no paths would intersect each other.

flag FValidCustom(CONST int *rgn, int iMax, int *rgx, flag fUI)
{
  int i, j, i0, i1, j0, j1;
  flag f0, f1;
  char szT[cchSzMax];

  if (rgx != NULL)
    ClearPb(rgx, (iMax + 1) * sizeof(int));
  for (i = 0; i <= iMax; i++) {
    i0 = rgn[i]; i1 = rgn[i + 1];
    SortN(&i0, &i1);
    for (j = i & 1; j <= iMax; j += 2) {
      j0 = rgn[j]; j1 = rgn[j + 1];
      SortN(&j0, &j1);
      f0 = FBetween(i0, j0, j1); f1 = FBetween(i1, j0, j1);
      if (f0 != f1) {
        if (fUI) {
          sprintf(S(szT),
            "The Custom Paths string doesn't form a valid Labyrinth.\n"
            "Circuit %d at position %d intersects a previous circuit.\n"
            "Passage from %d to %d crosses passage from %d to %d.",
            j1, j+1, j0, j1, i0, i1);
          PrintSz_W(szT);
        }
        return fFalse;
      }
      if (rgx != NULL && f0 && f1)
        rgx[i]++;
    }
  }
  return fTrue;
}


// Return whether a string of circuits forms a legal and valid custom
// Labyrinth. Also compute information about the Labyrinth's circuits.

flag FValidSzCustom(CONST char *szCustom, int *rgnOut, int *rgxOut)
{
  int rgn[iCircuitMax + 2], iMax, isz, grf, i, j;
  flag fNumber = fFalse, f0, f1;
  char szT[cchSzMax], ch;

  // Ensure characters are valid (digits or letters)
  for (isz = 0; ch = szCustom[isz]; isz++) {
    ch = ChCap(ch);
    f0 = FBetween(ch, '1', '9'); f1 = FCapCh(ch);
    if (!f0 && !f1) {
      sprintf(S(szT),
        "The Custom Paths string has an invalid character.\n"
        "Circuit character %c at position %d is illegal.\n"
        "Characters need to be digits 1-9 or letters A-Z.", ch, isz+1);
      PrintSz_W(szT);
      return fFalse;
    }
    if (f0)
      fNumber = fTrue;
  }

  // Convert characters to circuit numbers
  for (isz = 0; ch = szCustom[isz]; isz++) {
    ch = ChCap(ch);
    if (FBetween(ch, '1', '9'))
      i = ch - '0';
    else
      i = fNumber ? ch - 'A' + 10 : ch - 'A' + 1;
    rgn[isz + 1] = i;
  }
  iMax = Min(isz, iCircuitMax);
  rgn[0] = 0; rgn[iMax + 1] = iMax + 1;

  // Ensure circuits are within range and unique
  grf = 0;
  for (i = 1; i <= iMax; i++) {
    j = rgn[i];
    if (!FBetween(j, 1, iMax)) {
      sprintf(S(szT),
        "The Custom Paths string has an out of range circuit.\n"
        "Circuit %d at position %d is out of range.\n"
        "Circuits need to range from 1 to %d.", j, i, iMax);
      PrintSz_W(szT);
      return fFalse;
    }
    if (grf & (1 << j)) {
      sprintf(S(szT),
        "The Custom Paths string has a duplicate circuit.\n"
        "Circuit %d at position %d already exists in the string.", j, i);
      PrintSz_W(szT);
      return fFalse;
    }
    if (FOdd(i ^ j)) {
      sprintf(S(szT),
        "The Custom Paths string has an unaligned circuit.\n"
        "Circuit %d at position %d has bad parity.\n"
        "Odd/even positions must be odd/even circuits.", j, i);
      PrintSz_W(szT);
      return fFalse;
    }
    grf |= (1 << j);
  }

  // Ensure circuits compose a valid Labyrinth
  if (!FValidCustom(rgn, iMax, rgxOut, fTrue))
    return fFalse;

  if (rgnOut != NULL)
    CopyRgb((CONST char *)rgn, (char *)rgnOut, (iMax + 2) * sizeof(int));
  return fTrue;
}


// Change a string for a custom Labyrinth to be the next valid (based on
// current settings) Labyrinth of the same number of circuits.

flag FComputeNextCustom(char *szCustom, flag fUI)
{
  int rgn[iCircuitMax + 2], rga[iCircuitMax], rgo[iCircuitMax],
    rgx[iCircuitMax + 1], iMax, iMaxT, i, j, k, c, grf, iLo, iHi;
  flag fNone = fFalse;

  if (!FValidSzCustom(szCustom, rgn, NULL))
    return fFalse;

  // Disassemble circuit
  iMax = CchSz(ms.szCustom);
  for (i = 1; i <= iMax; i++) {
    c = 0;
    for (j = 1; j < i; j++)
      if (rgn[i] < rgn[j])
        c++;
    rgo[i-1] = c;
  }
  CopyRgb((CONST char *)rgo, (char *)rga, iMax * sizeof(int));

  // Increment circuit
LIncrement:
  if (fNone) {
    if (fUI)
      PrintSz("No custom Labyrinths found under current settings.");
    return fFalse;
  }
  for (k = iMax-1; k >= 0; k--) {
LInc:
    rgo[k]++;
    if (rgo[k] <= k)
      break;
    rgo[k] = 0;
  }
  if (ms.fInfAutoStart && k < 0) {
    if (fUI)
      PrintSz("No more custom Labyrinths with current settings.");
    return fFalse;
  }
  for (i = 0; i < iMax; i++)
    if (rgo[i] != rga[i])
      break;
  if (i >= iMax)
    fNone = fTrue;

  // Assemble circuit
  grf = 0;
  for (i = iMax; i >= 1; i--) {
    c = rgo[i-1];
    for (j = iMax; j >= 1; j--)
      if ((grf & (1 << j)) == 0) {
        c--;
        if (c < 0)
          break;
      }
    rgn[i] = j;
    grf |= (1 << j);
  }
  rgn[0] = 0; rgn[iMax + 1] = iMax + 1;

  // Verify circuit (basic)
  for (iMaxT = k; iMaxT <= iMax; iMaxT++) {
    if (!FValidCustom(rgn, iMaxT, iMaxT >= iMax ? rgx : NULL, fFalse)) {
      if (iMaxT >= iMax)
        goto LIncrement;
      k = iMaxT;
      goto LInc;
    }
  }

  // Verify circuit (restrictions)
  if (ms.nCustomPartX < 2) {
    if (rgn[1] == 1 || rgn[iMax] == iMax)
      goto LIncrement;
    if (ms.nCustomPartX == 0) {
      iLo = iHi = grf = 0;
      for (i = 1; i <= iMax; i++) {
        j = rgn[i];
        grf |= (1 << j);
        if (j + 1 > iHi)
          iHi = j + 1;
        for (j = 1; j <= iMax; j++)
          if ((grf & (1 << j)) == 0)
            break;
        iLo = j-1;
        if (iHi - iLo <= 1 && i < iMax)
          goto LIncrement;
      }
    }
  }
  if (ms.nCustomPartY < 2) {
    if ((rgn[1] == iMax) + (rgn[iMax] == 1) > ms.nCustomPartY)
      goto LIncrement;
  }
  if (ms.nCustomFold < 2)
    for (i = 1; i < iMax; i++) {
      if (rgn[i+1] - rgn[i] == 1 &&
        rgn[i] > rgn[i-1] && rgn[i+2] > rgn[i+1]) {
        if (ms.nCustomFold == 0 ||
          rgn[i] - rgn[i-1] == 1 || rgn[i+2] - rgn[i+1] == 1)
          goto LIncrement;
      }
      if (rgn[i+1] - rgn[i] == -1 &&
        rgn[i] < rgn[i-1] && rgn[i+2] < rgn[i+1]) {
        if (ms.nCustomFold == 0 ||
          rgn[i] - rgn[i-1] == -1 || rgn[i+2] - rgn[i+1] == -1)
          goto LIncrement;
      }
    }
  if (ms.nCustomAsym < 2)
    for (i = 1, j = iMax; i < j; i++, j--) {
      c = rgn[i] + rgn[j] - 1;
      if (c != iMax) {
        if (ms.nCustomAsym == 0 || c < iMax)
          goto LIncrement;
        break;
      }
    }

  // Compose custom string
  for (i = 1; i <= iMax; i++) {
    j = rgn[i];
    szCustom[i-1] = j < 10 ? '1' + j - 1 : 'A' + j - 10;
  }
  return fTrue;
}


// Draw a simple unicursal Labyrinth based on a custom string of circuits.

flag CMaz::CreateLabyrinthCustom()
{
  int rgn[iCircuitMax + 2], rgx[iCircuitMax + 1], rgd[iCircuitMax + 1][2],
    rggrf[2], x0, y0, zs, zs2, iMax, dx, xn, yn, xc, yc, xc2, yc2, xi = 0,
    i, i0, i1, f, d, y1, y2, z, dz = 0, q;
  flag fMerged;
  char sz[cchSzMax];

  // Get data of Labyrinth to draw
  if (ms.fCustomAuto && !FComputeNextCustom(ms.szCustom, fTrue))
    return fFalse;
  if (!FValidSzCustom(ms.szCustom, rgn, rgx))
    return fFalse;
  iMax = CchSz(ms.szCustom);

  // Determine size of center
  if (ms.lcs == lcsRound || ms.lcs == lcsFlat) {
    for (dx = i = 0; i <= iMax; i++)
      if (rgx[i] > dx)
        dx = rgx[i];
    dx <<= 1;
    if (FOdd(iMax) ? (ms.lcc == lccEntrance || ms.lcc == lccExit ||
      ms.lcc == lccMerged) : (ms.lcc != lccRadius))
      dx++;
    if (ms.zCustomMiddle > 0)
      d = ms.zCustomMiddle;
    else
      d = iMax + (FOdd(iMax) ? 1 + (ms.lcc == lccEntrance ||
        ms.lcc == lccExit || ms.lcc == lccMerged) : (ms.lcc != lccRadius));
  } else {
    dx = 2;
    if (FOdd(iMax) && ms.lcc == lccEntrance)
      dx++;
    else if (FOdd(iMax) ? (ms.lcc == lccExit || ms.lcc == lccMerged) :
      (ms.lcc != lccRadius))
      dx--;
    if (ms.zCustomMiddle > 0)
      d = ms.zCustomMiddle;
    else
      d = 0;
  }
  if (dx < d)
    dx = d;
  if (ms.lcs != lcsFlat)
    xn = yn = (dx + ((iMax + dz) << 1)) << 1;
  else {
    xn = dx; yn = iMax;
  }
  if (!FCenterCells(xn, yn, &x0, &y0, &zs))
    return fFalse;

  // Determine offset of entrance/exit
  zs2 = zs << 1;
  xc = x0 + (xn >> 1)*zs; yc = y0 + (yn >> 1)*zs;
  if (FOdd(iMax)) {
    if (ms.lcc == lccEntrance)
      xi = 1;
    else if (ms.lcc == lccExit || ms.lcc == lccMerged)
      xi = -1;
  } else {
    if (ms.lcc != lccRadius)
      xi = 1;
  }
  xc2 = xc + xi*zs; yc2 = y0 + yn*zs;

  // Compute horizontal offsets of circuit walls near entrance/exit
  ClearPb(rgd, sizeof(rgd));
  for (i = 0; i < iMax; i++) {
    i0 = rgn[i]; i1 = rgn[i + 1];
    SortN(&i0, &i1);
    i0 = Max(i0-1, 0);
    f = FOdd(i);
    rgd[i0][f] = rgd[i1][f] = rgx[i]-1;
    if (i1 - i0 == 2)
      rgd[(i0 + i1) >> 1][f] = rgx[i];
  }
  rgd[0][0] = rgd[iMax][FOdd(iMax)] = 1;
  fMerged = ms.lcc == lccMerged && FOdd(iMax) && ms.lcs != lcsFlat &&
    rgn[1] < rgn[iMax];

  switch (ms.lcs) {
  case lcsCircle:

    // Draw vertical walls near entrance/exit
    rggrf[0] = rggrf[1] = 0;
    for (i = 0; i < iMax; i++) {
      i0 = rgn[i]; i1 = rgn[i + 1];
      SortN(&i0, &i1);
      i0 = Max(i0-1, 0);
      f = FOdd(i); d = (f << 1) - 1;
      z = (yn >> 1)*zs;
      q = fMerged && (rgn[i] <= (!f ? rgn[1] : rgn[iMax]-1));
      ArcQuadrantSub(xc, yc, z - i0*zs2, !f,
        (rgd[i0][f] + q*d)*zs2 + d*xi*zs, 0, NULL, &y1, fOn);
      ArcQuadrantSub(xc, yc, z - i1*zs2, !f,
        (rgd[i1][f] + q*d)*zs2 + d*xi*zs, 0, NULL, &y2, fOn);
      rggrf[f] |= ((1 << i0) | (1 << i1));
      if (i < 1 && FOdd(iMax) && ms.lcc != lccEntrance && !fMerged)
        y1 = z;
      LineY(xc2 + (d*(rgx[i] - 1) + q)*zs2, yc + y1, yc + y2, fOn);
    }

    // Draw rings around circuit
    if (fMerged)
      rggrf[1] &= ~(1 << (rgn[iMax]-1));
    for (i = 0; i <= iMax; i++) {
      z = ((yn >> 1) - (i << 1))*zs;
      for (f = 0; f <= 1; f++) {
        d = (f << 1) - 1;
        q = fMerged && (i <= rgn[1] || (f && i < rgn[iMax]-1));
        dx = (rgd[i][f] + q*d)*zs2 + xi*d*zs;
        if ((rggrf[f] & (1 << i)) == 0)
          ArcQuadrantSub(xc, yc, z, !f, dx, 0, NULL, NULL, fOn);
        ArcQuadrant(xc, yc, z, 2 + f, fOn);
        if (dx < 0)
          LineX(xc, xc - d*zs, yc + z, fOn);
      }
    }
    break;
  case lcsRound:
  case lcsSquare:

    // Draw vertical walls near entrance/exit
    for (i = 0; i < iMax; i++) {
      i0 = rgn[i]; i1 = rgn[i + 1];
      SortN(&i0, &i1);
      i0 = Max(i0-1, 0);
      f = FOdd(i); d = (f << 1) - 1;
      q = fMerged && (rgn[i] <= (!f ? rgn[1] : rgn[iMax]-1));
      LineY(xc2 + (d*(rgx[i] - 1) + q)*zs2, yc2 - i0*zs2, yc2 - i1*zs2, fOn);
    }

    // Draw horizontal walls near entrance/exit
    for (f = 0; f <= 1; f++) {
      d = (f << 1) - 1;
      for (i = 0; i <= iMax; i++) {
        q = fMerged && (i <= rgn[1] || (f && i < rgn[iMax]-1));
        LineX(xc2 + (d*rgd[i][f] + q)*zs2, xc + (dx+(ms.lcs == lcsRound ?
          0 : (iMax-i) << 1))*zs*d, yc2 - i*zs2, fOn);
      }
    }

    // Draw rest of circuit
    for (i = 0; i <= iMax; i++) {
      i0 = (iMax + dz - i) << 1; i1 = dx << 1;
      if (ms.lcs == lcsRound)
        sprintf(S(sz), "BM%d,%dOG%dU%dOA%dR%dOC%dD%dOE%d",
          x0 + (iMax + dz)*zs2, yc2 - i*zs2, i0, i1, i0, i1, i0, i1, i0);
      else {
        z = i0 + i1 + i0;
        sprintf(S(sz), "BM%d,%dU%dR%dD%d", x0 + i*zs2, yc2 - i*zs2, z, z, z);
      }
      Turtle(sz);
    }
    break;
  case lcsFlat:

    // Draw vertical walls
    for (i = 0; i < iMax; i++) {
      i0 = rgn[i]; i1 = rgn[i + 1];
      SortN(&i0, &i1);
      i0 = Max(i0-1, 0);
      z = rgx[i] - 1;
      if (z <= 0)
        continue;
      LineY(x0 + (FOdd(i) ? z : dx-z)*zs, yc2 - i0*zs, yc2 - i1*zs, fOn);
    }

    // Draw horizontal walls
    for (i = 0; i <= dx; i += dx)
      LineY(x0 + i*zs, yc2, yc2 - iMax*zs, fOn);
    for (i = 0; i <= iMax; i++)
      LineX(x0 + rgd[i][1]*zs, x0 + (dx - rgd[i][0])*zs,
        yc2 - i*zs, fOn);
    break;
  default:
    Assert(fFalse);
  }
  return fTrue;
}

/* labyrnth.cpp */
