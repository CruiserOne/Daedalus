/*
** Daedalus (Version 3.4) File: inside.cpp
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
** This file contains routines to draw 3D first person perspective views
** from inside a Maze's passages.
**
** Created: 6/2/2001.
** Last code change: 8/29/2023.
*/

#include <stdio.h>
#include <math.h>

#include "resource.h"
#include "util.h"
#include "graphics.h"
#include "color.h"
#include "threed.h"
#include "maze.h"
#include "draw.h"
#include "daedalus.h"


// Max number of semitransparent walls in line with each other.
#define itransMax 1000

#define iScale 7
#define iStep (1 << iScale)
#define iRound (iStep - 1)
#define IO(y) ((y) >> iScale)
#define IP(y) ((y) << iScale)

#define LineITrans(yHi, yLo, kv, nTrans) \
  LineYTrans(c, x, IO(yHi), IO(yLo), kv, nTrans);


/*
******************************************************************************
** Inside Display Routines
******************************************************************************
*/

// Convert a high-res inside coordinate to an low-res array coordinate.

int AFromInside(long z)
{
  long i;

  if (dr.fNarrow) {
    if (z >= 0)
      i = z / (dr.zCell + dr.zCellNarrow);
    else
      i = (z + 1) / (dr.zCell + dr.zCellNarrow) - 1;
    return (int)(i * 2 +
      (z - i * (dr.zCell + dr.zCellNarrow) >= dr.zCellNarrow));
  }
  return (int)(z / dr.zCell - (z < 0));
}


// Convert a low-res array coordinate to a high-res inside coordinate, such
// that one's positioned in the middle of a cell.

long InsideFromA(int z)
{
  if (dr.fNarrow)
    return (z - (z < 0)) / 2 * (dr.zCell + dr.zCellNarrow) +
      (FOdd(z) ? dr.zCellNarrow + (dr.zCell >> 1) : (dr.zCellNarrow >> 1));
  return z * dr.zCell + (dr.zCell >> 1);
}


// Convert a low-res array coordinate to a high-res inside coordinate, such
// that one's positioned at the top left corner of a cell.

long InsideFromA0(int z)
{
  if (dr.fNarrow)
    return (z - (z < 0)) / 2 * (dr.zCell + dr.zCellNarrow) +
      (FOdd(z) * dr.zCellNarrow);
  return z * dr.zCell;
}


// Draw a gradient color vertical line on a color bitmap. Used for drawing
// background mountains in inside display modes.

void LineYGradient(CMazK &c, int x, int y1, int y2, KV kv1, KV kv2)
{
  byte *pb;
  int yClip1 = y1, yClip2 = y2, db, y,
    nR1, nG1, nB1, nR2, nG2, nB2, nRT, nGT, nBT, dR, dG, dB;

  // Don't worry about pixels that are off the bitmap.
  if (yClip1 < 0)
    yClip1 = 0;
  if (yClip2 > c.m_y)
    yClip2 = c.m_y;
  if (yClip1 < dr.yElevMin)
    yClip1 = dr.yElevMin;
  if (yClip2 > dr.yElevMax)
    yClip2 = dr.yElevMax;
  if (yClip2 <= yClip1)
    return;

  // Quickly draw the line, where things like the memory address only need to
  // be calculated once.
  nR1 = RgbR(kv1); nG1 = RgbG(kv1); nB1 = RgbB(kv1);
  pb = c._Pb(x, yClip1);
  db = c.m_clRow << 2;
  if (kv2 < 0) {

    // If no gradient, just draw a solid color line.
    for (y = yClip1; y < yClip2; y++) {
      c._Set(pb, nR1, nG1, nB1);
      pb += db;
    }
  } else {

    // If gradient, gradually blend color from color #1 to color #2.
    nR2 = RgbR(kv2); nG2 = RgbG(kv2); nB2 = RgbB(kv2);
    nRT = nR1 << 16; nGT = nG1 << 16; nBT = nB1 << 16;
    dR = ((nR2 - nR1) << 16) / (y2 - y1);
    dG = ((nG2 - nG1) << 16) / (y2 - y1);
    dB = ((nB2 - nB1) << 16) / (y2 - y1);
    if (y1 < yClip1) {
      nRT += dR * (yClip1 - y1);
      nGT += dG * (yClip1 - y1);
      nBT += dB * (yClip1 - y1);
    }
    for (y = yClip1; y < yClip2; y++) {
      c._Set(pb, nRT >> 16, nGT >> 16, nBT >> 16);
      nRT += dR; nGT += dG; nBT += dB;
      pb += db;
    }
  }
}


// Draw a solid color vertical line on a color bitmap. Used for drawing walls
// in inside display modes. Walls, floor markings, and mountains are all
// generated by drawing a sequence of vertical lines of appropriate heights.
// In contrast, gradient backgrounds and clouds are all generated by drawing a
// sequence of horizontal lines at appropriate locations.

void LineYTrans(CMazK &c, int x, int y1, int y2, KV kv, int nTrans)
{
  byte *pb;
  int db, y, nR, nG, nB, nRT, nGT, nBT;

  // Don't worry about pixels that are off the bitmap.
  if (y1 < 0)
    y1 = 0;
  if (y2 > c.m_y)
    y2 = c.m_y;
  if (y1 < dr.yElevMin)
    y1 = dr.yElevMin;
  if (y2 > dr.yElevMax)
    y2 = dr.yElevMax;

  // Quickly draw the line, where things like the memory address only need to
  // be calculated once.
  nR = RgbR(kv); nG = RgbG(kv); nB = RgbB(kv);
  pb = c._Pb(x, y1);
  db = c.m_clRow << 2;
  if (nTrans == 0) {

    // If no semitransparency, just draw a solid color line.
    for (y = y1; y < y2; y++) {
      c._Set(pb, nR, nG, nB);
      pb += db;
    }
  } else {

    // If semitransparent, blend color with what's already in place.
    nTrans = nTrans * 128 / 100;
    for (y = y1; y < y2; y++) {
      c._Get(pb, &nRT, &nGT, &nBT);
      nRT = nR + ((nRT - nR) * nTrans >> 7);
      nGT = nG + ((nGT - nG) * nTrans >> 7);
      nBT = nB + ((nBT - nB) * nTrans >> 7);
      c._Set(pb, nRT, nGT, nBT);
      pb += db;
    }
  }
}


// Draw a texture mapped vertical line on a color bitmap. Used for drawing
// walls in inside display modes.

void LineYTexture(
  CMazK &cDst, int xDst, int yDst1, int yDst2, int yClip1, int yClip2,
  CMazK &cSrc, int xSrc, int ySrc1, int ySrc2,
  int nTrans, KV kvFog, int nFog, KV kvWall)
{
  int dyDst = yDst2 - yDst1, dySrc = ySrc2 - ySrc1, dbDst, dbSrc,
    y1 = yClip1, y2 = yClip2, y, yT, dy,
    nRF, nGF, nBF, nR, nG, nB, nRT, nGT, nBT, nRW, nGW, nBW;
  byte *pbDst, *pbSrc, *pbBase;

  // Don't worry about pixels that are off the bitmap.
  if (y1 < 0)
    y1 = 0;
  if (y2 > cDst.m_y)
    y2 = cDst.m_y;
  if (y1 < dr.yElevMin)
    y1 = dr.yElevMin;
  if (y2 > dr.yElevMax)
    y2 = dr.yElevMax;

  pbDst = cDst._Pb(xDst, y1);
  pbBase = cSrc._Pb(xSrc, 0);
  dbDst = cDst.m_clRow << 2;
  dbSrc = cSrc.m_clRow << 2;
  if (dyDst == 0)
    dyDst = 1;
  yT = (y1 - yDst1) * dySrc;
  if ((yT << 10) >> 10 != yT)
    return;
  yT = ((ySrc1 << 16) + (yT << 10) / dyDst) << 6;
  if (yT < 0)
    return;
  dy = (dySrc << 16) / dyDst;

  if (nFog > 0) {
    nFog = nFog * 128 / 100;
    nRF = RgbR(kvFog); nGF = RgbG(kvFog); nBF = RgbB(kvFog);
  }
  if (nTrans != 0)
    nTrans = nTrans * 128 / 100;
  if (!dr.fTextureBlend)
    kvWall = ~0;
  else if (kvWall != ~0) {
    nRW = RgbR(kvWall); nGW = RgbG(kvWall); nBW = RgbB(kvWall);
  }

  // The texture isn't just drawn as is, but may be blended with a fixed
  // color, may be transparent or blended with the background, and finally
  // may be blended in a specified proportion with a fog color. This is a
  // low level function called in tight loops, so needs to be super fast.
  // Therefore each combination of potential blends is handled separately.

  switch (((kvWall != ~0) << 2) + ((nTrans != 0) << 1) + (nFog > 0)) {

  // Color blend: no, Transparency: no, Fog: no.
  case 0:
    for (y = y1; y < y2; y++) {
      pbSrc = pbBase + (yT >> 16) * dbSrc;
      cSrc._Get(pbSrc, &nR, &nG, &nB);
      cDst._Set(pbDst, nR, nG, nB);
      yT += dy;
      pbDst += dbDst;
    }
    break;

  // Color blend: no, Transparency: no, Fog: YES.
  case 1:
    for (y = y1; y < y2; y++) {
      pbSrc = pbBase + (yT >> 16) * dbSrc;
      cSrc._Get(pbSrc, &nR, &nG, &nB);
      nR += ((nRF - nR) * nFog >> 7);
      nG += ((nGF - nG) * nFog >> 7);
      nB += ((nBF - nB) * nFog >> 7);
      cDst._Set(pbDst, nR, nG, nB);
      yT += dy;
      pbDst += dbDst;
    }
    break;

  // Color blend: no, Transparency: YES, Fog: no.
  case 2:
    for (y = y1; y < y2; y++) {
      pbSrc = pbBase + (yT >> 16) * dbSrc;
      cSrc._Get(pbSrc, &nR, &nG, &nB);
      cDst._Get(pbDst, &nRT, &nGT, &nBT);
      nR += ((nRT - nR) * nTrans >> 7);
      nG += ((nGT - nG) * nTrans >> 7);
      nB += ((nBT - nB) * nTrans >> 7);
      cDst._Set(pbDst, nR, nG, nB);
      yT += dy;
      pbDst += dbDst;
    }
    break;

  // Color blend: no, Transparency: YES, Fog: YES.
  case 3:
    for (y = y1; y < y2; y++) {
      pbSrc = pbBase + (yT >> 16) * dbSrc;
      cSrc._Get(pbSrc, &nR, &nG, &nB);
      nR += ((nRF - nR) * nFog >> 7);
      nG += ((nGF - nG) * nFog >> 7);
      nB += ((nBF - nB) * nFog >> 7);
      cDst._Get(pbDst, &nRT, &nGT, &nBT);
      nR += ((nRT - nR) * nTrans >> 7);
      nG += ((nGT - nG) * nTrans >> 7);
      nB += ((nBT - nB) * nTrans >> 7);
      cDst._Set(pbDst, nR, nG, nB);
      yT += dy;
      pbDst += dbDst;
    }
    break;

  // Color blend: YES, Transparency: no, Fog: no.
  case 4:
    for (y = y1; y < y2; y++) {
      pbSrc = pbBase + (yT >> 16) * dbSrc;
      cSrc._Get(pbSrc, &nR, &nG, &nB);
      nR = (nR + nRW) >> 1;
      nG = (nG + nGW) >> 1;
      nB = (nB + nBW) >> 1;
      cDst._Set(pbDst, nR, nG, nB);
      yT += dy;
      pbDst += dbDst;
    }
    break;

  // Color blend: YES, Transparency: no, Fog: YES.
  case 5:
    for (y = y1; y < y2; y++) {
      pbSrc = pbBase + (yT >> 16) * dbSrc;
      cSrc._Get(pbSrc, &nR, &nG, &nB);
      nR = (nR + nRW) >> 1;
      nG = (nG + nGW) >> 1;
      nB = (nB + nBW) >> 1;
      nR += ((nRF - nR) * nFog >> 7);
      nG += ((nGF - nG) * nFog >> 7);
      nB += ((nBF - nB) * nFog >> 7);
      cDst._Set(pbDst, nR, nG, nB);
      yT += dy;
      pbDst += dbDst;
    }
    break;

  // Color blend: YES, Transparency: YES, Fog: no.
  case 6:
    for (y = y1; y < y2; y++) {
      pbSrc = pbBase + (yT >> 16) * dbSrc;
      cSrc._Get(pbSrc, &nR, &nG, &nB);
      cDst._Get(pbDst, &nRT, &nGT, &nBT);
      nR = (nR + nRW) >> 1;
      nG = (nG + nGW) >> 1;
      nB = (nB + nBW) >> 1;
      nR += ((nRT - nR) * nTrans >> 7);
      nG += ((nGT - nG) * nTrans >> 7);
      nB += ((nBT - nB) * nTrans >> 7);
      cDst._Set(pbDst, nR, nG, nB);
      yT += dy;
      pbDst += dbDst;
    }
    break;

  // Color blend: YES, Transparency: YES, Fog: YES.
  case 7:
    for (y = y1; y < y2; y++) {
      pbSrc = pbBase + (yT >> 16) * dbSrc;
      cSrc._Get(pbSrc, &nR, &nG, &nB);
      nR = (nR + nRW) >> 1;
      nG = (nG + nGW) >> 1;
      nB = (nB + nBW) >> 1;
      nR += ((nRF - nR) * nFog >> 7);
      nG += ((nGF - nG) * nFog >> 7);
      nB += ((nBF - nB) * nFog >> 7);
      cDst._Get(pbDst, &nRT, &nGT, &nBT);
      nR += ((nRT - nR) * nTrans >> 7);
      nG += ((nGT - nG) * nTrans >> 7);
      nB += ((nBT - nB) * nTrans >> 7);
      cDst._Set(pbDst, nR, nG, nB);
      yT += dy;
      pbDst += dbDst;
    }
    break;
  }
}


// Draw a texture map overlaying a vertical line on a color bitmap. The
// overlay shape comes from a monochrome bitmap, where optional coloring comes
// from another color bitmap.

void LineYMask(
  CMazK &cDst, int xDst, int yDst1, int yDst2, int yClip1, int yClip2,
  CMaz *b, CMazK *cSrc, int xSrc, int ySrc1, int ySrc2,
  KV kvFog, int nFog, flag fSet)
{
  int dyDst = yDst2 - yDst1, dySrc = ySrc2 - ySrc1, db,
    y1 = yClip1, y2 = yClip2, y, yT, dy, nRF, nGF, nBF, nR, nG, nB;
  byte *pbDst, *pbSrc;

  // Don't worry about pixels that are off the bitmap.
  if (y1 < 0)
    y1 = 0;
  if (y2 > cDst.m_y)
    y2 = cDst.m_y;
  if (y1 < dr.yElevMin)
    y1 = dr.yElevMin;
  if (y2 > dr.yElevMax)
    y2 = dr.yElevMax;

  pbDst = cDst._Pb(xDst, y1);
  db = cDst.m_clRow << 2;
  if (dyDst == 0)
    dyDst = 1;
  yT = (y1 - yDst1) * dySrc;
  if ((yT << 10) >> 10 != yT)
    return;
  yT = ((ySrc1 << 16) + (yT << 10) / dyDst) << 6;
  if (yT < 0)
    return;
  dy = (dySrc << 16) / dyDst;
  if (cSrc == NULL) {

    // Draw either black or white pixels on top of what's already present.
    nR = nG = nB = fSet * 255;
    if (nFog > 0 && dr.fFogFloor) {
      nFog = nFog * 128 / 100;
      nRF = RgbR(kvFog); nGF = RgbG(kvFog); nBF = RgbB(kvFog);
      nR += ((nRF - nR) * nFog >> 7);
      nG += ((nGF - nG) * nFog >> 7);
      nB += ((nBF - nB) * nFog >> 7);
    }
    for (y = y1; y < y2; y++) {
      if (b->_Get(xSrc, yT >> 16) == fSet)
        cDst._Set(pbDst, nR, nG, nB);
      yT += dy;
      pbDst += db;
    }
  } else {
    if (nFog <= 0 || !dr.fFogFloor) {

      // Draw a color mask from a texture on top of what's already present.
      for (y = y1; y < y2; y++) {
        if (b->_Get(xSrc, yT >> 16) == fSet) {
          pbSrc = cSrc->_Pb(xSrc, yT >> 16);
          cSrc->_Get(pbSrc, &nR, &nG, &nB);
          cDst._Set(pbDst, nR, nG, nB);
        }
        yT += dy;
        pbDst += db;
      }
    } else {

      // Draw a color mask from a texture bitmap, with fog.
      nFog = nFog * 128 / 100;
      nRF = RgbR(kvFog); nGF = RgbG(kvFog); nBF = RgbB(kvFog);
      for (y = y1; y < y2; y++) {
        if (b->_Get(xSrc, yT >> 16) == fSet) {
          pbSrc = cSrc->_Pb(xSrc, yT >> 16);
          cSrc->_Get(pbSrc, &nR, &nG, &nB);
          nR += ((nRF - nR) * nFog >> 7);
          nG += ((nGF - nG) * nFog >> 7);
          nB += ((nBF - nB) * nFog >> 7);
          cDst._Set(pbDst, nR, nG, nB);
        }
        yT += dy;
        pbDst += db;
      }
    }
  }
}


