/*
** Daedalus (Version 3.3) File: solids.cpp
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
** This file contains routines to generate Daedalus patch files, specifically
** the solids.dp sample file of the Daedalus distribution.
**
** Created: 7/2/1990.
** Last code change: 11/29/2018.
*/

#include <stdio.h>
#include <math.h>
#include "util.h"


/*
******************************************************************************
** Types
******************************************************************************
*/

typedef struct _pointreal3 {
  real x;
  real y;
  real z;
} POINT3;


/*
******************************************************************************
** Variables
******************************************************************************
*/

FILE *file;
real rotation = 10.0;


/*
******************************************************************************
** Polygon Routines
******************************************************************************
*/

void Polygon(POINT3 *a, int x0, int y0, int z0, int xr, int yr, int size)
{
  int i;
  real incr;

  incr = rDegMax / (real)size;
  for (i = 0; i < size; i++) {
    a[i].x = (real)x0 + (real)xr*RCosD((real)i*incr + rotation);
    a[i].y = (real)y0 + (real)yr*RSinD((real)i*incr + rotation);
    a[i].z = (real)z0;
  }
}

void PolygonAdd(POINT3 *a, int x, int y, int z, int size)
{
  int i;

  for (i = 0; i < size; i++) {
    a[i].x += (real)x;
    a[i].y += (real)y;
    a[i].z += (real)z;
  }
}

void PolygonAddhalf(POINT3 *a, int x, int y, int z, int size)
{
  int i;

  for (i = 1; i < size; i += 2) {
    a[i].x += (real)x;
    a[i].y += (real)y;
    a[i].z += (real)z;
  }
}


/*
******************************************************************************
** IO Routines
******************************************************************************
*/

void WritePoint(CONST POINT3 *p)
{
  fprintf(file, " %d %d %d",
    (int)(p->x + rRound), (int)(p->y + rRound), (int)(p->z + rRound));
}

void Triangle(CONST POINT3 *p1, CONST POINT3 *p2, CONST POINT3 *p3, int grf)
{
  fprintf(file, "%d", grf);
  WritePoint(p1); WritePoint(p2); WritePoint(p3);
  fprintf(file, "\n");
}

void Quadrangle(CONST POINT3 *p1, CONST POINT3 *p2, CONST POINT3 *p3,
  CONST POINT3 *p4, int grf)
{
  fprintf(file, "%d", grf);
  WritePoint(p1); WritePoint(p2); WritePoint(p3); WritePoint(p4);
  fprintf(file, "\n");
}

void Pentagon(CONST POINT3 *p1, CONST POINT3 *p2, CONST POINT3 *p3,
  CONST POINT3 *p4, CONST POINT3 *p5)
{
  Quadrangle(p1, p2, p3, p4, 39);
  Triangle(p4, p5, p1, 6);
}

void PolygonArray(CONST POINT3 *a, int size, flag above)
{
  int i;

  if (size == 4) {
    if (above)
      Quadrangle(&a[0], &a[1], &a[2], &a[3], 55);
    else
      Quadrangle(&a[3], &a[2], &a[1], &a[0], 55);
    return;
  }
  for (i = 1; i <= size-2; i++) {
    if (above)
      Triangle(&a[0], &a[i], &a[(i + 1) % size],
        (int)(i == size-2) | 2 | 4*(i == 1));
    else
      Triangle(&a[0], &a[(i + 1) % size], &a[i],
        (int)(i == 1) | 2 | 4*(i == size-2));
  }
}

void PolygonPointarray(CONST POINT3 *a, int x0, int y0, int z0,
  int start, int step, int size, flag above)
{
  int i;

  i = start;
  while (i < size) {
    fprintf(file, "%d %d %d %d", 7, x0, y0, z0);
    WritePoint(&a[i]);
    if (above)
      WritePoint(&a[(i + step) % size]);
    else
      WritePoint(&a[(i - step + size) % size]);
    fprintf(file, "\n");
    i += step;
  }
}

void PolygonArrayconsecutive(CONST POINT3 *a, int size)
{
  int i;

  for (i = 0; i < size; i++) {
    fprintf(file, "%d", 7);
    WritePoint(&a[i]);
    if (FOdd(i)) {
      WritePoint(&a[(i + 2) % size]);
      WritePoint(&a[(i + 1) % size]);
    } else {
      WritePoint(&a[(i + 1) % size]);
      WritePoint(&a[(i + 2) % size]);
    }
    fprintf(file, "\n");
  }
}


/*
******************************************************************************
** Circular Shape Routines
******************************************************************************
*/

void Pyramid(int x0, int y0, int z0, int xr, int yr, int zr, int size)
{
  POINT3 a[nDegMax];

  Polygon(a, x0, y0, z0, xr, yr, size);
  PolygonPointarray(a, x0, y0, z0 + zr, 0, 1, size, fFalse);
  PolygonArray(a, size, fFalse);
}

