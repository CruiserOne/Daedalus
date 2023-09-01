/*
** Daedalus (Version 3.4) File: graphics.h
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
** This file contains definitions for general graphics routines, that operate
** on a monochrome bitmap, unrelated to Mazes.
**
** Created: 7/1/1993.
** Last code change: 8/29/2023.
*/

/*
******************************************************************************
** Constants
******************************************************************************
*/

#define xBitmap (lHighest - 31)
#define yBitmap lHighest
#define xStart 63
#define yStart 63
#define zBitmapStack 1024
#define wClient1Def 16
#define wClient2Def 16
#define wClient3Def 16
#define wClient4Def 4

#define fOff 0
#define fOn 1

#define DIRS 4
#define DIRS1 (DIRS-1)
#define DIRS2 8


/*
******************************************************************************
** Macros
******************************************************************************
*/

#define CbBitmapRow(x) ((((x) + 31) >> 5) << 2)
#define CbBitmap(x, y) LMul(y, CbBitmapRow(x))
#define Lf(x) (1L << ((x)&31 ^ 7))

#define GetP(b, x, y) ((b) != NULL && !(b)->FNull() && (b)->Get(x, y))
#define DirInc(d) d = ((d) + 1) & DIRS1
#define DirDec(d) d = ((d) - 1) & DIRS1
#define FDitherCore(col, x, y) ((col) >= rgnDitherPoint[((y) << 2) + (x)])
#define FDither(col, x, y) FDitherCore(col, (x) & 3, (y) & 3)
#define NBinary(f0, f1, f2, f3, f4, f5, f6, f7) \
  ((f0 << 4) | (f1 << 1) | (f2 << 5) | (f3 << 2) | \
  (f4 << 6) | (f5 << 3) | (f6 << 7) | (f7 << 0))


/*
******************************************************************************
** Types
******************************************************************************
*/

class CMap; // Forward
typedef flag bit;
typedef long KV;

typedef struct _graphicssettings {
  // Display settings

  flag fTraceDot;
  flag fErrorCheck;
  CMap *bFocus;

  // Macro accessible only settings

  int turtlex;
  int turtley;
  int turtles;
  int turtlea;
  KV turtlec;
  flag turtleo;
  int turtlet;
  int grfLifeDie;
  int grfLifeBorn;
} GS;

extern CONST int xoff[DIRS2], yoff[DIRS2], xoff2[DIRS2], yoff2[DIRS2];
extern CONST int rgfOr[3], rgfAnd[3], rgfXor[3];
extern CONST int rgnDitherPoint[16];
extern CONST int rggrfNeighbor[25];
extern GS gs;

class CMap // Base bitmap independent of pixel type
{
public:
  int m_x;     // Horizontal pixel size of bitmap
  int m_y;     // Vertical pixel size of bitmap
  int m_clRow; // Longs per row in bitmap
  int m_cfPix; // Bits in each pixel
  int m_w3;    // 3D bitmap W size
  int m_x3;    // 3D bitmap X size
  int m_y3;    // 3D bitmap Y size
  int m_z3;    // 3D bitmap Z size
  byte *m_rgb; // Bytes of bitmap bits

