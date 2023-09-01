/*
** Daedalus (Version 3.4) File: util.h
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
** This file contains definitions for generic utilities. Nothing here is
** related to graphics or any other part of Daedalus.
**
** Created: 6/4/1993.
** Last code change: 8/29/2023.
*/

// Compile options

#pragma warning(disable:4250) // class inherits member via dominance
#define WIN
#define PC
#define ASSERT

// WIN: Comment out to produce command line version with no Windows interface.
// PC: Comment out if not using a Microsoft Visual Studio compiler.
// ASSERT: Comment out to avoid useful debugging checks and error messages.

#ifdef PC
#define SECURECRT
#define OVERRIDE override
#define INLINE __forceinline
#else
#define OVERRIDE
#define INLINE inline
#define __int64 long long
#endif
#ifndef NULL
#define NULL 0
#endif
#ifdef _DEBUG
#define DEBUG
#endif

#ifdef SECURECRT
#define sprintf sprintf_s
#define sscanf sscanf_s
#define fscanf fscanf_s
#define fopen fopen_s
#define S(sz) (sz), (int)sizeof(sz)
#define S1(ch) (ch), (int)sizeof(char)
#define SO(pch, sz) (pch), (sizeof(sz) - ((pch) - (sz)))
#define SS(sz, cch) (sz), (cch)
#else
#define S(sz) (sz)
#define S1(ch) (ch)
#define SO(pch, sz) (pch)
#define SS(sz, cch) (sz)
#endif
#define PD(dp) ((int)(dp))


/*
******************************************************************************
** Constants
******************************************************************************
*/

#define fFalse 0
#define fTrue 1

#define rPi      3.14159265358979323846
#define rPi2     (rPi*2.0)
#define rPiHalf  (rPi/2.0)
#define rSqr2    1.41421356237309504880
#define rSqr3    1.73205080756887729353
#define rSqr5    2.23606797749978969641
#define rPhi     ((rSqr5 + 1.0) / 2.0)
#define rDegMax  360.0
#define rDegHalf 180.0
#define rDegQuad 90.0
#define rDeg34   270.0
#define rDegRad  (rDegHalf/rPi)
#define rRound   0.5
#define rMinute  0.000001
#define lHighest 0x7FFFFFFF
#define nDegMax  360
#define nDegHalf 180
#define cLetter  26
#define iLetterX 24
#define iLetterY 25
#define iLetterZ 26
#define chNull   '\0'
#define chTab    '\t'
#define chMost   '\377'
#define dwSet    0xFFFFFFFF
#define dwCanary 0x12345678

#define cchSzDef 80
#define cchSzMax 255
#define chOff    ' '
#define chOn     '#'
#define chX      '-'
#define chY      '|'
#define chXY     '+'
#define chU      '/'
#define chD      '\\'
#define chUD     'X'

enum _printmode {
  nPrintNormal  = 0,
  nPrintNotice  = 1,
  nPrintWarning = 2,
  nPrintError   = 3,
};


/*
******************************************************************************
** Macros
******************************************************************************
*/

#define Max(v1, v2) ((v1) > (v2) ? (v1) : (v2))
#define Min(v1, v2) ((v1) < (v2) ? (v1) : (v2))
#define NAbs(n) ((int)abs(n))
#define NSgn(n) ((n) < 0 ? -1 : (n) > 0)
#define FOdd(n) ((n) & 1)
#define Even(n) ((n) + 1 & ~1)
#define Odd(n) ((n) - 1 | 1)
#define FPower2(n) (((n) & (n)-1) == 0)
#define FBetween(v, v1, v2) ((v) >= (v1) && (v) <= (v2))
#define FImplies(f1, f2) (!(f1) || (f2))
#define NMultDiv(n1, n2, n3) ((int)((long)(n1) * (n2) / (n3)))
#define Sq(v) ((v)*(v))
#define No0(n) ((n) != 0 ? (n) : 1)