void Prism(int x0, int y0, int z0, int xr, int yr, int zr, int size)
{
  POINT3 a[nDegMax];
  int i, j, x1, y1, x2, y2;

  Polygon(a, x0, y0, z0 - zr, xr, yr, size);
  for (i = 0; i < size; i++) {
    j = (i + 1) % size;
    x1 = (int)(a[i].x + rRound); y1 = (int)(a[i].y + rRound);
    x2 = (int)(a[j].x + rRound); y2 = (int)(a[j].y + rRound);
    fprintf(file, "%d %d %d %d %d %d %d %d %d %d %d %d %d\n", 55,
      x1, y1, z0 + zr, x2, y2, z0 + zr, x2, y2, z0 - zr, x1, y1, z0 - zr);
  }
  PolygonArray(a, size, fTrue);
  PolygonAdd(a, 0, 0, zr+zr, size);
  PolygonArray(a, size, fFalse);
}

void Crystal(int x0, int y0, int z0, int xr, int yr, int zr1, int zr2,
  int size)
{
  int i, x1, y1, x2, y2;
  real j;

  j = rDegMax / (real)size;
  for (i = 0; i < size; i++) {
    x1 = x0 + (int)((real)xr*RCosD(j*(real)i + rotation));
    y1 = y0 + (int)((real)yr*RSinD(j*(real)i + rotation));
    x2 = x0 + (int)((real)xr*RCosD(j*(real)(i + 1) + rotation));
    y2 = y0 + (int)((real)yr*RSinD(j*(real)(i + 1) + rotation));
    fprintf(file, "%d %d %d %d %d %d %d %d %d %d\n", 7,
      x0, y0, z0 - zr1 - zr2, x1, y1, z0 - zr1, x2, y2, z0 - zr1);
    fprintf(file, "%d %d %d %d %d %d %d %d %d %d %d %d %d\n", 55,
      x1, y1, z0 - zr1, x1, y1, z0 + zr1, x2, y2, z0 + zr1, x2, y2, z0 - zr1);
    fprintf(file, "%d %d %d %d %d %d %d %d %d %d\n", 7,
      x0, y0, z0 + zr1 + zr2, x2, y2, z0 + zr1, x1, y1, z0 + zr1);
  }
}

void Sphere(int x0, int y0, int z0, int xr, int yr, int zr, int size)
{
  int i, j, z1, z2, x1, x2, x3, x4, y1, y2, y3, y4, rx1, rx2, ry1, ry2;
  real k;

  k = rDegMax / (real)(size*4);
  for (i = 0; i < size; i++) {
    z1  = (int)((real)zr*RSinD(k*(real)i) + rRound);
    z2  = (int)((real)zr*RSinD(k*(real)(i + 1)) + rRound);
    rx1 = (int)((real)xr*RCosD(k*(real)i) + rRound);
    rx2 = (int)((real)xr*RCosD(k*(real)(i + 1)) + rRound);
    ry1 = (int)((real)yr*RCosD(k*(real)i) + rRound);
    ry2 = (int)((real)yr*RCosD(k*(real)(i + 1)) + rRound);
    for (j = 0; j < size * 4; j++) {
      x1 = x0 + (int)((real)rx1*RCosD(k*(real)j + rotation) + rRound);
      y1 = y0 + (int)((real)ry1*RSinD(k*(real)j + rotation) + rRound);
      x2 = x0 + (int)((real)rx1*RCosD(k*(real)(j + 1) + rotation) + rRound);
      y2 = y0 + (int)((real)ry1*RSinD(k*(real)(j + 1) + rotation) + rRound);
      x3 = x0 + (int)((real)rx2*RCosD(k*(real)j + rotation) + rRound);
      y3 = y0 + (int)((real)ry2*RSinD(k*(real)j + rotation) + rRound);
      x4 = x0 + (int)((real)rx2*RCosD(k*(real)(j + 1) + rotation) + rRound);
      y4 = y0 + (int)((real)ry2*RSinD(k*(real)(j + 1) + rotation) + rRound);
      if (i < size - 1) {
        fprintf(file, "%d %d %d %d %d %d %d %d %d %d %d %d %d\n", 55,
          x1, y1, z0 - z1, x2, y2, z0 - z1, x4, y4, z0 - z2, x3, y3, z0 - z2);
        fprintf(file, "%d %d %d %d %d %d %d %d %d %d %d %d %d\n", 55,
          x1, y1, z0 + z1, x3, y3, z0 + z2, x4, y4, z0 + z2, x2, y2, z0 + z1);
      } else {
        fprintf(file, "%d %d %d %d %d %d %d %d %d %d\n", 7,
          x2, y2, z0 - z1, x3, y3, z0 - z2, x1, y1, z0 - z1);
        fprintf(file, "%d %d %d %d %d %d %d %d %d %d\n", 7,
          x2, y2, z0 + z1, x1, y1, z0 + z1, x3, y3, z0 + z2);
      }
    }
  }
}