// Draw a variable color vertical line on a color bitmap. Used for drawing
// floor markings in inside display modes.

void LineYFloor(CMazK &c, int x, int yb, int dy1, int dy2,
  KV kv, int nTrans, flag fFog, real rScale)
{
  byte *pb;
  int y1 = yb + dy1, y2 = yb + dy2, db, y, n,
    nRF, nGF, nBF, nR, nG, nB, nRT, nGT, nBT, nRO, nGO, nBO;

  // Don't worry about pixels that are off the bitmap.
  if (y1 < 0)
    y1 = 0;
  if (y2 > c.m_y)
    y2 = c.m_y;
  if (y1 < dr.yElevMin)
    y1 = dr.yElevMin;
  if (y2 > dr.yElevMax)
    y2 = dr.yElevMax;

  // Quickly draw the line, where things like the memory address only need to
  // be calculated once.
  nR = RgbR(kv); nG = RgbG(kv); nB = RgbB(kv);
  pb = c._Pb(x, y1);
  db = c.m_clRow << 2;
  if (nTrans == 0) {
    if (!fFog) {

      // If no fog, just draw a solid color line.
      for (y = y1; y < y2; y++) {
        c._Set(pb, nR, nG, nB);
        pb += db;
      }
    } else {

      // If fog, blend each pixel with the fog color based on its distance.
      nRF = RgbR(dr.kvInFog); nGF = RgbG(dr.kvInFog); nBF = RgbB(dr.kvInFog);
      rScale = rScale / (real)(dr.nFog * 10) * (dy1 >= 0 ? 128.0 : -128.0);
      for (y = y1; y < y2; y++) {
        n = y-yb;
        n = IO((int)rScale / No0(n));
        if ((uint)n < 128) {
          nRT = nR + ((nRF - nR) * n >> 7);
          nGT = nG + ((nGF - nG) * n >> 7);
          nBT = nB + ((nBF - nB) * n >> 7);
          c._Set(pb, nRT, nGT, nBT);
        } else
          c._Set(pb, nRF, nGF, nBF);
        pb += db;
      }
    }
  } else {
    nTrans = nTrans * 128 / 100;
    if (!fFog) {

      // If no fog semitransparent, blend color with what's already in place.
      for (y = y1; y < y2; y++) {
        c._Get(pb, &nRT, &nGT, &nBT);
        nRT = nR + ((nRT - nR) * nTrans >> 7);
        nGT = nG + ((nGT - nG) * nTrans >> 7);
        nBT = nB + ((nBT - nB) * nTrans >> 7);
        c._Set(pb, nRT, nGT, nBT);
        pb += db;
      }
    } else {

      // If fog semitransparent, blend with the fog color based on distance.
      nRF = RgbR(dr.kvInFog); nGF = RgbG(dr.kvInFog); nBF = RgbB(dr.kvInFog);
      rScale = rScale / (real)(dr.nFog * 10) * (dy1 >= 0 ? 128.0 : -128.0);
      for (y = y1; y < y2; y++) {
        n = y-yb;
        n = IO((int)rScale / No0(n));
        if ((uint)n < 128) {
          nRT = nR + ((nRF - nR) * n >> 7);
          nGT = nG + ((nGF - nG) * n >> 7);
          nBT = nB + ((nBF - nB) * n >> 7);
        } else {
          nRT = nRF; nGT = nGF; nBT = nBF;
        }
        c._Get(pb, &nRO, &nGO, &nBO);
        nRT += ((nRO - nRT) * nTrans >> 7);
        nGT += ((nGO - nGT) * nTrans >> 7);
        nBT += ((nBO - nBT) * nTrans >> 7);
        c._Set(pb, nRT, nGT, nBT);
        pb += db;
      }
    }
  }
}


// Draw a texture mapped vertical line on a color bitmap. Used for drawing
// floor markings in inside display modes.

void LineYFloorTexture(CMazK &cDst, int x, int yb, int dy1, int dy2,
  CMaz *b, CMazK *cSrc, real m1, real n1, real m2, real n2,
  flag fSet, flag fFog, flag fTrans, real rScale)
{
  byte *pb, *pbT;
  int y1 = yb + IO(dy1), y2 = yb + IO(dy2), y, db, n, xT, yT, xMax, yMax,
    nRF, nGF, nBF, nR, nG, nB, nRT, nGT, nBT;
  real rScaleFog, dm, dn, rd1, rd2, rdd, rdy1, rddy, rdy, rdyCur, rdyInc, rT;
  flag fNoFog = !fFog || dr.nFog <= 0 || !dr.fFogFloor;

  // Don't worry about pixels that are off the bitmap.
  rdy = (real)(y2 - y1);
  if (y1 < 0) {
    y1 = 0;
    rdy1 = -IP(yb);
  } else
    rdy1 = (real)dy1;
  if (y2 > cDst.m_y)
    y2 = cDst.m_y;
  if (y1 < dr.yElevMin)
    y1 = dr.yElevMin;
  if (y2 > dr.yElevMax)
    y2 = dr.yElevMax;
  rddy = (real)(dy2 - dy1);

  rd1 = RDiv(rScale, (real)dy1);
  rd2 = RDiv(rScale, (real)dy2);
  rdd = rd2 - rd1;
  if (cSrc == NULL) {
    xMax = b->m_x; yMax = b->m_y;
  } else if (b == NULL) {
    xMax = cSrc->m_x; yMax = cSrc->m_y;
  } else {
    xMax = NMin(b->m_x, cSrc->m_x); yMax = NMin(b->m_y, cSrc->m_y);
  }
  dm = RDiv(m2 - m1, rdd) * (real)xMax;
  dn = RDiv(n2 - n1, rdd) * (real)yMax;
  m1 *= (real)xMax; n1 *= (real)yMax;
  rdyCur = rdy1 > rMinute || rdy1 < -rMinute ? rdy1 :
    (rdy >= 0 ? rMinute : -rMinute);
  rdyInc = rddy / rdy;

  pb = cDst._Pb(x, y1);
  db = cDst.m_clRow << 2;
  if (!fNoFog) {
    nRF = RgbR(dr.kvInFog); nGF = RgbG(dr.kvInFog); nBF = RgbB(dr.kvInFog);
    rScaleFog = rScale / (real)(dr.nFog * 10) * (dy1 >= 0 ? 128.0 : -128.0);
  }
  if (cSrc == NULL) {
    nR = nG = nB = fSet * 255;
    if (!fNoFog) {
      n = y2-yb;
      n = IO((int)rScaleFog / No0(n));
      if ((uint)n < 128) {
        nR += ((nRF - nR) * n >> 7);
        nG += ((nGF - nG) * n >> 7);
        nB += ((nBF - nB) * n >> 7);
      } else {
        nR = nRF; nG = nGF; nB = nBF;
      }
      fNoFog = fTrue;
    }
    fTrans = fFalse;
  }
  if (!dr.fTextureBlend)
    fTrans = fFalse;

  // Texture mapping floors is more complicated than walls. For floors the
  // vertical line crosses the texture at an oblique angle, requiring
  // calculating the texture pixel in both x and y. Also floors are viewed at
  // an oblique angle, so the rate moving across the texture changes.

  for (y = y1; y < y2; y++) {
    rT = (rScale / rdyCur) - rd1;
    xT = (int)(m1 + rT * dm);
    yT = (int)(n1 + rT * dn);
    if ((uint)xT >= (uint)xMax)
      xT = xMax - 1;
    if ((uint)yT >= (uint)yMax)
      yT = yMax - 1;
    if (b != NULL) {
      if (b->_Get(xT, yT) != fSet)
        goto LNext;
      if (cSrc != NULL) {
        pbT = cSrc->_Pb(xT, yT);
        cSrc->_Get(pbT, &nR, &nG, &nB);
      }
    } else {
      pbT = cSrc->_Pb(xT, yT);
      cSrc->_Get(pbT, &nR, &nG, &nB);
    }
    if (fTrans) {
      pbT = cDst._Pb(x, y);
      cDst._Get(pbT, &nRT, &nGT, &nBT);
      nR = (nR + nRT) >> 1;
      nG = (nG + nGT) >> 1;
      nB = (nB + nBT) >> 1;
    }
    if (fNoFog)
      cDst._Set(pb, nR, nG, nB);
    else {
      n = y-yb;
      n = IO((int)rScaleFog / No0(n));
      if ((uint)n < 128) {
        nRT = nR + ((nRF - nR) * n >> 7);
        nGT = nG + ((nGF - nG) * n >> 7);
        nBT = nB + ((nBF - nB) * n >> 7);
        cDst._Set(pb, nRT, nGT, nBT);
      } else
        cDst._Set(pb, nRF, nGF, nBF);
    }
LNext:
    pb += db;
    rdyCur += rdyInc;
  }
}


// Create and draw the stars for the inside display modes.

flag FCreateInsideStars(CMazK *c, real rd, int yAdd, flag fPerspective)
{
  int i, x, y, d;
  real rx, dInside2 = dr.dInside*2.0, rT;
  KV kv;

  // If the list of stars hasn't been generated yet, do so first.
  if (dr.rgstar == NULL) {
    dr.rgstar = RgAllocate(istarMax, STAR);
    if (dr.rgstar == NULL)
      return fFalse;
    for (i = 0; i < istarMax; i++) {
      dr.rgstar[i].x = Rnd(0, 16383);
      dr.rgstar[i].y = Rnd(0, 16383);
      dr.rgstar[i].kv = KvStarRandom();
    }
    if (c == NULL)
      return fTrue;
  }

  // Draw the list of stars in their appropriate locations and colors.
  if (fPerspective)
    rT = RTanD(dr.dInside) * 2.0;
  for (i = 0; i < ds.cStar; i++) {
    rx = (real)dr.rgstar[i].x * rDegMax / 16384.0;
    if ((rd + dInside2 < rDegMax) ?
      (rx > rd && rx < rd + dInside2) :
      (rx > rd || rx < rd + dInside2 - rDegMax)) {
      rx -= rd;
      if (rx < 0.0)
        rx += rDegMax;
      if (fPerspective)
        rx = RTanD(rx - dr.dInside) / rT + 0.5;
      else
        rx /= dInside2;
      x = c->m_x-1 - (int)((real)c->m_x * rx);
      y = yAdd + (int)((real)(c->m_y >> (int)!ds.fSkyAll) *
        (real)dr.rgstar[i].y / 16384.0);
      kv = dr.rgstar[i].kv;
      if (fPerspective || c->Get(x, y) == dr.kvInSky)
        c->Set(x, y, kv);
      if (ds.nStarSize < 0 ? FOdd(i) :
        RgbR(kv) + RgbG(kv) + RgbB(kv) >= ds.nStarSize)
        for (d = 0; d < DIRS; d++)
          if (fPerspective || c->Get(x+xoff[d], y+yoff[d]) == dr.kvInSky)
            c->Set(x+xoff[d], y+yoff[d], kv);
    }
  }
  return fTrue;
}


// Draw a solid circle, with an optional circle shaped bite out of it in a
// different color. Called from DrawSunMoon() to draw the Sun or Moon.

void DrawCrescent(CMazK &c, int x, int y, int r, int x2, int y2,
  KV kv, KV kv2)
{
  int h1 = x-r, v1 = y-r, h2 = x+r, v2 = y+r, h, v;
  long n = Sq(r) + r - 1, l;

  c.Legalize(&h1, &v1);
  c.Legalize(&h2, &v2);
  for (v = v1; v <= v2; v++)
    for (h = h1; h <= h2; h++) {
      l = Sq(v - y) + Sq(h - x);
      if (l <= n) {
        l = Sq(v - y2) + Sq(h - x2);
        if (l > n)
          c.Set(h, v, kv);
        else if (kv2 >= 0)
          c.Set(h, v, kv2);
      }
    }
}


// Draw the Sun or Moon for the inside display mode.

void DrawSunMoon(CMazK &c, real rd, int yAdd)
{
  int x, y, xi, yi, zSun, d;
  real rx = dr.dInside, dInside2 = dr.dInside*2.0;

  zSun = c.m_y / 20;
  if (dr.rLightFactor >= 0.0)
    rx += rDegHalf;
  if (!((rd + dInside2 < rDegMax) ?
    (rx >= rd - 0.9 && rx <= rd + dInside2 + 0.9) :
    (rx >= rd - 0.9 || rx <= rd + dInside2 + 0.9 - rDegMax)))
    return;

  rx -= rd;
  if (rx < 0.0)
    rx += rDegMax;
  rx = RTanD(rx - dr.dInside) / RTanD(dr.dInside) / 2.0 + 0.5;
  x = c.m_x-1 - (int)((real)c.m_x * rx);
  y = yAdd + (c.m_y >> 1) * dr.ySunMoon / 1000;
  if (ds.fStar) {

    // For the Moon, draw a bite out of the circle.
    d = NMultDiv(zSun*2, dr.nSunMoon, 100);
    DrawCrescent(c, x, y, zSun, x-d, y, dr.kvMoon,
      dr.kvInSky2 < 0 || dr.kvInSky2 == dr.kvInSky ? dr.kvInSky : -1);
  } else {

    // For the Sun, draw rays from the circle.
    c.Disk(x-zSun, y-zSun, x+zSun, y+zSun, dr.kvSun);
    zSun += NMultDiv(zSun*2, NAbs(dr.nSunMoon), 100);
    for (d = 0; d < 6; d++) {
      xi = (int)(((real)zSun)*RCosD((real)(d*30)));
      yi = (int)(((real)zSun)*RSinD((real)(d*30)));
      c.Line(x-xi, y-yi, x+xi, y+yi, dr.kvSun);
    }
  }
}


// Draw a rainbow opposite the Sun for the inside display mode.

flag FDrawRainbow(CMazK &c, real rd, int yAdd)
{
  int nRainbow, x, y, y2, m, n, xT, yT, nSav1, nSav2, nR, nG, nB,
    nHi, nLo, nWid, nTrans;
  long l, q1, q2;
  flag fSav;
  real rx = dr.dInside, dInside1 = 89.0, dInside2 = dInside1*2.0;
  KV kv1;
  byte *pb1, *pb2, *pbT;

  rx = dInside1;
  if (dr.rLightFactor < 0.0)
    rx += rDegHalf;
  if (!((rd + dInside2 < rDegMax) ?
    (rx >= rd - 0.9 && rx <= rd + dInside2 + 0.9) :
    (rx >= rd - 0.9 || rx <= rd + dInside2 + 0.9 - rDegMax)))
    return fFalse;

  nRainbow = NAbs(dr.nRainbow);
  nHi = nRainbow / 1000000;
  nLo = (nRainbow / 10000) % 100;
  nWid = 99 - ((nRainbow / 100) % 100);
  nTrans = 99 - (nRainbow % 100);

  rx -= rd;
  if (rx < 0.0)
    rx += rDegMax;
  rx = RTanD(rx - dInside1) / RTanD(dr.dInside) / 2.0 + 0.5;
  x = c.m_x-1 - (int)((real)c.m_x * rx);
  y = (c.m_y >> 1) * nHi / 99;
  if (nTrans <= 0 || x+y < 0 || x-y >= c.m_x)
    return fTrue;

  // If the rainbow pixels haven't been generated yet, do so first.
  if (bm.kR.FNull() || (bm.kR.m_y != y+1 && dr.nRainbow >= 0)) {
    if (!bm.kR.FBitmapSizeSet(y*2+1, y+1))
      return fFalse;
    bm.kR.BitmapSet(kvBlack);
    y2 = y * nWid / 99;
    q1 = Sq(y) + y - 1; q2 = Sq(y2) + y2 - 1;
    if (y2 == y)
      y2--;
    fSav = cs.fRainbowBlend; cs.fRainbowBlend = fTrue;
    nSav1 = cs.nRainbowStart; cs.nRainbowStart = 0;
    nSav2 = cs.nRainbowDistance; cs.nRainbowDistance = nDegMax;
    for (n = 0; n <= y; n++)
      for (m = -y; m <= y; m++) {
        l = Sq(n) + Sq(m);
        if (l <= q1 && l >= q2)
          bm.kR.Set(y + m, y - n,
            Hue((y*10 - NSqr(l*100)) * (nHueMax * 31 / 360) / (y-y2)));
      }
    cs.fRainbowBlend = fSav;
    cs.nRainbowStart = nSav1; cs.nRainbowDistance = nSav2;
  }

  // Blend custom rainbow bitmap if irregularly sized (slightly slower).
  if (dr.nRainbow < 0) {
    yT = yAdd + (c.m_y >> 1) + (y * nLo / 99);
    for (n = 0; n <= y; n++) {
      if (yT < 0 || yT >= c.m_y)
        continue;
      pb2 = c._Pb(0, yT);
      for (m = -y; m <= y; m++) {
        kv1 = bm.kR.Get((m+y) * bm.kR.m_x / (y*2+1),
          bm.kR.m_y - n * bm.kR.m_y / y);
        if (kv1 != kvBlack) {
          xT = x + m;
          if (xT >= 0 && xT < c.m_x) {
            pbT = pb2 + xT*cbPixelC;
            c._Get(pbT, &nR, &nG, &nB);
            nR += (RgbR(kv1) - nR) * nTrans / 99;
            nG += (RgbG(kv1) - nG) * nTrans / 99;
            nB += (RgbB(kv1) - nB) * nTrans / 99;
            c._Set(pbT, nR, nG, nB);
          }
        }
      }
      yT--;
    }
    return fTrue;
  }

  // Blend the rainbow bitmap with what's already present.
  yT = yAdd + (c.m_y >> 1) + (y * nLo / 99);
  for (n = 0; n <= y; n++) {
    if (yT < 0 || yT >= c.m_y)
      continue;
    pb1 = bm.kR._Pb(0, y - n);
    pb2 = c._Pb(0, yT);
    for (m = -y; m <= y; m++) {
      kv1 = bm.kR._Get(pb1);
      if (kv1 != kvBlack) {
        xT = x + m;
        if (xT >= 0 && xT < c.m_x) {
          pbT = pb2 + xT*cbPixelC;
          c._Get(pbT, &nR, &nG, &nB);
          nR += (RgbR(kv1) - nR) * nTrans / 99;
          nG += (RgbG(kv1) - nG) * nTrans / 99;
          nB += (RgbB(kv1) - nB) * nTrans / 99;
          c._Set(pbT, nR, nG, nB);
        }
      }
      pb1 += cbPixelC;
    }
    yT--;
  }
  return fTrue;
}


