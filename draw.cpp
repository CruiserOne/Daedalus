/*
** Daedalus (Version 3.4) File: draw.cpp
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
** This file contains advanced non-perspective graphics routines, unrelated to
** Mazes.
**
** Created: 6/15/1990.
** Last code change: 8/29/2023.
*/

#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "graphics.h"
#include "color.h"
#include "threed.h"
#include "draw.h"


DS ds = {
  // Draw settings
  10, 10, 10, 10, 10, 0, 0, 0, fFalse,
    0, 0, 0, 0, 10, 10, fTrue, fTrue,
  // Obscure draw settings
  fTrue, fFalse, fFalse, fTrue, fFalse, fFalse, 1, 1, 200.0, 20.0, 20.0, 20.0,
    kvBlack, kvBlue, kvYellow, kvGray, kvGreen, kvRed, {100.0, 75.0, 50.0},
  // Inside settings
  fFalse, 500, 0,
  // Macro accessible only settings
  10990099, -1, fFalse, fFalse, 0, 0, 0, 1000, fTrue, 0, fFalse,
  // Internal settings
  fFalse, xStart, 0.0, 0, NULL};


/*
******************************************************************************
** Render Overview
******************************************************************************
*/

// Draw some or all of the faces of a solid orthographic block, with one face
// oriented toward you. Used by various FillOverview functions.

void FillCube(CMap &b, int x, int y, int xs, int ys, int zs, int dir,
  KV kv0, KV kv1, real rx, real ry, real rz, flag fX, flag fY, flag fZ)
{
  int i;
  KV kvX, kvY, kvZ;

  // Draw the outline of the cube with one color.
  if (fX) {
    b.LineY(x + ys*dir, y - ys, y - ys + zs, kv0);
    b.LineZ(x + dir, y + zs - 1, y - ys + zs + 1, dir, kv0);
  }
  if (fY) {
    b.LineX(x - (xs - 1)*dir, x - dir, y + zs, kv0);
    b.LineY(x - xs*dir, y, y + zs, kv0);
  }
  if (fZ) {
    b.LineX(x - (xs - ys)*dir, x + (ys - 1)*dir, y - ys, kv0);
    b.LineZ(x - (xs - 1)*dir, y - 1, y - ys + 1, dir, kv0);
  }
  if (fX && fY)
    b.LineY(x, y, y + zs, kv0);
  if (fY && fZ)
    b.LineX(x - (xs - 1)*dir, x - dir, y, kv0);
  if (fX && fZ)
    b.LineZ(x + dir, y - 1, y - ys + 1, dir, kv0);

  // Fill in the three visible faces of the cube with another color. For color
  // bitmaps, each of the three faces gets shaded with a passed in value.
  if (!b.FColor()) {
    kvX = kv1; kvY = !ds.fShading; kvZ = !ds.fSkyShade;
  } else {
    kvX = KvShade(kv1, rx); kvY = KvShade(kv1, ry); kvZ = KvShade(kv1, rz);
  }
  if (fY && xs > 1 && zs > 1)
    b.Block(x - (xs - 1)*dir, y + 1, x - dir, y + zs - 1, kvY);
  for (i = 1; i < ys; i++) {
    if (fZ && xs > 1)
      b.LineX(x - (xs - 1 - i)*dir, x + (i - 1)*dir, y - i, kvZ);
    if (fX && zs > 1)
      b.LineY(x + i*dir, y - i + 1, y + zs - 1 - i, kvX);
  }
}


// Draw some or all of the faces of a solid angled orthographic block, with
// one edge oriented toward you. Used by various FillOverview2 functions.

void FillCube2(CMon &b, int x, int y, int xs, int ys, int zs,
  flag fX, flag fY, flag fZ)
{
  int xs2 = xs*ys, i, j;

  // Draw the outline of the cube.
  if (fX) {
    b.LineY(x + xs2 + 1, y - ys - 1, y - ys - 1 + zs, fOff);
    b.LineZ(x + 1, y + zs - 1, y + zs - ys, xs, fOff);
  }
  if (fY) {
    b.LineY(x - xs2 - 1, y - ys - 1, y - ys - 1 + zs, fOff);
    b.LineZ(x - 1, y + zs - 1, y + zs - ys, -xs, fOff);
  }
  if (fZ) {
    b.LineZ(x + xs2, y - ys - 2, y - ys*2 - 1, -xs, fOff);
    b.LineZ(x - xs2, y - ys - 2, y - ys*2 - 1, xs, fOff);
    b.Set0(x, y - ys*2 - 2);
  }
  if (fX && fY)
    b.LineY(x, y, y + zs, fOff);
  if (fY && fZ)
    b.LineZ(x - 1, y - 1, y - ys, -xs, fOff);
  if (fX && fZ)
    b.LineZ(x + 1, y - 1, y - ys, xs, fOff);

  // Fill in the three visible faces of the cube.
  if (fZ) {
    b.LineX(x - xs2, x + xs2, y - ys - 1, !ds.fSkyShade);
    for (i = 1; i <= ys; i++) {
      j = (ys - i)*xs;
      b.LineX(x - j, x + j, y - ys - 1 - i, !ds.fSkyShade);
      b.LineX(x - j, x + j, y - ys - 1 + i, !ds.fSkyShade);
    }
  }
  if ((fX || fY) && zs > 2)
    for (i = 0; i < ys; i++) {
      if (fX)
        b.Block(x + i*xs + 1, y - i, x + i*xs + xs, y - i + zs - 2, fOn);
      if (fY)
        b.Block(x - i*xs - 1, y - i, x - i*xs - xs, y - i + zs - 2,
          !ds.fShading);
    }
}


// Draw an orthographic overview of a monochrome bitmap within the bounds of a
// color bitmap. The color of each block comes from a second color bitmap.

void FillOverview(CMap &c, CONST CMon &b, CCol *c2)
{
  int x, y, horloc, verloc, dir;
  real rx = 0.0, ry = 0.0, rz = 0.0;
  KV kv, kvT;
  flag fColor = c.FColor();

  if (ds.depsiz < 1)
    return;
  if (fColor && ds.fShading) {
    rx = ds.vLight.m_x / 100.0;
    ry = ds.vLight.m_y / 100.0;
    rz = ds.vLight.m_z / 100.0;
  }
  dir = ds.fRight*2 - 1;
  for (y = 0; y < b.m_y; y++) {
    verloc = (y + 1)*ds.versiz;
    for (x = ds.fRight ? 0 : b.m_x - 1; x >= 0 && x < b.m_x; x += dir) {
      if (!b.Get(x, y))
        continue;
      if (ds.fRight)
        horloc = (x + 1)*ds.horsiz + (b.m_y - y - 1)*ds.versiz;
      else
        horloc = x*ds.horsiz + (y + 1)*ds.versiz;

      // Draw a colored block for each set pixel.
      kv = !fColor ? fOn : c2->Get(x, y);
      FillCube(c, horloc, verloc, ds.horsiz, ds.versiz, ds.depsiz - 1, dir,
        !fColor ? fOff : ds.kvTrim, kv, rx, ry, rz,
        !b.Get(x + dir, y), !b.Get(x, y + 1), fTrue);

      // Draw lines removing seams between this and earlier drawn blocks, in a
      // a color half way between the color used for the two faces.
      if (ds.fMerge) {
        if (b.Get(x - dir, y)) {
          if (fColor)
            kvT = KvBlend(kv, c2->Get(x - dir, y));
          if (ds.versiz > 1)
            c.LineZ(horloc - (ds.horsiz - 1)*dir, verloc - 1, verloc -
              ds.versiz + 1, dir, !fColor ? !ds.fSkyShade : KvShade(kvT, rz));
          if (ds.depsiz > 2)
            c.LineY(horloc - ds.horsiz*dir, verloc + 1, verloc +
              ds.depsiz - 2, !fColor ? !ds.fShading : KvShade(kvT, ry));
          if (b.Get(x, y - 1) && b.Get(x - dir, y - 1))
            c.Set(horloc - (ds.horsiz - ds.versiz)*dir, verloc - ds.versiz,
              !fColor ? !ds.fSkyShade : KvShade(KvBlend(kvT, KvBlend(c2->
              Get(x, y - 1), c2->Get(x - dir, y - 1))), rz));
        }
        if (b.Get(x, y - 1)) {
          if (fColor)
            kvT = KvBlend(kv, c2->Get(x, y - 1));
          if (ds.horsiz > 1)
            c.LineX(horloc - (ds.horsiz - ds.versiz - 1)*dir,
              horloc + (ds.versiz - 1)*dir, verloc - ds.versiz,
              !fColor ? !ds.fSkyShade : KvShade(kvT, rz));
          if (!b.Get(x + dir, y - 1) && ds.depsiz > 2)
            c.LineY(horloc + ds.versiz*dir, verloc - ds.versiz + 1,
              verloc - ds.versiz + ds.depsiz - 2,
              !fColor ? fOn : KvShade(kvT, rx));
        }
      }
    }
    UpdateDisplay();
  }
}


// Replace bitmap with an orthographic overview of a monochrome bitmap. The
// color of each block comes from the contents of an optional color bitmap.
// Implements the Render Bitmap Overview command for 2D bitmaps.

