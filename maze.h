/*
** Daedalus (Version 3.5) File: maze.h
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
** This file contains definitions for Maze creation algorithms, solving
** algorithms, and other Maze utilities.
**
** Created: 11/22/1996.
** Last code change: 10/30/2024.
*/


#define iVarScale 8
#define iVar (1 << iVarScale)
#define iCircuitMax 30

enum _ensuremazesize {
  femsEvenSize  = 0x1,
  femsEvenSize2 = 0x2,
  femsEvenStart = 0x4,
  femsNoResize  = 0x8,
  femsNoSection = 0x10,
  femsOddSize   = 0x20,
  femsMinSize   = 0x40,
  fems64K       = 0x80,
};

enum _entranceposition {
  epCorner   = 0,
  epMiddle   = 1,
  epBalanced = 2,
  epRandom   = 3,
};

enum _labyrinthtype {
  ltClassical  = 0,
  ltChartres   = 1,
  ltCretan     = 2,
  ltManInMaze  = 3,
  ltChartresR  = 4,
  ltCustom     = 5,
  ltClassicalF = 6,
  ltChartresF  = 7,
  ltPartial    = 8,
};

enum _labyrinthcircuitshape {
  lcsCircle = 0,
  lcsRound  = 1,
  lcsSquare = 2,
  lcsFlat   = 3,
};

enum _labyrinthcustomcenter {
  lccRadius   = 0,
  lccEntrance = 1,
  lccExit     = 2,
  lccBalanced = 3,
  lccMerged   = 4,
};


/*
******************************************************************************
** Types
******************************************************************************
*/

class CMazK; // Forward

typedef struct _mazesettings {
  // File settings

  long nInfGen;
  long nInfCutoff;
  long nInfFileCutoff;
  flag fInfEller;
  flag fInfAutoStart;
  int nInfFile;
  int xCutoff;

  // Random settings

  int nRndSeed;
  int nRndBias;
  int nRndRun;

  // Dot settings

  int nRadar;

  // Maze settings

  flag fSection;
  flag fTeleportEntrance;
  flag fPoleNoDeadEnd;
  flag fSolveEveryPixel;
  flag fSolveDotExit;
  flag fRandomPath;
  flag fCountShortest;
  int nTweakPassage;
  long cMaze;
  int nWeaveOff;
  int nWeaveOn;
  int nWeaveRail;
  int nEntrancePos;

  // Labyrinth settings

  int nLabyrinth;
  int lcs;
  int nClassical;
  flag fClassicalCirc;
  char szCustom[cchSzDef];
  flag fCustomAuto;
  int nCustomFold;
  int nCustomAsym;
  int nCustomPartX;
  int nCustomPartY;
  int lcc;
  int zCustomMiddle;

  // Create settings

  flag fRiver;
  flag fRiverEdge;
  flag fRiverFlow;
  int omega;
  int omega2;
  int omegas;
  int omegaf;
  int nOmegaDraw;
  char szPlanair[cchSzMax];
  flag fTreeWall;
  flag fTreeRandom;
  int nTreeRiver;
  int nForsInit;
  int nForsAdd;
  int cSpiral;
  int cSpiralWall;
  int cRandomAdd;
  int xFractal;
  int yFractal;
  int zFractal;
  flag fFractalI;
  flag fCrackOff;
  int nCrackLength;
  int nCrackPass;
  int nCrackSector;
  int nBraid;
  int nSparse;
  flag fKruskalPic;
  flag fWeaveCorner;
  flag fTiltDiamond;
  int nTiltSize;

  // Macro accessible only settings

  int nCellMax;
  int nHuntType;
  int nFractalD;
  int nFractalL;
  int nFractalT;

  // Internal settings

  int xEntrance;
  int yEntrance;
  int xExit;
  int yExit;
  int cRunRnd;
  int dirRnd;
  int cRunRndSeg;
  int dirRndSeg;
  long iSpiralIndex;
  FILE *fileInf;
  flag fInfMiddle;
  long nInfCount;
  long *rgnInfEller;
  int cnInfEller;
} MS;