  INLINE CMap()
    { Init(); }
  INLINE ~CMap()
    { Free(); }
  INLINE void Init()
    { m_x = m_y = 0; m_rgb = NULL; }
  INLINE void Free()
    { if (m_rgb != NULL) { DeallocateP(m_rgb); m_rgb = NULL; } }
  INLINE dword *_Rgl(long i) CONST
    { return (dword *)&m_rgb[i << 2]; }
  INLINE flag FVisible() CONST
    { return this == gs.bFocus; }
  INLINE void Copy3(CONST CMap &b)
    { m_w3 = b.m_w3; m_x3 = b.m_x3; m_y3 = b.m_y3; m_z3 = b.m_z3; }
  INLINE void CopyFrom(CMap &b)
    { m_x = b.m_x; m_y = b.m_y; m_clRow = b.m_clRow; m_cfPix = b.m_cfPix;
    Copy3(b); Free(); m_rgb = b.m_rgb; b.m_rgb = NULL; }
  INLINE void SwapWith(CMap &b)
    { SwapN(m_x, b.m_x); SwapN(m_y, b.m_y); SwapN(m_clRow, b.m_clRow);
    SwapN(m_cfPix, b.m_cfPix); SwapN(m_w3, b.m_w3); SwapN(m_x3, b.m_x3);
    SwapN(m_y3, b.m_y3); SwapN(m_z3, b.m_z3);
    byte *pT; pT = m_rgb; m_rgb = b.m_rgb; b.m_rgb = pT; }

  INLINE flag FNull() CONST
    { return m_rgb == NULL; }
  INLINE flag FZero() CONST
    { return m_x < 1 || m_y < 1; }
  INLINE flag F64K() CONST
    { return m_x > 0xFFFF || m_y > 0xFFFF; }
  INLINE flag FLegal(int x, int y) CONST
    { return (uint)x < (uint)m_x && (uint)y < (uint)m_y; }
  INLINE flag FLegalOff(int x, int y) CONST
    { return FLegal(x, y) && !Get(x, y); }
  INLINE flag FColor() CONST
    { return m_cfPix > 1; }
  INLINE KV GetSafe(int x, int y)
    { return FNull() ? fOff : Get(x, y); }
  INLINE void BitmapOff()
    { BitmapSet(0); }
  INLINE void BitmapOn()
    { BitmapSet(dwSet); }
  INLINE void Edge(int x1, int y1, int x2, int y2, KV kv)
    { Box(x1, y1, x2, y2, 1, 1, kv); }
  INLINE void BoxAll(int xsiz, int ysiz, KV kv)
    { Box(0, 0, m_x-1, m_y-1, xsiz, ysiz, kv); }
  INLINE void BitmapRotateR()
    { FBitmapTranspose(); BitmapFlipX(); }
  INLINE void BitmapRotateL()
    { FBitmapTranspose(); BitmapFlipY(); }
  INLINE void BitmapRotateA()
    { BitmapFlipX(); BitmapFlipY(); }
  INLINE flag FBitmapZoomBy(real x, real y, flag f)
    { return FBitmapZoomTo((int)((real)m_x*x + rMinute),
    (int)((real)m_y*y + rMinute), f); }
  INLINE flag FBitmapTessellateBy(real x, real y)
    { return FBitmapTessellateTo((int)((real)m_x*x + rMinute),
    (int)((real)m_y*y + rMinute)); }

  // Core methods each bitmap type implements
  virtual CMap *Create() = 0;
  virtual void Destroy() = 0;
  virtual flag FAllocate(int, int, CONST CMap *) = 0;

  virtual KV Get(int, int) CONST = 0;
  virtual void Set(int, int, KV) = 0;
  virtual void Set0(int, int) = 0;
  virtual void Set1(int, int) = 0;
  virtual void Inv(int, int) = 0;
  virtual void LineX(int, int, int, KV) = 0;
  virtual void LineY(int, int, int, KV) = 0;
  virtual void Line(int, int, int, int, KV) = 0;
  virtual flag FTriangle(int, int, int, int, int, int, KV, int) = 0;
  virtual flag FQuadrilateral(int, int, int, int, int, int, int, int,
    KV, int) = 0;
  virtual void Block(int, int, int, int, KV) = 0;
  virtual void BlockMove(CONST CMap &, int, int, int, int, int, int) = 0;

  virtual void BitmapSet(KV) = 0;
  virtual long BitmapCount() CONST = 0;
  virtual void BitmapFlipX() = 0;
  virtual flag FBitmapSizeSet(int, int) = 0;
  virtual flag FBitmapZoomTo(int, int, flag) = 0;
  virtual flag FBitmapBias(int, int, int, int) = 0;
  virtual flag FBitmapAccentBoundary() = 0;

