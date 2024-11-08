/*
** Daedalus (Version 3.5) File: color.h
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
** on a color bitmap, unrelated to Mazes.
**
** Old last code change: 6/29/1990.
** Last code change: 10/30/2024.
*/

/*
******************************************************************************
** Constants
******************************************************************************
*/

#define xColmap ((lHighest - 3) / 3)
#define yColmap lHighest
#define cColorMain 16
#define cColor     29

#define kvBlack   Rgb(  0,   0,   0)
#define kvMaroon  Rgb(127,   0,   0)
#define kvDkGreen Rgb(  0, 127,   0)
#define kvMaize   Rgb(127, 127,   0)
#define kvDkBlue  Rgb(  0,   0, 127)
#define kvPurple  Rgb(127,   0, 127)
#define kvDkCyan  Rgb(  0, 127, 127)
#define kvLtGray  Rgb(191, 191, 191)
#define kvGray    Rgb(127, 127, 127)
#define kvRed     Rgb(255,   0,   0)
#define kvGreen   Rgb(  0, 255,   0)
#define kvYellow  Rgb(255, 255,   0)
#define kvBlue    Rgb(  0,   0, 255)
#define kvMagenta Rgb(255,   0, 255)
#define kvCyan    Rgb(  0, 255, 255)
#define kvWhite   Rgb(255, 255, 255)
#define kvDkGray  Rgb( 63,  63,  63)
#define kvOrange  Rgb(255, 127,   0)
#define kvPink    Rgb(255, 127, 127)
#define kvBrown   Rgb(127,  63,   0)

#define nHueMax  3600
#define nHueHalf 1800
#define nHue13   1200
#define nHue23   2400
#define nHue16   600
#define rHueMax  3600.0

enum _colorreplace {
  crpRing    = 1,
  crpSpoke   = 2,
  crpFade    = 3,
  crpY       = 4,
  crpX       = 5,
  crpDiagUL  = 6,
  crpDiagUR  = 7,
  crpSquare  = 8,
  crpDiamond = 9,
  crpSpiral  = 10,
  crpRing2   = 11,
  crpRings   = 12,
  crpSpoke2  = 13,
  crpRainbow = 14,
  crpRandom  = 15,
  crpRandom2 = 16,
  crpUnknown = 17,
};


/*
******************************************************************************
** Macros
******************************************************************************
*/

#define cbPixelC 3
#define CbColmapRow(x) ((x)*cbPixelC + 3 & ~3)
#define CbColmap(x, y) LMul(y, CbColmapRow(x))

#define cTexture 64
#define NWSE(n, w, s, e) ((n) | (w) << 6 | (long)(s) << 12 | (long)(e) << 18)
#define UD(u, d) ((u) | (d) << 12)
#define UdU(l) ((int)((dword)(l) & 4095))
#define UdD(l) ((int)((dword)(l) >> 12))
#define ITextureWall(kv, d) ((int)(((kv) >> ((d) * 6)) & (cTexture-1)))
#define ITextureWall2(kv, d) ((int)(((kv) >> (FOdd(d) * 12)) & 4095))
#define ITextureWall3(kv, d) ((int)(((kv) >> (((d) > 1) * 12)) & 4095))
#define GetT(c, x, y, d) ITextureWall((c).Get(x, y), d)
#define SetT(c, x, y, d, t) ((c).Get(x, y) & \
  ~((KV)(cTexture-1) << ((d) * 6)) | ((KV)(t) << ((d) * 6)))
#define SetU(c, x, y, f, t) ((c).Get(x, y) & \
  ~((KV)4095 << ((f) * 12)) | ((KV)(t) << ((f) * 12)))


/*
******************************************************************************
** Types
******************************************************************************
*/

typedef struct _colorsettings {
  // Color replace settings

  flag fRainbowBlend;
  int nRainbowStart;
  int nRainbowDistance;
  KV kvBlend1;
  KV kvBlend2;
  int nReplacePattern;

  // Macro accessible only settings

  flag fGraphNumber;
  long lGrayscale;
  flag fMandelbrot;
} CS;

extern CONST KV rgkv[cColor];
extern CONST char *rgszColor[cColor];
extern CS cs;

class CCol : virtual public CMap // Color bitmap
{
public:
  INLINE long _Ib(int x, int y) CONST
    { return y*(m_clRow << 2) + (x * cbPixelC); }
  INLINE byte *_Pb(long i) CONST
    { return &m_rgb[i]; }
  INLINE byte *_Pb(int x, int y) CONST
    { return &m_rgb[_Ib(x, y)]; }
  INLINE KV _Get(CONST byte *pb) CONST
    { return (*pb << 16) | (*(pb+1) << 8) | *(pb+2); }
  INLINE KV _Get(int x, int y) CONST
    { return _Get(_Pb(x, y)); }
  INLINE void _Get(CONST byte *pb, int *r, int *g, int *b) CONST
    { *b = *pb; *g = *(pb+1); *r = *(pb+2); }
  INLINE void _Set(byte *pb, int r, int g, int b)
    { *pb = b; *(pb+1) = g; *(pb+2) = r; }

  INLINE flag FLegalFill(int x, int y, KV kv) CONST
    { return FLegal(x, y) && Get(x, y) == kv; }
  INLINE void BlockPut(CCol &c1, int x, int y, int nOp)
    { BlockMove(c1, 0, 0, c1.m_x-1, c1.m_y-1, x, y, nOp); }
  INLINE void ColmapReplaceSimple(KV kvFrom, KV kvTo)
    { ColmapReplace(kvFrom, kvTo, 0, 0, 0, 0); }
  INLINE void ColmapReplacePatternSimple(KV kvFrom, int nMode)
    { ColmapReplacePattern(kvFrom, nMode, 0, 0, 0, 0); }