#define RAbs(r) fabs(r)
#define RFloor(r) floor(r)
#define RSqr(r) sqrt(r)
#define NSqr(n) ((int)(RSqr((real)(n)) + rRound))
#define RSin(r) sin(r)
#define RCos(r) cos(r)
#define RTan(r) tan(r)
#define RAtn(r) atan(r)
#define RAsin(r) (Sq(r) == 1.0 ? (r)*rPiHalf : RAtn((r) / RSqr(1.0-Sq(r))))
#define RAcos(r) ((r) == 0.0 ? 0.0 : RAtn(RSqr(1.0-Sq(r))/(r)))
#define RSinD(r) RSin(RFromD(r))
#define RCosD(r) RCos(RFromD(r))
#define RTanD(r) RTan(RFromD(r))
#define RAsinD(r) DFromR(RAsin(r))
#define RAcosD(r) DFromR(RAcos(r))
#define RAtnD(r) DFromR(RAtn(r))
#define NSinRD(r, d) ((int)((r)*RSinD(d) + rRound))
#define NCosRD(r, d) ((int)((r)*RCosD(d) + rRound))
#define RFromD(r) ((r)/rDegRad)
#define DFromR(r) ((r)*rDegRad)

#define FDigitCh(ch) FBetween(ch, '0', '9')
#define FCapCh(ch) FBetween(ch, 'A', 'Z')
#define ChCap(ch) (char)((ch) >= 'a' && (ch) <= 'z' ? (ch) - 'a' + 'A' : (ch))
#define ChUncap(ch) (char)(FCapCh(ch) ? (ch) - 'A' + 'a' : (ch))
#define ChHex(n) (char)((n) < 10 ? '0' + (n) : 'a' + (n) - 10)
#define NHex(ch) ((int)((ch) <= '9' ? (ch) - '0' : (ch) - 'a' + 10) & 15)
#define ChHex2(n) (char)((n) < 10 ? '0' + (n) : 'A' + (n) - 10)
#define NHex2(ch) ((int)((ch) <= '9' ? (ch) - '0' : (ch) - 'A' + 10) & 15)

#define BLo(w) ((byte)(w))
#define BHi(w) ((byte)((word)(w) >> 8 & 0xFF))
#define WLo(l) ((word)(dword)(l))
#define WHi(l) ((word)((dword)(l) >> 16 & 0xFFFF))
#define WFromBB(bLo, bHi) ((word)BLo(bLo) | (word)((byte)(bHi)) << 8)
#define LFromWW(wLo, wHi) ((dword)WLo(wLo) | (dword)((word)(wHi)) << 16)
#define LFromBB(b1, b2, b3, b4) LFromWW(WFromBB(b1, b2), WFromBB(b3, b4))
#define ClearPb(pb, cb) memset(pb, 0, cb)
#define CopyPb(pbFrom, pbTo, cb) memcpy(pbTo, pbFrom, cb)

#define Rgb(bR, bG, bB) \
  (((dword)(bR)) | ((dword)(bG)<<8) | ((dword)(bB)<<16))
#define RgbReal(r, g, b) \
  Rgb((int)((r)*255.0), (int)((g)*255.0), (int)((b)*255.0))
#define RgbR(l) ((int)((dword)(l) & 255))
#define RgbG(l) ((int)(((dword)(l) >> 8) & 255))
#define RgbB(l) ((int)((dword)(l) >> 16))
#define GrayN(b) Rgb(b, b, b)

#define loop for (;;)
#define inv(n) n = !(n)
#define neg(n) n = -(n)
#define SwapN(n1, n2) (n1)^=(n2)^=(n1)^=(n2)
#define EnsureBetween(n, nLo, nHi) if ((n) < (nLo)) (n) = (nLo); \
  else if ((n) > (nHi)) (n) = (nHi);
#define SkipSpaces(pch) while (*(pch) == ' ') (pch)++
#define SkipToSpace(pch) while (*(pch) && *(pch) != ' ') (pch)++

#define getbyte() BRead(file)
#define getword() WRead(file)
#define getlong() LRead(file)
#define skipbyte() ch = getbyte()
#define skipword() skipbyte(); skipbyte()
#define skiplong() skipword(); skipword()
#define skiplf() if (ch != '\n') skipbyte()
#define skipcrlf() skipbyte(); skiplf()