  // Shared methods covering all bitmap implementations
  void Legalize(int *, int *) CONST;
  void Legalize2(int *, int *) CONST;

  void LineZ(int, int, int, int, KV);
  void LineSetup(MM *, int, int, int, int) CONST;
  void BlockReverse(int, int, int, int);
  void Box(int, int, int, int, int, int, KV);
  void Disk(int, int, int, int, KV);
  void Circle(int, int, int, int, KV);
  void ArcQuadrant(int, int, int, int, KV);
  void ArcQuadrantSub(int, int, int, int, int, int, int *, int *, KV);
  void ArcPolygon(int, int, int, int, int, int, int, int, KV);
  void ArcReal(real, real, real, real, real, KV);

  flag FBitmapCopy(CONST CMap &);
  flag FBitmapResizeTo(int, int);
  flag FBitmapShiftBy(int, int);
  void BitmapFlipY();
  flag FBitmapTranspose();
  void BitmapTessellate(CONST CMap &);
  flag FBitmapTessellateTo(int x, int y);
  flag FBitmapCollapse();
  flag FBitmapSlide(int, int);
  flag FBitmapEditRowColumn(int, int, flag, flag);

  flag FZoomSprite(CMap *bMaz, CONST CMap &bPic);
};

class CMon : virtual public CMap // Monochrome bitmap
{
public:
  INLINE long _Il(int x, int y) CONST
    { return y*m_clRow + (x >> 5); }
  INLINE dword *_Pl(long i) CONST
    { return (dword *)&m_rgb[i << 2]; }
  INLINE dword *_Pl(int x, int y) CONST
    { return _Pl(_Il(x, y)); }
  INLINE dword _L(int x, int y) CONST
    { return *_Pl(x, y); }
  INLINE flag _Get(int x, int y) CONST
    { return (_L(x, y) & Lf(x)) != 0; }
  INLINE KV GetFast(int x, int y) CONST
    { return FLegal(x, y) ? _Get(x, y) : fOff; }

  INLINE flag FLegalFill(int x, int y, KV kv) CONST
    { return FLegal(x, y) && Get(x, y) != kv; }
  INLINE flag FFill(int x, int y, KV kv)
    { return FFillCore(x, y, kv, fFalse); }
  INLINE flag FFlood(int x, int y, KV kv)
    { return FFillCore(x, y, kv, fTrue); }
  INLINE flag FFillReverse(int x, int y)
    { return FFill(x, y, !Get(x, y)); }
  INLINE flag FFloodReverse(int x, int y)
    { return FFlood(x, y, !Get(x, y)); }
  INLINE void BlockPutOrAnd(CONST CMon &b, int x, int y, CONST int *rgf)
    { BlockMoveOrAnd(b, 0, 0, b.m_x-1, b.m_y-1, x, y, rgf); }

  // Core methods each bitmap type implements
  virtual CMap *Create() OVERRIDE
    { CMon *b; b = new CMon; if (b == NULL) return NULL; return (CMap *)b; }
  virtual void Destroy() OVERRIDE
    { delete this; }
  virtual flag FTriangle(int x1, int y1, int x2, int y2, int x3, int y3,
    KV kv, int nTrans) OVERRIDE
    { return FTriangle(x1, y1, x2, y2, x3, y3, kv); }
  virtual flag FQuadrilateral(int x1, int y1, int x2, int y2, int x3, int y3,
    int x4, int y4, KV kv, int nTrans) OVERRIDE
    { return FQuadrilateral(x1, y1, x2, y2, x3, y3, x4, y4, kv); }

  virtual flag FAllocate(int, int, CONST CMap *) OVERRIDE;