flag DrawOverview(CMap &c, CONST CMon &b)
{
  CMon bCopy;
  CCol cCopy;
  flag fColor = c.FColor();

  if (fColor ? !cCopy.FBitmapCopy(c) : !bCopy.FBitmapCopy(c))
    return fFalse;
  if (!c.FBitmapSizeSet(b.m_x*ds.horsiz + b.m_y*ds.versiz + 1,
    b.m_y*ds.versiz + ds.depsiz))
    return fFalse;
  c.BitmapSet(!fColor ? !ds.fGroundShade : ds.kvGroundLo);
  UpdateDisplay();
  FillOverview(c, !fColor ? bCopy : b, !fColor ? NULL : &cCopy);
  return fTrue;
}


// Draw an orthographic overview of a 3D bitmap within the bounds of another
// bitmap. The color of each block comes from an optional second color bitmap.

void FillOverviewCube(CMap &c, CONST CMon3 &t, CONST CCol3 *c2,
  int lensiz, int widsiz, int depsiz, flag fView, flag fMerge)
{
  int x, y, z, horloc, verloc, dir;
  real rx = 0.0, ry = 0.0, rz = 0.0;
  KV kv, kvT;
  flag fColor = c.FColor(), fRainbow = fColor &&
    (t.m_x3 != c2->m_x3 || t.m_y3 != c2->m_y3 || t.m_z3 != c2->m_z3), f1, f2;

  if (fColor && ds.fShading) {
    rx = ds.vLight.m_x / 100.0;
    ry = ds.vLight.m_y / 100.0;
    rz = ds.vLight.m_z / 100.0;
  }
  dir = fView*2 - 1;
  for (y = 0; y < t.m_y3; y++) {
    for (z = t.m_z3-1; z >= 0; z--) {
      verloc = z*depsiz + (y + 1)*widsiz;
      for (x = fView ? 0 : t.m_x3 - 1; x >= 0 && x < t.m_x3; x += dir) {
        if (!t.Get3(x, y, z))
          continue;
        if (fView)
          horloc = (x + 1)*lensiz + (t.m_y3 - 1 - y)*widsiz;
        else
          horloc = x*lensiz + (y + 1)*widsiz;

        // Draw a colored block for each set pixel.
        kv = !fColor ? fOn : c2->Get3R(x, y, z, fRainbow);
        FillCube(c, horloc, verloc, lensiz, widsiz, depsiz, dir,
          !fColor ? fOff : ds.kvTrim, kv, rx, ry, rz,
          !t.Get3(x + dir, y, z), !t.Get3(x, y + 1, z), !t.Get3(x, y, z - 1));

        // Draw lines removing seams between this and earlier drawn blocks,
        // in a color half way between the color used for the two faces.
        if (fMerge) {
          if (t.Get3(x - dir, y, z)) {
            if (fColor)
              kvT = KvBlend(kv, c2->Get3R(x - dir, y, z, fRainbow));
            if (depsiz > 1)
              c.LineY(horloc - lensiz*dir, verloc + 1, verloc + depsiz - 1,
                !fColor ? !ds.fShading : KvShade(kvT, ry));
            if (widsiz > 1)
              c.LineZ(horloc - (lensiz - 1)*dir, verloc - 1, verloc - widsiz +
                1, dir, !fColor ? !ds.fSkyShade : KvShade(kvT, rz));
            f1 = t.Get3(x, y, z + 1) && t.Get3(x - dir, y, z + 1);
            if (f1)
              c.Set(horloc - lensiz*dir, verloc + depsiz,
                !fColor ? !ds.fShading : KvShade(KvBlend(kvT,
                KvBlend(c2->Get3R(x, y, z + 1, fRainbow),
                c2->Get3R(x - dir, y, z + 1, fRainbow))), ry));
            f1 = t.Get3(x, y - 1, z) && t.Get3(x - dir, y - 1, z);
            f2 = !(t.Get3(x, y - 1, z - 1) || t.Get3(x - dir, y - 1, z - 1));
            if (f1 && f2)
              c.Set(horloc - (lensiz - widsiz)*dir, verloc - widsiz,
                !fColor ? !ds.fSkyShade : KvShade(KvBlend(kvT,
                KvBlend(c2->Get3R(x, y - 1, z, fRainbow),
                c2->Get3R(x - dir, y - 1, z, fRainbow))), rz));
          }
          if (t.Get3(x, y, z + 1)) {
            if (fColor)
              kvT = KvBlend(kv, c2->Get3R(x, y, z + 1, fRainbow));
            if (lensiz > 1)
              c.LineX(horloc - (lensiz - 1)*dir, horloc - dir,
                verloc + depsiz, !fColor ? !ds.fShading : KvShade(kvT, ry));
            if (!t.Get3(x + dir, y, z + 1) && widsiz > 1)
              c.LineZ(horloc + dir, verloc + depsiz - 1, verloc - widsiz +
                depsiz + 1, dir, !fColor ? fOn : KvShade(kvT, rx));
            f1 = t.Get3(x, y - 1, z) && t.Get3(x, y - 1, z + 1);
            f2 = !(t.Get3(x + dir, y - 1, z) || t.Get3(x + dir, y, z + 1));
            if (f1 && f2 && !t.Get3(x + dir, y - 1, z + 1))
              c.Set(horloc + widsiz*dir, verloc - widsiz + depsiz,
                !fColor ? fOn : KvShade(KvBlend(kvT,
                KvBlend(c2->Get3R(x, y - 1, z, fRainbow),
                c2->Get3R(x, y - 1, z + 1, fRainbow))), rx));
          }
          if (t.Get3(x, y - 1, z)) {
            if (fColor)
              kvT = KvBlend(kv, c2->Get3R(x, y - 1, z, fRainbow));
            if (!t.Get3(x + dir, y - 1, z) && depsiz > 1)
              c.LineY(horloc + widsiz*dir,
                verloc - widsiz + 1, verloc - widsiz + depsiz - 1,
                !fColor ? fOn : KvShade(kvT, rx));
            if (!t.Get3(x, y - 1, z - 1) && lensiz > 1)
              c.LineX(horloc - (lensiz - widsiz - 1)*dir,
                horloc + (widsiz - 1)*dir, verloc - widsiz,
                !fColor ? !ds.fSkyShade : KvShade(kvT, rz));
          }
        }
      }
    }
    UpdateDisplay();
  }
}


// Replace bitmap with an orthographic overview of a 3D monochrome bitmap. The
// color of each block comes from the contents of an optional color bitmap.
// Implements the Render Bitmap Overview command for 3D bitmaps.

flag DrawOverviewCube(CMap &c, CONST CMon3 &t)
{
  CMon3 tCopy;
  CCol3 cCopy;
  flag fColor = c.FColor();

  if (fColor ? !cCopy.FBitmapCopy(c) : !tCopy.FBitmapCopy(c))
    return fFalse;
  if (!c.FBitmapSizeSet(t.m_x3*ds.horsiz + t.m_y3*ds.versiz + 1,
    t.m_z3*ds.depsiz + t.m_y3*ds.versiz + 1))
    return fFalse;
  c.BitmapSet(!fColor ? !ds.fGroundShade : ds.kvGroundLo);
  UpdateDisplay();
  FillOverviewCube(c, !fColor ? tCopy : t, !fColor ? NULL : &cCopy,
    ds.horsiz, ds.versiz, ds.depsiz, ds.fRight, ds.fMerge);
  return fTrue;
}


// Draw an angled orthographic overview of one bitmap within the bounds of
// another.

void FillOverview2(CMon &b2, CONST CMon &b1)
{
  int xs = ds.horsiz*ds.versiz, x, y, horloc, verloc;

  if (ds.depsiz < 1)
    return;
  for (y = 0; y < b1.m_y; y++) {
    for (x = 0; x < b1.m_x; x++) if (b1.Get(x, y)) {
      horloc = (x + (b1.m_y-y)) * (xs + 1);
      verloc = (x + y + 2)*(ds.versiz + 1);

      // Draw a block for each set pixel.
      FillCube2(b2, horloc, verloc, ds.horsiz, ds.versiz, ds.depsiz - 1,
        !b1.Get(x + 1, y), !b1.Get(x, y + 1), fTrue);

      // Draw lines removing seams between this and earlier drawn blocks.
      if (ds.fMerge) {
        if (b1.Get(x - 1, y)) {
          b2.LineZ(horloc - xs, verloc - ds.versiz - 2,
            verloc - ds.versiz*2 - 1, ds.horsiz, !ds.fSkyShade);
          if (ds.depsiz > 2)
            b2.LineY(horloc - xs - 1, verloc - ds.versiz,
              verloc - ds.versiz + ds.depsiz - 3, !ds.fShading);
          if (b1.Get(x, y - 1) && b1.Get(x - 1, y - 1))
            b2.Set(horloc, verloc - ds.versiz*2 - 2, !ds.fSkyShade);
        }
        if (b1.Get(x, y - 1)) {
          b2.LineZ(horloc + xs, verloc - ds.versiz - 2,
            verloc - ds.versiz*2 - 1, -ds.horsiz, !ds.fSkyShade);
          if (!b1.Get(x + 1, y - 1) && ds.depsiz > 2)
            b2.LineY(horloc + xs + 1, verloc - ds.versiz,
              verloc - ds.versiz + ds.depsiz - 3, fOn);
        }
      }
    }
    UpdateDisplay();
  }
}


// Replace a bitmap with an angled orthographic overview of itself. Implements
// the Overview2 operation for monochrome 2D bitmaps.