#define putbyte(b) putc((byte)(b), file)
#define putword(w) putbyte(BLo(w)); putbyte(BHi(w))
#define putlong(l) putword(WLo(l)); putword(WHi(l))


/*
******************************************************************************
** Types
******************************************************************************
*/

#define CONST const

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;
typedef unsigned __int64 qword;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef unsigned __int64 uquad;
typedef unsigned int uint;
typedef double real;
typedef __int64 quad;
typedef int flag;
typedef short *TRIE;

typedef struct _point {
  ushort x;
  ushort y;
} PT;

typedef struct _point3 {
  ushort w;
  ushort x;
  ushort y;
  ushort z;
} PT3;

typedef struct _bfss { // Breadth First Search Struct
  ushort x, y;
  long parent;
} BFSS;

typedef struct _minmax {
  int min;
  int max;
} MM;

typedef struct _utilitysettings {
  // Macro accessible only settings

  flag fRndOld;
  long cAlloc;
  long cAllocTotal;
  long cAllocSize;
} US;

#define PutPt(ipt, xval, yval) rgpt[ipt].x = xval; rgpt[ipt].y = yval;
#define GetPt(ipt, xval, yval) xval = rgpt[ipt].x; yval = rgpt[ipt].y;
#define PutPt3(ipt, xval, yval, zval) \
  rgpt[ipt].x = xval; rgpt[ipt].y = yval; rgpt[ipt].z = zval;
#define GetPt3(ipt, xval, yval, zval) \
  xval = rgpt[ipt].x; yval = rgpt[ipt].y; zval = rgpt[ipt].z;
#define BfssPush(i, m, n, o) \
  bfss[i].x = (m); bfss[i].y = (n); bfss[i].parent = (o); (i)++


/*
******************************************************************************
** General Routines
******************************************************************************
*/

// Variables defined locally
extern US us;

// Function hooks implemented elsewhere
#ifdef ASSERT
extern void AssertCore(flag);
#define Assert(f) AssertCore(f)
#else
#define Assert(f)
#endif
extern int PrintSzCore(CONST char *, int);
extern void *PAllocate(long);
extern void DeallocateP(void *);
extern byte BRead(FILE *);

