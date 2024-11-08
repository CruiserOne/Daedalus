/*
** Daedalus (Version 3.5) File: util.cpp
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
** This file contains generic utilities. Nothing here is related to graphics
** or any other part of Daedalus.
**
** Created: 6/4/1993.
** Last code change: 10/30/2024.
*/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include "util.h"


US us = {fTrue, 0L, 0L, 0L};


/*
******************************************************************************
** General Routines
******************************************************************************
*/

// Global new and delete operator implementations.

void *operator new(size_t cb)
{
  void *pv;

  pv = PAllocate((long)cb);
  return pv;
}

void operator delete(void *pv)
{
  DeallocateP(pv); 
}

void *operator new(size_t cb, void *pv)
{
  return pv;
}

void operator delete(void *pv, void *pv2)
{
}


// Change the size of a memory allocation, containing a list of cElem items of
// cbElem size, to a list of cElemNew items of cbElem size.

void *ReallocateArray(void *rgElem, int cElem, int cbElem, int cElemNew)
{
  void *rgElemNew;

  rgElemNew = PAllocate(cElemNew * cbElem);
  if (rgElemNew == NULL)
    return NULL;
  ClearPb(rgElemNew, cElemNew * cbElem);
  if (rgElem != NULL)
    CopyPb(rgElem, rgElemNew, Min(cElem, cElemNew) * cbElem);
  return rgElemNew;
}


// Display a string with an integer parameter embedded in it.

int PrintSzNCore(CONST char *sz, int n, int nPriority)
{
  char szT[cchSzDef];

  sprintf(S(szT), sz, n);
  return PrintSzCore(szT, nPriority);
}


// Display a string with two integer parameters embedded in it.

int PrintSzNNCore(CONST char *sz, int n1, int n2, int nPriority)
{
  char szT[cchSzDef];

  sprintf(S(szT), sz, n1, n2);
  return PrintSzCore(szT, nPriority);
}


// Display a string with a long integer parameter embedded in it.

int PrintSzLCore(CONST char *sz, long l, int nPriority)
{
  char szT[cchSzDef];

  sprintf(S(szT), sz, l);
  return PrintSzCore(szT, nPriority);
}


// Return whether n is in the range from n1 to n2, inclusive. If not, display
// a suitable warning message.

flag FErrorRange(CONST char *sz, int n, int n1, int n2)
{
  char szT[cchSzDef];

  if (n < n1 || n > n2) {
    sprintf(S(szT), "%s value %d is out of range from %d to %d.\n",
      sz, n, n1, n2);
    PrintSz_W(szT);
    return fTrue;
  }
  return fFalse;
}


// Copy bytes from one buffer to another. The buffers shouldn't overlap.

void CopyRgb(CONST char *pbFrom, char *pbTo, long cb)
{
  while (cb--)
    *pbTo++ = *pbFrom++;
}


// Return length of a zero terminated string, not including the terminator.

int CchSz(CONST char *sz)
{
  CONST char *pch = sz;

  while (*pch)
    pch++;
  return PD(pch - sz);
}


// Compare two strings case sensitively. Return 0 if equal, negative number if
// first less than second, and positive number if first greater than second.

int CompareSz(CONST char *sz1, CONST char *sz2)
{
  while (*sz1 && (*sz1 == *sz2))
    sz1++, sz2++;
  return (int)*sz1 - *sz2;
}


// Compare two strings case insensitively. Return 0 if equal, negative number
// if first less than second, and positive if first greater than second.

int CompareSzI(CONST char *sz1, CONST char *sz2)
{
  while (*sz1 && (ChCap(*sz1) == ChCap(*sz2)))
    sz1++, sz2++;
  return (int)ChCap(*sz1) - ChCap(*sz2);
}


// Compare two ranges of characters case sensitively. Return 0 if equal,
// negative if first less than second, and positive if greater than second.

int CompareRgch(CONST char *rgch1, int cch1, CONST char *rgch2, int cch2)
{
  while (cch1 > 0 && cch2 > 0 && *rgch1 == *rgch2)
    rgch1++, rgch2++, cch1--, cch2--;
  return cch1 > 0 ? (cch2 > 0 ? (int)*rgch1 - *rgch2 : (int)*rgch1) :
    cch2 == 0 ? 0 : -(int)*rgch2;
}