  virtual KV Get(int, int) CONST OVERRIDE;
  virtual void Set(int, int, KV) OVERRIDE;
  virtual void Set0(int, int) OVERRIDE;
  virtual void Set1(int, int) OVERRIDE;
  virtual void Inv(int, int) OVERRIDE;
  virtual void LineX(int, int, int, KV) OVERRIDE;
  virtual void LineY(int, int, int, KV) OVERRIDE;
  virtual void Line(int, int, int, int, KV) OVERRIDE;
  virtual void Block(int, int, int, int, KV) OVERRIDE;
  virtual void BlockMove(CONST CMap &, int, int, int, int, int, int) OVERRIDE;

  virtual void BitmapSet(KV) OVERRIDE;
  virtual long BitmapCount() CONST OVERRIDE;
  virtual void BitmapFlipX() OVERRIDE;
  virtual flag FBitmapSizeSet(int, int) OVERRIDE;
  virtual flag FBitmapZoomTo(int, int, flag) OVERRIDE;
  virtual flag FBitmapBias(int, int, int, int) OVERRIDE;
  virtual flag FBitmapAccentBoundary() OVERRIDE;

  // Extra methods specific to monochrome bitmaps
  flag FLineUntil(int, int, int, int, int *, int *, KV, flag, flag);
  flag FTriangle(int, int, int, int, int, int, int);
  flag FQuadrilateral(int, int, int, int, int, int, int, int, int);
  void BlockMoveOrAnd(CONST CMon &, int, int, int, int, int, int,
    CONST int *);
  flag Turtle(CONST char *);
  flag FFillCore(int, int, KV, flag);
  long AnalyzePerimeter(int, int, flag) CONST;

  flag FBitmapFind(int *, int *, bit) CONST;
  flag FBitmapSubset(CONST CMon &) CONST;
  void BitmapReverse();
  void BitmapRandom(int, int);
  void BitmapMoire();
  void BitmapMoveOrAnd(CONST CMon &, CONST int *);
  void BitmapOr(CONST CMon &);
  void BitmapAnd(CONST CMon &);
  void BitmapXor(CONST CMon &);
  void BitmapBlend(CONST CMon &);

  long BitmapSmooth(flag);
  long BitmapSmoothCorner(bit);
  long BitmapThicken();
  long BitmapThicken2(int, flag);
  flag FBitmapThinCore(int, int, flag);
  long BitmapThinner(flag);
  flag FBitmapConvex(int, int, int, int);
  flag FBitmapAccentContrast(flag);
  void BitmapSeen(int, int, int);
  flag FStereogram(CONST CMon &, int, int);
  long LifeGenerate(flag);

  flag FReadBitmapCore(FILE *, int, int);
  void WriteBitmap(FILE *, KV, KV) CONST;
  flag FReadText(FILE *);
  void WriteText(FILE *, flag, flag, flag) CONST;
  flag FReadXbm(FILE *);
  void WriteXbm(FILE *, CONST char *, char) CONST;
  flag FReadDaedalusBitmapCore(FILE *, int, int);
  void WriteDaedalusBitmap(FILE *, flag) CONST;
};


/*
******************************************************************************
** Bitmap Primitives
******************************************************************************
*/

extern void UpdateDisplay(void);
extern void ScreenDot(int, int, bit, KV);


/*
******************************************************************************
** Bitmap Flooding
******************************************************************************
*/

#define FillPush(xnew, ynew) rgpt[ipt].x = (xnew); rgpt[ipt].y = (ynew); ipt++
#define FillPop(xnew, ynew) ipt--; xnew = rgpt[ipt].x; ynew = rgpt[ipt].y


/*
******************************************************************************
** Bitmap Transformations
******************************************************************************
*/

extern flag FSetLife(CONST char *, int);


/*
******************************************************************************
** Bitmap File Routines
******************************************************************************
*/

extern flag FReadBitmapHeader(FILE *, flag, int *, int *, int *, int *);

/* graphics.h */