// Create and draw background mountains for the inside display mode.

flag FCreateInsideMountains(CMazK *c, real rd, int yAdd)
{
  int i, i1, i2, x, y, y0, y1, y2, yb;
  real rx;

  // If the list of elevations hasn't been generated yet, do so first.
  if (dr.rgmtn == NULL) {
    dr.rgmtn = RgAllocate(imtnMax, short);
    if (dr.rgmtn == NULL)
      return fFalse;

    // Start with large blocky shapes to give general definition.
    y0 = y1 = Rnd(0, 10000);
    i1 = 0;
    while (i1 < imtnMax) {
      if (i1 + 90 < imtnMax) {
        i2 = i1 + Rnd(30, 60);
        y2 = Rnd(Max(y1 - 5000, 0), Min(y1 + 5000, 10000));
      } else {
        // Make sure the last elevation matches up with the first.
        i2 = imtnMax;
        y2 = y0;
      }
      for (i = i1; i < i2; i++)
        dr.rgmtn[i] = y1 + NMultDiv(i - i1, y2 - y1, i2 - i1);
      i1 = i2;
      y1 = y2;
    }

    // Then add in smaller adjustments to make the mountains seem more random
    // and natural. Two stages makes these simple fractal mountains! :)
    y0 = y1 = Rnd(-3000, -1000);
    i1 = 0;
    while (i1 < imtnMax) {
      if (i1 + 30 < imtnMax) {
        i2 = i1 + Rnd(10, 20);
        y2 = Rnd(-3000, -1000);
      } else {
        // Make sure the last elevation matches up with the first.
        i2 = imtnMax;
        y2 = y0;
      }
      for (i = i1; i < i2; i++)
        dr.rgmtn[i] += y1 + NMultDiv(i - i1, y2 - y1, i2 - i1);
      i1 = i2;
      y1 = y2;
    }
    for (i = 0; i < imtnMax; i++)
      if (dr.rgmtn[i] < 0)
        dr.rgmtn[i] = 0;
    if (c == NULL)
      return fTrue;
  }

  // Draw each part of the horizon at its appropriate elevation.
  yb = (c->m_y >> (int)!ds.fSkyAll) + yAdd;
  for (x = 0; x < c->m_x; x++) {
    rx = rd - dr.rgcalc[x].rAngleColumn;
    if (rx < 0)
      rx += rDegMax;
    else if (rx >= rDegMax)
      rx -= rDegMax;
    i = (int)(rx * (real)imtnMax / rDegMax);
    y = NMultDiv(c->m_y, (long)dr.rgmtn[i] * dr.nPeakHeight, 1600000);
    LineYGradient(*c, x, yb-y, yb,
      dr.kvInMtn2 < 0 ? dr.kvInMtn : dr.kvInMtn2, dr.kvInMtn);
  }
  return fTrue;
}


#define dyCld 250
#define csCld (4 + (dyCld << 1))

// Create and draw background clouds for the inside display mode.

flag FCreateInsideClouds(CMazK *c, real rd, int yAdd)
{
  int i, is, j, k, x1, y1, x2, y2, i1, i2, ySky;
  real rx1, rx2, rx3, rx4,
    dInside2 = dr.dInside*2.0, rT = RTanD(dr.dInside) * 2.0;
  KV kv = dr.kvInCld;
  flag f;

  // If the list of cloud positions hasn't been generated yet, do so first.
  if (dr.rgcld == NULL) {
    dr.rgcld = RgAllocate(icldMax*csCld, short);
    if (dr.rgcld == NULL)
      return fFalse;
    for (i = 0; i < icldMax; i++) {
      is = i * csCld;

      // Determine bounding box for this cloud. The first 16 clouds will never
      // be due south or north, i.e. they will never block the Sun/Moon.
      do {
        j = Rnd(0, 16383);
      } while (i < 16 && (j <= 3000 || FBetween(j, 8192, 8192+3000)));
      k = Rnd(3000, 6000);
      dr.rgcld[is] = j;
      dr.rgcld[is + 1] = Rnd(-(k >> 1), 16383 - (k >> 1));
      dr.rgcld[is + 2] = Rnd(1000, 3000);
      dr.rgcld[is + 3] = k;

      // Randomize the left and right sides of the cloud.
      for (f = 0; f <= 1; f++) {

        // Start with large blocky shapes to give general definition.
        y1 = Rnd(0, 4000);
        i1 = 0;
        while (i1 < dyCld) {
          if (i1 + 80 < dyCld)
            i2 = i1 + Rnd(40, 60);
          else
            i2 = dyCld;
          y2 = Rnd(0, 4000);
          for (j = i1; j < i2; j++) {
            k = y1 + NMultDiv(j - i1, y2 - y1, i2 - i1);
            dr.rgcld[is + 4 + f + (j << 1)] = 8192 - k;
          }
          i1 = i2;
          y1 = y2;
        }

        // Then add in smaller adjustments to make the cloud seem more random
        // and natural. Note how similar this process is to doing mountains.
        y1 = Rnd(0, 1000);
        i1 = 0;
        while (i1 < dyCld) {
          if (i1 + 20 < dyCld)
            i2 = i1 + Rnd(10, 15);
          else
            i2 = dyCld;
          y2 = Rnd(0, 1000);
          for (j = i1; j < i2; j++) {
            k = y1 + NMultDiv(j - i1, y2 - y1, i2 - i1);
            dr.rgcld[is + 4 + f + (j << 1)] -= k;
          }
          i1 = i2;
          y1 = y2;
        }
      }

      // Finally round the corners of the cloud so it has a bounding ellipse.
      for (j = 0; j < dyCld; j++) {
        k = NMultDiv(NAbs(j - 125), 8192, 125);
        k = NSqr(Sq(8192) - Sq(k));
        dr.rgcld[is + 4 + (j << 1)] = 8192 -
          NMultDiv(dr.rgcld[is + 4 + (j << 1)], k, 8192);
        dr.rgcld[is + 5 + (j << 1)] = 8192 +
          NMultDiv(dr.rgcld[is + 5 + (j << 1)], k, 8192);
      }
    }
    if (c == NULL)
      return fTrue;
  }

  // Draw each cloud at its appropriate position.
  ySky = c->m_y >> (int)!ds.fSkyAll;
  f = (rd + dInside2 < rDegMax);
  for (i = 0; i < dr.cCloud; i++) {
    is = i * csCld;
    rx1 = (real)dr.rgcld[is] * rDegMax / 16384.0;
    rx2 = (real)(dr.rgcld[is] + dr.rgcld[is + 2]) * rDegMax / 16384.0;
    rx4 = rx2;
    if (rx4 >= rDegMax)
      rx4 -= rDegMax;
    if (!(f ? (rx1 > rd && rx1 < rd + dInside2) ||
      (rx4 > rd && rx4 < rd + dInside2) :
      (rx1 > rd || rx1 < rd + dInside2 - rDegMax) ||
      (rx4 > rd || rx4 < rd + dInside2 - rDegMax)))
      continue;
    y1 = yAdd + (int)((real)ySky * (real)dr.rgcld[is + 1] / 16384.0);
    y2 = y1   + (int)((real)ySky * (real)dr.rgcld[is + 3] / 16384.0);

    // For each pixel row, draw a horizontal line for that part of the cloud.
    for (j = Max(y1, 0); j < Min(y2, yAdd + ySky); j++) {
      if (dr.kvInCld2 >= 0)
        kv = KvBlendN(dr.kvInCld, dr.kvInCld2, j - y1, y2 - y1);
      k = is + 4 + (NMultDiv(j - y1, dyCld, y2 - y1) << 1);
      rx3 = rx1 + (real)dr.rgcld[k] * (rx2 - rx1) / 16384.0;
      rx4 = rx1 + (real)dr.rgcld[k + 1] * (rx2 - rx1) / 16384.0;
      if (rx3 >= rDegMax)
        rx3 -= rDegMax;
      if (rx4 >= rDegMax)
        rx4 -= rDegMax;
      if (f ? (rx3 > rd && rx3 < rd + dInside2) ||
        (rx4 > rd && rx4 < rd + dInside2) :
        (rx3 > rd || rx3 < rd + dInside2 - rDegMax) ||
        (rx4 > rd || rx4 < rd + dInside2 - rDegMax)) {
        if (f) {
          if (rx3 < rd || rx3 > rd + dInside2)
            rx3 = rd - 0.5;
          if (rx4 < rd || rx4 > rd + dInside2)
            rx4 = rd + dInside2 + 0.5;
        } else {
          if (rx3 < rd && rx3 > rd + dInside2 - rDegMax)
            rx3 = rd - 0.5;
          if (rx4 < rd && rx4 > rd + dInside2 - rDegMax)
            rx4 = rd + dInside2 - rDegMax + 0.5;
        }
        rx3 -= rd;
        if (rx3 < 0.0)
          rx3 += rDegMax;
        rx3 = RTanD(rx3 - dr.dInside) / rT + 0.5;
        rx4 -= rd;
        if (rx4 < 0.0)
          rx4 += rDegMax;
        rx4 = RTanD(rx4 - dr.dInside) / rT + 0.5;
        x1 = c->m_x-1 - (int)((real)c->m_x * rx3);
        x2 = c->m_x-1 - (int)((real)c->m_x * rx4);
        c->LineX(x1, x2, j, kv);
      }
    }
  }
  return fTrue;
}


// Adjust a cloud's horizontal coordinate by a specified offset.

flag FMoveCloud(int iCloud, int x)
{
  int icldLo, icldHi, icld, is, x0, dx;
  flag fVariable;

  // Return right away if there are no clouds, or not moving any distance.
  if (dr.rgcld == NULL)
    return fFalse;
  if (x == 0)
    return fTrue;

  // An invalid cloud index means move all clouds.
  if (FBetween(iCloud, 0, icldMax-1))
    icldLo = icldHi = iCloud;
  else {
    icldLo = 0; icldHi = icldMax-1;
  }

  // An invalid (too extreme) delta means move each cloud a different amount.
  fVariable = !FBetween(x, -16383, 16383);
  if (!fVariable)
    dx = x;
  else {
    x0 = NSgn(x);
    x -= x0 * 16384;
  }

  // Loop over the set of clouds to move.
  for (icld = icldLo; icld <= icldHi; icld++) {
    if (fVariable)
      dx = x0 + NMultDiv(icld - icldLo, x, 16384);
    is = icld * csCld;
    dr.rgcld[is] = dr.rgcld[is] - dx & 16383;
  }
  return fTrue;
}


// Regenerate the background components of the inside display mode.

void ResetInside()
{
  if (ds.rgstar != NULL) {
    DeallocateP(ds.rgstar);
    ds.rgstar = NULL;
  }
  if (dr.rgstar != NULL) {
    DeallocateP(dr.rgstar);
    dr.rgstar = NULL;
  }
  if (dr.rgmtn != NULL) {
    DeallocateP(dr.rgmtn);
    dr.rgmtn = NULL;
  }
  if (dr.rgcld != NULL) {
    DeallocateP(dr.rgcld);
    dr.rgcld = NULL;
  }
  FCreateInsideStars(NULL, 0.0, 0, fFalse);
  FCreateInsideMountains(NULL, 0.0, 0);
  FCreateInsideClouds(NULL, 0.0, 0);
}


// Show a compass, i.e. print the direction the dot is facing.

void FormatCompass(char sz[cchSzDef], flag fOffset, int *pcch)
{
  CONST char *pch1, *pch2;
  int cch, a, d;

  if (fOffset || dr.fCompassAngle) {

    // Complex version: Figure out which of the eight directions to print.
    pch1 = pch2 = NULL;
    a = dr.dir * 90 + dr.nOffsetD;
    if (a < 0)
      a += nDegMax;
    d = a + 22;
    if (d > nDegMax)
      d -= nDegMax;
    d /= 45;
    if (d >= 7 || d <= 1)
      pch1 = rgszDir[0];
    else if (FBetween(d, 3, 5))
      pch1 = rgszDir[2];
    if (FBetween(d, 1, 3))
      pch2 = rgszDir[1];
    else if (FBetween(d, 5, 7))
      pch2 = rgszDir[3];
    if (dr.nOffsetD == 0) {
      if (pch1 != NULL)
        pch2 = pch1;
      pch1 = dr.fCompassAngle ? "Due " : NULL;
    }
    if (pch1 == NULL)
      pch1 = "";
    else if (pch2 == NULL)
      pch2 = "";
    if (dr.fCompassAngle)
      sprintf(SS(sz, cchSzDef), "%s%s a=%d", pch1, pch2, a);
    else
      sprintf(SS(sz, cchSzDef), "%s%s", pch1, pch2);
    cch = CchSz(sz);
  } else {

    // Simple version: Just print North, West, South, or East.
    sprintf(SS(sz, cchSzDef), rgszDir[dr.dir]);
    cch = 5 - (dr.dir & 1);
  }
  *pcch = cch;
}


// Show the dot's location, i.e. print its 2D or 3D coordinates.

void FormatLocation(char sz[cchSzDef], flag f3D)
{
  if (!f3D)
    sprintf(SS(sz, cchSzDef), "x=%d, y=%d",
      dr.fNarrow ? dr.x >> 1 : dr.x, dr.fNarrow ? dr.y >> 1 : dr.y);
  else
    sprintf(SS(sz, cchSzDef), "x=%d, y=%d, z=%d",
      dr.fNarrow ? (dr.x+1) >> 1 : dr.x, dr.fNarrow ? (dr.y+1) >> 1 : dr.y,
      (dr.fNarrow ? dr.z >> 1 : dr.z) + 1);
}


// Show the number of milliseconds it took to render the current scene.

void FormatTimer(char sz[cchSzDef], ulong lTimer, ulong lTimer2)
{
  dr.nDelay = lTimer2 - lTimer;
  sprintf(SS(sz, cchSzDef), "Frame Delay: %d", dr.nDelay);
}


// Do all background drawing, i.e. all things the actual walls are later drawn
// on top of, for the inside display mode.