flag DrawOverview2(CMon &b)
{
  CMon b1;

  if (!b1.FBitmapCopy(b))
    return fFalse;
  if (!b.FBitmapSizeSet((b1.m_x + b1.m_y)*(ds.horsiz*ds.versiz + 1) + 1,
    (b1.m_x + b1.m_y)*(ds.versiz + 1) + ds.depsiz))
    return fFalse;
  b.BitmapSet(!ds.fGroundShade);
  UpdateDisplay();
  FillOverview2(b, b1);
  return fTrue;
}


// Draw an angled orthographic overview of a 3D bitmap within the bounds of
// another bitmap.

void FillOverviewCube2(CMon &b, CONST CMon3 &t,
  int lensiz, int widsiz, int depsiz, flag fMerge)
{
  int xs = ds.horsiz*ds.versiz, x, y, z, horloc, verloc;
  flag f1, f2;

  for (y = 0; y < t.m_y3; y++) {
    for (z = t.m_z3-1; z >= 0; z--)
      for (x = 0; x < t.m_x3; x++)
        if (t.Get3(x, y, z)) {
          horloc = (x + (t.m_y3-y)) * (xs + 1);
          verloc = (x + y + 2)*(ds.versiz + 1) + z*ds.depsiz;

          // Draw a block for each set pixel.
          FillCube2(b, horloc, verloc, lensiz, widsiz, depsiz,
            !t.Get3(x + 1, y, z), !t.Get3(x, y + 1, z), !t.Get3(x, y, z - 1));

          // Draw lines removing seams between this and earlier drawn blocks.
          if (fMerge) {
            if (t.Get3(x - 1, y, z)) {
              if (depsiz > 1)
                b.LineY(horloc - xs - 1, verloc - widsiz,
                  verloc - widsiz + depsiz - 2, !ds.fShading);
              if (widsiz > 0)
                b.LineZ(horloc - xs, verloc - widsiz - 2,
                  verloc - widsiz*2 - 1, lensiz, !ds.fSkyShade);
              f1 = t.Get3(x, y, z + 1) && t.Get3(x - 1, y, z + 1);
              if (f1)
                b.Set(horloc - xs - 1, verloc - widsiz + depsiz - 1,
                  !ds.fShading);
              f1 = t.Get3(x, y - 1, z) && t.Get3(x - 1, y - 1, z);
              f2 = !(t.Get3(x, y - 1, z - 1) || t.Get3(x - 1, y - 1, z - 1));
              if (f1 && f2)
                b.Set(horloc, verloc - widsiz*2 - 2, !ds.fSkyShade);
            }
            if (t.Get3(x, y, z + 1)) {
              if (lensiz > 0)
                b.LineZ(horloc - 1, verloc + depsiz - 1,
                  verloc + depsiz - widsiz, -lensiz, !ds.fShading);
              if (!t.Get3(x + 1, y, z + 1) && widsiz > 0)
                b.LineZ(horloc + 1, verloc + depsiz - 1,
                  verloc + depsiz - widsiz, lensiz, fOn);
              f1 = t.Get3(x, y - 1, z) && t.Get3(x, y - 1, z + 1);
              f2 = !(t.Get3(x + 1, y - 1, z) || t.Get3(x + 1, y, z + 1));
              if (f1 && f2 && !t.Get3(x + 1, y - 1, z + 1))
                b.Set1(horloc + xs + 1, verloc - widsiz + depsiz - 1);
            }
            if (t.Get3(x, y - 1, z)) {
              if (!t.Get3(x + 1, y - 1, z) && depsiz > 1)
                b.LineY(horloc + xs + 1, verloc - widsiz,
                  verloc - widsiz + depsiz - 2, fOn);
              if (!t.Get3(x, y - 1, z - 1) && lensiz > 0)
                b.LineZ(horloc + xs, verloc - widsiz - 2,
                  verloc - widsiz*2 - 1, -lensiz, !ds.fSkyShade);
            }
          }
        }
    UpdateDisplay();
  }
}


// Replace a 3D bitmap with an angled orthographic overview of itself.
// Implements the Overview2 operation for monochrome 3D bitmaps.

flag DrawOverviewCube2(CMon3 &b)
{
  CMon3 t;

  if (!t.FBitmapCopy(b))
    return fFalse;
  if (!b.FBitmapSizeSet((t.m_x3 + t.m_y3)*(ds.horsiz*ds.versiz + 1) + 1,
    (t.m_x3 + t.m_y3)*(ds.versiz + 1) + t.m_z3*ds.depsiz + 1))
    return fFalse;
  b.BitmapSet(!ds.fGroundShade);
  UpdateDisplay();
  FillOverviewCube2(b, t, ds.horsiz, ds.versiz, ds.depsiz, ds.fMerge);
  return fTrue;
}


#define ZAltitude(kv) (NMin(NMultDiv(kv, zscale, 1000), zmax) - 1)

// Draw an orthographic overview of a color bitmap, where the color value of
// each pixel indicates the height of that block, within the bounds of another
// bitmap. The color for each block comes from an optional third bitmap.

void FillOverviewAltitude(CMap &c, CONST CMap &c1, CONST CMap *c2,
  int zscale, int zmax)
{
  int x, y, z, z1, z2, horloc, verloc, dir;
  real rx = 0.0, ry = 0.0, rz = 0.0;
  KV kv, kvT;
  flag fColor = c.FColor();

  if (fColor && ds.fShading) {
    rx = ds.vLight.m_x / 100.0;
    ry = ds.vLight.m_y / 100.0;
    rz = ds.vLight.m_z / 100.0;
  }
  dir = ds.fRight*2 - 1;
  for (y = 0; y < c1.m_y; y++) {
    verloc = (y + 1)*ds.versiz + zmax - 1;
    for (x = ds.fRight ? 0 : c1.m_x - 1; x >= 0 && x < c1.m_x; x += dir) {
      kvT = c1.Get(x, y);
      z = ZAltitude(kvT);
      if (z < 0)
        continue;
      if (ds.fRight)
        horloc = (x + 1)*ds.horsiz + (c1.m_y - y - 1)*ds.versiz;
      else
        horloc = x*ds.horsiz + (y + 1)*ds.versiz;

      // Draw a colored block of appropriate height for each non-zero pixel.
      kv = !fColor ? fOn : c2->Get(x, y);
      FillCube(c, horloc, verloc - z, ds.horsiz, ds.versiz, z, dir,
        ds.kvTrim, kv, rx, ry, rz,
        c1.Get(x + dir, y) < kvT, c1.Get(x, y + 1) < kvT, fTrue);

      // Draw lines removing seams between this and earlier drawn blocks, in a
      // color half way between the color used for the two faces.
      if (ds.fMerge) {
        z1 = ZAltitude(c1.Get(x - dir, y));
        if (z1 >= 0) {
          if (fColor)
            kvT = KvBlend(kv, c2->Get(x - dir, y));
          if (z == z1 && ds.versiz > 1)
            c.LineZ(horloc - (ds.horsiz - 1)*dir, verloc - z - 1,
              verloc - z - ds.versiz + 1, dir,
              !fColor ? !ds.fSkyShade : KvShade(kvT, rz));
          z2 = Min(z1, z);
          if (z2 > 1)
            c.LineY(horloc - ds.horsiz*dir, verloc - z2 + 1, verloc - 1,
              !fColor ? !ds.fShading : KvShade(kvT, ry));
          if (z == z1 && z == ZAltitude(c1.Get(x, y - 1)) &&
            z == ZAltitude(c1.Get(x - dir, y - 1)))
            c.Set(horloc - (ds.horsiz - ds.versiz)*dir, verloc - z -
              ds.versiz, !fColor ? !ds.fSkyShade : KvShade(KvBlend(kvT,
              KvBlend(c2->Get(x, y - 1), c2->Get(x - dir, y - 1))), rz));
        }
        z1 = ZAltitude(c1.Get(x, y - 1));
        if (z1 >= 0) {
          if (fColor)
            kvT = KvBlend(kv, c2->Get(x, y - 1));
          if (z == z1 && ds.horsiz > 1)
            c.LineX(horloc - (ds.horsiz - ds.versiz - 1)*dir,
              horloc + (ds.versiz - 1)*dir, verloc - z - ds.versiz,
              !fColor ? !ds.fSkyShade : KvShade(kvT, rz));
          z1 = Min(z1, z);
          if (z1 > 1) {
            z2 = ZAltitude(c1.Get(x + dir, y - 1)); z2 = Max(z2, 0);
            if (z1 > z2 + 1)
              c.LineY(horloc + ds.versiz*dir, verloc - ds.versiz - z1 + 1,
                verloc - ds.versiz - z2 - 1,
                !fColor ? fOn : KvShade(kvT, rx));
          }
        }
      }
    }
    UpdateDisplay();
  }
}


// Draw an orthographic overview of a color bitmap, where the color value of
// each pixel indicates the height of that block, in a different bitmap. The
// color to use for each block comes from an optional second bitmap.
// Implements the Altitude operation for monochrome and color bitmaps.