typedef struct _rc {
  long x1;
  long y1;
  long x2;
  long y2;
} RC;

typedef struct _rc3 {
  long x1;
  long y1;
  long z1;
  long x2;
  long y2;
  long z2;
} RC3;

typedef struct _isolationdetachment {
  long zList;
  long iBack;
  long set;
} ID;

typedef struct _krus {
  struct _krus *next;
  long count;
} KRUS;

extern MS ms;
extern int xl, yl, xh, yh;
extern CONST char *rgszDir[DIRS];
extern CONST char *rgszChip[7];

class CMaz : public CMon3 // Monochrome bitmap Maze
{
public:
  INLINE void SetXyh()
    { xl = yl = 0; xh = m_x-1; yh = m_y-1; }
  INLINE flag FOnMaze(int x, int y) CONST
    { return Get(x-1, y-1) && Get(x+1, y-1) && Get(x-1, y+1) &&
    Get(x+1, y+1); }
  INLINE flag FOnMaze2(int x, int y)
    { return Get(x-2, y-2) && Get(x+2, y-2) && Get(x-2, y+2) &&
    Get(x+2, y+2); }
  INLINE flag FOnMazeZeta(int x, int y)
    { return Get(x-1, y-2) && Get(x+2, y-1) && Get(x-2, y+1) &&
    Get(x+1, y+2); }
  INLINE flag FOnWall(int x, int y) CONST
    { return Get(x & ~1, y & ~1) & Get(x + FOdd(x), y + FOdd(y)); }
  INLINE flag FOnPassage(int x, int y) CONST
    { return !Get(x | 1, y | 1) && !Get(x - FOdd(y), y - FOdd(x)); }
  INLINE flag FLegalMaze3(int x, int y, int z) CONST
    { return x > 0 && x < m_x3-1 && y > 0 && y < m_y3-1 && z >= 0 &&
    z <= m_z3-1; }
  INLINE flag FLegalMaze4(int w, int x, int y, int z) CONST
    { return x > 0 && x < m_x3-1 && y > 0 && y < m_y3-1 && z >= 0 &&
    z <= m_z3-1 && w >= 0 && w <= m_w3-1; }
  INLINE bit GetI(int x, int y, int z, flag f3D) CONST
    { return f3D ? Get3I(x, y, z) : Get(x, y); }
  INLINE bit GetIFast(int x, int y, int z, flag f3D) CONST
    { return f3D ? Get3I(x, y, z) : GetFast(x, y); }
  INLINE int Count(int x, int y) CONST
    { return Get(x, y-1) + Get(x-1, y) + Get(x, y+1) + Get(x+1, y); }
  INLINE int Count2(int x, int y) CONST
    { return Get(x, y-2) + Get(x-2, y) + Get(x, y+2) + Get(x+2, y); }
  INLINE int CountWeave(int x, int y) CONST
    { return Get(x, y-4) + Get(x-4, y) + Get(x, y+4) + Get(x+4, y); }
  INLINE int Count3(int x, int y, int z) CONST
    { return Get3(x, y-1, z) + Get3(x-1, y, z) + Get3(x, y+1, z) +
    Get3(x+1, y, z) + (z <= 0 || Get3(x, y, z-1)) +
    (z >= Even(m_z3)-2 || Get3(x, y, z+1)); }
  INLINE int Count4(int w, int x, int y, int z) CONST
    { return Get4(w, x, y-1, z) + Get4(w, x-1, y, z) +
    Get4(w, x, y+1, z) + Get4(w, x+1, y, z) +
    (z <= 0 || Get4(w, x, y, z-1)) +
    (z >= Even(m_z3)-2 || Get4(w, x, y, z+1)) +
    (w <= 0 || Get4(w-1, x, y, z)) +
    (w >= Even(m_w3)-2 || Get4(w+1, x, y, z)); }
  INLINE long DoCrackIsolations()
    { return DoRemoveIsolationDetachment(fFalse); }
  INLINE long DoConnectDetachments()
    { return DoRemoveIsolationDetachment(fTrue); }