void DrawBackground(CMazK &c, flag f3D, real rd, int yb)
{
  CMazK *cT;
  real rx, rT;
  int x, y, xSrc1, xSrc2, xDst, iTexture, xT, yT;
  KV kvSky = f3D && dr.z > 0 ? dr.kvInCeil : dr.kvInSky,
    kvSky2 = f3D && dr.z > 0 ? dr.kvInCeil2 : dr.kvInSky2;

  // Draw a rectangle for the sky or ceiling half of the screen.
  c.Block(0, 0, c.m_x-1, yb-1, kvSky);

  // Draw a rectangle for the ground or floor half of the screen.
  if (ds.fSkyAll) {
    c.Block(0, yb, c.m_x-1, c.m_y-1, dr.kvInDirt);
    if (dr.kvInDirt2 >= 0) {
      yT = c.m_y >> 1;
      for (y = Min(yT, c.m_y - yb)-1; y >= 0; y--)
        c.LineX(0, c.m_x, yb+y,
          KvBlendN(dr.kvInDirt2, dr.kvInDirt, y, yT));
    }
  }

  // Draw gradient between two colors over the sky.
  if (kvSky2 >= 0 && kvSky2 != kvSky) {
    yT = c.m_y >> 1;
    for (y = Min(yT, yb)-1; y >= 0; y--)
      c.LineX(0, c.m_x, yb-1-y,
        KvBlendN(kvSky2, kvSky, y, yT));
  }

  // Do texture mapping over both the sky and ground part of the screen.
  if (dr.fTexture && (dr.nTexture > 0 || dr.nTexture2 > 0)) {
    iTexture = !f3D || dr.z <= 0 || dr.fSky3D ? dr.nTexture : dr.nTexture2;
    cT = ColmapGetTexture(iTexture);
    if (iTexture > 0 && cT != NULL && !cT->FNull()) {
      yb -= c.m_y >> 1;
      rT = RTanD(dr.dInside);
      yT = c.m_y >> (int)!ds.fSkyAll;
      xDst = xSrc1 = 0;
      for (x = 0; x < c.m_x; x++) {
        rx = (real)(x << 1) / (real)c.m_x - 1.0;
        rx = rd - RAtnD(rx * rT);
        if (rx < 0)
          rx += rDegMax;
        else if (rx >= rDegMax)
          rx -= rDegMax;
        xSrc2 = cT->m_x - 1 - (int)(rx * (real)cT->m_x / rDegMax);
        if (xSrc2 != xSrc1 || x == c.m_x-1) {
          if (x > 0) {
            for (xT = xDst; xT <= x; xT++)
              LineYTexture(c, xT, yb, yb + yT, yb, yb + yT,
                *cT, xSrc1, 0, cT->m_y, 0, kvBlack, 0, ~0);
            xDst = x+1;
          }
          xSrc1 = xSrc2;
        }
      }
      yb += c.m_y >> 1;
    }
  }

  // Normally only do the outside things for 2D Mazes or the top level of 3D
  // Mazes. Draw things in order from farthest away to nearest.
  if (!f3D || dr.z <= 0 || dr.fSky3D) {
    if (ds.fStar)
      FCreateInsideStars(&c, rd, yb-(c.m_y >> 1), fTrue);
    if (dr.fSunMoon)
      DrawSunMoon(c, rd, yb-(c.m_y >> 1));

    // Draw meteors if appropriate.
    if (ds.fStar && dr.nMeteor > 0) {
      yb -= c.m_y >> 1;
      y = (c.m_y >> (int)!ds.fSkyAll) - 1;
      while (Rnd(0, 9999) < dr.nMeteor)
        c.Line(Rnd(0, c.m_x-1), yb + Rnd(0, y),
          Rnd(0, c.m_x-1), yb + Rnd(0, y), KvStarRandom());
      yb += c.m_y >> 1;
    }

    if (dr.cCloud > 0)
      FCreateInsideClouds(&c, rd, yb-(c.m_y >> 1));
    if (dr.fRainbow)
      FDrawRainbow(c, rd, yb-(c.m_y >> 1));
    if (dr.fMountain)
      FCreateInsideMountains(&c, rd, yb-(c.m_y >> 1));
  }

  // Draw a rectangle for the ground or floor half of the screen.
  if (!ds.fSkyAll) {
    c.Block(0, yb, c.m_x-1, c.m_y-1, dr.kvInDirt);
    if (dr.kvInDirt2 >= 0 && dr.kvInDirt2 != dr.kvInDirt) {
      yT = c.m_y >> 1;
      for (y = Min(yT, c.m_y - yb)-1; y >= 0; y--)
        c.LineX(0, c.m_x, yb+y,
          KvBlendN(dr.kvInDirt2, dr.kvInDirt, y, yT));
    }
  }
}


// A bunch of macros used by RedrawInsidePerspective().

#define ypWallMax (1 << 29)
#define LengthI(z, t) (Sq(z) + (Sq((z)*(t))))
#define DistanceM() RAbs((real)(me - ml) * md0)
#define DistanceN() RAbs((real)(ne - nl) * nd0)
#define LineI(y1, y2, kv) LineYTrans(c, x, y1, y2, kv, 0)

#define FMark(b, x, y) \
  (fMarkAll ? FBetween(x, 0, xmax-1) && FBetween(y, 0, ymax-1) : \
  (b).GetI(x, y, dr.z, f3D))

#define ComputeDir(k) \
  k = (yo0 - yg0 + 1) + ((xo0 > xg0) << 1)

#define ShadeKvCore(kv) \
  if (k & 1) \
    ; \
  else if (k == 0) \
    kv = KvShade(kv, -dr.rLightFactor); \
  else \
    kv = KvShade(kv, dr.rLightFactor); \
  if (FFogSide(xg, yg)) \
    kv = KvApplyFog(kv);

#define ShadeKv(kv) \
  ComputeDir(k); \
  ShadeKvCore(kv);

#define GetKvCore2(kv, c, x, y) \
  if ((c).FLegal(x, y)) \
    kv = (c).Get(x, y);

#define GetKvCore(kv, c, x, y, z) \
  if (f3D) { \
    GetKvCore2(kv, c, bm.b.X2(x, z), bm.b.Y2(y, z)); \
  } else { \
    GetKvCore2(kv, c, x, y); \
  }

#define GetKv(kv, c, x, y, z, kvElse) \
  kv = kvElse; \
  GetKvCore(kv, c, x, y, z)

#define GetKvIf(kv, x, y, z, kvElse) \
  kv = kvElse; \
  if (fColor) { \
    GetKvCore(kv, bm.k, x, y, z) \
  }

#define GetKvMark(kv, c, x, y) \
  GetKv(kv, c, x, y, dr.z, kvMarkDef)

#define Ensure3DColor(x, y, dz) \
  GetKvIf(kv3D, x, y, dr.z + dz, dr.kvIn3D) \
  ShadeKv(kv3D);

#define KvApplyFog(kv) \
  KvBlendR(kv, dr.kvInFog, Min(zd / rFogDist, 1.0))

#define FFogSide(x, y) \
  (fFog && (!fFogLit || !ws.rgbMask[dr.nFogLit].Get(x, y)))

#define FFogDirt(x, y) \
  (fFogDirt && (!fFogLit || !ws.rgbMask[dr.nFogLit].Get(x, y)))

#define fFogCompute (ptrans->nFog > 0)

#define nFogCompute \
  (FFogSide(xg, yg) ? NMax((int)(RMin(zd / rFogDist, 1.0) * 100.0), 1) : 0)

#define DrawTexture3D(dz) \
  if (fTextureWall) { \
    ComputeTexture(&iTexture, &iMask, &rTexture, dr.nTextureWall, \
      xg, yg, dz, k, xq, yq, me, ml, ne, nl, md, nd, mt, nt); \
    if (iTexture > 0 || iMask > 0) \
      DrawTexture(c, x, y, dz > 0 ? yExtra : -yExtra2, yk, IP(yb), \
        iTexture, iMask, rTexture, 0, nFogCompute, \
        !ws.rgbMask[0].FNull() && ws.rgbMask[0].Get(xg, yg), ~0); \
  }

#define FWallVar(x, y) \
  ((dr.zElev != 0) || (fWallVar2 && bWallVar->GetI(x, y, dr.z, f3D)))

#define ComputeTextureOffset(x, y) \
  if (md < nd) \
    rTexture = RAbs((real)(me - ml)*mt) - RAbs((real)(ne - nl)); \
  else \
    rTexture = RAbs((real)(ne - nl)*nt) - RAbs((real)(me - ml)); \
  rTexture = RAbs(rTexture); \
  if (!dr.fNarrow || FOdd(x & y) || FOdd(x & (k ^ 1)) || FOdd(y & k)) \
    rTexture /= (real)dr.zCell; \
  else \
    rTexture /= (real)dr.zCellNarrow; \
  if ((xq < 1) ^ (yq < 1) ^ FOdd(k)) \
    rTexture = 1.0 - rTexture;

#define ComputeTextureOffsetFloor1(rx1, ry1) \
  switch (k) { \
  case 0 : rx1 = 1.0 - rTexture; ry1 = 0.0; break; \
  case 1 : rx1 = 0.0; ry1 = rTexture; break; \
  case 2 : rx1 = rTexture; ry1 = 1.0; break; \
  default: rx1 = 1.0; ry1 = 1.0 - rTexture; break; \
  }

#define ComputeTextureOffsetFloor2(rx2, ry2) \
  switch (k) { \
  case 0 : rx2 = 1.0 - rTexture; ry2 = 1.0; break; \
  case 1 : rx2 = 1.0; ry2 = rTexture; break; \
  case 2 : rx2 = rTexture; ry2 = 0.0; break; \
  default: rx2 = 0.0; ry2 = 1.0 - rTexture; break; \
  }

#define ComputeTextureOffsetFloor3(rx3, ry3) \
  switch (k) { \
  case 0 : rx3 = rTexture; ry3 = 0.0; break; \
  case 1 : rx3 = 1.0; ry3 = rTexture; break; \
  case 2 : rx3 = 1.0 - rTexture; ry3 = 1.0; break; \
  default: rx3 = 0.0; ry3 = 1.0 - rTexture; break; \
  }

#define ComputeTextureOffsetFloor4(rx4, ry4) \
  switch (k) { \
  case 0 : rx4 = rTexture; ry4 = 1.0; break; \
  case 1 : rx4 = 0.0; ry4 = rTexture; break; \
  case 2 : rx4 = 1.0 - rTexture; ry4 = 0.0; break; \
  default: rx4 = 1.0; ry4 = 1.0 - rTexture; break; \
  }


// Determine what texture mapping to apply to the current wall at the current
// column, if any.

void ComputeTexture(int *piTexture, int *piMask, real *prTexture, int ic,
  int xg, int yg, int dz, int k, int xq, int yq,
  long me, long ml, long ne, long nl, real md, real nd, real mt, real nt)
{
  int iTexture = 0, iMask = 0;
  real rTexture = 0.0;
  CMazK *c = &ws.rgcTexture[ic];
  KV kv;

  if (dr.f3D) {
    xg = bm.b.X2(xg, dr.z + dz); yg = bm.b.Y2(yg, dr.z + dz);
  }
  if (!c->FLegal(xg, yg))
    goto LDone;
  kv = c->_Get(xg, yg);
  if (kv == kvBlack)
    goto LDone;

  // Determine the color and monochrome texture to apply to this wall.
  if (!dr.fTextureDual)
    iTexture = ITextureWall(kv, k);
  else if (!dr.fTextureDual2)
    iTexture = ITextureWall2(kv, k);
  else
    iTexture = ITextureWall3(kv, k);
  if (iTexture > 0) {
    if (iTexture < ws.cbMask && !ws.rgbMask[iTexture].FNull())
      iMask = iTexture;
    if (iTexture >= ws.ccTexture || ws.rgcTexture[iTexture].FNull())
      iTexture = 0;

    // If any texture, determine horizontal fraction within texture to draw.
    if (iTexture > 0 || iMask > 0) {
      ComputeTextureOffset(xg, yg);
    }
  }
LDone:
  *piTexture = iTexture; *piMask = iMask; *prTexture = rTexture;
}


// Determine what texture mapping to apply to the current floor cell, if any.

void ComputeTextureFloor(int *piTexture, int *piMask, int xg, int yg,
  flag fBlock, flag fCeiling)
{
  int nTexture = fBlock ? dr.nTextureVar : dr.nTextureDirt,
    iTexture = 0, iMask = 0;
  CMazK *c = &ws.rgcTexture[nTexture];
  KV kv;

  if (dr.f3D) {
    xg = bm.b.X2(xg, dr.z); yg = bm.b.Y2(yg, dr.z);
  }
  if (!c->FLegal(xg, yg))
    goto LDone;
  kv = c->_Get(xg, yg);
  if (kv == kvBlack)
    goto LDone;

  // Determine the color and monochrome texture to apply to the floor here.
  iTexture = fCeiling ? UdD(kv) : UdU(kv);
  if (iTexture > 0) {
    if (iTexture < ws.cbMask && !ws.rgbMask[iTexture].FNull())
      iMask = iTexture;
    if (iTexture >= ws.ccTexture || ws.rgcTexture[iTexture].FNull())
      iTexture = 0;
  }
LDone:
  *piTexture = iTexture; *piMask = iMask;
}


// Draw a texture at the given column. For solid color textures, this
// completely covers whatever solid wall color would otherwise be present. For
// textures overlays, this is done on top of whatever solid wall color is
// already present.

void DrawTexture(CMazK &c, int x, int y, int y2, int yk, int yb, int iTexture,
  int iMask, real rTexture, int nTrans, int nFog, flag fMaskSet, KV kv)
{
  CMaz *bT;
  CMazK *cT;
  int xTexture, yDst1, yDst2, yClip1, yClip2;

  // Figure out the pixel offset within the texture bitmaps. Color bitmap
  // only: Solid texture. Monochrome bitmap only: Black or white overlay on
  // top of the wall. Both color and monochrome: Merge color texture with the
  // black or white overlay, resulting in a color overlay on top of the wall.

  if (iMask > 0)
    bT = &ws.rgbMask[iMask];
  if (iTexture > 0) {
    cT = &ws.rgcTexture[iTexture];
    xTexture = (int)((real)cT->m_x * rTexture);
    if (xTexture >= cT->m_x)
      xTexture = cT->m_x-1;
  } else {
    cT = NULL;
    xTexture = (int)((real)bT->m_x * rTexture);
    if (xTexture >= bT->m_x)
      xTexture = bT->m_x-1;
  }

  // Figure out the actual pixel coordinates to draw between.
  if (yb == ~0) {
    yDst1 = y; yDst2 = y2;
    yClip1 = yDst1; yClip2 = Min(yDst2, yk);
  } else if (y2 > 0) {
    yDst1 = IO(yb+y); yDst2 = IO(yb+y+yk*2);
    yClip1 = yDst1; yClip2 = IO(yb+Min(y2, y+yk*2));
  } else { // y2 < 0
    yDst1 = IO(yb-y-yk*2+iRound); yDst2 = IO(yb-y+iRound);
    yClip1 = IO(yb-Min(-y2, y+yk*2)+iRound); yClip2 = yDst2;
  }

  // Go draw the solid texture map or texture overlay.
  if (iTexture > 0 && iMask == 0)
    LineYTexture(c, x, yDst1, yDst2, yClip1, yClip2,
      *cT, xTexture, 0, cT->m_y, nTrans, dr.kvInFog, nFog, kv);
  else
    LineYMask(c, x, yDst1, yDst2, yClip1, yClip2,
      bT, cT, xTexture, 0, cT != NULL ? cT->m_y : bT->m_y, dr.kvInFog, nFog,
      fMaskSet);
}


// Compute and return the height in inside units of the bottom and top of a
// variable height wall.

void ComputeWallVar(flag fWallVar2, CMaz *bWallVar, CMazK *cWallVar,
  int x, int y, int *pyLo, int *pyHi)
{
  int yLo, yHi, yT;

  if (fWallVar2 && bWallVar->GetI(x, y, dr.z, dr.f3D)) {

    // For variable height walls, translate from percentage to inside units.
    yT = cWallVar->GetF(x, y, dr.z, dr.f3D);
    yLo = UdD(yT);
    yHi = UdU(yT);
    if (yLo > yHi)
      SwapN(yLo, yHi);
  } else {

    // Non-variable height walls are always the default wall height.
    yLo = 0; yHi = 256;
  }
  *pyLo = yLo; *pyHi = yHi;
}


// Draw the appropriate set of precomputed semitransparent and variable height
// walls at a column. These are drawn on top of whatever solid walls or
// background are already in place.