flag DrawOverviewAltitude(CMap &c, CONST CCol &c2, int zscale, int zmax)
{
  CCol c1;
  CONST CCol *pc;
  long l;
  flag fColor = c.FColor();

  pc = !fColor ? &c2 : &c1;
  if (fColor && !c1.FBitmapCopy(c))
    return fFalse;
  if (zscale >= 0)
    zmax = NMultDiv(zmax, zscale, 1000);
  else {
    l = pc->ColmapFind(0);
    zscale = (zmax * 1000 + 499) / (l == 0 ? 1 : l);
  }
  if (!c.FBitmapSizeSet(pc->m_x*ds.horsiz + pc->m_y*ds.versiz + 1,
    pc->m_y*ds.versiz + zmax))
    return fFalse;
  c.BitmapSet(!fColor ? !ds.fGroundShade : ds.kvGroundLo);
  UpdateDisplay();
  if (!fColor)
    FillOverviewAltitude(c, c2, NULL, zscale, zmax);
  else
    FillOverviewAltitude(c, c1, &c2, zscale, zmax);
  return fTrue;
}


/*
******************************************************************************
** Render Pyramid
******************************************************************************
*/

// Return whether a point between four blocks forms a visible corner that
// should have an edge line drawn at that point.

flag FCorner(CONST CMon &b, int x, int y)
{
  flag f0, f1, f2, f3;
  int count;

  // A single block forms a convex corner, three blocks form a concave corner,
  // and two opposite blocks form a checkerboard corner.

  f0 = b.Get(x, y);
  f1 = b.Get(x - 1, y);
  f2 = b.Get(x, y - 1);
  f3 = b.Get(x - 1, y - 1);
  count = f0 + f1 + f2 + f3;
  return FOdd(count) || (count == 2 && f0 == f3);
}


// Draw an edge line from an exposed block corner to the vanishing point. Stop
// drawing if the line becomes no longer visible behind other blocks.

void PLine(CMap &b2, CONST CMon &b1, int x1, int y1,
  int x2, int y2, int xv, int yv)
{
  int x = x1, y = y1, dx = x2 - x1, dy = y2 - y1, xInc, yInc, xInc2, yInc2,
    d, dInc, z, zMax, xT, yT;
  flag fColor = b2.FColor(), fStarted = fFalse, fProbation = fFalse,
    f0, f1, f2, f3;

  // If blocks are transparent, just draw the whole line.
  if (!ds.fMerge) {
    b2.Line(x1, y1, x2, y2, !fColor ? fOff : ds.kvTrim);
    return;
  }

  // If vanishing point is covered by a block, don't draw edges of that block.
  xT = x1 / ds.horsiz; yT = y1 / ds.versiz;
  if (b1.Get(xT - (xT > xv), yT - (yT > yv)))
    return;

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
    f0 = b1.Get(x / ds.horsiz, y / ds.versiz);
    f1 = b1.Get((x-1) / ds.horsiz, y / ds.versiz);
    f2 = b1.Get(x / ds.horsiz, (y-1) / ds.versiz);
    f3 = b1.Get((x-1) / ds.horsiz, (y-1) / ds.versiz);

    // If entirely within a block, definitely stop drawing.
    if (f0 && f1 && f2 && f3)
      break;

    // If adjacent to a block, that's ok unless it happens twice in a row.
    if (f0 || f1 || f2 || f3) {
      if (fStarted) {
        if (fProbation)
          break;
        fProbation = fTrue;
      }
    } else {
      fStarted = fTrue;
      fProbation = fFalse;
    }

    // Not within a block, so draw next pixel of line.
    if (!fColor)
      b2.Set0(x, y);
    else
      b2.Set(x, y, ds.kvTrim);
    x += xInc; y += yInc; d += dInc;
    if (d >= zMax) {
      x += xInc2; y += yInc2; d -= zMax;
    }
  }
}


// Draw a single block in a single point perspective graphic.

void FillBlock(CMap &b2, CONST CMon &b1, int x, int y, int xv, int yv)
{
  int x1, y1, x2, y2;
  flag fColor = b2.FColor();

  if (!b1.Get(x, y) || !ds.fMerge)
    return;

  // Draw the front face, and the faces that go to the vanishing point.

  if (!fColor) {
    if ((x < xv && !b1.Get(x+1, y)) || (x > xv && !b1.Get(x-1, y)) ||
      (y < yv && !b1.Get(x, y+1)) || (y > yv && !b1.Get(x, y-1))) {
      x1 = (x + (y == yv && x < xv))*ds.horsiz;
      y1 = (y + ((y < yv && x <= xv) || (y > yv && x > xv)))*ds.versiz;
      x2 = (x + (y != yv || x < xv))*ds.horsiz;
      y2 = (y + ((y >= yv || x >= xv) && (y <= yv || x < xv)))*ds.versiz;
      b2.FTriangle(ds.horv, ds.verv, x1, y1, x2, y2, fOn, 0);
    }
    b2.Block(x*ds.horsiz, y*ds.versiz, (x+1)*ds.horsiz, (y+1)*ds.versiz,
      !ds.fShading);
  } else {
    x1 = x*ds.horsiz; y1  = y*ds.versiz;
    x2 = x1 + ds.horsiz; y2 = y1 + ds.versiz;
    if (x < xv && !b1.Get(x+1, y))
      b2.FTriangle(ds.horv, ds.verv, x2, y1, x2, y2,
        ds.fShading ? ds.kvGroundLo : ds.kvSkyHi, 0);
    if (x > xv && !b1.Get(x-1, y))
      b2.FTriangle(ds.horv, ds.verv, x1, y1, x1, y2,
        ds.fShading ? ds.kvGroundHi : ds.kvSkyHi, 0);
    if (y < yv && !b1.Get(x, y+1))
      b2.FTriangle(ds.horv, ds.verv, x1, y2, x2, y2,
        ds.fShading ? ds.kvSkyLo    : ds.kvSkyHi, 0);
    if (y > yv && !b1.Get(x, y-1))
      b2.FTriangle(ds.horv, ds.verv, x1, y1, x2, y1,
        ds.kvSkyHi, 0);
    b2.Block(x*ds.horsiz, y*ds.versiz, (x+1)*ds.horsiz, (y+1)*ds.versiz,
      ds.kvObject);
  }
}


// Draw a line of blocks in a single point perspective graphic, in the given
// order. It's assumed the vanishing point doesn't intersect the line.

void FillLine(CMap &b2, CONST CMon &b1, int x1, int y1, int x2, int y2,
  int xv, int yv)
{
  int xi, yi, x, y;

  // Draw blocks in a line from (x1, y1) to (x2, y2).
  Assert(x1 == x2 || y1 == y2);
  if (x1 == x2) {
    yi = y1 < y2 ? 1 : -1; xi = 0;
  } else {
    xi = x1 < x2 ? 1 : -1; yi = 0;
  }
  x = x1; y = y1;
  loop {
    FillBlock(b2, b1, x, y, xv, yv);
    if (x == x2 && y == y2)
      break;
    x += xi; y += yi;
  }
}


// Draw a grid of blocks in a single point perspective graphic, in the given
// order from one corner to the opposite. It's assumed the vanishing point
// doesn't intersect the grid. Lines of blocks are drawn touching the diagonal
// leading up to the end corner. For example, a 5x4 grid from lower left to
// upper right has blocks drawn in the following order:
//
// 16 17 18 19 20
//  9 10 11 12 15
//  4  5  6  8 14
//  1  2  3  7 13

void FillSection(CMap &b2, CONST CMon &b1, int x1, int y1, int x2, int y2,
  int xv, int yv)
{
  int xi, yi, xd, yd, xo, yo, iMax, i;

  // Draw blocks in a grid from (x1, y1) to (x2, y2).
  if (x2 < 0 || y2 < 0)
    return;
  xi = x2 > x1 ? 1 : -1; yi = y2 > y1 ? 1 : -1;
  xd = NAbs(x2 - x1) + 1; yd = NAbs(y2 - y1) + 1;
  if (xd > yd) {
    xo = xd - yd; yo = 0;
  } else {
    yo = yd - xd; xo = 0;
  }

  // Draw the first line touching the start corner.
  FillLine(b2, b1, x1, y1, x1 + xo*xi, y1 + yo*yi, xv, yv);

  // Draw pairs of lines growing out from the first line like layers of an
  // onion, until the final layer forming the edges touching the end corner.
  iMax = xd < yd ? xd : yd;
  for (i = 1; i < iMax; i++) {
    FillLine(b2, b1,
      x1 + (xo+i)*xi, y1, x1 + (xo+i)*xi, y1 + (yo+i-1)*yi, xv, yv);
    FillLine(b2, b1,
      x1, y1 + (yo+i)*yi, x1 + (xo+i)*xi, y1 + (yo+i)*yi, xv, yv);
  }
}


// Draw a single point perspective graphic of one bitmap in another. Each
// pixel becomes a rectangle with lines leading to the vanishing point.