  // From maze.cpp
  flag FEnsureMazeSize(int, int);
  void BlockOuter(KV);
  void MazeClear(KV);
  int AddEntranceExit(flag);
  void MakeEntranceExit(int);
  flag MakeIsolationDetachment(flag);
  flag FWouldMakeDeadEnd(int, int) CONST;
  flag FWouldMakePole(int, int) CONST;
  flag FWouldMakeIsolation(int, int) CONST;
  int PeekWall(int, int, int, int, flag, int) CONST;
  int FollowWall(int *, int *, int, int) CONST;
  flag FBlindAlley(int, int, int) CONST;
  int FollowPassage(int *, int *, int *, int, flag) CONST;
  int PeekRandom(int, int, int, int, flag) CONST;
  flag FFindPassage(int *, int *, flag) CONST;
  int DirFindUncreated(int *, int *, flag) CONST;

  long MazeNormalize(flag fWall);
  long MazeZoomAndExpandSetCells();
  long MazeRoomifyThinnedCells();
  long MazeTweakEndpoints();
  long MazeTweakPassages();
  long DoSetAllCellsToPoles();
  void RemoveIdIn(ID *, int, int, int, int, long *, long *, flag);
  long DoRemoveIsolationDetachment(flag);
  long DoConnectPoles(flag);
  long DoDeletePoles(flag);
  long DoCrackDeadEnds();
  long DoCrackEntrances();
  long DoCrackPassages();
  flag FCrackRoom(int, int);
  long DoCrackRooms();
  KV GetB(int, int, int);
  long DoCrackBoundaryWall(flag, int);
  long DoCrackTubes();
  long DoCrackIsland(int, int, flag);
  long DoCrackIslands(flag);
  long DoMakeSymmetric(int nOp, flag fBraid);
  flag MazeZoomWithPicture(CONST CCol *);
  void BlockMoveMaze(CONST CMaz &, int, int, int, int, int, int,
    int, int, int, int);
  void BlockMoveMaze3(CONST CMaz &, int, int, int, int, int, int,
    int, int, int, int, int, int);
  void MazeAnalyze() CONST;
  void MazeAnalyze2() CONST;
  void MazeAnalyze3D() CONST;

  void WriteText2(FILE *, flag) CONST;
  void WriteTextDOS(FILE *, int, int) CONST;
  void WriteText3D(FILE *, flag, flag) CONST;

  // From create.cpp
  flag PerfectGenerate(flag, int, int);
  flag CreateMazePerfect();
  flag CreateMazePerfect2();
  long BraidConnectWalls();
  flag CreateMazeBraid();
  flag CreateMazeBraidTilt();
  void SpiralMakeNew(ushort *, ushort *);
  int SpiralMakeTemplate();
  flag CreateMazeSpiral();
  flag CreateMazeDiagonal();
  flag RecursiveGenerate(int, int);
  flag CreateMazeRecursive();
  flag PrimGenerate(flag, flag, int, int);
  flag CreateMazePrim();
  flag CreateMazePrim2(int, int);
  flag CreateMazeKruskal(flag, CCol *, CCol *);
  flag TreeGenerate(flag, int, int);
  flag CreateMazeTree();
  flag CreateMazeForest();
  flag CreateMazeAldousBroder();
  flag CreateMazeWilson();
  void EllerMakeRow(long *, long *, int, int, int, int, int, flag);
  flag CreateMazeEller();
  void BraidMakeRow(long *, long *, int, int, int, int, int, int);
  flag CreateMazeBraidEller();
  flag CreateMazeDivision();
  flag CreateMazeBinary();
  flag CreateMazeSidewinder();