// Compare two ranges of characters case insensitively. Return 0 if equal,
// negative if first less than second, and positive if greater than second.

int CompareRgchI(CONST char *rgch1, int cch1, CONST char *rgch2, int cch2)
{
  while (cch1 > 0 && cch2 > 0 && ChCap(*rgch1) == ChCap(*rgch2))
    rgch1++, rgch2++, cch1--, cch2--;
  return cch1 > 0 ? (cch2 > 0 ? (int)ChCap(*rgch1) - ChCap(*rgch2) :
    (int)ChCap(*rgch1)) : cch2 == 0 ? 0 : -(int)ChCap(*rgch2);
}


// Return whether a zero terminated string is equal to a range of characters,
// case sensitively.

flag FCompareSzRgch(CONST char *sz, CONST char *pch, int cch)
{
  if (sz[0] == chNull)  // Note the empty string is NOT equal.
    return fFalse;
  while (cch > 0 && *sz == *pch)
    sz++, pch++, cch--;
  return cch == 0 && sz[0] == chNull;
}


// Return whether a zero terminated string is equal to a range of characters,
// case insensitively.

flag FCompareSzRgchI(CONST char *sz, CONST char *pch, int cch)
{
  if (sz[0] == chNull)  // Note the empty string is NOT equal.
    return fFalse;
  while (cch > 0 && ChCap(*sz) == ChCap(*pch))
    sz++, pch++, cch--;
  return cch == 0 && sz[0] == chNull;
}


// Search for and return the position of a character within a string.

int FindCh(CONST char *pch, int cch, char ch)
{
  int ich;

  for (ich = 0; ich < cch; ich++)
    if (*pch++ == ch)
      return ich;
  return -1;
}


// Copy a range of characters and zero terminate it. If there are too many
// characters to fit in the destination buffer, the string is truncated.

void CopyRgchToSz(CONST char *pch, int cch, char *sz, int cchMax)
{
  cch = Min(cch, cchMax-1);
  CopyRgb(pch, sz, cch);
  sz[cch] = chNull;
}


// Convert a range of characters to upper case.

void UpperRgch(char *rgch, int cch)
{
  while (cch > 0) {
    *rgch = ChCap(*rgch);
    rgch++, cch--;
  }
}


// Parse and return a number contained in a range of characters.

long LFromRgch(CONST char *rgch, int cch)
{
  char sz[cchSzDef], *pch;
  long l;
  flag fBinary;

  CopyRgchToSz(rgch, cch, sz, cchSzDef);
  if (*sz != '#')
    return atol(sz);
  fBinary = (*(sz+1) == '#');

  // Strings starting with "#" are considered hexadecimal numbers.
  // Strings starting with "##" are considered binary numbers.
  l = 0;
  for (pch = sz+1+fBinary; *pch; pch++)
    l = fBinary ? ((l << 1) | (*pch == '1')) :
      ((l << 4) | NHex2(ChCap(*pch)));
  return l;
}


// Parse a real number contained in a range of characters, returning it as an
// fixed point integer with the decimal point at the specified power of ten.

long LFromRgch2(CONST char *rgch, int cch, int nDecimal)
{
  char sz[cchSzDef];
  real r;
  long l;

  CopyRgchToSz(rgch, cch, sz, cchSzDef);
  r = atof(sz);
  l = LPower(10, NAbs(nDecimal));
  if (nDecimal >= 0)
    r *= (real)l;
  else
    r /= (real)l;
  return (long)r;
}


// Return the smaller of two integers.

int NMin(int n1, int n2)
{
  return Min(n1, n2);
}


// Return the larger of two integers.

int NMax(int n1, int n2)
{
  return Max(n1, n2);
}


// Return the smaller of two floating point numbers.

real RMin(real n1, real n2)
{
  return Min(n1, n2);
}


// Return the larger of two floating point numbers.

real RMax(real n1, real n2)
{
  return Max(n1, n2);
}


// Ensure n1 < n2. If not, swap them.

void SortN(int *n1, int *n2)
{
  if (*n1 > *n2)
    SwapN(*n1, *n2);
}