void Ring(int x0, int y0, int z0, int xr1, int xr2, int yr1, int yr2, int zr,
  int size)
{
  POINT3 a[nDegMax];
  real j;
  int i, x1, y1, x2, y2, x3, y3, x4, y4;

  j = rDegMax / (real)size;
  for (i = 0; i <= size; i++) {
    a[i].x = RCosD(j*(real)i + rotation);
    a[i].y = RSinD(j*(real)i + rotation);
  }
  for (i = 0; i < size; i++) {
    x1 = x0 + (int)((real)xr1*a[i].x);
    y1 = y0 + (int)((real)yr1*a[i].y);
    x2 = x0 + (int)((real)xr2*a[i].x);
    y2 = y0 + (int)((real)yr2*a[i].y);
    x3 = x0 + (int)((real)xr1*a[i + 1].x);
    y3 = y0 + (int)((real)yr1*a[i + 1].y);
    x4 = x0 + (int)((real)xr2*a[i + 1].x);
    y4 = y0 + (int)((real)yr2*a[i + 1].y);
    fprintf(file, "%d %d %d %d %d %d %d %d %d %d %d %d %d\n", 55,
      x1, y1, z0 - zr, x2, y2, z0 - zr, x4, y4, z0 - zr, x3, y3, z0 - zr);
    fprintf(file, "%d %d %d %d %d %d %d %d %d %d %d %d %d\n", 55,
      x2, y2, z0 - zr, x2, y2, z0 + zr, x4, y4, z0 + zr, x4, y4, z0 - zr);
    fprintf(file, "%d %d %d %d %d %d %d %d %d %d %d %d %d\n", 55,
      x3, y3, z0 - zr, x3, y3, z0 + zr, x1, y1, z0 + zr, x1, y1, z0 - zr);
    fprintf(file, "%d %d %d %d %d %d %d %d %d %d %d %d %d\n", 55,
      x1, y1, z0 + zr, x3, y3, z0 + zr, x4, y4, z0 + zr, x2, y2, z0 + zr);
  }
}


/*
******************************************************************************
** Other Shape Routines
******************************************************************************
*/

void Torus(int x0, int y0, int z0, int xr, int yr, int zr,
  int size1, int size2)
{
  POINT3 a[nDegMax], a1[nDegMax], a2[nDegMax];
  int i1, i2;
  real j1, j2, x, y;

  j1 = rDegMax / (real)size1;
  j2 = rDegMax / (real)size2;
  for (i1 = 0; i1 <= size1; i1++) {
    a[i1].x = (real)x0 + (real)xr*RCosD(j1*(real)i1 + rotation);
    a[i1].y = (real)y0 + (real)yr*RSinD(j1*(real)i1 + rotation);
  }
  for (i1 = 0; i1 <= size1; i1++) {
    x = (real)zr*RCosD(j1*(real)i1 + rotation);
    y = (real)zr*RSinD(j1*(real)i1 + rotation);
    for (i2 = 0; i2 < size2; i2++) {
      a2[i2].x = a[i1].x + x*RCosD(j2*(real)i2);
      a2[i2].y = a[i1].y + y*RCosD(j2*(real)i2);
      a2[i2].z = (real)z0 + (real)zr*RSinD(j2*(real)i2);
    }
    if (i1 > 0)
      for (i2 = 0; i2 < size2; i2++)
        Quadrangle(&a1[(i2 + 1) % size2], &a2[(i2 + 1) % size2],
          &a2[i2], &a1[i2], 55);
    for (i2 = 0; i2 <= size2; i2++)
      a1[i2] = a2[i2];
  }
}

real ZenithHeight(real i)
{
  return (1.0 - RCosD(i)) / 2.0;
}

void Zenith(int x0, int y0, int z0, int xsiz, int ysiz, int xnum, int ynum,
  int xcount, int ycount, int zr)
{
  POINT3 a1[nDegMax], a2[nDegMax];
  int x, y;
  real xd, yd;

  for (y = 0; y <= ynum; y++) {
    yd = (real)(y*ycount)*rDegMax/(real)ynum;
    for (x = 0; x <= xnum; x++) {
      xd = (real)(x*xcount)*rDegMax/(real)xnum;
      a2[x].x = (real)(x0 - xnum*xsiz / 2 + x*xsiz);
      a2[x].y = (real)(y0 - ynum*ysiz / 2 + y*ysiz);
      a2[x].z = (real)z0 + (real)zr*ZenithHeight(xd)*ZenithHeight(yd) +
        rRound;
    }
    if (y > 0)
      for (x = 0; x < xnum; x++)
        Quadrangle(&a1[x], &a2[x], &a2[x + 1], &a1[x + 1], 55);
    for (x = 0; x <= xnum; x++)
      a1[x] = a2[x];
  }
}