void DrawTransVar(CMazK &c, int x, int yb, int cTrans, TRANS *trans,
  real rScale)
{
  int i, zEye = ((dr.zElev + (dr.zWall >> 1)) << iVarScale) / dr.zWall,
    yLo, yHi, yLo1, yHi1, yLo2, yHi2, ypLo, ypHi,
    ypHiU = c.m_y, ypLoU = -1, ypMax = c.m_y-1;
  KV kv, kvT;
  flag fWall;
  TRANS *ptrans;

  for (i = 0; i < cTrans; i++) {
    ptrans = &trans[i];
    yLo = ptrans->yLo; yHi = ptrans->yHi;

    // Compute pixel coordinates.
    if (ptrans->y < ypWallMax && ptrans->yk < ypWallMax) {
      yLo1 = yHi1 = IP(yb) + ptrans->y;
      yLo1 -= LMultShift(yLo, ptrans->yk, 7);
      yHi1 -= LMultShift(yHi, ptrans->yk, 7);
    } else {
      yLo1 = zEye < yLo ? -ypWallMax : ypWallMax;
      yHi1 = zEye < yHi ? -ypWallMax : ypWallMax;
    }
    if (ptrans->y2 < ypWallMax && ptrans->yk2 < ypWallMax) {
      yLo2 = yHi2 = IP(yb) + ptrans->y2;
      yLo2 -= LMultShift(yLo, ptrans->yk2, 7);
      yHi2 -= LMultShift(yHi, ptrans->yk2, 7);
    } else {
      yLo2 = zEye < yLo ? -ypWallMax : ypWallMax;
      yHi2 = zEye < yHi ? -ypWallMax : ypWallMax;
    }

    // Don't bother with this wall if it's entirely behind other solid walls.
    ptrans->fSkip = fFalse;
    ypLo = IO(Max(yLo1, yLo2)); ypLo = Min(ypLo, ypMax);
    ypHi = IO(Min(yHi1, yHi2)); ypHi = Max(ypHi, 0);
    if (ypLo <= ypLoU && ypHi >= ypHiU) {
      ptrans->fSkip = fTrue;
      continue;
    }

    // If wall is solid and overlaps current solid range, expand the range.
    if (!ptrans->fTrans) {
      if ((ypHi < ypHiU && ypLo >= ypHiU) ||
        (ypLo > ypLoU && ypHi <= ypLoU)) {
        if (ypLo > ypLoU)
          ypLoU = ypLo;
        if (ypHi < ypHiU)
          ypHiU = ypHi;
      } else if (ypLo - ypHi > ypLoU - ypHiU) {
        ypLoU = ypLo; ypHiU = ypHi;
      }
    }
    ptrans->yLo1 = yLo1; ptrans->yHi1 = yHi1;
    ptrans->yLo2 = yLo2; ptrans->yHi2 = yHi2;

    // If solid wall range covers entire screen, skip anything else behind.
    if (ypHiU <= 0 && ypLoU >= ypMax) {
      cTrans = i+1;
      break;
    }
  }

  // Draw semitransparent and variable height walls from back to front,
  // blending each semitransparent pixel with whatever is already in place.

  if (zEye != 0)
    rScale /= (real)zEye;
  for (i = cTrans-1; i >= 0; i--) {
    ptrans = &trans[i];
    if (ptrans->fSkip)
      continue;
    kv = ptrans->kv;
    yLo  = ptrans->yLo;  yHi  = ptrans->yHi;
    yLo1 = ptrans->yLo1; yHi1 = ptrans->yHi1;
    yLo2 = ptrans->yLo2; yHi2 = ptrans->yHi2;
    dr.yElevMax = yb+IO(ptrans->yElev);
    dr.yElevMin = yb+IO(ptrans->yElev2);
    fWall = !(dr.nTrans == nTransFast && ptrans->fTrans && i > 0 &&
      trans[i-1].fTrans && ptrans->y == trans[i-1].y2 &&
      yHi == trans[i-1].yHi && yLo == trans[i-1].yLo &&
      ((ptrans->nFog > 0) == (trans[i-1].nFog > 0)));

    if (yHi >= 0) {

      // Draw a variable height wall.
      if (!ptrans->fTrans || dr.nTrans != nTransVery) {

        // Draw a solid or simple semitransparent variable height wall.
        fWall &= (ptrans->fTrans || i == 0 || yHi > trans[i-1].yHi ||
          yLo < trans[i-1].yLo || ((zEye > yHi || zEye < yLo) &&
          ptrans->y != trans[i-1].y2) || trans[i-1].fTrans);
        if (fWall)
          LineITrans(yHi1, yLo1, kv, ptrans->fTrans ? dr.nTransPct : 0);
        if (zEye > yHi) {

          // Looking down on top of wall.
          kvT = KvShade(ptrans->kvOrig, RAbs(dr.rLightFactor) / 2.0);
          if (!dr.fFogFloor) {
            LineITrans(yHi2, yHi1, kvT,
              ptrans->fTrans ? dr.nTransPct2 : 0);
          } else
            LineYFloor(c, x, yb, IO(yHi2)-yb, IO(yHi1)-yb, kvT,
              ptrans->fTrans ? dr.nTransPct2 : 0, fFogCompute,
              rScale * (real)(zEye - yHi));
          if (ptrans->iMaskVar > 0 || ptrans->iTextureVar > 0)
            LineYFloorTexture(c, x, yb, yHi2-IP(yb), yHi1-IP(yb),
              ptrans->iMaskVar > 0 ? &ws.rgbMask[ptrans->iMaskVar] : NULL,
              ptrans->iTextureVar > 0 ? &ws.rgcTexture[ptrans->iTextureVar] :
              NULL, ptrans->rx2, ptrans->ry2, ptrans->rx1, ptrans->ry1,
              ptrans->fMaskSetVar, fFogCompute, fTrue,
              rScale * (real)(zEye - yHi));
        } else if (zEye < yLo) {

          // Looking up at bottom of wall.
          kvT = KvShade(ptrans->kvOrig, RAbs(dr.rLightFactor) / -2.0);
          if (!dr.fFogFloor) {
            LineITrans(yLo1, yLo2, kvT, ptrans->fTrans ? dr.nTransPct2 : 0);
          } else
            LineYFloor(c, x, yb, IO(yLo1)-yb, IO(yLo2)-yb, kvT,
              ptrans->fTrans ? dr.nTransPct2 : 0, fFogCompute,
              rScale * (real)(yLo - zEye));
          if (ptrans->iMaskVar2 > 0 || ptrans->iTextureVar2 > 0)
            LineYFloorTexture(c, x, yb, yLo1-IP(yb), yLo2-IP(yb),
              ptrans->iMaskVar2 > 0 ? &ws.rgbMask[ptrans->iMaskVar2] : NULL,
              ptrans->iTextureVar2 > 0 ? &ws.rgcTexture[ptrans->iTextureVar2]
              : NULL, ptrans->rx3, ptrans->ry3, ptrans->rx4, ptrans->ry4,
              ptrans->fMaskSetVar, fFogCompute, fTrue,
              rScale * (real)(yLo - zEye));
        }
      } else {

        // Draw a very thorough semitransparent variable height wall.
        if (zEye > yHi) {

          // Looking down on top of wall.
          kvT = KvShade(ptrans->kvOrig, RAbs(dr.rLightFactor) / 2.0);
          if (yHi1 <= yLo2) {
            if (fWall) {
              LineITrans(yLo2, yLo1, kv, dr.nTransPct2);
              LineITrans(yHi1, yLo2, kv, dr.nTransPct);
            }
            if (!dr.fFogFloor) {
              LineITrans(yHi2, yHi1, kvT, dr.nTransPct2);
            } else
              LineYFloor(c, x, yb, IO(yHi2)-yb, IO(yHi1)-yb, kvT,
                dr.nTransPct2, fFogCompute, rScale * (real)(zEye - yHi));
          } else {
            if (fWall)
              LineITrans(yHi1, yLo1, kv, dr.nTransPct2);
            if (!dr.fFogFloor) {
              LineITrans(yLo2, yHi1, ptrans->kvOrig, dr.nTransPct);
              LineITrans(yHi2, yLo2, kvT, dr.nTransPct2);
            } else {
              LineYFloor(c, x, yb, IO(yLo2)-yb, IO(yHi1)-yb,
                ptrans->kvOrig, dr.nTransPct, fFogCompute,
                rScale * (real)(zEye - yHi));
              LineYFloor(c, x, yb, IO(yHi2)-yb, IO(yLo2)-yb,
                kvT, dr.nTransPct2, fFogCompute, rScale * (real)(zEye - yHi));
            }
          }
          if (ptrans->iMaskVar > 0 || ptrans->iTextureVar > 0)
            LineYFloorTexture(c, x, yb, yHi2-IP(yb), yHi1-IP(yb),
              ptrans->iMaskVar > 0 ? &ws.rgbMask[ptrans->iMaskVar] : NULL,
              ptrans->iTextureVar > 0 ? &ws.rgcTexture[ptrans->iTextureVar] :
              NULL, ptrans->rx2, ptrans->ry2, ptrans->rx1, ptrans->ry1,
              ptrans->fMaskSetVar, fFogCompute, fTrue,
              rScale * (real)(zEye - yHi));
        } else if (zEye < yLo) {

          // Looking up at bottom of wall.
          kvT = KvShade(ptrans->kvOrig, RAbs(dr.rLightFactor) / -2.0);
          if (yLo1 >= yHi2) {
            if (fWall) {
              LineITrans(yHi1, yHi2, kv, dr.nTransPct2);
              LineITrans(yHi2, yLo1, kv, dr.nTransPct);
            }
            if (!dr.fFogFloor) {
              LineITrans(yLo1, yLo2, kvT, dr.nTransPct2);
            } else
              LineYFloor(c, x, yb, IO(yLo1)-yb, IO(yLo2)-yb, kvT,
                dr.nTransPct2, fFogCompute, rScale * (real)(yLo - zEye));
          } else {
            if (fWall)
              LineITrans(yHi1, yLo1, kv, dr.nTransPct2);
            if (!dr.fFogFloor) {
              LineITrans(yLo1, yHi2, ptrans->kvOrig, dr.nTransPct);
              LineITrans(yHi2, yLo2, kvT, dr.nTransPct2);
            } else {
              LineYFloor(c, x, yb, IO(yLo1)-yb, IO(yHi2)-yb,
                ptrans->kvOrig, dr.nTransPct, fFogCompute,
                rScale * (real)(yLo - zEye));
              LineYFloor(c, x, yb, IO(yHi2)-yb, IO(yLo2)-yb,
                kvT, dr.nTransPct2, fFogCompute, rScale * (real)(yLo - zEye));
            }
          }
          if (ptrans->iMaskVar2 > 0 || ptrans->iTextureVar2 > 0)
            LineYFloorTexture(c, x, yb, yLo1-IP(yb), yLo2-IP(yb),
              ptrans->iMaskVar2 > 0 ? &ws.rgbMask[ptrans->iMaskVar2] : NULL,
              ptrans->iTextureVar2 > 0 ? &ws.rgcTexture[ptrans->iTextureVar2]
              : NULL, ptrans->rx3, ptrans->ry3, ptrans->rx4, ptrans->ry4,
              ptrans->fMaskSetVar, fFogCompute, fTrue,
              rScale * (real)(yLo - zEye));
        } else {

          // Looking straight through wall.
          if (fWall) {
            LineITrans(yHi1, yHi2, kv, dr.nTransPct2);
            LineITrans(yHi2, yLo2, kv, dr.nTransPct);
            LineITrans(yLo2, yLo1, kv, dr.nTransPct2);
          }
        }
      }
    } else if (fWall) {

      // Draw a default height wall.
      if (dr.nTrans != nTransVery) {
        LineITrans(yHi1, yLo1, kv, dr.nTransPct);
      } else {
        LineITrans(yHi1, yHi2, kv, dr.nTransPct2);
        LineITrans(yHi2, yLo2, kv, dr.nTransPct);
        LineITrans(yLo2, yLo1, kv, dr.nTransPct2);
      }
    }

    // Texture map the wall.
    if (fWall && (ptrans->iTexture > 0 || ptrans->iMask > 0))
      DrawTexture(c, x, IO(yHi1), IO(yLo1), yb+IO(ptrans->yElev), ~0,
        ptrans->iTexture, ptrans->iMask, ptrans->rTexture, ptrans->fTrans ?
        dr.nTransPct : 0, ptrans->nFog, ptrans->fMaskSet, kv);
  }
}


// Draw the 3D first person perspective inside view in a bitmap. This is one
// of the largest and most interesting functions in Daedalus. It has many
// cases, but its core is a pretty simple ray casting algorithm. It's very
// fast because it only shoots one ray per pixel column. Each ray is shot from
// the viewing position, looking at each bitmap array cell it passes over
// until it hits a wall i.e. set cell. Take the distance to the hit, and that
// determines the height of the wall at that column. This method has the same
// frame rate for a 10x10 bitmap array as it does in a 10000x10000 array,
// where it doesn't matter how many polygons the array contains. The classic
// PC games Wolfenstein 3D and DOOM do similar "one dimensional ray tracing".