// Open a file.

FILE *FileOpen(CONST char *sz, CONST char *szMode)
{
#ifdef SECURECRT
  FILE *file;
  if (fopen_s(&file, sz, szMode))
    file = NULL;
  return file;
#else
  return fopen(sz, szMode);
#endif
}


// Write a string to a file.

void WriteSz(FILE *file, CONST char *sz)
{
  CONST char *pch;

  for (pch = sz; *pch; pch++) {
    if (*pch == '\n')  // Translate LF to CRLF for PC's
      putc('\r', file);
    putc(*pch, file);
  }
}


// Read a 16 bit word from a file.

word WRead(FILE *file)
{
  byte b1, b2;

  b1 = getbyte(); b2 = getbyte();
  return WFromBB(b1, b2);
}


// Read a 32 bit long from a file.

dword LRead(FILE *file)
{
  byte b1, b2, b3, b4;

  b1 = getbyte(); b2 = getbyte(); b3 = getbyte(); b4 = getbyte();
  return LFromWW(WFromBB(b1, b2), WFromBB(b3, b4));
}


// Multiplication function. Return x*y, checking for overflow.

long LMul(long x, long y)
{
  long z = x * y;

  if (y != 0 && z / y != x)
    return 0x80000000;
  return z;
}


// Division function. Return x/y, checking for illegal parameters.

long LDiv(long x, long y)
{
  if (y == 0)  // Dividing by 0 is assumed to be 0.
    return 0;
  else if (x == 0x80000000 && y == -1)  // This will crash on x86 too.
    return x;
  return x / y;
}


// Modulus function. Return remainder of x/y.

long LMod(long x, long y)
{
  if (y == 0)  // Remainder when dividing by 0 is assumed to be 0.
    return 0;
  else if (x == 0x80000000 && y == -1)  // This will crash on x86 too.
    return 0;
  return x % y;
}


// Integer power raising function. Return x^y.

long LPower(long x, long y)
{
  long pow;

  if (y < 1)
    return 1;
  if (y == 1 || x == 0 || x == 1)
    return x;
  if (x == -1)
    return FOdd(y) ? -1 : 1;
  for (pow = x; y > 1; y--) {
    if (pow * x / x != pow)   // Return 0 if overflow 32 bits.
      return 0;
    pow *= x;
  }
  return pow;
}


// Division function. Return x/y, checking for illegal parameters.

real RDiv(real x, real y)
{
  if (y >= 0.0) {
    if (y < rMinute)
      y = rMinute;
  } else {
    if (y > -rMinute)
      y = -rMinute;
  }
  return x / y;
}


// Calculate x and y coordinates along a circle or ellipse, given a center
// point, radius, and angle.

void AngleR(int *h, int *v, int x, int y, real rx, real ry, real d)
{
  *h = x + (int)(rx*RCosD(d));
  *v = y + (int)(ry*RSinD(d));
}


// Return the angle of the line between two points.

real GetAngle(int x1, int y1, int x2, int y2)
{
  real d;

  if (x1 != x2) {
    d = RAtnD((real)(y2 - y1) / (real)(x2 - x1));
    if (d < 0.0)
      d += rDegQuad;
  } else
    d = 0.0;
  if (x2 <= x1 && y2 > y1)
    d += rDegQuad;
  else if (x2 < x1 && y2 <= y1)
    d += rDegHalf;
  else if (x2 >= x1 && y2 < y1)
    d += rDeg34;
  Assert(FBetween(d, 0.0, rDegMax));
  return d;
}


// Rotate a point around the origin by the given number of degrees.

void RotateR(real *h, real *v, real d)
{
  real m = *h, n = *v, rS, rC;

  rS = RSinD(d); rC = RCosD(d);
  *h = m*rC - n*rS;
  *v = n*rC + m*rS;
}


// Fast version of RotateR that assumes the slow trigonometry values have
// already been computed. Useful when rotating many points by the same angle.

void RotateR2(real *h, real *v, real rS, real rC)
{
  real m = *h, n = *v;

  *h = m*rC - n*rS;
  *v = n*rC + m*rS;
}


/*
******************************************************************************
** Vector Routines
******************************************************************************
*/