  // From create2.cpp
  bit Get3M(int x, int y, int z) CONST;
  bit Get3I(int x, int y, int z) CONST;
  bit Get4M(int w, int x, int y, int z) CONST;

  void TwoByTwoGenerate();
  void FractalGenerate(CMaz &, int, int, int);
  flag CreateMazeFractal();
  void FractalPartialGenerate(CMaz &, int, long, long, int, int, int,
    long *, long *);
  flag FractalPartial(long, long, int, int, long *, long *);
  flag DivisionPartial(int, int, int, int, int, long *, long *);
  flag CreateMaze3D();
  flag CreateMazeBinary3D();
  flag CreateMazeSidewinder3D();
  flag CreateMazeDivision3D();
  void FractalGenerate3D(CMaz &, int, int, int, int);
  flag CreateMazeFractal3D();
  void FractalPartialGenerate3D(CMaz &, int, long, long, long, int, int, int,
    int, long *, long *);
  flag FractalPartial3D(long, long, long, int, int, long *, long *);
  flag DivisionPartial3D(int, int, int, int, int, int, int, long *, long *);
  flag FClarify3D(CMap &, KV, KV) CONST;
  flag CreateMaze4D();
  flag FClarify4D(CMap &, KV, KV) CONST;
  flag FConvert4DTo3D(CMon &, CCol &, KV, KV, KV, KV, KV);
  flag CreateMazeHyper();
  void MapND(CONST int *, int *, int *) CONST;
  flag CreateMazeND();
  void CenterND(CONST long *, int *, int *) CONST;
  void MapNDRender(int *, int *, CONST int *, CONST int *,
    CONST long *) CONST;
  void RenderND(CMap &, int *, int *, int *, int *,
    CONST long *, CONST long *, KV kv, int grf) CONST;
  flag MoveND(long *, CONST long *, int *, int *, flag) CONST;
  flag FValidPlanair(CONST char *) CONST;
  flag CreateMazePlanair();
  flag CreateMazeSegment();
  flag CreateMazeInfinite(int);
  flag PerfectPartial(int, int, int, int *, int *, int *);
  flag AldousBroderPartial(int, int, int, int *, int *, int *);
  flag RecursivePartial(int, int, int, int, int *, int *, int *, int *,
    CMaz &);

  // From create3.cpp
  flag CreateMazeGamma();
  flag CreateMazeDelta();
  flag CreateMazeSigma();
  flag CreateMazeTheta();
  flag CreateMazeUpsilon();
  flag CreateMazeOmicron();
  void FractalSet(CMazK *c, KV kv, int x, int y);
  flag FFractalLine(CMaz *, CMazK *, KV, int, int, int, int);
  int CreateMazeFractal2(CMazK *);
  flag WeaveGenerate(flag, int, int);
  flag CreateMazeWeave();
  flag ConvertWeaveTo3D(CMaz *);
  flag ConvertWeaveToInside(CMazK &, CMazK &, CMaz &, int, KV, KV, KV);
  flag FClarifyWeave(int, int, int);
  int DirFindUncreatedCavern(int *, int *, flag);
  flag CavernGenerate(flag, int, int);
  flag CreateMazeCavern();
  long CreateMazeCrack(flag);
  flag ZetaGenerate(flag, int, int);
  flag CreateMazeZeta();
  void ArrowDraw(CCol *, int, int, int, KV);
  flag CreateMazeArrow(CCol *);
  flag CreateMazeDungeon(CCol &, CMon &, int, int, int, int, flag, int,
    int *, int *);

  // From labyrnth.cpp
  long UnicursalApply(flag);
  flag CreateMazeUnicursal();
  flag CreateMazeHilbert(flag);
  void CreateMazeTilt();
  void LabyrinthClassicalPartial(int, int, int);
  void LabyrinthHilbertPartial(int, int, int);
  void LabyrinthHilbertPartial3D(int, int, int, int);
  flag FCenterCells(int, int, int *, int *, int *);
  flag CreateLabyrinth();
  flag CreateLabyrinthChartres();
  flag CreateLabyrinthCustom();

