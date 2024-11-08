/*
** Daedalus (Version 3.5) File: threed.h
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
** This file contains definitions for general graphics routines, that operate
** on a monochrome bitmap being treated as a 3D/4D bitmap, unrelated to Mazes.
**
** Created: 3/12/1990.
** Last code change: 10/30/2024.
*/

/*
******************************************************************************
** Constants
******************************************************************************
*/

#define xCubeMax xBitmap
#define yCubeMax yBitmap
#define zCubeMax xBitmap

#define DIRS3 6
#define DIRS4 8


/*
******************************************************************************
** Macros
******************************************************************************
*/

class CMap3 : virtual public CMap // 3D bitmap
{
public:
  INLINE int X2(int x, int z) CONST // Map 3D coordinate to 2D X coordinate
    { return (z % m_w3) * m_x3 + x; }
  INLINE int Y2(int y, int z) CONST // Map 3D coordinate to 2D Y coordinate
    { return (z / m_w3) * m_y3 + y; }
  INLINE int X3(int x) CONST // Map 2D X coordinate to 3D X coordinate
    { return x % m_x3; }
  INLINE int Y3(int y) CONST // Map 2D Y coordinate to 3D Y coordinate
    { return y % m_y3; }
  INLINE int Z3(int x, int y) CONST // Map 2D coordinate to 3D Z coordinate
    { return (y / m_y3) * m_w3 + (x / m_x3); }
  INLINE KV _Get3(int x, int y, int z) CONST
    { return Get(X2(x, z), Y2(y, z)); }
  INLINE void Set3(int x, int y, int z, KV kv)
    { if (FLegalCube(x, y, z)) Set(X2(x, z), Y2(y, z), kv); }
  INLINE void Set30(int x, int y, int z)
    { if (FLegalCube(x, y, z)) Set0(X2(x, z), Y2(y, z)); }
  INLINE void Set31(int x, int y, int z)
    { if (FLegalCube(x, y, z)) Set1(X2(x, z), Y2(y, z)); }
  INLINE void Inv3(int x, int y, int z)
    { if (FLegalCube(x, y, z)) Inv(X2(x, z), Y2(y, z)); }
  INLINE KV GetF(int x, int y, int z, flag f3D) CONST
    { return f3D ? Get3(x, y, z) : Get(x, y); }

  INLINE int X4(int w, int x) CONST // Map 4D coordinate to 2D X coordinate
    { return w * m_x3 + x; }
  INLINE int Y4(int y, int z) CONST // Map 4D coordinate to 2D Y coordinate
    { return z * m_y3 + y; }
  INLINE int Get4(int w, int x, int y, int z) CONST
    { return Get(X4(w, x), Y4(y, z)); }
  INLINE void Set4(int w, int x, int y, int z, KV kv)
    { Set(X4(w, x), Y4(y, z), kv); }
  INLINE void Set40(int w, int x, int y, int z)
    { Set0(X4(w, x), Y4(y, z)); }
  INLINE void Set41(int w, int x, int y, int z)
    { Set1(X4(w, x), Y4(y, z)); }
  INLINE void Inv4(int w, int x, int y, int z)
    { Inv(X4(w, x), Y4(y, z)); }
  INLINE flag FZero3() CONST
    { return m_x3 < 1 || m_y3 < 1 || m_z3 < 1; }
  INLINE flag F64K3() CONST
    { return m_x3 > 0xFFFF || m_y3 > 0xFFFF || m_z3 > 0xFFFF; }
  INLINE flag FLegalCubeLevel(int x, int y) CONST
    { return x >= 0 && x < m_x3 && y >= 0 && y < m_y3; }
  INLINE flag FLegalCube(int x, int y, int z) CONST
    { return FLegalCubeLevel(x, y) && z >= 0 && z < m_z3; }
  INLINE flag FLegalLevel(int x, int y, flag f3D)
    { return f3D ? FLegalCubeLevel(x, y) : FLegal(x, y); }
  INLINE flag FLegalFillCube(int x, int y, int z, KV o) CONST
    { return FLegalCube(x, y, z) && Get3(x, y, z) != o; }

  KV Get3(int x, int y, int z) CONST
    { return !FLegalCube(x, y, z) ? fOff : _Get3(x, y, z); }
  KV Get3Safe(int x, int y, int z) CONST
    { return FNull() || !FLegalCube(x, y, z) ? fOff : _Get3(x, y, z); }

  virtual CMap3 *Create3() = 0;
  virtual void Destroy3() = 0;

  flag FAllocateCube(int, int, int, int);
  flag FCubeSizeSet(int, int, int, int);
  flag FTesseractSizeSet(int, int, int, int);
  void Legalize3(int *x, int *y, int *z) CONST;
  void CubeBlock(int, int, int, int, int, int, KV);
  void CubeMove(CONST CMap3 &, int, int, int, int, int, int, int, int, int);
  flag FCubeResizeTo(int, int, int);
  flag FCubeShiftBy(int, int, int);
  flag FCubeCollapse();
  int CubeFlipCore(int, int, int, char) CONST;
  flag FCubeFlip(CONST char *);
  flag FCubeFlip2(int, int);
  flag FCubeFill(int, int, int, KV);
};

class CMon3 : virtual public CMap3, virtual public CMon // 3D mono bitmap
{
public:
  virtual CMap3 *Create3() OVERRIDE
    { CMon3 *b; b = new CMon3; if (b == NULL) return NULL;
    return (CMap3 *)b; }
  virtual void Destroy3() OVERRIDE
    { delete this; };

  flag FReadCube(FILE *, int);
  void WriteCube(FILE *, int, flag) CONST;
};

class CCol3 : virtual public CMap3, virtual public CCol // 3D color bitmap
{
public:
  virtual CMap3 *Create3() OVERRIDE
    { CCol3 *c; c = new CCol3; if (c == NULL) return NULL;
    return (CMap3 *)c; }
  virtual void Destroy3() OVERRIDE
    { delete this; };

  INLINE KV Get3R(int x, int y, int z, flag fRainbow) CONST
    { return fRainbow ? Hue(z * nHueMax / m_z3) : Get3(x, y, z); }

  int ColmapGraphDistance3(CONST CMon3 &, CONST CMon3 &, KV, KV,
    int, int, int);
};


/*
******************************************************************************
** 3D Bitmap Routines
******************************************************************************
*/

extern CONST int woff3[DIRS4 + 4], xoff3[DIRS4 + 4], yoff3[DIRS4 + 4],
  zoff3[DIRS4 + 4];

/* threed.h */