// Assign the value of a vector given its components along each axis.

void CVector::Set(real x, real y, real z)
{
  m_x = x;
  m_y = y;
  m_z = z;
}


// Multiply or stretch a vector by a factor.

void CVector::Mul(real len)
{
  m_x *= len;
  m_y *= len;
  m_z *= len;
}


// Return the length of a vector.

real CVector::Length() CONST
{
  return RSqr(m_x*m_x + m_y*m_y + m_z*m_z);
}


// Create a unit length vector given spherical coordinate angles.

void CVector::Sphere(real theta, real phi)
{
  real r;

  m_x = RCosD(theta);
  m_y = RSinD(theta);
  r = RCosD(phi);
  m_z = RSinD(phi);
  m_x *= r; m_y *= r;
}


// Return the dot product of two vectors.

real CVector::Dot(CONST CVector &v2) CONST
{
  return m_x*v2.m_x + m_y*v2.m_y + m_z*v2.m_z;
}


// Determine the cross product of two vectors, i.e. create a new vector that's
// perpendicular to both.

void CVector::Cross(CONST CVector &v1, CONST CVector &v2)
{
  Set(v1.m_y*v2.m_z - v1.m_z*v2.m_y, v1.m_z*v2.m_x - v1.m_x*v2.m_z,
    v1.m_x*v2.m_y - v1.m_y*v2.m_x);
}


// Return the angle between two vectors.

real CVector::Angle(CONST CVector &v2) CONST
{
  real angle, len1, len2;

  len1 = Length();
  len2 = v2.Length();
  if (len1 != 0.0 && len2 != 0.0) {
    angle = Dot(v2)/len1/len2;
    if (angle == 0.0)
      return rPiHalf;
    else if (angle <= -1.0)
      return rPi;
    angle = RAtn(RSqr(1.0 - Sq(angle)) / angle);
    if (angle >= 0.0)
      return angle;
    else
      return angle + rPi;
  } else
    return rPiHalf;
}


// Create a normal vector to a plane, i.e. a vector perpendicular to the plane
// passing through three coordinates.

void CVector::Normal(real x1, real y1, real z1, real x2, real y2, real z2,
  real x3, real y3, real z3)
{
  CVector v1, v2;

  v1.Set(x2 - x1, y2 - y1, z2 - z1);
  v2.Set(x3 - x1, y3 - y1, z3 - z1);
  Cross(v1, v2);
}


/*
******************************************************************************
** Trie Tree Routines
******************************************************************************
*/

// Return whether two ranges of characters are equal. Either string ending
// prematurely with a zero terminator makes the strings not equal.

flag FEqRgch(CONST char *rgch1, CONST char *rgch2, int cch, flag fInsensitive)
{
  int ich;

  if (!fInsensitive) {
    for (ich = 0; ich < cch; ich++) {
      if (rgch1[ich] == '\0' || rgch1[ich] != rgch2[ich])
        return fFalse;
    }
  } else {
    for (ich = 0; ich < cch; ich++) {
      if (rgch1[ich] == '\0' || ChCap(rgch1[ich]) != ChCap(rgch2[ich]))
        return fFalse;
    }
  }
  return fTrue;
}


// Create a trie tree within the buffer rgsOut of size csTrieMax. Returns the
// size of the trie in shorts, or -1 on failure. The input list of strings is
// used to create the trie. It does not have to be in sorted order. Trie
// lookups on strings will return the index they are in the original list. No
// two strings should be the same. Individual strings shouldn't be longer than
// 255 characters, or rather trailing unique substrings can't be longer than
// 255. With signed 16 bit indexes, these tries can't be larger than 32K
// shorts, or 64K bytes. That can store about 2500 strings.
//
// The trie format is a sequence of nodes. "Pointers" to other nodes are
// indexes into the array of shorts. There are two types of nodes: Standard
// nodes which handle all the branches from a particular leading substring,
// and leaf nodes for unique trailing substrings. Standard node format:
//
// Short 0: 0 = No string ends at this point. Non-zero = Payload + 1 for
//   substring leading up to this node.
// Short 1: High byte = Highest character covered by this node. Low byte =
//   Lowest character covered. If high character is 0, this is a leaf node.
// Short 2+: Pointers to nodes for the range of low through high characters.
//   0 = null pointer. Less than 0 means a string ends with this character,
//   where value contains -(Payload + 1).
//
// Leaf node format:
// Short 0: Same as standard node.
// Short 1: Length of trailing substring.
// Short 2: Payload for the string ending after this point.
// Short 3+: Range of characters storing final substring to compare against.