void FillPyramid(CMap &b2, CONST CMon &b1)
{
  int xv, yv, x, y;
  flag fColor = b2.FColor();

  xv = ds.horv / ds.horsiz;
  yv = ds.verv / ds.versiz;
  if (xv > b1.m_x)
    xv = b1.m_x;
  if (yv > b1.m_y)
    yv = b1.m_y;

  // Draw the blocks in four quadrants, divided by the location of the
  // vanishing point.

  FillSection(b2, b1, 0, 0, xv-1, yv-1, xv, yv);
  FillSection(b2, b1, b1.m_x-1, 0, xv, yv-1, xv, yv);
  FillSection(b2, b1, 0, b1.m_y-1, xv-1, yv, xv, yv);
  FillSection(b2, b1, b1.m_x-1, b1.m_y-1, xv, yv, xv, yv);

  // Draw the front edges of the blocks, where not adjacent to other blocks.

  for (y = 0; y <= b1.m_y; y++)
    for (x = 0; x <= b1.m_x; x++) {
      if (b1.Get(x, y) != b1.Get(x, y - 1))
        b2.LineX(x*ds.horsiz, (x+1)*ds.horsiz, y*ds.versiz,
          !fColor ? fOff : ds.kvTrim);
      if (b1.Get(x, y) != b1.Get(x - 1, y))
        b2.LineY(x*ds.horsiz, y*ds.versiz, (y+1)*ds.versiz,
          !fColor ? fOff : ds.kvTrim);

      // Draw visible edges of the block that go to the vanishing point. Don't
      // draw lines if the block is solid and the line would be behind it.

      if (FCorner(b1, x, y))
        PLine(b2, b1, x*ds.horsiz, y*ds.versiz, ds.horv, ds.verv, xv, yv);
    }
}


// Replace a bitmap with a single point perspective graphic of it. Implements
// the Render Pyramid command.

flag DrawPyramid(CMon &b, CCol *c)
{
  CMon bCopy;
  int x, y;
  flag fColor = (c != NULL);

  x = b.m_x*ds.horsiz + 1; y =  b.m_y*ds.versiz + 1;
  if (FErrorRange("X cell size", ds.horsiz, 2, (xBitmap-1) / b.m_x))
    return fFalse;
  if (FErrorRange("Y cell size", ds.versiz, 2, (yBitmap-1) / b.m_y))
    return fFalse;
  if (FErrorRange("X vanishing point", ds.horv, 0, x-1))
    return fFalse;
  if (FErrorRange("Y vanishing point", ds.verv, 0, y-1))
    return fFalse;
  if (!fColor) {
    if (!bCopy.FBitmapCopy(b))
      return fFalse;
    if (b.FBitmapSizeSet(x, y)) {
      b.BitmapSet(!ds.fGroundShade);
      UpdateDisplay();
      FillPyramid(b, bCopy);
    }
  } else {
    if (c->FBitmapSizeSet(x, y)) {
      c->BitmapSet(kvWhite);
      UpdateDisplay();
      FillPyramid(*c, b);
    }
  }
  return fTrue;
}


/*
******************************************************************************
** File Reading Routines
******************************************************************************
*/

// Load a Daedalus wireframe file, into a new list of lines.

long ReadWirelist(COOR **pcoor, FILE *file)
{
  char szLine[cchSzDef];
  COOR *coor;
  long count = 0, ichFile, i, j;
  real x1, y1, z1, x2, y2, z2;
  KV kv = ds.kvTrim;
  char ch, ch2;

  ch = getbyte(); ch2 = getbyte();
  if (ch != 'D' || ch2 != 'W') {
    PrintSz_W("This file does not look like a Daedalus wireframe.\n");
    return -1;
  }

  // Figure out how many coordinate pairs are in this wireframe file.
  ch = getbyte();
  if (ch == '#')
    fscanf(file, "%ld", &count);
  else {
    ichFile = ftell(file);
    loop {
      while (!feof(file) && (ch = getc(file)) < ' ')
        ;
      if (feof(file))
        break;
      if (!FDigitCh(ch) && ch != '-') {
        for (szLine[0] = ch, j = 1; j < cchSzDef-1 && !feof(file) &&
          (ch = getbyte()) >= ' '; j++)
          ;
        continue;
      } else
        ungetc(ch, file);
      ReadCoordinates(file, &x1, &y1, &z1, &x2, &y2, &z2);
      count++;
    }
    fseek(file, ichFile, SEEK_SET);
  }

  // Allocate a line list of the appropriate size.
  coor = RgAllocate(count, COOR);
  if (coor == NULL)
    return -1;
  if (*pcoor != NULL)
    DeallocateP(*pcoor);
  *pcoor = coor;

  // Load the appropriate number of actual lines from the file.
  for (i = 0; i < count; i++) {
    while (!feof(file) && (ch = getc(file)) < ' ')
      ;
    if (feof(file))
      break;
    if (!FDigitCh(ch) && ch != '-') {
      for (szLine[0] = ch, j = 1; j < cchSzDef-1 && !feof(file) &&
        (ch = getbyte()) >= ' '; j++)
        szLine[j] = ch;
      szLine[j] = chNull;
      kv = ParseColor(szLine, fFalse);
      i--;
      continue;
    } else
      ungetc(ch, file);
    ReadCoordinates(file, &x1, &y1, &z1, &x2, &y2, &z2);
    coor[i].x1 = x1; coor[i].y1 = y1; coor[i].z1 = z1;
    coor[i].x2 = x2; coor[i].y2 = y2; coor[i].z2 = z2;
    coor[i].kv = kv;
  }
  return count;
}


// Load a Daedalus patch file, into a new list of patches.

long ReadPatchlist(PATCH **ppatch, FILE *file)
{
  PATCH *patch;
  long count = 0, i;
  int nTrans = ds.nTrans, status, nT;
  real x1, y1, z1, x2, y2, z2, x3, y3, z3;
  KV kv = ds.kvObject;
  char ch, ch2;

  ch = getbyte(); ch2 = getbyte();
  if (ch != 'D' || ch2 != 'P') {
    PrintSz_W("This file does not look like a Daedalus patch file.\n");
    return -1;
  }

  // Figure out how many patches are in this patch file.
  ch = getbyte();
  if (ch == '#')
    fscanf(file, "%ld", &count);
  else {
    i = ftell(file);
    loop {
      ReadPatch1(file, &status);
      switch (status) {
      case -1: case 8: ReadColor(file); break;
      case -2: ReadPatch1(file, &nT); break;
      default:
        count++;
        ReadPatch2(file, &x1, &y1, &z1, &x2, &y2, &z2, &x3, &y3, &z3);
        if (status & 32)
          ReadPatch3(file, &x1, &y1, &z1);
      }
      if (fscanf(file, "%c", S1(&ch)) < 0) {
        count--;
        break;
      }
    }
    fseek(file, i, SEEK_SET);
  }

  // Allocate a patch list of the appropriate size.
  patch = RgAllocate(count, PATCH);
  if (patch == NULL)
    return -1;
  if (*ppatch != NULL)
    DeallocateP(*ppatch);
  *ppatch = patch;

  // Load the appropriate number of actual patches from the file.
  for (i = 0; i < count; ) {
    ReadPatch1(file, &status);
    switch (status) {
    case -1: case 8: kv = ReadColor(file); break;
    case -2: ReadPatch1(file, &nTrans); break;
    default:
      ReadPatch2(file, &x1, &y1, &z1, &x2, &y2, &z2, &x3, &y3, &z3);
      patch[i].p[0].x = x1;
      patch[i].p[0].y = y1;
      patch[i].p[0].z = z1;
      patch[i].p[1].x = x2;
      patch[i].p[1].y = y2;
      patch[i].p[1].z = z2;
      patch[i].p[2].x = x3;
      patch[i].p[2].y = y3;
      patch[i].p[2].z = z3;
      patch[i].p[0].fLine = (status & 4) > 0;
      patch[i].p[1].fLine = (status & 2) > 0;
      patch[i].p[2].fLine = FOdd(status);

      // Load the fourth point if this is a quadrilateral patch.
      if (status & 32) {
        ReadPatch3(file, &x1, &y1, &z1);
        patch[i].p[3].x = x1;
        patch[i].p[3].y = y1;
        patch[i].p[3].z = z1;
        patch[i].p[3].fLine = (status & 16) > 0;
        patch[i].cpt = 4;
      } else {
        patch[i].p[3].x = patch[i].p[3].y = patch[i].p[3].z = 0.0;
        patch[i].p[3].fLine = fFalse;
        patch[i].cpt = 3;
      }
      patch[i].kv = kv;
      patch[i].nTrans = nTrans;
      i++;
    }
  }
  return count;
}


/*
******************************************************************************
** File Writing Routines
******************************************************************************
*/

// Save a list of lines to a Daedalus wireframe file.

void WriteWireframe(FILE *file, CONST COOR *coor, long ccoor)
{
  char sz[cchSzDef];
  long i;
  KV kvLast = ds.kvObject;

  fprintf(file, "DW#\n%ld\n", ccoor);
  for (i = 0; i < ccoor; i++) {
    if (coor[i].kv != kvLast) {
      kvLast = coor[i].kv;
      ConvertKvToSz(coor[i].kv, sz, cchSzDef);
      fprintf(file, "%s\n", sz);
    }
    fprintf(file, "%d %d %d %d %d %d\n",
      (int)coor[i].x1, (int)coor[i].y1, (int)coor[i].z1,
      (int)coor[i].x2, (int)coor[i].y2, (int)coor[i].z2);
  }
}


// Save a list of patches to a Daedalus patch file.