// Functions implemented locally
extern void *operator new(size_t);
extern void *operator new(size_t, void *);
extern void operator delete(void *);
extern void operator delete(void *, void *);
#define RgAllocate(c, t) ((t *)PAllocate((c) * sizeof(t)))
extern void *ReallocateArray(void *, int, int, int);
extern int PrintSzNCore(CONST char *, int, int);
extern int PrintSzNNCore(CONST char *, int, int, int);
extern int PrintSzLCore(CONST char *, long, int);
#define PrintSz(sz) PrintSzCore(sz, nPrintNormal)
#define PrintSz_N(sz) PrintSzCore(sz, nPrintNotice)
#define PrintSz_W(sz) PrintSzCore(sz, nPrintWarning)
#define PrintSz_E(sz) PrintSzCore(sz, nPrintError)
#define PrintSzN(sz, n) PrintSzNCore(sz, n, nPrintNormal)
#define PrintSzNN(sz, n1, n2) PrintSzNNCore(sz, n1, n2, nPrintNormal)
#define PrintSzL(sz, l) PrintSzLCore(sz, l, nPrintNormal)
#define PrintSzN_N(sz, n) PrintSzNCore(sz, n, nPrintNotice)
#define PrintSzNN_N(sz, n1, n2) PrintSzNNCore(sz, n1, n2, nPrintNotice)
#define PrintSzL_N(sz, l) PrintSzLCore(sz, l, nPrintNotice)
#define PrintSzN_W(sz, n) PrintSzNCore(sz, n, nPrintWarning)
#define PrintSzNN_W(sz, n1, n2) PrintSzNNCore(sz, n1, n2, nPrintWarning)
#define PrintSzL_W(sz, l) PrintSzLCore(sz, l, nPrintWarning)
#define PrintSzN_E(sz, n) PrintSzNCore(sz, n, nPrintError)
#define PrintSzNN_E(sz, n1, n2) PrintSzNNCore(sz, n1, n2, nPrintError)
#define PrintSzL_E(sz, l) PrintSzLCore(sz, l, nPrintError)
extern flag FErrorRange(CONST char *, int, int, int);
extern void CopyRgb(CONST char *, char *, long);
extern int CchSz(CONST char *);
extern int CompareSz(CONST char *, CONST char *);
extern int CompareSzI(CONST char *, CONST char *);
#define FEqSz(sz1, sz2) (CompareSz(sz1, sz2) == 0)
#define FEqSzI(sz1, sz2) (CompareSzI(sz1, sz2) == 0)
extern int CompareRgch(CONST char *, int, CONST char *, int);
extern int CompareRgchI(CONST char *, int, CONST char *, int);
extern flag FCompareSzRgch(CONST char *, CONST char *, int);
extern flag FCompareSzRgchI(CONST char *, CONST char *, int);
extern int FindCh(CONST char *, int, char);
extern void CopyRgchToSz(CONST char *, int, char *, int);
#define CopySz(sz1, sz2, cch) CopyRgchToSz(sz1, CchSz(sz1) + 1, sz2, cch)
extern long LFromRgch(CONST char *, int);
extern long LFromRgch2(CONST char *, int, int);
extern int NMin(int n1, int n2);
extern int NMax(int n1, int n2);
extern real RMin(real n1, real n2);
extern real RMax(real n1, real n2);
extern void SortN(int *, int *);
extern FILE *FileOpen(CONST char *, CONST char *);
extern void WriteSz(FILE *file, CONST char *);
extern word WRead(FILE *file);
extern dword LRead(FILE *file);
extern long LMul(long, long);
extern long LDiv(long, long);
extern long LMod(long, long);
extern long LPower(long, long);
extern real RDiv(real, real);
#define NMultShift(n1, n2, n) (int)((long)(n1) * (long)(n2) >> (n))
#define LMultShift(l1, l2, n) (long)((quad)(l1) * (quad)(l2) >> (n))
#define LengthR(x1, y1, x2, y2) RSqr(Sq((x1)-(x2))+Sq((y1)-(y2)))
#define LengthN(x1, y1, x2, y2) \
  LengthR((real)(x1), (real)(y1), (real)(x2), (real)(y2))
#define PlotAngle(h, v, x, y, r, d) \
  h = (x) + (r)*RCosD(d); v = (y) + (r)*RSinD(d);
#define AngleN(h, v, x, y, rx, ry, d) \
  AngleR(h, v, x, y, (real)rx, (real)ry, (real)d)
extern void AngleR(int *, int *, int, int, real, real, real);
extern real GetAngle(int, int, int, int);
extern void RotateR(real *, real *, real);
extern void RotateR2(real *, real *, real, real);
#define RotateR2Init(rS, rC, d) rS = RSinD(d); rC = RCosD(d)


/*
******************************************************************************
** Vector Routines
******************************************************************************
*/

class CVector
{
public:
  real m_x;
  real m_y;
  real m_z;

  INLINE int NX() CONST { return (int)m_x; }
  INLINE int NY() CONST { return (int)m_y; }
  INLINE int NZ() CONST { return (int)m_z; }

  void Set(real, real, real);
  void Mul(real);
  real Length() CONST;
  void Sphere(real, real);
  real Dot(CONST CVector &) CONST;
  void Cross(CONST CVector &, CONST CVector &);
  real Angle(CONST CVector &) CONST;
  void Normal(real, real, real, real, real, real, real, real, real);
};


/*
******************************************************************************
** Trie Tree Routines
******************************************************************************
*/

extern int CsCreateTrie(CONST uchar *[], int, TRIE, int, flag);
extern int ILookupTrie(CONST TRIE, CONST char *, int, flag);
#define ILookupTrieSz(trie, sz, f) ILookupTrie(trie, sz, CchSz(sz), f)


/*
******************************************************************************
** Random Number Routines
******************************************************************************
*/

void InitRndL(ulong);
void InitRndRgl(ulong[], int);
extern int Rnd(int, int);

/* util.h */