int CsCreateTrie(CONST uchar *rgszIn[], int cszIn, TRIE rgsOut, int csTrieMax,
  flag fInsensitive)
{
  uchar rgchStack[cchSzMax];
  int rgisStack[cchSzMax];
  long rgchUsed[256], iUsed = 0;
  int iStack, csOut = 0, isz, ich, chLo, chHi, chT, is, cch, isT,
    isRemember = 0, csz, iszSav;

  for (ich = 0; ich < 256; ich++)
    rgchUsed[ich] = 0;

  for (iStack = 0;; iStack++) {
    if (iStack >= cchSzMax) {
      Assert(fFalse);
      return -1;
    }
    rgisStack[iStack] = csOut;
    chLo = 255, chHi = 0, iUsed++, csz = 0;

    // Count how many strings match the current leading substring. Also
    // get the low and high character for these strings.
    for (isz = 0; isz < cszIn; isz++) {
      if (FEqRgch((CONST char *)rgszIn[isz], (CONST char *)rgchStack, iStack,
        fInsensitive)) {
        chT = rgszIn[isz][iStack];
        if (chT != 0) {
          if (fInsensitive)
            chT = ChCap(chT);
          csz++;
          iszSav = isz;
          rgchUsed[chT] = iUsed;
          if (chT < chLo)
            chLo = chT;
          if (chT > chHi)
            chHi = chT;
        }
      }
    }

    // If no strings match, back up to an earlier node.
    if (csz <= 0) {
LPop:
      loop {
        // Pop the stack to the parent node.
        iStack--;
        if (iStack < 0)
          goto LDone;
        is = rgisStack[iStack];
        chLo = (word)rgsOut[is + 1] & 255;
        chHi = (word)rgsOut[is + 1] >> 8;

        // Scan for a pointer that hasn't been filled out yet.
        for (ich = chLo + 1; ich <= chHi; ich++) {
          if (rgsOut[is + 2 + (ich - chLo)] == 1) {
            chT = ich;
            goto LPush;
          }
        }
      }
    }

    // Since there's at least one string, there will be a new node. Set the
    // pointer in the parent node to here.
    rgsOut[isRemember] = csOut;

    if (csOut >= csTrieMax - 1) {
      Assert(fFalse);
      return -1;
    }
    rgsOut[csOut++] = 0;  // Short 0

    // If exactly one string matches, create a leaf node.
    if (csz == 1) {
      for (ich = iStack; rgszIn[iszSav][ich] != 0; ich++)
        ;
      cch = ich - iStack;
      if (cch > 255) {
        Assert(fFalse);
        return -1;
      }
      if (csOut >= csTrieMax - 2 - ((cch + 1) >> 1)) {
        Assert(fFalse);
        return -1;
      }
      rgsOut[csOut++] = cch;  // Short 1
      rgsOut[csOut++] = 0;    // Short 2
      CopyRgb((char *)&rgszIn[iszSav][iStack], (char *)&rgsOut[csOut],
        (cch + 1) & ~1);
      if (fInsensitive)
        UpperRgch((char *)&rgsOut[csOut], cch);
      csOut += (cch + 1) >> 1;
      goto LPop;
    }

    // Create a standard node.
    if (csOut >= csTrieMax - 1 - (chHi - chLo + 1)) {
      Assert(fFalse);
      return -1;
    }
    rgsOut[csOut++] = (chHi << 8) | chLo;  // Short 1
    // Set those characters in use to the temporary pointer value 1, which
    // will be filled out later.
    for (ich = chLo; ich <= chHi; ich++)
      rgsOut[csOut++] = (rgchUsed[ich] == iUsed);
    chT = chLo;

LPush:
    rgchStack[iStack] = chT;
    isRemember = rgisStack[iStack] + 2 + (chT - chLo);
    rgsOut[isRemember] = -1;
  }

LDone:
  // Fill out payloads. For each string in the input list, walk the trie and
  // set the appropriate point in it to the string's index in the input list.
  Assert(csOut != 0);
  for (isz = 0; isz < cszIn; isz++) {
    is = 0;
    for (ich = 0;; ich++) {
      if (rgszIn[isz][ich] == '\0') {
        // Handle the substring case (short 0).
        Assert(rgsOut[is] == 0 || !fInsensitive);
        rgsOut[is] = isz+1;
        break;
      }
      chLo = (word)rgsOut[is + 1] & 255;
      chHi = (word)rgsOut[is + 1] >> 8;
      if (chHi == 0) {
        // Handle the leaf node case (short 2).
        Assert(rgsOut[is + 2] == 0 && FEqRgch((char *)&rgsOut[is+3],
          (char *)&rgszIn[isz][ich], chLo, fInsensitive));
        rgsOut[is + 2] = isz;
        break;
      }
      chT = rgszIn[isz][ich];
      if (fInsensitive)
        chT = ChCap(chT);
      Assert(chT >= chLo && chT <= chHi);
      isT = rgsOut[is + 2 + (chT - chLo)];
      Assert(isT != 0);
      if (isT <= 0) {
        // Handle the payload pointer within standard node case.
        Assert(rgszIn[isz][ich + 1] == '\0');
        rgsOut[is + 2 + (chT - chLo)] = -(isz+1);
        break;
      }
      is = isT;
    }
  }

  return csOut;
}