flag RedrawInsidePerspective(CMazK &c)
{
  // Main variables for standard walls.
  KV kvWall, kv1, kv1Old, kv2, kv2Old;
  long xl, yl, ml, nl, me, ne;
  int xc = c.m_x >> 1, yc = c.m_y >> 1, yb, x, y, yk, xq, yq,
    xo, yo, xg, yg, xo0, yo0, xg0, yg0, xmax, ymax, xmax2, ymax2, mi, ni,
    ms, ns, mg, ng, i, k = 1, kOld = 1;
  real zl, z, zi, zj, mt, nt, md, nd, md0, nd0, zd, rScaleK, rScale, rd,
    rFogDist, rT;
  flag fOffset = FBetween(dr.nInside, nInsideSmooth, nInsideVeryFree),
    fBeenIn, fColor = !bm.k.FNull(), fAdd;

  // Variables for special features, such as colored floor and ceiling
  // squares, 3D passages in the floor and ceiling, semitransparent walls, and
  // variable height walls.
  CMaz *bMark2, *bWallVar = NULL, *bFogLit = NULL;
  CMazK *cMark = (dr.fMarkColor ? &bm.k2 : &bm.k), *cMark2, *cElev = NULL,
    *cWallVar = NULL;
  KV kv3D, kvMark, kvMarkOld, kvMark2, kvMarkOld2, kvMarkDef, kvT;
  flag fUnder = dr.zElev < -dr.zWall / 2, fExtra,
    fFog = (dr.nFog > 0), fFogLit, fFogDirt,
    fWallVar, fWallVar2, fInWallVar = fFalse,
    f3D = dr.f3D, fIn3D = fFalse, fIn3D2 = fFalse,
    fMark = (dr.fMarkAll || (!bm.b2.FNull() && bm.b2.m_x <= bm.b.m_x &&
    bm.b2.m_y <= bm.b.m_y)) && !fUnder,
    fInMark = fFalse, fMark2, fInMark2 = fFalse,
    fTrans = (dr.nTrans != nTransNone && !bm.b3.FNull()), fInTrans = fFalse;
  int zEye = ((dr.zElev + (dr.zWall >> 1)) << iVarScale) / dr.zWall,
    yExtra, yExtra2, yExtraK, yExtra2K,
    yElevMax, yElevMin, itransMac = 10, cTrans, kElev = 1, nFogT;
  flag fMarkAll = dr.fMarkAll, fColorMark = !cMark->FNull(), fColorMark2,
    fDoCompute = fFalse, fDoEnd = fFalse, fDoWall = fFalse,
    fDoSeal = fFalse, fDoTrans = fFalse, fDoWallVar = fFalse,
    fDoEnterMark = fFalse, fDoEnterMark2 = fFalse, fDoEnter3D = fFalse,
    fDoEnter3D2 = fFalse, fDoExitMark = fFalse, fDoExitMark2 = fFalse,
    fDoExit3D = fFalse, fDoExit3D2 = fFalse, fDoExitTrans = fFalse,
    fDoExitWallVar = fFalse, fTransStart = fFalse, fWallVarStart = fFalse,
    fDoTextureVar = fFalse;
  TRANS *trans, *ptransCur, *ptransOld;

  // Variables for texture mapping.
  int iTexture, iMask = 0,
    iTextureMark = 0, iMaskMark = 0, iTextureMark2 = 0, iMaskMark2 = 0;
  real rTexture, rx1, ry1, rx2, ry2, rx3, ry3, rx4, ry4;
  flag fTextureWall = dr.fTexture && dr.nTextureWall >= 0 &&
    ws.ccTexture > dr.nTextureWall && !ws.rgcTexture[dr.nTextureWall].FNull();
  flag fTextureDirt = dr.fTexture && dr.nTextureDirt >= 0 &&
    ws.ccTexture > dr.nTextureDirt && !ws.rgcTexture[dr.nTextureDirt].FNull();
  flag fTextureElev = dr.fTexture && dr.nTextureElev >= 0 &&
    ws.ccTexture > dr.nTextureElev && !ws.rgcTexture[dr.nTextureElev].FNull();
  CMaz *bTextureDirt = fTextureDirt && ws.cbMask > dr.nTextureDirt ?
    &ws.rgbMask[dr.nTextureDirt] : NULL;
  flag fTextureVar = dr.fTexture && dr.nTextureVar >= 0 &&
    ws.ccTexture > dr.nTextureVar && !ws.rgcTexture[dr.nTextureVar].FNull();
  CMaz *bTextureVar = fTextureVar && ws.cbMask > dr.nTextureVar ?
    &ws.rgbMask[dr.nTextureVar] : NULL;
  int nElev, yElev1, yElev2, iTextureElev, iMaskElev, fMaskSetElev;
  real zdElev, rTextureElev;

  // Prepare the bitmap for drawing.
  rT = RTanD(dr.dInside); rd = (real)xc / rT;
  if (c.m_x != dr.xCalc || rd != dr.rdCalc) {
    if (c.m_x > dr.ccalc) {
      if (dr.rgcalc != NULL)
        DeallocateP(dr.rgcalc);
      dr.ccalc = 0;
      dr.rgcalc = RgAllocate(c.m_x, CALC);
      if (dr.rgcalc == NULL)
        return fFalse;
      dr.ccalc = c.m_x;
    }
    for (x = 0; x < c.m_x; x++) {
      dr.rgcalc[x].rAngleColumn = RAtnD((real)(x - xc) / rd);
      dr.rgcalc[x].rDistCorrect = RCosD(dr.rgcalc[x].rAngleColumn);
    }
    dr.xCalc = c.m_x;
    dr.rdCalc = rd;
  }
  dr.yElevMax = ypWallMax; dr.yElevMin = -ypWallMax;

  // Initialize a bunch of variables.
  if (f3D) {
    xmax = bm.b.m_x3; ymax = bm.b.m_y3;
  } else {
    xmax = bm.b.m_x; ymax = bm.b.m_y;
  }
  if (dr.fNarrow) {
    xmax2 = Even(xmax); ymax2 = Even(ymax);
  } else {
    xmax2 = xmax; ymax2 = ymax;
  }
  xl = InsideFromA(dr.x) + dr.nOffsetX; yl = InsideFromA(dr.y) + dr.nOffsetY;
  zl = (real)(dr.dir * 90 + dr.nOffsetD);
  if (zl < 0.0)
    zl += rDegMax;
  yb = yc * (dr.yHorizon + 1000) / 1000;
  if (fOffset && f3D)
    yb -= NMultDiv(dr.nOffsetZ, yc, dr.zCell >> 1);
  xo0 = xo = dr.x; yo0 = yo = dr.y;
  rScaleK = (real)IP(dr.zWall * yc) * 0.801 / rT;
  rScale = (real)IP((dr.zWall + dr.zElev*2) * yc) * 0.801 / rT;
  rFogDist = (real)(dr.nFog * 10);
  kvWall = (f3D ? dr.kvIn3D : dr.kvInWall);
  GetKvIf(kv1, xo, yo, dr.z, kvWall);
  kv2 = kv1; kv1Old = kv1; kv2Old = kv2;
  kvMark = kvMarkOld = kvMark2 = kvMarkOld2 = kvMarkDef =
    (f3D ? dr.kvInWall : dr.kvIn3D);
  fMark2 = (dr.nMarkSky >= 0 && ws.cbMask > dr.nMarkSky &&
    !(bMark2 = &ws.rgbMask[dr.nMarkSky])->FNull()) && dr.zElev <= 0;
  cMark2 = fMark2 && ws.ccTexture > dr.nMarkSky &&
    !ws.rgcTexture[dr.nMarkSky].FNull() ? &ws.rgcTexture[dr.nMarkSky] : cMark;
  fColorMark2 = !cMark2->FNull();
  cElev = dr.nMarkElev >= 0 && ws.ccTexture > dr.nMarkElev &&
    !ws.rgcTexture[dr.nMarkElev].FNull() ? &ws.rgcTexture[dr.nMarkElev] :
    NULL;
  fWallVar2 = (dr.nWallVar >= 0 && ws.cbMask > dr.nWallVar && ws.ccTexture >
    dr.nWallVar && !(bWallVar = &ws.rgbMask[dr.nWallVar])->FNull() &&
    !(cWallVar = &ws.rgcTexture[dr.nWallVar])->FNull());
  fWallVar = fWallVar2 || dr.zElev != 0;
  fFogLit = fFog && dr.nFogLit >= 0 && ws.cbMask > dr.nFogLit &&
    !(bFogLit = &ws.rgbMask[dr.nFogLit])->FNull();
  fFogDirt = fFog && dr.fFogFloor;
  fExtra = fMark || fMark2 || f3D;
  if (fTrans || fWallVar) {
    if (fWallVar)
      itransMac = Min(dr.nClip + 1, itransMax);
    if (dr.rgtrans == NULL)
      dr.rgtrans = RgAllocate(itransMax, TRANS);
    trans = dr.rgtrans;
    if (trans == NULL)
      return fFalse;
    dr.nTransPct = ds.nTrans > 0 ? ds.nTrans / 100 : 50;
    dr.nTransPct2 = dr.nTransPct * 4 / 5;
  }

  // Clear the bitmap and draw all background stuff behind the walls.
  DrawBackground(c, f3D, zl, yb);

  // If the dot is located within a solid floor or wall, or semitransparent
  // wall, draw or prepare to draw later over the whole bitmap.
  if (fMark && cElev != NULL) {
    kvT = cElev->Get(xo, yo);
    if (kvT > 0 && zEye > 0 && zEye < kvT) {
      GetKvMark(kvT, *cMark, xo, yo)
      c.BitmapSet(kvT);
      return fTrue;
    }
  }
  if (!dr.fNoSubmerge) {
    if (fTrans && bm.b3.GetI(xo, yo, dr.z, f3D))
      fTransStart = fTrue;
    if (fTransStart || bm.b.GetI(xo, yo, dr.z, f3D)) {
      if (fWallVar && FWallVar(xo, yo)) {
        fWallVarStart = fTrue;
        ComputeWallVar(fWallVar2, bWallVar, cWallVar, xo, yo,
          &trans[0].yLo, &trans[0].yHi);
      }
      if (!fTransStart && (!fWallVarStart ||
        (zEye > trans[0].yLo && zEye < trans[0].yHi))) {
        c.BitmapSet(kv2);
        return fTrue;
      }
    }
  }
  if (fTransStart || fWallVarStart) {
    GetKvIf(kvT, xo, yo, dr.z, kvWall)
    trans[0].kv = trans[0].kvOrig = kvT;
    if (!fWallVarStart) {
      trans[0].yLo = 0; trans[0].yHi = 256;
    }
    trans[0].y = trans[0].y2 = ypWallMax;
    trans[0].yk = trans[0].yk2 = 0;
    trans[0].yElev = ypWallMax;
    trans[0].fTrans = fTransStart;
    trans[0].iTexture = trans[0].iMask = 0;
    trans[0].nFog = fFog;
    if (fTextureVar) {
      ComputeTextureFloor(&trans[0].iTextureVar,
        &trans[0].iMaskVar, xo, yo, fTrue, fFalse);
      ComputeTextureFloor(&trans[0].iTextureVar2,
        &trans[0].iMaskVar2, xo, yo, fTrue, fTrue);
      trans[0].fMaskSetVar = GetP(bTextureVar, xo, yo);
    } else
      trans[0].iTextureVar = trans[0].iMaskVar =
        trans[0].iTextureVar2 = trans[0].iMaskVar2 = 0;
  }

  // Loop over each column in the bitmap, to draw each column separately.
  for (x = 0; x < c.m_x; x++) {
    dr.yElevMax = yElevMax = ypWallMax;
    dr.yElevMin = yElevMin = -ypWallMax;

    // Figure out the angle of this ray, the corresponding horizontal and
    // vertical increments to get from cell to cell, and the characteristics
    // of the starting cell.
    zi = dr.rgcalc[x].rAngleColumn;
    z = zl - zi;
    if (z < 0.0)
      z += rDegMax;
    else if (z >= rDegMax)
      z -= rDegMax;
    xq = (z >= rDegHalf) ? 1 : -1;
    yq = (z >= rDegQuad && z < rDeg34) ? 1 : -1;
    zj = z - ((real)(dr.dir*90));
    if (zj > rDegHalf)
      zj -= rDegMax;
    if FOdd(dr.dir) {
      mi = xq; ni = yq;
      ml = xl; nl = yl;
    } else {
      mi = yq; ni = xq;
      ml = yl; nl = xl;
    }
    mg = AFromInside(ml); ng = AFromInside(nl);
    mt = RAbs(RTanD(zj));
    nt = RDiv(1.0, mt);
    me = InsideFromA0(mg + (mi > 0));
    ne = InsideFromA0(ng + (ni > 0));
    ms = ns = dr.zCell;
    if (dr.fNarrow) {
      if (FOdd(mg))
        ms = dr.zCellNarrow;
      if (FOdd(ng))
        ns = dr.zCellNarrow;
    }
    rT = (real)(dr.zCell << 1);
    md0 = RSqr(LengthI(rT, mt)) / rT;
    nd0 = RSqr(LengthI(rT, nt)) / rT;
    md = DistanceM();
    nd = DistanceN();

    fBeenIn = dr.nEdge == nEdgeTorus || bm.b.FLegalLevel(xo, yo, f3D);
    fInMark = fInMark2 = fIn3D = fIn3D2 = fFalse;
    if (fExtra || fTextureVar) {
      if (f3D && !fUnder && !bm.b.Get3I(xo, yo, dr.z + 1)) {
        fIn3D = fTrue;
        yExtra = yExtraK = ypWallMax;
      } else if (fMark && FMark(bm.b2, xo, yo)) {
        fInMark = fTrue;
        yExtra = yExtraK = yElev2 = ypWallMax;
        if (cElev != NULL)
          nElev = cElev->GetF(xo, yo, dr.z, dr.f3D);
      }
      if (f3D && !fUnder && !bm.b.Get3I(xo, yo, dr.z - 1)) {
        fIn3D2 = fTrue;
        yExtra2 = yExtra2K = ypWallMax;
      } else if (fMark2 && FMark(*bMark2, xo, yo)) {
        fInMark2 = fTrue;
        yExtra2 = yExtra2K = ypWallMax;
      }
      if (fInMark || fInMark2 || fTextureVar) {
        if (fColorMark) {
          GetKvMark(kvMark, *cMark, xo, yo);
        }
        if (fColorMark2) {
          GetKvMark(kvMark2, *cMark2, xo, yo);
        }
        if (fTextureDirt || fTextureVar) {
          if (fTextureDirt) {
            ComputeTextureFloor(&iTextureMark,  &iMaskMark,  xo, yo,
              fFalse, fFalse);
            ComputeTextureFloor(&iTextureMark2, &iMaskMark2, xo, yo,
              fFalse, fTrue);
          }
          i = !dr.fNarrow || FOdd(xo) ? dr.zCell : dr.zCellNarrow;
          rx1 = (real)(dr.nOffsetX + (i >> 1)) / (real)i;
          i = !dr.fNarrow || FOdd(yo) ? dr.zCell : dr.zCellNarrow;
          ry1 = (real)(dr.nOffsetY + (i >> 1)) / (real)i;
          rx3 = 1.0 - rx1; ry3 = ry1;
          EnsureBetween(rx1, 0.0, 1.0);
          EnsureBetween(ry1, 0.0, 1.0);
        }
        if (fTextureVar && fWallVarStart) {
          trans[0].rx1 = rx1; trans[0].ry1 = ry1;
          trans[0].rx3 = rx3; trans[0].ry3 = ry3;
          if (trans[0].iTextureVar > 0 || trans[0].iMaskVar > 0 ||
            trans[0].iTextureVar2 > 0 || trans[0].iMaskVar2 > 0)
            fDoTextureVar = fTrue;
        }
      }
    }
    fInTrans = fTransStart;
    fInWallVar = fWallVarStart;
    cTrans = fInTrans || fInWallVar;

    // Shoot the ray. Jump from cell to neighboring cell until a wall is hit,
    // not giving up until the clipping plane is reached.
    for (i = 0; i < dr.nClip; i++) {
      if (md < nd)
        mg += mi;
      else
        ng += ni;
      if (FOdd(dr.dir)) {
        xg0 = mg; yg0 = ng;
      } else {
        xg0 = ng; yg0 = mg;
      }
      xg = xg0; yg = yg0;

      // Is the ray not on the bitmap?
      if (xg < 0 || xg >= xmax || yg < 0 || yg >= ymax) {
        if (dr.nEdge == nEdgeTorus) {
          while (xg < 0)
            xg += xmax2;
          while (xg >= xmax2)
            xg -= xmax2;
          while (yg < 0)
            yg += ymax2;
          while (yg >= ymax2)
            yg -= ymax2;
          fBeenIn = fTrue;
        } else {

          // If the ray from on to off the bitmap, do some cleanup, such as
          // finish the current floor marking in progress.
          if (fBeenIn) {
            fDoEnd = fTrue;
            if (dr.fSealEntrance)
              fDoSeal = fDoCompute = fTrue;
            if (fTrans && dr.nTrans == nTransVery && fInTrans)
              fDoExitTrans = fDoCompute = fTrue;
            if (fWallVar && fInWallVar)
              fDoExitWallVar = fDoCompute = fTrue;
          }

          // Performance: If off the bitmap, and moving farther away from the
          // bitmap, a wall will never be hit, so finish this column now.
          if ((yo < 0 && yg < yo) || (xo < 0 && xg < xo) ||
            (yo >= ymax && yg > yo) || (xo >= xmax && xg > xo))
            break;
        }
      } else
        fBeenIn = fTrue;

      // Check for existence of a semitransparent or variable height wall. If
      // so, remember it for later, then keep searching for a solid wall.
      if (fTrans) {
        if (bm.b3.GetI(xg, yg, dr.z, f3D) && cTrans < itransMac)
          fDoTrans = fDoCompute = fTrue;
        else if (fInTrans) {
          fDoExitTrans = fTrue;
          if (dr.nTrans == nTransVery)
            fDoCompute = fTrue;
        }
      }
      if (fWallVar) {
        if ((fDoTrans || bm.b.GetI(xg, yg, dr.z, f3D)) && FWallVar(xg, yg) &&
          cTrans < itransMac)
          fDoWallVar = fDoCompute = fTrue;
        else if (fInWallVar)
          fDoExitWallVar = fDoCompute = fTrue;
      }
      if (fDoTrans || fDoWallVar)
        goto LNotSet;

      // Is this a solid wall? If so go draw it.
      if (bm.b.GetIFast(xg, yg, dr.z, f3D)) {
        fDoWall = (cTrans < itransMac);
        fDoEnd = fDoCompute = fTrue;
      } else {

        // This is not a solid wall. There are things that might need drawing
        // in the floor or ceiling area at the current cell as the search
        // continues for the wall in the distance. Colored floor and ceiling
        // cell markings are implemented by getting the vertical coordinate as
        // if there were a wall there, then drawing the column when the floor
        // changes color. Pits in the floor and ceiling are the same as floor
        // markings, just that columns are colored appropriately based on what
        // direction they exit the cell, i.e. what side of the pit seems to
        // face you.
LNotSet:
        if (fExtra) {
          if (f3D) {
            if (!fIn3D && !fUnder && !bm.b.Get3I(xg, yg, dr.z + 1)) {
              fDoEnter3D = fDoCompute = fTrue;
              if (fInMark)
                fDoExitMark = fTrue;
            } else if (fIn3D && bm.b.Get3I(xg, yg, dr.z + 1))
              fDoExit3D = fDoCompute = fTrue;
            if (!fIn3D2 && !fUnder && !bm.b.Get3I(xg, yg, dr.z - 1)) {
              fDoEnter3D2 = fDoCompute = fTrue;
              if (fInMark2)
                fDoExitMark2 = fTrue;
            } else if (fIn3D2 && bm.b.Get3I(xg, yg, dr.z - 1))
              fDoExit3D2 = fDoCompute = fTrue;
          }
          if (fMark) {
            if (!fInMark && (!fIn3D || fDoExit3D) && !fDoEnter3D &&
              FMark(bm.b2, xg, yg))
              fDoEnterMark = fDoCompute = fTrue;
            else if (fInMark) {
              if (!FMark(bm.b2, xg, yg))
                fDoExitMark = fDoCompute = fTrue;
              else {
                if (fColorMark) {
                  kvMarkOld = kvMark;
                  GetKvMark(kvMark, *cMark, xg, yg);
                  if (kvMark != kvMarkOld)
                    fDoExitMark = fDoEnterMark = fDoCompute = fTrue;
                }
                if (fTextureDirt || fFogLit || cElev != NULL)
                  fDoExitMark = fDoEnterMark = fDoCompute = fTrue;
              }
            }
          }
          if (fMark2) {
            if (!fInMark2 && (!fIn3D2 || fDoExit3D2) && !fDoEnter3D2 &&
              FMark(*bMark2, xg, yg))
              fDoEnterMark2 = fDoCompute = fTrue;
            else if (fInMark2) {
              if (!FMark(*bMark2, xg, yg))
                fDoExitMark2 = fDoCompute = fTrue;
              else {
                if (fColorMark2) {
                  kvMarkOld2 = kvMark2;
                  GetKvMark(kvMark2, *cMark2, xg, yg);
                  if (kvMark2 != kvMarkOld2)
                    fDoExitMark2 = fDoEnterMark2 = fDoCompute = fTrue;
                }
                if (fTextureDirt || fFogLit)
                  fDoExitMark2 = fDoEnterMark2 = fDoCompute = fTrue;
              }
            }
          }
        }

        // Ran off the end of the clipping plane, so leave current column.
        if (i >= dr.nClip - 1)
          fDoEnd = fTrue;
      }

      // If ending column, indicate anything in progress should be finished.
      if (fDoEnd) {
        if (fExtra) {
          if (fIn3D)
            fDoExit3D = fDoCompute = fTrue;
          else if (fInMark)
            fDoExitMark = fDoCompute = fTrue;
          if (fIn3D2)
            fDoExit3D2 = fDoCompute = fTrue;
          else if (fInMark2)
            fDoExitMark2 = fDoCompute = fTrue;
        }
        if (fInTrans)
          fDoExitTrans = fDoCompute = fTrue;
        if (fInWallVar)
          fDoExitWallVar = fDoCompute = fTrue;
      }
      if (fDoCompute) {
        zd = Min(md, nd) * dr.rgcalc[x].rDistCorrect;
        if (zd < 1.0)
          zd = 1.0;
        rT = rScale / zd;
        y = rT > (real)ypWallMax ? ypWallMax : (int)rT;
        rT = rScaleK / zd;
        yk = rT > (real)ypWallMax ? ypWallMax : (int)rT;
        fDoCompute = fFalse;
      }

      // There's a floor/ceiling marking/pit here.
      if (fExtra) {
        if (fDoExit3D) {
          Ensure3DColor(xg, yg, 1);
          LineI(yb+IO(y), yb+IO(yExtra), kv3D);
          DrawTexture3D(1);
          fIn3D = fDoExit3D = fFalse;
        } else if (fDoExitMark) {
          if (cElev == NULL) {
            if (iTextureMark == 0 || iMaskMark > 0)
              LineYFloor(c, x, yb, IO(y), IO(yExtra), fDoEnterMark ?
                kvMarkOld : kvMark, 0, FFogDirt(xo, yo), rScale);
            if (fTextureDirt && (iTextureMark > 0 || iMaskMark > 0)) {
              ComputeDir(k);
              ComputeTextureOffset(xo, yo);
              ComputeTextureOffsetFloor2(rx2, ry2);
              LineYFloorTexture(c, x, yb, y, yExtra,
                iMaskMark > 0 ? &ws.rgbMask[iMaskMark] : NULL,
                iTextureMark > 0 ? &ws.rgcTexture[iTextureMark] : NULL,
                rx2, ry2, rx1, ry1,
                GetP(bTextureDirt, xo, yo), FFogDirt(xo, yo), fFalse, rScale);
            }
          } else {
            yElev1 = y;
            if (nElev > 0 && yElev1 < ypWallMax && yk < ypWallMax)
              yElev1 -= LMultShift(yk, nElev, 7);
            if (yElev1 < yElevMax) {
              kvT = fDoEnterMark ? kvMarkOld : kvMark;
              if (yElev1 <= yElev2) {
                if (iTextureMark == 0 || iMaskMark > 0)
                  LineYFloor(c, x, yb, IO(yElev1), IO(yElev2),
                    dr.fMarkBlock && nElev > 0 ? KvShade(kvT,
                    RAbs(dr.rLightFactor) / 2.0) : kvT, 0, FFogDirt(xo, yo),
                    rScale * (real)No0(zEye - nElev) / (real)No0(zEye));
                if (fTextureDirt && (iTextureMark > 0 || iMaskMark > 0)) {
                  ComputeDir(k);
                  ComputeTextureOffset(xo, yo);
                  ComputeTextureOffsetFloor2(rx2, ry2);
                  LineYFloorTexture(c, x, yb, yElev1, yElev2,
                    iMaskMark > 0 ? &ws.rgbMask[iMaskMark] : NULL,
                    iTextureMark > 0 ? &ws.rgcTexture[iTextureMark] : NULL,
                    rx2, ry2, rx1, ry1, GetP(bTextureDirt, xo, yo),
                    FFogDirt(xo, yo), fFalse,
                    rScale * (real)No0(zEye - nElev) / (real)No0(zEye));
                }
              }
              yElevMax = yElev1;
              dr.yElevMax = yb+IO(yElevMax);
              dr.yElevMin = yb+IO(yElevMin);
            }
          }
          fInMark = fDoExitMark = fFalse;
        }
        if (fDoExit3D2) {
          Ensure3DColor(xg, yg, -1);
          LineI(yb-IO(yExtra2), yb-IO(y), kv3D);
          DrawTexture3D(-1);
          fIn3D2 = fDoExit3D2 = fFalse;
        } else if (fDoExitMark2) {
          if (iTextureMark2 == 0 || iMaskMark2 > 0)
            LineYFloor(c, x, yb, -IO(yExtra2), -IO(y), fDoEnterMark2 ?
              kvMarkOld2 : kvMark2, 0, FFogDirt(xo, yo), rScale);
          if (fTextureDirt && (iTextureMark2 > 0 || iMaskMark2 > 0)) {
            ComputeDir(k);
            ComputeTextureOffset(xo, yo);
            ComputeTextureOffsetFloor4(rx4, ry4);
            LineYFloorTexture(c, x, yb, iRound - yExtra2, iRound - y,
              iMaskMark2 > 0 ? &ws.rgbMask[iMaskMark2] : NULL,
              iTextureMark2 > 0 ? &ws.rgcTexture[iTextureMark2] : NULL,
              rx3, ry3, rx4, ry4,
              FFogDirt(xo, yo), GetP(bTextureDirt, xo, yo), fFalse, rScale);
          }
          if (y > yElevMin)
            yElevMin = -y;
          fInMark2 = fDoExitMark2 = fFalse;
        }

        if (fDoEnter3D) {
          yExtra = y; yExtraK = yk;
          fIn3D = fTrue;
          fDoEnter3D = fFalse;
        } else if (fDoEnterMark) {
          yExtra = y; yExtraK = yk;
          if (fColorMark) {
            GetKvMark(kvMark, *cMark, xg, yg);
          }
          if (cElev != NULL) {
            nElev = cElev->GetF(xg, yg, dr.z, dr.f3D);
            yElev2 = yExtra;
            if (nElev > 0 && yElev2 < ypWallMax && yExtraK < ypWallMax)
              yElev2 -= LMultShift(yExtraK, nElev, 7);
            ComputeDir(kElev);
            zdElev = zd;
            if (fTextureElev) {
              ComputeTexture(&iTextureElev, &iMaskElev,
                &rTextureElev, dr.nTextureElev,
                xg, yg, 0, kElev, xq, yq, me, ml, ne, nl, md, nd, mt, nt);
              fMaskSetElev =
                !ws.rgbMask[0].FNull() && ws.rgbMask[0].Get(xg, yg);
            } else
              iTextureElev = iMaskElev = 0;
            if (nElev > 0 && yElev2 < yElevMax) {
              kvT = kvMark;
              if (kElev & 1) {
                if (!dr.fMarkBlock)
                  kvT = KvShade(kvT, RAbs(dr.rLightFactor) / -2.0);
              } else if (kElev == 0)
                kvT = KvShade(kvT, -dr.rLightFactor);
              else
                kvT = KvShade(kvT, dr.rLightFactor);
              if (FFogSide(xg, yg))
                kvT = KvApplyFog(kvT);
              dr.yElevMax = yb+IO(yElevMax);
              LineI(yb+IO(yElev2), yb+IO(yExtra), kvT);
              if (iTextureElev > 0 || iMaskElev > 0)
                DrawTexture(c, x, yb+IO(yElev2), yb+IO(yExtra),
                  yb+IO(yElevMax), ~0, iTextureElev, iMaskElev, rTextureElev,
                  0, nFogCompute, fMaskSetElev, kvT);
              yElevMax = yElev2;
            }
          }
          if (fTextureDirt) {
            ComputeTextureFloor(&iTextureMark, &iMaskMark, xg, yg,
              fFalse, fFalse);
            if (iTextureMark > 0 || iMaskMark > 0) {
              ComputeDir(k);
              ComputeTextureOffset(xo, yo);
              ComputeTextureOffsetFloor1(rx1, ry1);
            }
          }
          fInMark = fTrue;
          fDoEnterMark = fFalse;
        }
        if (fDoEnter3D2) {
          yExtra2 = y; yExtra2K = yk;
          fIn3D2 = fTrue;
          fDoEnter3D2 = fFalse;
        } else if (fDoEnterMark2) {
          yExtra2 = y; yExtra2K = yk;
          if (fColorMark2) {
            GetKvMark(kvMark2, *cMark2, xg, yg);
          }
          if (fTextureDirt) {
            ComputeTextureFloor(&iTextureMark2, &iMaskMark2, xg, yg,
              fFalse, fTrue);
            if (iTextureMark2 > 0 || iMaskMark2 > 0) {
              ComputeDir(k);
              ComputeTextureOffset(xo, yo);
              ComputeTextureOffsetFloor3(rx3, ry3);
            }
          }
          fInMark2 = fTrue;
          fDoEnterMark2 = fFalse;
        }
      }

      // There's a semitransparent or variable height wall here. Compute its
      // coordinates for later drawing, then keep searching for a solid wall.
      if (fDoTextureVar) {
        ptransOld = cTrans > 0 ? &trans[cTrans-1] : NULL;
        ComputeDir(k);
        ComputeTextureOffset(xg, yg);
        if (ptransOld != NULL) {
          if (ptransOld->iTextureVar > 0 || ptransOld->iMaskVar > 0) {
            ComputeTextureOffsetFloor2(ptransOld->rx2, ptransOld->ry2);
          }
          if (ptransOld->iTextureVar2 > 0 || ptransOld->iMaskVar2 > 0) {
            ComputeTextureOffsetFloor4(ptransOld->rx4, ptransOld->ry4);
          }
        }
        fDoTextureVar = fFalse;
      }
      if (fDoTrans || fDoWallVar) {
        ptransOld = cTrans > 0 ? &trans[cTrans-1] : NULL;
        GetKvIf(kvT, xg, yg, dr.z, kvWall);
        nFogT = nFogCompute;
        fAdd = cTrans <= 0 || kvT != ptransOld->kvOrig ||
          ((nFogT > 0) != (ptransOld->nFog > 0)) ||
          (fDoTrans && (!fInTrans || fDoExitWallVar)) ||
          (fDoWallVar && (!fInWallVar || (!fDoTrans && ptransOld->fTrans)));
        if (fAdd || fDoWallVar || fTextureWall || fTextureVar) {

          // Add new entry to the semitransparent / variable height wall list.
          ptransCur = &trans[cTrans];
          if (fDoWallVar)
            ComputeWallVar(fWallVar2, bWallVar, cWallVar, xg, yg,
              &ptransCur->yLo, &ptransCur->yHi);
          else {
            ptransCur->yLo = 0; ptransCur->yHi = 256;
          }
          if (fTextureWall) {
            ComputeDir(k);
            ComputeTexture(&ptransCur->iTexture, &ptransCur->iMask,
              &ptransCur->rTexture, dr.nTextureWall,
              xg, yg, 0, k, xq, yq, me, ml, ne, nl, md, nd, mt, nt);
            ptransCur->fMaskSet = ws.cbMask > 0 &&
              !ws.rgbMask[0].FNull() && ws.rgbMask[0].Get(xg, yg);
          } else
            ptransCur->iTexture = ptransCur->iMask = 0;
          if (fTextureVar) {
            ComputeTextureFloor(&ptransCur->iTextureVar,
              &ptransCur->iMaskVar, xg, yg, fTrue, fFalse);
            ComputeTextureFloor(&ptransCur->iTextureVar2,
              &ptransCur->iMaskVar2, xg, yg, fTrue, fTrue);
            if (ptransCur->iTextureVar > 0 || ptransCur->iMaskVar > 0 ||
              ptransCur->iTextureVar2 > 0 || ptransCur->iMaskVar2 > 0) {
              ComputeDir(k);
              ComputeTextureOffset(xg, yg);
              if (ptransCur->iTextureVar > 0 || ptransCur->iMaskVar > 0) {
                ComputeTextureOffsetFloor1(ptransCur->rx1, ptransCur->ry1);
              }
              if (ptransCur->iTextureVar2 > 0 || ptransCur->iMaskVar2 > 0) {
                ComputeTextureOffsetFloor3(ptransCur->rx3, ptransCur->ry3);
              }
              ptransCur->fMaskSetVar = GetP(bTextureVar, xg, yg);
              fDoTextureVar = fTrue;
            }
          } else
            ptransCur->iTextureVar = ptransCur->iMaskVar =
              ptransCur->iTextureVar2 = ptransCur->iMaskVar2 = 0;
          if (fAdd || ptransCur->iTexture > 0 || ptransCur->iMask > 0 ||
            ptransCur->yLo != ptransOld->yLo ||
            ptransCur->yHi != ptransOld->yHi || fTextureVar) {
            ptransCur->y = ptransCur->y2 = y;
            ptransCur->yk = ptransCur->yk2 = yk;
            ptransCur->yElev = yElevMax;
            ptransCur->yElev2 = yElevMin;
            ptransCur->fTrans = fDoTrans;
            ptransCur->nFog = nFogT;
            ptransCur->kvOrig = kvT;
            ShadeKv(kvT);
            ptransCur->kv = kvT;
            if ((fInTrans || fInWallVar) && ptransOld != NULL) {
              ptransOld->y2 = ptransCur->y;
              ptransOld->yk2 = ptransCur->yk;
            }
            cTrans++;
          }
        }
        fInTrans = fDoTrans;
        fInWallVar = fDoWallVar;
        fDoTrans = fDoWallVar = fFalse;
      }
      if (fDoExitTrans || fDoExitWallVar) {
        if (cTrans > 0) {
          ptransOld = &trans[cTrans-1];
          ptransOld->y2 = y;
          ptransOld->yk2 = yk;
        }
        if (fDoExitTrans)
          fInTrans = fDoExitTrans = fFalse;
        if (fDoExitWallVar)
          fInWallVar = fDoExitWallVar = fFalse;
      }

      if (fDoEnd) {
        if (fDoSeal) {
          dr.yElevMax = yb+IO(yElevMax);
          dr.yElevMin = yb+IO(yElevMin);
          LineI(yb+IO(y - (yk << 1)), yb+IO(y), dr.kvInEdge);
          fDoSeal = fFalse;
        }
        fDoEnd = fFalse;
        break;
      }

      // Follow the ray to the next neighboring cell.
      xo0 = xg0; yo0 = yg0; xo = xg; yo = yg;
      if (md < nd) {
        me += mi*ms;
        if (dr.fNarrow)
          ms = (ms == dr.zCell ? dr.zCellNarrow : dr.zCell);
        md = DistanceM();
      } else {
        ne += ni*ns;
        if (dr.fNarrow)
          ns = (ns == dr.zCell ? dr.zCellNarrow : dr.zCell);
        nd = DistanceN();
      }
    }

    if (fDoWall) {
      fDoWall = fFalse;

      // Figure out what color to draw the wall at the current column in.
      ComputeDir(k);
      if (fColor) {
        GetKv(kv1, bm.k, xg, yg, dr.z, kvWall);
      }
      if (k != kOld || kv1 != kv1Old || fFog) {
        kOld = k; kv1Old = kv1;
        kv2 = kv1;
        ShadeKvCore(kv2);
      }

      // Do texture mapping for the wall at the current column.
      if (fTextureWall) {
        ComputeTexture(&iTexture, &iMask, &rTexture, dr.nTextureWall,
          xg, yg, 0, k, xq, yq, me, ml, ne, nl, md, nd, mt, nt);
        if (iTexture > 0 && iMask == 0) {
          DrawTexture(c, x, yb-IO(y), yb+IO(y), yb+IO(yElevMax), ~0, iTexture,
            0, rTexture, 0, nFogCompute, fFalse, kv2);
          goto LNext;
        }
      }

      // Actually draw the wall for the current column here.
      dr.yElevMax = yb+IO(yElevMax);
      dr.yElevMin = yb+IO(yElevMin);
      LineYGradient(c, x, yb-IO(y), yb+IO(y), kv2, dr.kvInWall2);

      // Do texture mapping on top of the drawn wall at the current column.
      if (fTextureWall && iMask > 0)
        DrawTexture(c, x, yb-IO(y), yb+IO(y), yb+IO(yElevMax), ~0,
          iTexture, iMask, rTexture, 0, nFogCompute,
          !ws.rgbMask[0].FNull() && ws.rgbMask[0].Get(xg, yg), kv2);
    }

LNext:
    // Finally draw the list of semitransparent lines over top of this column.
    if ((fTrans || fWallVar) && cTrans > 0)
      DrawTransVar(c, x, yb, cTrans, trans, rScale);
    xo0 = xo = AFromInside(xl); yo0 = yo = AFromInside(yl);
  }
  return fTrue;
}