void WritePatches(FILE *file, CONST PATCH *patch, long cpatch)
{
  char sz[cchSzDef];
  CONST PATCH *pat;
  long i;
  int nTransLast = 0, j;
  KV kvPatchLast = ds.kvObject;

  fprintf(file, "DP#\n%ld\n", cpatch);
  for (i = 0; i < cpatch; i++) {
    pat = &patch[i];

    // Save color or transparency change if this patch differs from previous.
    if (pat->kv != kvPatchLast) {
      kvPatchLast = pat->kv;
      ConvertKvToSz(pat->kv, sz, cchSzDef);
      fprintf(file, "-1 %s\n", sz);
    }
    if (pat->nTrans != nTransLast) {
      nTransLast = pat->nTrans;
      fprintf(file, "-2 %d\n", nTransLast);
    }

    // Save the actual patch points.
    Assert(pat->cpt == 3 || pat->cpt == 4);
    fprintf(file, "%d", (pat->cpt == 4 ? 32 : 0) |
      pat->p[0].fLine*4 | pat->p[1].fLine*2 | pat->p[2].fLine |
      pat->p[3].fLine*16);
    for (j = 0; j < pat->cpt; j++)
      fprintf(file, " %d %d %d",
        (int)pat->p[j].x, (int)pat->p[j].y, (int)pat->p[j].z);
    fprintf(file, "\n");
  }
}


// Append a line to a coordinate list. Called by the functions that create
// wireframes in memory.

void WriteCoordinates(COOR *coor, int x1, int y1, int z1,
  int x2, int y2, int z2, KV kv)
{
  COOR *pcoor;

  if (coor != NULL) {
    pcoor = &coor[ds.cCoorPatch];
    pcoor->x1 = x1; pcoor->y1 = y1; pcoor->z1 = z1;
    pcoor->x2 = x2; pcoor->y2 = y2; pcoor->z2 = z2;
    pcoor->kv = kv;
  }
  ds.cCoorPatch++;
}


// Given a pair of coordinates assumed to form opposite corners of a
// rectangle of a given size in a grid of such rectangles, adjust them so even
// rows and columns will have the specified adjusted size.

void BiasCoordinates(int x, int y, int horsiz, int versiz,
  int horbias, int verbias, int *x1, int *y1, int *x2, int *y2)
{
  if (FOdd(x))
    *x1 += horbias - horsiz;
  else
    *x2 += horbias - horsiz;
  if (FOdd(y))
    *y1 += verbias - versiz;
  else
    *y2 += verbias - versiz;
}


// Append a quadrilateral patch to a patch list. Called by the functions that
// create patches in memory.

void WritePatch(PATCH *patch, PATN pat[cPatch], flag fReflect, KV kv)
{
  PATCH *ppatch;
  int i, j;

  if (patch != NULL) {
    ppatch = &patch[ds.cCoorPatch];
    ppatch->cpt = 4;
    ppatch->kv = kv;
    ppatch->nTrans = ds.nTrans;
    for (i = 0; i < 4; i++) {
      j = fReflect ? i : 4-i & 3;
      ppatch->p[i].x = pat[j].x;
      ppatch->p[i].y = pat[j].y;
      ppatch->p[i].z = pat[j].z;
      j = fReflect ? i : i^3;
      ppatch->p[i].fLine = pat[j].fLine;
    }
  }
  ds.cCoorPatch++;
}


#define MetaWord(w) putword(w)
#define MetaLong(l) putlong(l)
#define MetaRecord(cw, n) MetaLong(cw); MetaWord(n)
#define MetaSaveDc() MetaRecord(3, 0x01E)
#define MetaRestoreDc() MetaRecord(4, 0x127); MetaWord((word)-1)
#define MetaWindowOrg(x, y) MetaRecord(5, 0x20B); MetaWord(y); MetaWord(x)
#define MetaWindowExt(x, y) MetaRecord(5, 0x20C); MetaWord(y); MetaWord(x)
#define MetaBkMode(n) MetaRecord(4, 0x102); MetaWord(n)
#define MetaEscape(cw) MetaRecord(cw, 0x626); \
  MetaWord(15 /* MFCOMMENT */); MetaWord(((cw)-5)*2 /* Bytes in comment */)

// Save a list of lines to a Windows metafile format file.

void WriteWireframeMetafile(FILE *file, CONST COOR *coor, long ccoor)
{
  long i;
  int xmin = 0, ymin = 0, xmax = 0, ymax = 0, x1, y1, x2, y2, xd, yd;

  // Figure out the bounds of all the line segments.
  for (i = 0; i < ccoor; i++) {
    x1 = (int)coor[i].x1; y1 = (int)coor[i].y1;
    x2 = (int)coor[i].x2; y2 = (int)coor[i].y2;
    xmin = Min(x1, xmin); xmin = Min(x2, xmin);
    ymin = Min(y1, ymin); ymin = Min(y2, ymin);
    xmax = Max(x1, xmax); xmax = Max(x2, xmax);
    ymax = Max(y1, ymax); ymax = Max(y2, ymax);
  }
  xd = xmax - xmin; yd = ymax - ymin;

  // Placeable Metaheader
  MetaLong(0x9AC6CDD7L);
  MetaWord(0);       // Not used
  MetaWord(0); MetaWord(0);
  MetaWord(xd); MetaWord(yd);
  MetaWord(xd/6);    // Units per inch
  MetaLong(0);       // Not used
  MetaWord(0x9AC6 ^ 0xCDD7 ^ xd ^ yd ^ xd/6);  // Checksum

  // Metaheader
  MetaWord(1);       // Metafile type
  MetaWord(9);       // Size of header in words
  MetaWord(0x300);   // Windows version
  MetaLong(9+9+17+3+5+5+4+ccoor*8+4+3);  // Size of entire metafile in words
  MetaWord(0);       // Number of objects in metafile
  MetaLong(17);      // Size of largest record in words
  MetaWord(0);       // Not used

  // Setup
  MetaEscape(17);
  MetaLong(LFromBB('D', 'a', 'e', 'd'));  // "Daed"
  MetaWord(4);                            // Creator
  MetaLong(14);                           // Bytes in string
  MetaLong(LFromBB('D', 'a', 'e', 'd'));  // "Daed"
  MetaLong(LFromBB('a', 'l', 'u', 's'));  // "alus"
  MetaLong(LFromBB(' ', '3', '.', '4'));  // " 3.4"
  MetaWord(WFromBB('0', 0));              // "0"
  MetaSaveDc();
  MetaWindowOrg(xmin, ymin);
  MetaWindowExt(xmax, ymax);
  MetaBkMode(1 /* Transparent */);

  // Records
  for (i = 0; i < ccoor; i++) {
    MetaRecord(8, 0x325);        // Polyline
    MetaWord(2);
    MetaWord((int)coor[i].x1); MetaWord((int)coor[i].y1);
    MetaWord((int)coor[i].x2); MetaWord((int)coor[i].y2);
  }

  // Finalize
  MetaRestoreDc();
  MetaRecord(3, 0);  // End record
}


// Save a list of lines to a Scalable Vector Graphics format file.

void WriteWireframeVector(FILE *file, CONST COOR *coor, long ccoor)
{
  int xmin = 0, ymin = 0, x1, y1, x2, y2, i;
  KV kvCur = -1, kv;

  // Figure out the bounds of all the line segments.
  for (i = 0; i < ccoor; i++) {
    x1 = (int)coor[i].x1; y1 = (int)coor[i].y1;
    x2 = (int)coor[i].x2; y2 = (int)coor[i].y2;
    xmin = Min(x1, xmin); xmin = Min(x2, xmin);
    ymin = Min(y1, ymin); ymin = Min(y2, ymin);
  }

  // Write the coordinates (and color if necessary) of each line segment.
  fprintf(file, "<svg xmlns=\"http://www.w3.org/2000/svg\" "
    "xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n");
  for (i = 0; i < ccoor; i++) {
    kv = coor[i].kv;
    if (kv != kvCur) {
      kvCur = kv;
      if (i > 0)
        fprintf(file, "</g>\n");
      fprintf(file, "<g style=\"stroke:#%c%c%c%c%c%c;\">\n",
        ChHex(RgbR(kv) >> 4), ChHex(RgbR(kv) & 15), ChHex(RgbG(kv) >> 4),
        ChHex(RgbG(kv) & 15), ChHex(RgbB(kv) >> 4), ChHex(RgbB(kv) & 15));
    }
    fprintf(file, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"/>\n",
      (int)coor[i].x1 - xmin, (int)coor[i].y1 - ymin,
      (int)coor[i].x2 - xmin, (int)coor[i].y2 - ymin);
    if (i >= ccoor-1)
      fprintf(file, "</g>\n");
  }
  fprintf(file, "</svg>\n");
}


/*
******************************************************************************
** Create Wireframe
******************************************************************************
*/

// Figure out the color to use for a line.
#define KvLineColor(c, x, y) \
  ((c) != NULL && (c)->FLegal(x, y) ? (c)->Get(x, y) : ds.kvTrim)
#define KvLineColor3(c, x, y, z, t) \
  KvLineColor(c, (t).X2(x, z), (t).Y2(y, z))

// Generate lines in a plane, forming rectangles around on pixels in a bitmap.
// Called from CreateOverviewLine().