// Lookup a string in a trie created with CsCreateTrie. Return -1 if string
// not found, otherwise return the index of the string in the original list
// used to create the trie. This lookup is very fast, and not much slower than
// a single string compare. For strings not in the list, usually don't even
// have to look at all of its characters before knowing it's not in the trie.

int ILookupTrie(CONST TRIE rgsIn, CONST char *rgch, int cch,
  flag fInsensitive)
{
  int is = 0, chLo, chHi, ch, cchT;
  CONST char *pchEnd = rgch + cch, *pch, *pch1, *pch2;

  // Walk the input string, while going from node to node in the trie.
  for (pch = rgch;; pch++) {
    if (pch >= pchEnd) {

      // At end of input string. Check whether current node has a payload.
      if (rgsIn[is] != 0)
        return rgsIn[is]-1;
      else
        return -1;
    }
    chLo = (word)rgsIn[is + 1];
    chHi = chLo >> 8;
    if (chHi == 0) {

      // Leaf node. Compare rest of input string with substring in node.
      pch1 = (char *)&rgsIn[is + 3], pch2 = pch, cchT = chLo;
      if (!fInsensitive) {
        while (cchT > 0 && *pch1 == *pch2) {
          Assert(*pch1 != '\0');
          pch1++, pch2++, cchT--;
        }
      } else {
        while (cchT > 0 && *pch1 == ChCap(*pch2)) {
          Assert(*pch1 != '\0');
          pch1++, pch2++, cchT--;
        }
      }
      if (cchT > 0 || pch + chLo < pchEnd)
        return -1;
      else
        return rgsIn[is + 2];
    }

    // Standard node. Get pointer to appropriate child node.
    chLo &= 255;
    ch = *pch;
    if (fInsensitive)
      ch = ChCap(ch);
    if (ch < chLo || ch > chHi)
      return -1;
    is = rgsIn[is + 2 + (ch - chLo)];
    if (is < 0) {
      if (pch + 1 == pchEnd)
        return -is-1;
      else
        return -1;
    } else if (is == 0)
      return -1;
  }
}


/*
******************************************************************************
** Random Number Routines
******************************************************************************
*/

// C code for MT19937, with initialization improved Jan 26, 2002.
// Coded by Takuji Nishimura and Makoto Matsumoto.
// http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
// Copyright (C) 1997-2002 by Makoto Matsumoto and Takuji Nishimura,
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1) Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2) Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3) The names of its contributors may not be used to endorse or promote
// products derived from this software without specific prior written
// permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// Period parameters
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   // Constant vector a
#define UPPER_MASK 0x80000000UL // Most significant w-r bits
#define LOWER_MASK 0x7fffffffUL // Least significant r bits