// Draw a stereoscopic 3D version of the first person perspective inside view
// in a bitmap. This involves drawing "left eye" and "right eye" versions of
// the scene from slightly different points of view.

flag RedrawInsidePerspectiveStereo(CMazK &c, CMazK &cs)
{
  int mSav = dr.nOffsetX, nSav = dr.nOffsetY, dSav = dr.nOffsetD,
    x, f, m, n, d;

  x = !ds.fStereo3D ? c.m_x >> 1 : c.m_x;
  if (!cs.FBitmapSizeSet(x, c.m_y))
    return fFalse;
  f = FOdd(c.m_x) && !ds.fStereo3D;
  d = 180 - (dr.dir*90 + dr.nOffsetD);
  if (d >= nDegMax)
    d -= nDegMax;
  m = NCosRD(ds.nStereo, d); n = NSinRD(ds.nStereo, d);

  // Draw left eye view of scene.
  dr.nOffsetX += m; dr.nOffsetY += n; //dr.nOffsetD += 1;
  if (!RedrawInsidePerspective(cs))
    return fFalse;
  if (ds.fStereo3D) {
    cs.ColmapGrayscale();
    cs.ColmapOrAndKv(kvCyan, 0);
  }
  c.BlockMove(cs, 0, 0, x-1, c.m_y-1, 0, 0);

  // Draw right eye view of scene.
  dr.nOffsetX -= m*2; dr.nOffsetY -= n*2; //dr.nOffsetD -= 1*2;
  if (!RedrawInsidePerspective(cs))
    return fFalse;
  if (!ds.fStereo3D)
    c.BlockMove(cs, 0, 0, x-1, c.m_y-1, x+f, 0);
  else {
    cs.ColmapGrayscale();
    cs.ColmapOrAndKv(kvRed, 0);
    c.ColmapOrAnd(cs, 1);
  }

  // Draw line down middle if outer bitmap is odd sized.
  if (f)
    c.LineY(x, 0, c.m_y-1, dr.kvInEdge);
  dr.nOffsetX = mSav; dr.nOffsetY = nSav; dr.nOffsetD = dSav;
  return fTrue;
}


/*
******************************************************************************
** Simple Perspective Inside Display
******************************************************************************
*/

#define ZCell(z) ((z) * 3 / 5)
#define ZCellNarrow(z) ((z) * (32 - 16 * dr.zCellNarrow / dr.zCell) / 32)