  // Core methods each bitmap type implements
  virtual CMap *Create() OVERRIDE
    { CCol *c; c = new CCol; if (c == NULL) return NULL; return (CMap *)c; }
  virtual void Destroy() OVERRIDE
    { delete this; };
  virtual void Set0(int x, int y) OVERRIDE
    { Set(x, y, kvBlack); }
  virtual void Set1(int x, int y) OVERRIDE
    { Set(x, y, kvWhite); }

  virtual flag FAllocate(int, int, CONST CMap *) OVERRIDE;

  virtual KV Get(int, int) CONST OVERRIDE;
  virtual void Set(int, int, KV) OVERRIDE;
  virtual void Inv(int, int) OVERRIDE;
  virtual void LineX(int, int, int, KV) OVERRIDE;
  virtual void LineY(int, int, int, KV) OVERRIDE;
  virtual void Line(int, int, int, int, KV) OVERRIDE;
  virtual flag FTriangle(int, int, int, int, int, int, KV, int) OVERRIDE;
  virtual flag FQuadrilateral(int, int, int, int, int, int, int, int,
    KV, int) OVERRIDE;
  virtual void Block(int, int, int, int, KV) OVERRIDE;
  virtual void BlockMove(CONST CMap &, int, int, int, int, int, int) OVERRIDE;

  virtual void BitmapSet(KV) OVERRIDE;
  virtual long BitmapCount() CONST OVERRIDE;
  virtual void BitmapFlipX() OVERRIDE;
  virtual flag FBitmapSizeSet(int, int) OVERRIDE;
  virtual flag FBitmapZoomTo(int xnew, int ynew, flag fPreserve) OVERRIDE;
  virtual flag FBitmapBias(int, int, int, int) OVERRIDE;
  virtual flag FBitmapAccentBoundary() OVERRIDE;

  // Extra methods specific to color bitmaps
  flag FColmapGetFromBitmap(CONST CMon &b, KV kv0, KV kv1);
  flag FColmapAntialias(CONST CMon &, KV, KV, int);
  void ColmapPalette(CONST CCol &);
  int ColmapGraphDistance(CONST CMon &, CONST CMon &, KV, KV, int, int, flag);
  long ColmapGraphDistance2(CONST CMon &, KV, KV, flag);

  void ColmapGrayscale();
  void ColmapBrightness(int, real, int);
  void ColmapReplace(KV, KV, int, int, int, int);
  void ColmapReplacePattern(KV, int, int, int, int, int);
  void ColmapExchange(KV, KV);
  void ColmapOrAndKv(KV, int);
  void ColmapOrAnd(CONST CCol &, int);
  void ColmapAlpha(CONST CCol &, CONST CCol &);
  void BlockMove(CONST CCol &, int, int, int, int, int, int, int);
  flag FColmapPutToBitmap(CMon &, int) CONST;
  void ColmapOrAndFromBitmap(CONST CMon &, KV, KV, int);
  flag FColmapBlendFromBitmap(CONST CMon *, CONST CMon *, CONST CMon *);
  void ColmapBlendBitmaps(CONST CCol &);

  flag FColmapBlur(flag);
  void ColmapContrast(flag);
  flag FColmapTransform(int, int);

  long ColmapFind(int) CONST;
  flag FColmapAccentContrast(flag);
  void DrawSeen(CONST CMon &, CONST CCol &, int, int, int, flag);
  flag FColmapStereogram(CONST CMon &, CONST CCol &, int, int);
  long ColmapLifeGenerate(CCol &, flag);
  long Evolution(CCol &, int, flag, flag, flag);
  void Mandelbrot(real, real, real, real, int, flag);

  flag FReadColmapCore(FILE *, int, int, int, int);
  void WriteColmap(FILE *) CONST;
  flag FReadColmapTarga(FILE *);
  void WriteColmapTarga(FILE *) CONST;
  flag FReadColmapPaint(FILE *);
  flag FReadDaedalusBitmapCore(FILE *, int, int);
  void WriteDaedalusBitmap(FILE *, flag) CONST;
  void WriteTextColmap(FILE *, flag) CONST;

  flag FFill(int, int, KV, KV, flag);
  void ColmapStretchToEdge(CONST CCol &, int, int, int, int, int, int,
    int, int);
};


/*
******************************************************************************
** Color Primitives
******************************************************************************
*/

// Function hooks implemented by client of module
extern long ParseExpression(char *);
extern KV KvDialog(void);

// Functions implemented locally
extern KV KvShade(KV, real);
extern KV KvBlend(KV, KV);
extern KV KvBlendR(KV, KV, real);
extern KV KvBlendN(KV, KV, int, int);
extern KV KvDiff(KV, KV);
extern KV Hue(int);
#define HueD(d) Hue((int)((d) * (nHueMax / nDegMax)))
#define HueR(r) Hue((int)((r) * (rHueMax / rDegMax)))
#define kvRandom Rgb(Rnd(0, 255), Rnd(0, 255), Rnd(0, 255))
#define kvRandomRainbow Hue(Rnd(0, nHueMax-1))
extern KV Hsl(int, int, int);


/*
******************************************************************************
** Color Bitmap IO Routines
******************************************************************************
*/

extern int GetEditColor(char *);
extern void ConvertKvToSz(KV, char *, int);
extern KV ParseColor(char *, flag);
extern KV ReadColor(FILE *);

/* color.h */