/*
******************************************************************************
** Platonic Shape Routines
******************************************************************************
*/

void Tetrahedron(int x0, int y0, int z0, int xr, int yr, int zr)
{
  Pyramid(x0, y0, z0, xr, yr, (int)((real)zr*RSqr(2.5)), 3);
}

void Cube(int x0, int y0, int z0, int xr, int yr, int zr)
{
  Prism(x0, y0, z0, xr, yr, zr / 2, 4);
}

void Octahedron(int x0, int y0, int z0, int xr, int yr, int zr)
{
  Sphere(x0, y0, z0, xr, yr, zr, 1);
}

void Icosahedron(int x0, int y0, int z0, int xr, int yr, int zr)
{
  POINT3 a[nDegMax];
  int h;
  real r;

  h = (int)((real)zr / (rPhi*2.0 - 1.0) + rRound);
  r = RSqr(1.0 - 0.2);
  Polygon(a, x0, y0, z0 - h, (int)((real)xr*r), (int)((real)yr*r), 10);
  PolygonAddhalf(a, 0, 0, h*2, 10);
  PolygonPointarray(a, x0, y0, z0 - zr, 0, 2, 10, fTrue);
  PolygonArrayconsecutive(a, 10);
  PolygonPointarray(a, x0, y0, z0 + zr, 1, 2, 10, fFalse);
}

void Dodecahedron(int x0, int y0, int z0, int xr, int yr, int zr)
{
  POINT3 a1[nDegMax], a2[nDegMax];
  real r1, r2;
  int h1, h2, i;

  r2 = 1.0 / rSqr3 / rPhi / RCosD(54.0);
  r1 = RSqr(1.0 - 1.0/(rPhi*2.0 + 1.0)/(rPhi*2.0 + 1.0));
  h2 = (int)((real)zr * RSqr(1.0 - r2*r2) + rRound);
  h1 = (int)((real)h2 / (rPhi*2.0 + 1.0) + rRound);
  Polygon(a1, x0, y0, z0 - h1, (int)((real)xr*r1), (int)((real)yr*r1), 10);
  PolygonAddhalf(a1, 0, 0, h1*2, 10);
  Polygon(a2, x0, y0, z0 - h2, (int)((real)xr*r2), (int)((real)yr*r2), 10);
  PolygonAddhalf(a2, 0, 0, h2*2, 10);
  Pentagon(&a2[0], &a2[2], &a2[4], &a2[6], &a2[8]);
  Pentagon(&a2[9], &a2[7], &a2[5], &a2[3], &a2[1]);
  for (i = 0; i < 10; i++) {
    if (FOdd(i))
      Pentagon(&a1[(i + 2) % 10], &a1[(i + 1) % 10], &a1[i],
        &a2[i], &a2[(i + 2) % 10]);
    else
      Pentagon(&a1[i], &a1[(i + 1) % 10], &a1[(i + 2) % 10],
        &a2[(i + 2) % 10], &a2[i]);
  }
}


/*
******************************************************************************
** Entry Point
******************************************************************************
*/

flag CreateSolids(CONST char *wfilename)
{
  file = FileOpen(wfilename, "w");
  if (file == NULL)
    return fFalse;

  fprintf(file, "DP#\n3312\n");
  fprintf(file, "-1 Yellow\n");
  Icosahedron (-180, 0, 90, 90, 90, 90);
  fprintf(file, "-1 Brown\n");
  Dodecahedron(180, 0, 90, 90, 90, 90);
  fprintf(file, "-1 Pink\n");
  Tetrahedron (105, 180, 45, 95, 95, 95);
  fprintf(file, "-1 Orange\n");
  Octahedron  (-105, 180, 90, 100, 100, 100);
  fprintf(file, "-1 Green\n");
  Sphere      (0, 0, 90, 80, 80, 80, 6);
  fprintf(file, "-1 Red\n");
  Cube        (0, 360, 90, 90, 90, 150);
  fprintf(file, "-1 Purple\n");
  Crystal     (0, 180, 360, 40, 40, 50, 40, 6);
  fprintf(file, "-1 Cyan\n");
  Ring        (180, 180, 250, 95, 125, 95, 125, 12, 32);
  fprintf(file, "-1 Blue\n");
  Torus       (-180, 180, 250, 100, 100, 25, 32, 16);
  fprintf(file, "-1 LtGray\n");
  Zenith      (0, 240, 0, 15, 15, 48, 48, 3, 3, -150);
  fclose(file);
  printf("\n");
  return fTrue;
}

/* solids.cpp */