  // From solve.cpp
  long SolveMazeFillDeadEnds(int, int, int, int);
  long DoFillDeadEnds(int, int, int, int);
  long DoMarkDeadEnds(int, int, int, int);
  long DoMarkCulDeSacs(int, int, int, int);
  long SolveMazeFillCulDeSacs(int, int, int, int);
  long DoCrackBlindAlleys(CONST CMaz &);
  long DoMarkBlindAlleys();
  long SolveMazeFillBlindAlleys(int, int, int, int);
  long DoMarkCollisions(int, int);
  long SolveMazeFillCollisions(int, int, int, int);
  long SolveMazeRecursive(int, int, int, int, flag);
  long SolveMazeShortest(int, int, int, int, flag);
  long SolveMazeShortest2(int, int, int, int, flag);
  long SolveMazeFollowWall(int, int, int, int, int, flag);
  long SolveMazePledge(int, int, int, int, int, flag);
  long SolveMazeChain(int, int, int, int);
  long SolveMazeTremaux(CMazK &, int, int, int, int, KV, KV, KV, KV) CONST;
  flag DoShowBottlenecks(int, int, int, int, int);
  long SolveMazeWeave();
  long SolveMaze3D();
  long SolveMaze4D();
  long SolveMazeND();
  long SolveMazeHyper();
  int SolveMazeFractal2() CONST;
};

class CMazK : public CCol3 // Color bitmap Maze
{
public:
};


/*
******************************************************************************
** Maze Utilities
******************************************************************************
*/

#define LPrime(l) ((l) % 109 != 0 ? 109 : ((l) % 191 != 0 ? 191 : \
  ((l) % 199 != 0 ? 199 : ((l) % 911 != 0 ? 911 : 919))))
#define RndSkip(n1, n2) ((n1) + (Rnd(0, (((n2) - (n1)) >> 1)) << 1))
#define fCellMax (ms.nCellMax >= 0 && --ms.nCellMax < 0)
#define ZEye(z) ((z) * dr.zWall >> iVarScale)

#define FLegalMaze(x, y) \
  ((x) >= xl && (x) < xh && (y) >= yl && (y) < yh)
#define FLegalMaze2(x, y) \
  ((x) >= xl && (x) <= xh && (y) >= yl && (y) <= yh)

extern flag FMazeSizeError(int, int);
extern int RndDir(void);
extern ulong MazeCountPossible(int, int);

// Function hooks implemented elsewhere
extern int InitCoordinates(int);
extern flag FSetCoordinates(int, int, int, int, int, int, KV);
extern int InitPatch(int);
extern flag FSetPatch(int, int, int, int, int, int, KV);
extern int InitCoorPatch(flag, int);
extern flag FSetCoorPatch(flag, int, int, int, int, int, int, KV);


/*
******************************************************************************
** Maze Creation Routines
******************************************************************************
*/

#define SpiralMax     30
#define SpiralWallMax 60
#define SpiralDead    0xFFFF

KRUS *KruskalFind(KRUS *);
void KruskalUnion(KRUS *, KRUS *);

#define dimMax 18
#define CLAR 10

extern void SegmentGetSz(char *, int);
extern void SegmentParseSz(CONST char *);

// Planair Normal: "a0a1a2a3"
// Planair Torus: "a2a3a0a1"
// Planair Cube: "e2d3f0b1 e7a3f3c1 e4b3f6d1 e1c3f5a1 c4d0a0b4 a2d6c6b2"

extern flag FCopyRgchToSzCustom(CONST char *, int, char *);
extern flag FValidSzCustom(CONST char *, int *, int *);
extern flag FComputeNextCustom(char *, flag);

/* maze.h */