void WriteLineLevel(COOR *coor, CONST CMon &b, int height, CONST CCol *c)
{
  int x, y, x1, y1, x2, y2, count, f;

  // For each point, generate up to two lines, one for each axis.
  for (x = 0; x <= b.m_x; x++)
    for (y = 0; y <= b.m_y; y++) {
      f = b.Get(x, y);
      count = f + b.Get(x, y - 1);
      if (count == 1 || count == 2 - ds.fMerge) {
        x1 = ds.hormin + ds.horsiz * x;
        y1 = ds.vermin + ds.versiz * y;
        x2 = x1 + ds.horsiz;
        y2 = y1;
        BiasCoordinates(x, y, ds.horsiz, ds.versiz, ds.horbias, ds.verbias,
          &x1, &y1, &x2, &y2);
        if (x1 != x2)
          WriteCoordinates(coor, x1, y1, height, x2, y1, height,
            KvLineColor(c, x, y - !f));
      }
      count = f + b.Get(x - 1, y);
      if (count == 1 || count == 2 - ds.fMerge) {
        x1 = ds.hormin + ds.horsiz * x;
        y1 = ds.vermin + ds.versiz * y;
        x2 = x1;
        y2 = y1 + ds.versiz;
        BiasCoordinates(x, y, ds.horsiz, ds.versiz, ds.horbias, ds.verbias,
          &x1, &y1, &x2, &y2);
        if (y1 != y2)
          WriteCoordinates(coor, x1, y1, height, x1, y2, height,
            KvLineColor(c, x - !f, y));
      }
    }
}


// Generate a wireframe forming blocks around pixels in a bitmap. Called by
// CreateWireframe for 2D bitmaps.

flag CreateOverviewLine(CONST CMon &b, COOR *coor, CONST CCol *c)
{
  int x, y, x1, y1, x2, y2, count, f, fx, fy, fxy;

  if (FErrorRange("X block size", ds.horsiz, 1, 1000))
    return fFalse;
  if (FErrorRange("Y block size", ds.versiz, 1, 1000))
    return fFalse;
  if (FErrorRange("Block height", ds.depsiz, 0, 1000))
    return fFalse;
  if (FErrorRange("X thickness bias", ds.horbias, 0, ds.horsiz*2 - 1))
    return fFalse;
  if (FErrorRange("Y thickness bias", ds.verbias, 0, ds.versiz*2 - 1))
    return fFalse;

  // Generate lines for the top and bottom of blocks.
  WriteLineLevel(coor, b, ds.depmin, c);
  if (ds.depsiz == 0)
    return fTrue;
  WriteLineLevel(coor, b, ds.depmin + ds.depsiz, c);

  // Generate lines through the z-axis from the top to bottom of blocks.
  for (y = 0; y <= b.m_y; y++)
    for (x = 0; x <= b.m_x; x++) {
      f   = b.Get(x,   y);
      fx  = b.Get(x-1, y);
      fy  = b.Get(x,   y-1);
      fxy = b.Get(x-1, y-1);
      count = f + fx + fy + fxy;
      if (FOdd(count) || (count == 2 && f == fxy) ||
        (!ds.fMerge && count > 0)) {
        x1 = ds.hormin + ds.horsiz*x;
        y1 = ds.vermin + ds.versiz*y;
        x2 = x1;
        y2 = y1;
        BiasCoordinates(x, y, ds.horsiz, ds.versiz, ds.horbias, ds.verbias,
          &x1, &y1, &x2, &y2);
        WriteCoordinates(coor,
          x1, y1, ds.depmin, x1, y1, ds.depmin + ds.depsiz,
            KvLineColor(c, x - (!f && (fx || !fy)), y - (!f && (fy || !fx))));
      }
    }
  return fTrue;
}


// Generate a wireframe forming blocks around pixels in a 3D bitmap. Called by
// CreateWireframe for 3D bitmaps.

flag CreateCubicLine(CONST CMon3 &t, COOR *coor, CONST CCol *c)
{
  int x, y, z, x1, y1, z1, count, f, fx, fy, fz;
  flag fTest;

  if (FErrorRange("Block length", ds.horsiz, 1, 1000))
    return fFalse;
  if (FErrorRange("Block width", ds.versiz, 1, 1000))
    return fFalse;
  if (FErrorRange("Block depth", ds.depsiz, 1, 1000))
    return fFalse;

  // For each point, generate up to three lines, one for each axis.
  for (y = 0; y <= t.m_x3; y++)
    for (z = 0; z <= t.m_z3; z++)
      for (x = 0; x <= t.m_y3; x++) {
        x1 = ds.hormin + ds.horsiz*x;
        y1 = ds.vermin + ds.versiz*(t.m_y3-y);
        z1 = ds.depmin + ds.depsiz*(t.m_z3-z);
        f  = t.Get3(x,   y,   z);
        fx = t.Get3(x-1, y,   z);
        fy = t.Get3(x,   y-1, z);
        fz = t.Get3(x,   y,   z-1);
        count = f + fx + fy + t.Get3(x - 1, y - 1, z);
        fTest = t.Get3(x - 1, y - 1, z) == f;
        if (FOdd(count) || (count == 2 && fTest) || (!ds.fMerge && count > 0))
          WriteCoordinates(coor, x1, y1, z1, x1, y1, z1 - ds.depsiz,
            KvLineColor3(c,
            x - (!f && (fx || !fy)), y - (!f && (fy || !fx)), z, t));
        count = f + fx + fz + t.Get3(x - 1, y, z - 1);
        fTest = t.Get3(x - 1, y, z - 1) == f;
        if (FOdd(count) || (count == 2 && fTest) || (!ds.fMerge && count > 0))
          WriteCoordinates(coor, x1, y1, z1, x1, y1 - ds.versiz, z1,
            KvLineColor3(c,
            x - (!f && (fx || !fz)), y, z - (!f && (fz || !fx)), t));
        count = f + fy + fz + t.Get3(x, y - 1, z - 1);
        fTest = t.Get3(x, y - 1, z - 1) == f;
        if (FOdd(count) || (count == 2 && fTest) || (!ds.fMerge && count > 0))
          WriteCoordinates(coor, x1, y1, z1, x1 + ds.horsiz, y1, z1,
            KvLineColor3(c,
            x, y - (!f && (fy || !fz)), z - (!f && (fz || !fy)), t));
      }
  return fTrue;
}


// Create a wireframe forming blocks around pixels in a bitmap, in a new line
// list. Implements the Make Wireframe Bitmap Overview command.

long CreateWireframe(CONST CMon3 &b, COOR **pcoor, flag f3D, CONST CCol *c)
{
  COOR *coor;

  // Figure out how many lines will be in the wireframe.
  ds.cCoorPatch = 0;
  if (!f3D)
    CreateOverviewLine(b, NULL, c);
  else
    CreateCubicLine(b, NULL, c);

  // Create a line list of appropriate size.
  coor = RgAllocate(ds.cCoorPatch, COOR);
  if (coor == NULL)
    return -1;
  if (*pcoor != NULL)
    DeallocateP(*pcoor);
  *pcoor = coor;

  // Actually generate the wireframe.
  ds.cCoorPatch = 0;
  if (!f3D)
    CreateOverviewLine(b, coor, c);
  else
    CreateCubicLine(b, coor, c);
  return ds.cCoorPatch;
}


// Change the size of the wireframe coordinate list memory allocation.

flag FSetWireSize(COOR **pcoor, int ccoorOld, int ccoorNew)
{
  COOR *coor;

  coor = (COOR *)ReallocateArray(*pcoor, ccoorOld, sizeof(COOR), ccoorNew);
  if (coor == NULL)
    return fFalse;
  if (*pcoor != NULL)
    DeallocateP(*pcoor);
  *pcoor = coor;
  return fTrue;
}


/*
******************************************************************************
** Create Patches
******************************************************************************
*/

// Figure out the color to use for a patch.
#define KvPatchColor(c, x, y) \
  ((c) != NULL && (c)->FLegal(x, y) ? (c)->Get(x, y) : ds.kvObject)
#define KvPatchColor3(c, x, y, z, t) \
  KvPatchColor(c, (t).X2(x, z), (t).Y2(y, z))

// Generate patches in a plane, forming rectangles corresponding to on pixels
// in a bitmap. Called from CreateOverviewPatch().

void WritePatchLevel(PATCH *patch, CONST CMon &b, int height, flag fReflect,
  CONST CCol *c)
{
  int x, y, x1, y1, x2, y2;
  PATN pat[cPatch];
  KV kv;

  pat[0].z = pat[1].z = pat[2].z = pat[3].z = height;
  for (y = 0; y < b.m_y; y++)
    for (x = 0; x < b.m_x; x++)
      if (b.Get(x, y)) {

        // For each on pixel, generate a rectangular patch for it.
        kv = KvPatchColor(c, x, y);
        x1 = ds.hormin + ds.horsiz * x;
        y1 = ds.vermin + ds.versiz * y;
        x2 = x1 + ds.horsiz;
        y2 = y1 + ds.versiz;
        BiasCoordinates(x, y, ds.horsiz, ds.versiz, ds.horbias, ds.verbias,
          &x1, &y1, &x2, &y2);
        pat[0].x = x1; pat[0].y = y1;
        pat[1].x = x1; pat[1].y = y2;
        pat[2].x = x2; pat[2].y = y2;
        pat[3].x = x2; pat[3].y = y1;
        pat[0].fLine = !ds.fMerge || !b.Get(x-1, y);
        pat[1].fLine = !ds.fMerge || !b.Get(x, y+1);
        pat[2].fLine = !ds.fMerge || !b.Get(x+1, y);
        pat[3].fLine = !ds.fMerge || !b.Get(x, y-1);
        WritePatch(patch, pat, fReflect, kv);
      }
}