// Draw a line on the bitmap twice, at the given coordinates and also at the
// coordinates flipped vertically. Used by RedrawInside(), where the top and
// bottom halves of the bitmap when drawing a Maze in simple perspective mode
// are mirror images of each other.

void DrawInside(CMazK &c, int x1, int y1, int x2, int y2, int i)
{
  int x = c.m_x-1, y = c.m_y-1;

  i = i > 0;
  c.Line(i ? x1 : x-x1, y1,   i ? x2 : x-x2, y2,   dr.kvInEdge);
  c.Line(i ? x1 : x-x1, y-y1, i ? x2 : x-x2, y-y2, dr.kvInEdge);
}


// Draw the simple mode version of 3D first person perspective inside view.
// This only works for 2D Mazes, and can only display views looking straight
// down passages. The algorithm draws lines indicating corners between walls.
// There's a finite set of lines, where it's a matter of determining whether
// or not to draw each line.

void RedrawInside(CMazK &c)
{
  KV kv = dr.kvInWall, kvT;
  int xc = c.m_x >> 1, yc = c.m_y >> 1,
    xm, ym, xn, yn, xd, yd, xe, ye, xl, yl, xf, yf, xx, i, j, k;

  // Initialize background color.
  if (!bm.k.FNull() && bm.k.FLegal(dr.x, dr.y)) {
    kvT = bm.k.Get(dr.x, dr.y);
    if (kvT != dr.kvInEdge)
      kv = kvT;
  }
  c.BitmapSet(kv);

  // Draw the left and right halves of the bitmap separately.
  for (i = -1; i <= 1; i += 2) {
    xm = dr.x; ym = dr.y;
    j = (dr.dir + i) & DIRS1;
    xd = xm + xoff[j]; yd = ym + yoff[j];
    xl = (xc * 3 / 4) * 5 / 3; yl = (yc * 3 / 4) * 5 / 3;
    xx = 0;

    // Loop over each cell looking down the passage from the viewing point.
    for (j = 0; j < 20; j++) {
      xn = xm + xoff[dr.dir]; yn = ym + yoff[dr.dir];
      xe = xd + xoff[dr.dir]; ye = yd + yoff[dr.dir];
      if (!dr.fNarrow || !FOdd(j)) {
        xf = ZCell(xl); yf = ZCell(yl);
      } else {
        xf = ZCellNarrow(xl); yf = ZCellNarrow(yl);
      }

      // Done when a wall is reached, or when the walls will be drawn at a
      // size of one pixel.
      if (bm.b.Get(xm, ym) ||
        ((xl == xf || yl == yf) && (dr.zCellNarrow != 0 || !FOdd(j))))
        break;

      // For passages that go outside the Maze, draw a horizon line.
      if (bm.b.Get(xd, yd)) {
        DrawInside(c, xc-xl, yc-yl, xc-xf, yc-yf, i);
        xx = xc-xf;
      } else if (bm.b.Get(xe, ye)) {
        DrawInside(c, xx, yc-yf, xc-xf, yc-yf, i);
        xx = xc-xf;
      }

      // Draw lines on the bitmap where appropriate.
      if (bm.b.Get(xn, yn)) {
        DrawInside(c, xc-xf, yc-yf, xc, yc-yf, i);
        if (bm.b.Get(xd, yd) || !bm.b.Get(xe, ye))
          DrawInside(c, xc-xf, yc-yf, xc-xf, yc, i);
        if (!bm.b.Get(xe, ye)) {
          if (j == 0)
            DrawInside(c, xc-xl, yc-yf+(yl-yf), xc-xf, yc-yf, i);
          else
            DrawInside(c, xx, yc, xc-xf, yc, i);
        }
      } else if (bm.b.Get(xd, yd) != bm.b.Get(xe, ye)) {
        DrawInside(c, xc-xf, yc-yf, xc-xf, yc, i);
        xx = xc-xf;
      }

      // Draw "X" on the ground if marking there.
      if (!bm.b2.FNull() && bm.b2.FLegal(xm, ym) && bm.b2.Get(xm, ym) &&
        !bm.b.Get(xm, ym) && j > 0 && (!dr.fNarrow || !FOdd(j))) {
        k = yc + yl+(yf-yl)*63/100;
        c.Line(xc + i*xl*50/100, yc + yl+(k-yc-yl)*50/100, xc, k,
          dr.kvInEdge);
        c.Line(xc + i*xf*63/100, yc + yf+(k-yc-yf)*37/100, xc, k,
          dr.kvInEdge);
      }

      // Figure out the coordinates for the next cell down the passage.
      xm = xn; ym = yn;
      xn += xoff[dr.dir]; yn += yoff[dr.dir];
      xd = xe; yd = ye;
      xe += xoff[dr.dir]; ye += yoff[dr.dir];
      xl = xf; yl = yf;
    }
    if (yl == yf && (dr.zCellNarrow != 0 || !bm.b.FLegal(xm, ym)))
      DrawInside(c, xx, yc, xc, yc, i);
  }

  // Decorate the exposed parts of the sky and ground.
  if (!bm.b.Get(dr.x, dr.y)) {
    c.FFill(xc, c.m_y-1, dr.kvInDirt, -1, fFalse);
    c.FFill(xc, 0,             dr.kvInSky,  -1, fFalse);
  }
  if (ds.fStar)
    FCreateInsideStars(&c, (real)(dr.dir * 90), 0, fFalse);
}


// Draw a line on the bitmap, flipping the horizontal and vertical coordinates
// when specified.

void DrawInside3(CMazK &c, int x1, int y1, int x2, int y2, int i, int j)
{
  int x = c.m_x-1, y = c.m_y-1;

  i = i > 0;
  c.Line(i ? x1 : x-x1, j ? y-y1 : y1,
    i ? x2 : x-x2, j ? y-y2 : y2, dr.kvInEdge);
}


// A version of RedrawInside() that works for 3D Mazes.

void RedrawInside3(CMazK &c)
{
  KV kv = dr.kvIn3D, kvT;
  int xc = c.m_x >> 1, yc = c.m_y >> 1,
    xm, ym, z, xn, yn, xd, yd, xe, ye, xl, yl, xf, yf, i, j, k;

  // Initialize background color.
  if (!bm.k.FNull() && bm.k.FLegal(dr.x, dr.y)) {
    kvT = bm.k.Get(dr.x, dr.y);
    if (kvT != dr.kvInEdge)
      kv = kvT;
  }
  c.BitmapSet(kv);

  // Draw the left and right halves of the bitmap separately.
  for (i = -1; i <= 1; i += 2) {
    xm = dr.x; ym = dr.y; z = dr.z;
    j = (dr.dir + i) & DIRS1;
    xd = xm + xoff[j]; yd = ym + yoff[j];
    xl = (xc * 3 / 4) * 5 / 3; yl = (yc * 3 / 4) * 5 / 3;

    // Loop over each cell looking down the passage from the viewing point.
    for (j = 0; j < 20; j++) {
      xn = xm + xoff[dr.dir]; yn = ym + yoff[dr.dir];
      xe = xd + xoff[dr.dir]; ye = yd + yoff[dr.dir];
      if (!dr.fNarrow || !FOdd(j)) {
        xf = ZCell(xl); yf = ZCell(yl);
      } else {
        xf = ZCellNarrow(xl); yf = ZCellNarrow(yl);
      }

      // Done when a wall is reached, or when the walls will be drawn at a
      // size of one pixel.
      if (bm.b.Get3(xm, ym, z) ||
        ((xl == xf || yl == yf) && (dr.zCellNarrow != 0 || !FOdd(j))))
        break;

      // Draw lines on the bitmap where appropriate.
      if (bm.b.Get3(xm, ym, z-1) ^ bm.b.Get3(xn, yn, z-1) ^
        bm.b.Get3(xn, yn, z))
        DrawInside3(c, xc-xf, yc-yf, xc, yc-yf, i, fFalse);
      if (bm.b.Get3(xm, ym, z+1) ^ bm.b.Get3(xn, yn, z+1) ^
        bm.b.Get3(xn, yn, z))
        DrawInside3(c, xc-xf, yc-yf, xc, yc-yf, i, fTrue);
      if (!bm.b.Get3(xd, yd, z) && bm.b.Get3(xe, ye, z))
        DrawInside(c, xc-xl, yc-yf, xc-xf, yc-yf, i);
      if (bm.b.Get3(xe, ye, z) ^ bm.b.Get3(xd, yd, z) ^ bm.b.Get3(xn, yn, z))
        DrawInside(c, xc-xf, yc-yf, xc-xf, yc, i);
      if (!bm.b.Get3(xm, ym, z-1) && bm.b.Get3(xn, yn, z-1))
        DrawInside3(c, xc-xf, yc-yl, xc-xf, yc-yf, i, fFalse);
      if (!bm.b.Get3(xm, ym, z+1) && bm.b.Get3(xn, yn, z+1))
        DrawInside3(c, xc-xf, yc-yl, xc-xf, yc-yf, i, fTrue);
      if (bm.b.Get3(xd, yd, z) ^ bm.b.Get3(xm, ym, z-1) ^
        bm.b.Get3(xd, yd, z-1))
        DrawInside3(c, xc-xl, yc-yl, xc-xf, yc-yf, i, fFalse);
      if (bm.b.Get3(xd, yd, z) ^ bm.b.Get3(xm, ym, z+1) ^
        bm.b.Get3(xd, yd, z+1))
        DrawInside3(c, xc-xl, yc-yl, xc-xf, yc-yf, i, fTrue);

      // Draw "X" on the ground if marking there.
      if (!bm.b2.FNull() && bm.b2.FLegalMaze3(xm, ym, z) &&
        bm.b2.Get3(xm, ym, z) && !bm.b.Get3(xm, ym, z) && j > 0 &&
        (!dr.fNarrow || !FOdd(j))) {
        k = yc + yl+(yf-yl)*63/100;
        c.Line(xc + i*xl*50/100, yc + yl+(k-yc-yl)*50/100, xc, k,
          dr.kvInEdge);
        c.Line(xc + i*xf*63/100, yc + yf+(k-yc-yf)*37/100, xc, k,
          dr.kvInEdge);
      }

      // Figure out the coordinates for the next cell down the passage.
      xm = xn; ym = yn;
      xn += xoff[dr.dir]; yn += yoff[dr.dir];
      xd = xe; yd = ye;
      xe += xoff[dr.dir]; ye += yoff[dr.dir];
      xl = xf; yl = yf;
    }
  }

  // For passages that go outside the Maze, fill them with the ground or
  // sky color to indicate an entrance or exit door.
  if ((ym < 0 && dr.dir == 0) || (ym >= bm.b.m_y3 && dr.dir == 2))
    c.FFill(xc, yc, ym < 0 ? dr.kvInDirt : dr.kvInSky, -1, fFalse);
}


// Draw a Maze contained in one bitmap as line segments overlaying another
// bitmap. Implements the Map Overlay setting in perspective inside view.

void DrawOverlay(CMazK &c, CMaz &b)
{
  int zd, zs, xc, yc, d, xx, xy, yx, yy, x0, y0, xo, yo,
    x1, y1, x2, y2, x, y, xp, yp, zT;
  flag fX, fY, fArrow;

  // Determine segment length and how much of the Maze is visible.
  xc = c.m_x >> 1; yc = c.m_y >> 1;
  if (bm.nWhat != 1) {
    zd = Min(bm.xCell, bm.yCell) << 1;
  } else {
    zd = Max(c.m_x, c.m_y) / Min(bm.xCell, bm.yCell);
  }
  zd = Max(zd, 10);
  zs = Max(c.m_x, c.m_y) / zd;
  zs = NSqr(Sq(zs) << 1);

  // Determine x and y pixel offsets of the rotated Maze axes.
  d = nDegMax - (dr.dir * 90 + dr.nOffsetD);
  if (d > nDegMax)
    d -= nDegMax;
  xx = (int)((real)zd * RCosD((real)d));
  xy = (int)((real)zd * RSinD((real)d));
  yx = (int)((real)zd * RCosD((real)(d + 90)));
  yy = (int)((real)zd * RSinD((real)(d + 90)));

  // Determine the offset of the dot within the Maze.
  x0 = dr.x; y0 = dr.y;
  if (dr.fNarrow) {
    zT = dr.zCell + dr.zCellNarrow;
    x = dr.nOffsetX - FOdd(x0)*(zT >> 1);
    y = dr.nOffsetY - FOdd(y0)*(zT >> 1);
  } else {
    zT = dr.zCell;
    x = dr.nOffsetX;
    y = dr.nOffsetY;
  }
  xo = xc - (x * xx + y * xy) / zT;
  yo = yc - (x * yx + y * yy) / zT;

  x1 = x0 - zs; x1 = Max(x1, 0);
  y1 = y0 - zs; y1 = Max(y1, 0);
  x2 = x0 + zs; x2 = Min(x2, b.m_x-1);
  y2 = y0 + zs; y2 = Min(y2, b.m_y-1);

  // Draw the Maze overlay.
  if (dr.fNarrow) {

    // Narrow Walls on means each segment spans two Maze bitmap pixels.
    x1 &= ~1; y1 &= ~1; x2 &= ~1; y2 &= ~1;
    for (y = y1; y <= y2; y += 2)
      for (x = x1; x <= x2; x += 2)
        if (b.GetF(x, y, dr.z, dr.f3D)) {
          xp = xo + ((x - x0) >> 1) * xx + ((y - y0) >> 1) * xy;
          yp = yo + ((x - x0) >> 1) * yx + ((y - y0) >> 1) * yy;
          fX = b.GetF(x+1, y, dr.z, dr.f3D);
          fY = b.GetF(x, y+1, dr.z, dr.f3D);
          if (fX)
            c.Line(xp, yp, xp + xx, yp - xy, dr.kvMap);
          if (fY)
            c.Line(xp, yp, xp - yx, yp + yy, dr.kvMap);
          else if (!fX)
            c.Set(xp, yp, dr.kvMap);
        }
  } else {

    // Narrow Walls off means each segment spans one Maze bitmap pixel.
    for (y = y1; y <= y2; y++)
      for (x = x1; x <= x2; x++)
        if (b.GetF(x, y, dr.z, dr.f3D)) {
          xp = xo + (x - x0) * xx + (y - y0) * xy;
          yp = yo + (x - x0) * yx + (y - y0) * yy;
          fX = b.GetF(x+1, y, dr.z, dr.f3D);
          fY = b.GetF(x, y+1, dr.z, dr.f3D);
          if (fX)
            c.Line(xp, yp, xp + xx, yp - xy, dr.kvMap);
          if (fY)
            c.Line(xp, yp, xp - yx, yp + yy, dr.kvMap);
          else if (!fX)
            c.Set(xp, yp, dr.kvMap);

          // Draw diagonal lines if no x or y axis connections available.
          if (!fX && !fY && b.GetF(x+1, y+1, dr.z, dr.f3D))
            c.Line(xp, yp, xp + xx - yx, yp - xy + yy, dr.kvMap);
          if (!fY &&
            !b.GetF(x-1, y, dr.z, dr.f3D) && b.GetF(x-1, y+1, dr.z, dr.f3D))
            c.Line(xp, yp, xp - xx - yx, yp + xy + yy, dr.kvMap);
        }
  }

  // Draw the 2nd dot.
  d = (zd << 1) / 3;
  x = dr.x2; y = dr.y2;
  if (dr.fNarrow) {
    x |= 1; y |= 1; d >>= 1;
  }
  zT = (d << 1) + 1;

  if (dr.fDot2 && FBetween(x, x1, x2) && FBetween(y, y1, y2)) {
    if (x == dr.x && y == dr.y && dr.nOffsetX == 0 && dr.nOffsetY == 0) {
      xp = xc; yp = yc;
    } else {
      xp = xo + ((x - x0 - 1) >> dr.fNarrow) * xx +
        ((y - y0 + 1) >> dr.fNarrow) * xy + ((xx - xy) >> dr.fNarrow);
      yp = yo + ((x - x0 - 1) >> dr.fNarrow) * yx +
        ((y - y0 + 1) >> dr.fNarrow) * yy + ((yx - yy) >> dr.fNarrow);
    }
    if (dr.fCircle && zT > 2)
      c.Disk(xp-d, yp-d, xp+d, yp+d, dr.kvDot2);
    else
      c.Block(xp - d, yp - d, xp + d, yp + d, dr.kvDot2);
  }

  // Draw the dot.
  if (dr.fDot) {
    fArrow = dr.fArrow && zT >= 9;
    if (!fArrow || dr.kvDot != dr.kvInEdge) {
      if (dr.fCircle && zT > 2)
        c.Disk(xc-d, yc-d, xc+d, yc+d, dr.kvDot);
      else
        c.Block(xc - d, yc - d, xc + d, yc + d, dr.kvDot);
    }
    if (fArrow) {
      x = xc; y = yc - zT*3/8;
      c.Line(x, y, x, y + zT*6/8, dr.kvInEdge);
      c.Line(x, y, x - zT*11/32, y + zT*11/32, dr.kvInEdge);
      c.Line(x, y, x + zT*11/32, y + zT*11/32, dr.kvInEdge);
    }
  }
}

/* inside.cpp */