ulong mt[N];   // The array for the state vector
int imt = N+1; // imt == N+1 means mt[N] is not initialized


// Initialize mt[N] with a seed.

void InitRndL(ulong l)
{
  if (us.fRndOld) {
    srand(l);
    return;
  }

  mt[0] = l & 0xffffffffUL;
  for (imt = 1; imt < N; imt++) {
    mt[imt] = 1812433253UL * (mt[imt-1] ^ (mt[imt-1] >> 30)) + imt;
    // See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. In the previous
    // versions, MSBs of the seed affect only MSBs of the array mt[].
    // Modified by Makoto Matsumoto, Jan 9, 2002.
  }
}


// Initialize by an array with array-length; rgl is the array for
// initializing keys; cl is its length. Slight change for C++, Feb 26, 2004.

void InitRndRgl(ulong rgl[], int cl)
{
  int i = 1, j = 0, c;

  InitRndL(19650218UL);
  c = (N > cl ? N : cl);
  for (; c; c--) {
    mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL)) +
      rgl[j] + j; // Non-linear
    i++, j++;
    if (i >= N) {
      mt[0] = mt[N-1]; i=1;
    }
    if (j >= cl)
      j = 0;
  }
  for (c = N-1; c; c--) {
    mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL)) -
      i; // Non-linear
    i++;
    if (i >= N) {
      mt[0] = mt[N-1]; i = 1;
    }
  }
  mt[0] = 0x80000000UL; // MSB is 1; assuring non-zero initial array
}


// Generate a random 32 bit number on interval [0,0xffffffff].

ulong LRnd(void)
{
  // mag01[x] = x * MATRIX_A for x=0,1.
  CONST ulong mag01[2] = {0x0UL, MATRIX_A};
  ulong l;

  if (imt >= N) { // Generate N longs at one time.
    int kk;

    if (imt == N+1)     // If InitRndL() has not been called,
      InitRndL(5489UL); // a default initial seed is used.

    for (kk = 0; kk < N-M; kk++) {
      l = (mt[kk] & UPPER_MASK) | (mt[kk+1] & LOWER_MASK);
      mt[kk] = mt[kk+M] ^ (l >> 1) ^ mag01[l & 0x1UL];
    }
    for (; kk < N-1; kk++) {
      l = (mt[kk] & UPPER_MASK) | (mt[kk+1] & LOWER_MASK);
      mt[kk] = mt[kk+(M-N)] ^ (l >> 1) ^ mag01[l & 0x1UL];
    }
    l = (mt[N-1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
    mt[N-1] = mt[M-1] ^ (l >> 1) ^ mag01[l & 0x1UL];

    imt = 0;
  }
  l = mt[imt++];

  // Tempering
  l ^= (l >> 11);
  l ^= (l << 7) & 0x9d2c5680UL;
  l ^= (l << 15) & 0xefc60000UL;
  l ^= (l >> 18);
  return l;
}


// Return a random integer between n1 and n2, inclusive.

int Rnd(int n1, int n2)
{
  int d, r, n, nT;

  if (n1 > n2) {
    // Special parameters allow returning any 32 bit number.
    if (n1 == lHighest && n2 == (int)0x80000000)
      return LRnd();
    SwapN(n1, n2);
  }
  d = n2 - n1 + 1;

  if (d <= 16384) {
    // Old way of computing compatible with previous versions.
    if (us.fRndOld)
      return NMultShift(rand() & 16383, d, 14) + n1;

    // For ranges that are a power of 2, equal probability across range.
    if (FPower2(d))
      return NMultShift(LRnd() >> 18, d, 14) + n1;

  } else if (FPower2(d)) {
    // Start with a random number between 0 and 2^31-1.
    r = LRnd() >> 1;

    // Compute (r * d / 2^31) without overflowing a 32 bit long.
    return LMultShift(r, d, 31) + n1;
  }

  // For irregular ranges, recalculate random numbers within remainder.
  n = lHighest / d;
  do {
    r = LRnd() >> 1;
    nT = r / n;
  } while (nT >= d);
  return nT + n1;
}

/* util.cpp */