// Generate patches forming blocks around pixels in a bitmap. Called by
// CreatePatches for 2D bitmaps.

void CreateOverviewPatch(CONST CMon &b, PATCH *patch, CONST CCol *c)
{
  int x, y, x1, y1, x2, y2;
  PATN pat[cPatch];
  KV kv;

  // Generate patches for the top and bottom of blocks.
  if (ds.horbias != 0 && ds.verbias != 0)
    WritePatchLevel(patch, b, ds.depmin, fFalse, c);
  if (ds.depsiz == 0)
    return;
  if (ds.horbias != 0 && ds.verbias != 0)
    WritePatchLevel(patch, b, ds.depmin + ds.depsiz, fTrue, c);

  // Generate patches through the z-axis from the top to bottom of blocks.
  pat[0].z = pat[3].z = ds.depmin + ds.depsiz;
  pat[1].z = pat[2].z = ds.depmin;
  pat[1].fLine = pat[3].fLine = fTrue;
  for (y = 0; y <= b.m_y; y++)
    for (x = 0; x <= b.m_x; x++) {
      x1 = ds.hormin + ds.horsiz*x;
      y1 = ds.vermin + ds.versiz*y;
      x2 = x1 + ds.horsiz;
      y2 = y1 + ds.versiz;
      BiasCoordinates(x, y, ds.horsiz, ds.versiz, ds.horbias, ds.verbias,
        &x1, &y1, &x2, &y2);

      // For each point, generate up to two patches, one for the face
      // perpendicular to each axis.
      if (b.Get(x, y) != b.Get(x, y-1) && x1 != x2) {
        kv = KvPatchColor(c, x, y - b.Get(x, y-1));
        pat[0].x = pat[1].x = x1;
        pat[2].x = pat[3].x = x2;
        pat[0].y = pat[1].y = pat[2].y = pat[3].y = y1;
        pat[0].fLine = !ds.fMerge || b.Get(x, y) == b.Get(x-1, y-1) ||
          b.Get(x, y-1) == b.Get(x-1, y);
        pat[2].fLine = !ds.fMerge || b.Get(x, y) == b.Get(x+1, y-1) ||
          b.Get(x, y-1) == b.Get(x+1, y);
        WritePatch(patch, pat, b.Get(x, y-1), kv);
      }
      if (b.Get(x, y) != b.Get(x-1, y) && y1 != y2) {
        kv = KvPatchColor(c, x - b.Get(x-1, y), y);
        pat[0].x = pat[1].x = pat[2].x = pat[3].x = x1;
        pat[0].y = pat[1].y = y1;
        pat[2].y = pat[3].y = y2;
        pat[0].fLine = !ds.fMerge || b.Get(x, y) == b.Get(x-1, y-1) ||
          b.Get(x-1, y) == b.Get(x, y-1);
        pat[2].fLine = !ds.fMerge || b.Get(x, y) == b.Get(x-1, y+1) ||
          b.Get(x-1, y) == b.Get(x, y+1);
        WritePatch(patch, pat, b.Get(x, y), kv);
      }
    }
}


// Generate patches forming blocks around pixels in a 3D bitmap. Called by
// CreatePatches for 3D bitmaps.

void CreateCubicPatch(CONST CMon3 &t, PATCH *patch, CONST CCol *c)
{
  int x, y, z, x1, y1, z1, x2, y2, z2, i;
  PATN pat[cPatch];
  KV kv;

  for (y = 0; y <= t.m_y3; y++) {
    y1 = ds.vermin + ds.versiz*(t.m_y3-y);
    y2 = y1 - ds.versiz;
    for (z = 0; z <= t.m_z3; z++) {
      z1 = ds.depmin + ds.depsiz*(t.m_z3-z);
      z2 = z1 - ds.depsiz;
      for (x = 0; x <= t.m_x3; x++) {
        x1 = ds.hormin + ds.horsiz*x;
        x2 = x1 + ds.horsiz;

        // For each point, generate up to three patches, one for the face
        // perpendicular to each axis.
        if (t.Get3(x, y, z) != t.Get3(x, y, z-1)) {
          kv = KvPatchColor3(c, x, y, z - t.Get3(x, y, z-1), t);
          for (i = 0; i < 4; i++) {
            pat[i].x = x1; pat[i].y = y1; pat[i].z = z1;
          }
          pat[2].x = pat[3].x = x2;
          pat[1].y = pat[2].y = y2;
          pat[0].fLine = !ds.fMerge || t.Get3(x, y, z) != t.Get3(x-1, y, z) ||
            t.Get3(x, y, z-1) != t.Get3(x-1, y, z-1);
          pat[1].fLine = !ds.fMerge || t.Get3(x, y, z) != t.Get3(x, y+1, z) ||
            t.Get3(x, y, z-1) != t.Get3(x, y+1, z-1);
          pat[2].fLine = !ds.fMerge || t.Get3(x, y, z) != t.Get3(x+1, y, z) ||
            t.Get3(x, y, z-1) != t.Get3(x+1, y, z-1);
          pat[3].fLine = !ds.fMerge || t.Get3(x, y, z) != t.Get3(x, y-1, z) ||
            t.Get3(x, y, z-1) != t.Get3(x, y-1, z-1);
          WritePatch(patch, pat, t.Get3(x, y, z-1), kv);
        }
        if (t.Get3(x, y, z) != t.Get3(x-1, y, z)) {
          kv = KvPatchColor3(c, x - t.Get3(x-1, y, z), y, z, t);
          for (i = 0; i < 4; i++) {
            pat[i].x = x1; pat[i].y = y1; pat[i].z = z1;
          }
          pat[2].y = pat[3].y = y2;
          pat[1].z = pat[2].z = z2;
          pat[0].fLine = !ds.fMerge || t.Get3(x, y, z) != t.Get3(x, y-1, z) ||
            t.Get3(x-1, y, z) != t.Get3(x-1, y-1, z);
          pat[1].fLine = !ds.fMerge || t.Get3(x, y, z) != t.Get3(x, y, z+1) ||
            t.Get3(x-1, y, z) != t.Get3(x-1, y, z+1);
          pat[2].fLine = !ds.fMerge || t.Get3(x, y, z) != t.Get3(x, y+1, z) ||
            t.Get3(x-1, y, z) != t.Get3(x-1, y+1, z);
          pat[3].fLine = !ds.fMerge || t.Get3(x, y, z) != t.Get3(x, y, z-1) ||
            t.Get3(x-1, y, z) != t.Get3(x-1, y, z-1);
          WritePatch(patch, pat, t.Get3(x-1, y, z), kv);
        }
        if (t.Get3(x, y, z) != t.Get3(x, y-1, z)) {
          kv = KvPatchColor3(c, x, y - t.Get3(x, y-1, z), z, t);
          for (i = 0; i < 4; i++) {
            pat[i].x = x1; pat[i].y = y1; pat[i].z = z1;
          }
          pat[2].x = pat[3].x = x2;
          pat[1].z = pat[2].z = z2;
          pat[0].fLine = !ds.fMerge || t.Get3(x, y, z) != t.Get3(x-1, y, z) ||
            t.Get3(x, y-1, z) != t.Get3(x-1, y-1, z);
          pat[1].fLine = !ds.fMerge || t.Get3(x, y, z) != t.Get3(x, y, z+1) ||
            t.Get3(x, y-1, z) != t.Get3(x, y-1, z+1);
          pat[2].fLine = !ds.fMerge || t.Get3(x, y, z) != t.Get3(x+1, y, z) ||
            t.Get3(x, y-1, z) != t.Get3(x+1, y-1, z);
          pat[3].fLine = !ds.fMerge || t.Get3(x, y, z) != t.Get3(x, y, z-1) ||
            t.Get3(x, y-1, z) != t.Get3(x, y-1, z-1);
          WritePatch(patch, pat, t.Get3(x, y, z), kv);
        }
      }
    }
  }
}


// Create patches forming blocks around pixels in a bitmap, in a new patch
// list. Implements the Make Patch Bitmap Overview command.

long CreatePatches(CONST CMon3 &b, PATCH **ppatch, flag f3D, CONST CCol *c)
{
  PATCH *patch;

  // Figure out how many patches will be in the patch list.
  ds.cCoorPatch = 0;
  if (!f3D)
    CreateOverviewPatch(b, NULL, c);
  else
    CreateCubicPatch(b, NULL, c);

  // Create a patch list of appropriate size.
  patch = RgAllocate(ds.cCoorPatch, PATCH);
  if (patch == NULL)
    return -1;
  if (*ppatch != NULL)
    DeallocateP(*ppatch);
  *ppatch = patch;

  // Actually generate the patches.
  ds.cCoorPatch = 0;
  if (!f3D)
    CreateOverviewPatch(b, patch, c);
  else
    CreateCubicPatch(b, patch, c);
  return ds.cCoorPatch;
}


// Change the size of the patch list memory allocation.

flag FSetPatchSize(PATCH **ppatch, int cpatchOld, int cpatchNew)
{
  PATCH *patch;

  patch = (PATCH *)ReallocateArray(*ppatch, cpatchOld, sizeof(PATCH),
    cpatchNew);
  if (patch == NULL)
    return fFalse;
  if (*ppatch != NULL)
    DeallocateP(*ppatch);
  *ppatch = patch;
  return fTrue;
}

/* draw.cpp */
