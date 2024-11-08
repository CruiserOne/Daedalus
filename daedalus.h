/*
** Daedalus (Version 3.5) File: daedalus.h
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
** This file contains system independent definitions for Daedalus as a whole.
**
** Created: 11/22/1993.
** Last code change: 10/30/2024.
*/


/*
******************************************************************************
** Macros
******************************************************************************
*/

#define szDaedalus "Daedalus"
#define szVersion "3.5"
#define cchSzMaxFile 128
#define cchSzOpr (cchSzMax*4)
#define cMacro 48
#define nScrollPage 8
#define zJump 16
#define imtnMax 3600
#define icldMax 64
#define szFileTempCore "daedalus.tmp"

#define icmdBase 1001
#define cmdScriptLast cmdScript31
#define cmdSizeLast cmdSize19
#define iActionMax ccmd
#define ccmd 470
#define copr 183
#define cvar 328
#define cfun 125

enum _edgebehavior {
  nEdgeVoid  = 0,
  nEdgeStop  = 1,
  nEdgeTorus = 2,
};

enum _insidedisplaymode {
  nInsideNone     = 0,
  nInsideJumpy    = 1,
  nInsideSmooth   = 2,
  nInsideFree     = 3,
  nInsideVeryFree = 4,
  nInsideSimple   = 5,
};

enum _semitransparentwalls {
  nTransNone     = 0,
  nTransFast     = 1,
  nTransThorough = 2,
  nTransVery     = 3,
  nTransDefault  = nTransVery,
};

enum _commandtag {
  fCmtRedraw       = 0x1,
  fCmtDirtyView    = 0x2,
  fCmtHourglass    = 0x4,
  fCmtBitmapBefore = 0x8,
  fCmtBitmapAfter  = 0x10,
  fCmtColmapBefore = 0x20,
  fCmtColmapAfter  = 0x40,
  fCmtRequireTemp  = 0x80,
  fCmtRequireExtra = 0x100,
  fCmtRequireColor = 0x200,
  fCmtSpree        = 0x400,
  fCmtSet2D        = 0x800,
  fCmtSet3D        = 0x1000,
  fCmtMaze         = 0x2000,
  fCmtMazeEntrance = 0x4000,
  fCmtMazeCenter   = 0x8000,
  fCmtSolve        = 0x10000,
  fCmtString       = 0x20000,
};

enum _messagebox {
  nAskOkCancel         = -1,
  nAskRetryCancel      = -2,
  nAskYesNo            = -3,
  nAskYesNoCancel      = -4,
  nAskAbortRetryIgnore = -5,
};

enum _commandvirtual {
  cmdOpenDB          = 1,
  cmdSaveDB          = 2,
  cmdSave3DC         = 3,
  cmdZapTexture      = 4,
  cmdOpenColmapPaint = 5,
};

enum _operationoperatingsystem {
  oosMessageIns,
  oosDrawText,
  oosTitle,
  oosBeep,
  oosBeep2,
  oosSound,
  oosDelay,
  oosSystem,
  oosMenu,
  oosCheck,
};

enum _variableoperatingsystem {
  vosEnsureTimer,
  vosPauseTimer,
  vosGetTimer,
  vosGetTick,
  vosScrollX,
  vosScrollY,
  vosSizeX,
  vosSizeY,
  vosPositionX,
  vosPositionY,
  vosDrawable,
  vosWindowTop,
  vosWindowMenu,
  vosWindowScroll,
  vosShowColmap,
  vosEventSpace,
  vosEventKey,
  vosEventMouse,
  vosKvDialog,
};

enum _hookoperatingsystem {
  hosRedraw,
  hosDirtyView,
  hosYield,
  hos3DOff,
  hos3DOn,
  hosDlgString,
  hosDlgString2,
  hosDlgString3,
};

#define BFocus() (!bm.fColor ? (CMap3 &)bm.b : (CMap3 &)bm.k)
#define PbFocus() (!bm.fColor ? (CMap3 *)&bm.b : (CMap3 *)&bm.k)
#define FWantTimer() (ws.fSpree || dr.fFollowMouse || \
  (dr.fInside && dr.nInside == nInsideVeryFree) || \
  (ds.fArrow && ds.fArrowFree && !dr.fInside))
#define kvSet (dr.fSet ? dr.kvOn : dr.kvOff)
#define SetMacroReturn(l) \
  if (FEnsureLVar(cLetter + 1)) ws.rglVar[iLetterZ] = (l)
#define LVar(i) ((i) >= ws.clVar ? 0 : ws.rglVar[i])
#define SzVar(i) ((i) >= ws.cszVar ? NULL : ws.rgszVar[i])
#define FSzVar(i) ((i) >= 0 && (i) < ws.cszVar && ws.rgszVar[i] != NULL)


/*
******************************************************************************
** Structures
******************************************************************************
*/

typedef struct _windowstate {
  // Window settings

  flag fAbort;
  short xpClient;
  short ypClient;
  short xScroll;
  short yScroll;
  size_t lTimer;

  // File dialog settings

  int xCutoff;
  int yCutoff;
  flag fTextClip;
  flag fLineChar;
  flag fTextTab;

  // Event dialog settings

  int iEventEdge;
  int iEventWall;
  int iEventMove;
  int iEventMove2;
  int iEventFollow;
  int iEventInside;
  int iEventInside2;
  int iEventCommand;
  int iEventLeft;
  int iEventRight;
  int iEventMiddle;
  int iEventPrev;
  int iEventNext;
  int iEventMouse;
  int iEventQuit;

  // Redraw dialog settings

  flag fNoDirty;
  flag fAllowUpdate;
  flag fHourglass;
  flag fResizeClient;
  flag fWindowTop;
  flag fWindowMenu;
  flag fWindowScroll;
  int nDisplayDelay;
  int nTimerDelay;
  flag fSound;

  // Menu settings

  flag fWindowFull;
  flag fIgnorePrint;
  flag fTimePause;
  flag fSpree;

  // Macro accessible only settings

  int nIgnorePrint;
  int nTitleMessage;
  flag fCopyMessage;
  flag fNoExit;
  int nSoundDelay;
  int nFileLock;

  // Internal settings

  char *szAppName;
  char *szFileTemp;
  flag fSlowCopy;
  flag fStarting;
  flag fQuitting;
  flag fSaverRun;    // /s: Run screen saver full screen
  flag fSaverConfig; // /c or /a: Configure screen saver in window
  flag fSaverMouse;  // Mouse moved in screen saver mode, so exit
  flag fNoWindow;    // /w: Run in windowless mode
  flag fSkipSystem;
  flag fMenuDirty;
  ulong timeReset;
  ulong timePause;
  int wCmdRepeat;
  int wCmdSpree;
  int nKey;
  flag fInRedraw;
  flag fInSpree;
  CONST char *szTitle;
  char *szDialog;
  char *szDialog2;
  char *szDialog3;
  int iszDialog;
  int cDialog;
  int xMouse;
  int yMouse;
  int nFontSize;
  int nFontWeight;
  CONST char **rgszStartup;
  int iszStartup;
  int ichStartup;
  int nMacroDepth;
  int nMacroDepthMax;
  flag fReturning;
  byte *rgbCmdMacro;
  char **rgszMacro;
  int cszMacro;
  long *rglVar;
  int clVar;
  long *rglLocal;
  char **rgszVar;
  int cszVar;
  CMaz *rgbMask;
  int cbMask;
  CMazK *rgcTexture;
  int ccTexture;
  TRIE rgsTrieAlloc;
  TRIE rgsTrieCmd;
  TRIE rgsTrieAbbrev;
  TRIE rgsTrieOpr;
  TRIE rgsTrieVar;
  TRIE rgsTrieFun;
  TRIE rgsTrieKv;
  TRIE rgsTrieConst;
  int *rgnTrieConst;
  KV rgkv[cColorMain];
  char szFontName[cchSzDef];
  char szFileName[cchSzMaxFile];
  char szFileTitle[cchSzMaxFile];
} WS;

typedef struct _bitmap {
  int xa;
  int ya;
  int xaC;
  int yaC;
  int xaI;
  int yaI;
  int nWhat;
  int nHow;
  int xCell;
  int yCell;
  COOR *coor;
  long ccoor;
  PATCH *patch;
  long cpatch;
  flag fColor;
  flag fDrewDot;
  int xpOrigin;
  int ypOrigin;
  int xpDot;
  int ypDot;
  int xpPoint;
  int ypPoint;
  int xaOrigin;
  int yaOrigin;
  int xaView;
  int yaView;
  CMaz b;   // Main      monochrome bitmap
  CMaz b2;  // Temporary monochrome bitmap
  CMaz b3;  // Extra     monochrome bitmap
  CMazK k;  // Main      color bitmap
  CMazK k2; // Temporary color bitmap
  CMazK k3; // Extra     color bitmap
  CMazK kI; // Inside    color bitmap
  CMazK kR; // Rainbow   color bitmap
  CMazK kS; // Stereo    color bitmap
} BM;

typedef struct _insidecalc {
  real rAngleColumn; // The angle of this pixel column
  real rDistCorrect; // The height correction factor for this column
} CALC;

// Information about a semitransparent or variable height wall encountered.
typedef struct _transparentwall {
  KV kv;         // Color of the wall, including shading
  KV kvOrig;     // Color of the wall, not including shading
  int y;         // Screen height of front of default sized wall
  int y2;        // Screen height of back of default wall, for Very Thorough
  int yk;        // Distance of front of default sized wall
  int yk2;       // Distance of back of default wall, for Very Thorough
  flag fTrans;   // Whether this wall is semitransparent
  flag fSkip;    // Whether this wall is concealed so doesn't need drawing
  int yLo;       // Inside unit height of bottom of wall
  int yHi;       // Inside unit height of top of wall
  int yLo1;      // Scaled pixel height of bottom of front wall
  int yHi1;      // Scaled pixel height of top of front wall
  int yLo2;      // Scaled pixel height of bottom of back wall
  int yHi2;      // Scaled pixel height of top of back wall
  int yElev;     // Pixel height of ground elevation clipping
  int yElev2;    // Pixel height of ceiling elevation clipping
  int nFog;      // Fog percentage at depth of this wall
  int iTexture;  // What color texture to apply to this wall, if any
  int iMask;     // What monochrome texture to apply to this wall, if any
  real rTexture; // The horizontal fraction within the texture to draw
  flag fMaskSet; // Whether monochrome wall texture should be drawn as white

  int iTextureVar;  // What color texture to apply to top of block, if any
  int iMaskVar;     // What monochrome texture to apply to top of block
  int iTextureVar2; // What color texture to apply to bottom of block, if any
  int iMaskVar2;    // What monochrome texture to apply to bottom of block
  real rx1;         // Starting X coordinate for block top texture
  real ry1;         // Starting Y coordinate for block top texture
  real rx2;         // Ending X coordinate for block top texture
  real ry2;         // Ending Y coordinate for block top texture
  real rx3;         // Starting X coordinate for block bottom texture
  real ry3;         // Starting Y coordinate for block bottom texture
  real rx4;         // Ending X coordinate for block bottom texture
  real ry4;         // Ending Y coordinate for block bottom texture
  flag fMaskSetVar; // Whether monochrome block texture should be drawn white
} TRANS;

typedef struct _dotrendering {
  // Dot dialog settings - Location

  int x;
  int y;
  int z;
  int dir;
  int x2;
  int y2;
  int z2;
  int dir2;

  // Dot dialog settings - Motion

  flag fDot;
  flag fCircle;
  flag fArrow;
  int nDotSize;
  flag fWall;
  flag fNoCorner;
  flag fGlance;
  flag fFollowMouse;
  flag fNoDiagonal;
  flag fFollow;
  long cMove;

  // Dot dialog settings - Editing

  flag fTwo;
  flag fDrag;
  flag fSet;
  flag fBig;
  flag fBoth;
  flag fOnTemp;
  int nEdge;

  // Inside dialog Settings - Mode & What To Draw

  int nInside;
  flag fCompass;
  flag fCompassAngle;
  flag fLocation;
  flag fMap;
  flag fSealEntrance;
  flag fMountain;
  int nPeakHeight;
  int cCloud;
  flag fSunMoon;
  //flag fStar;  // Stored in DS structure
  //int cStar;   // Stored in DS structure
  int nMeteor;

  // Inside dialog Settings - Walls

  flag fNarrow;
  int zCell;
  int zCellNarrow;
  int zWall;
  int zElev;
  int zStep;
  flag fTexture;
  real rLightFactor;
  int nFog;
  int nClip;
  real dInside;
  //int nStereo;  // Stored in DS structure
  int nTrans;

  // Inside dialog Settings - Smooth & Free Movement

  int nFrameXY;
  int nFrameD;
  int nFrameZ;
  int nSpeedXY;
  int nSpeedD;
  int nSpeedZ;
  int nOffsetX;
  int nOffsetY;
  int nOffsetZ;
  int nOffsetD;
  flag f3DNotFree;

  // Color dialog settings

  KV kvOn;
  KV kvOff;
  KV kvEdge;
  KV kvDot;
  KV kvMap;
  KV kvInWall;
  KV kvInSky;
  KV kvInDirt;
  KV kvInEdge;
  KV kvIn3D;
  KV kvInCeil;
  KV kvInMtn;
  KV kvInCld;
  KV kvInFog;
  flag fSpreeRandom;

  // Menu settings

  flag fInside;
  flag f3D;
  flag fPolishMaze;

  // Macro accessible only settings

  flag fDot2;
  flag fNoMouseDrag;
  flag fFogFloor;
  KV kvDot2;
  KV kvSun;
  KV kvMoon;
  KV kvInWall2;
  KV kvInSky2;
  KV kvInDirt2;
  KV kvInCeil2;
  KV kvInMtn2;
  KV kvInCld2;
  flag fRainbow;
  int nRainbow;
  int zGround;
  int yHorizon;
  int nSunMoon;
  int ySunMoon;
  //long lStarColor;  // Stored in DS structure
  //flag fSkyAll;     // Stored in DS structure
  flag fSky3D;
  flag fDelay;
  int nDelay;
  flag fNoCompass;
  flag fNoLocation;
  flag fNoSubmerge;
  flag fRedrawAfter;
  flag fMarkColor;
  flag fMarkAll;
  flag fMarkBlock;
  int nMarkSky;
  int nMarkElev;
  int nWallVar;
  int nFogLit;
  int zStep2;
  int nTexture;
  int nTexture2;
  int nTextureWall;
  int nTextureDirt;
  int nTextureVar;
  int nTextureElev;
  flag fTextureDual;
  flag fTextureDual2;
  flag fTextureBlend;
  int nMarkElevX1;
  int nMarkElevY1;
  int nMarkElevX2;
  int nMarkElevY2;
  int nStretchMode;

  // Internal settings

  flag fInSmooth;
  int xCalc;
  real rdCalc;
  CALC *rgcalc;
  int ccalc;
  STAR *rgstar;
  short *rgmtn;
  short *rgcld;
  TRANS *rgtrans;
  int nTransPct;
  int nTransPct2;
  int yElevMax;
  int yElevMin;
} DR;


typedef struct _command {
  int wCmd;
  char *szName;
  char *szAbbrev;
  long grf;
} CMD;

typedef struct _operation {
  int iopr;
  char *szName;
  int nParam;
  long grf;
} OPR;

typedef struct _variable {
  int ivar;
  char *szName;
  long grf;
} VAR;

typedef struct _function {
  int ifun;
  char *szName;
  int nParam;
} FUN;


/*
******************************************************************************
** Externs
******************************************************************************
*/

// Global variables

extern WS ws;
extern BM bm;
extern DR dr;
extern CONST CMD rgcmd[ccmd];
extern CONST OPR rgopr[copr];
extern CONST VAR rgvar[cvar];
extern CONST FUN rgfun[cfun];
extern CONST int rgcmdMouse[11];
extern CONST PT rgptSize[cmdSizeLast - cmdSize01 + 1];
extern CONST char *rgszScript[cmdScriptLast - cmdScript01 + 1];
extern CONST char *rgszShortcut[cmdScriptLast - cmdScript01 + 1];
extern CONST char *rgszStartup[];

// From daedalus.cpp

flag FReadBitmap(FILE *, flag);
flag FReadDaedalusBitmap(FILE *);
flag FReadFile(int, FILE *, flag);
flag FWriteFile(CONST CMaz &, CONST CMazK &, int, CONST char *, CONST char *);
flag FShowColmap(flag);
void DoSize(int, int, flag, flag);
flag DoDefineMacro(int, CONST char *, int, CONST char *);
flag FEnsureMacro(int);
flag FEnsureLVar(int);
flag FEnsureSzVar(int);
flag SetSzVar(CONST char *, int, int);
CMaz *BitmapGetMask(int);
CMazK *ColmapGetTexture(int);
void DeallocateTextures(void);
flag FTextureBackground(void);
flag FTextureWall(int, bit);
flag FTextureGround(flag, int, bit);
flag FTextureCeiling(void);
flag FWallVariable(int);
flag FGroundVariable(void);
flag FGroundVariable2(void);
void FormatRgchToSz(CONST char *, int, char *, int);
void DotTemp(int, int, int);
void DotZap(int, int);
void PolishMaze(int, int);
flag FCheckEvent(int, int, int, int);
void CheckEventMove(int, int, int, int, int, flag);
flag FCheckEventClick(int, long);
void MouseClickInside(int, int);
int NDistance(int, int, int);
flag FGetWallVar(int, int, int, int *, int *, int *);
void CenterWithinCell(void);
flag FHitWall(int, int, int, int *);
flag FUndoIfHitWall(int, int, int, int, int, int, CONST CMap &);
void DotMove(int);
void DotJump(int);
void DotSmoothAnimate(int, int, int, int, int);
void DotMouseChase(int, int);
flag FCreateConstTrie(CONST char *, int);

// From command.cpp

char *PchGetParameter(char *, char **, int *, long *, int);
int DoCommand(int);
int RunCommandLine(char *, FILE *);
void RunCommandLines(CONST char *rgsz[]);
int RunMacro(int);
flag FReadScript(FILE *);
char *ReadEmbedLines(FILE *);

// From inside.cpp

int AFromInside(long);
long InsideFromA(int);
flag FCreateInsideStars(CMazK *, real, int, flag);
flag FMoveCloud(int, int);
void ResetInside();
void FormatCompass(char *, flag, int *);
void FormatLocation(char *, flag);
void FormatTimer(char *, ulong, ulong);
flag RedrawInsidePerspective(CMazK &);
flag RedrawInsidePerspectiveStereo(CMazK &, CMazK &);
void RedrawInside(CMazK &);
void RedrawInside3(CMazK &);
void DrawOverlay(CMazK &, CMaz &);

// From game.cpp

flag FHungerGame(int);

// Function hooks from wutil.cpp

void BitmapDotCore(int, int, bit);
void BitmapDot(int, int);
flag FBootExternal(CONST char *, flag);
void EnsureTimer(int);
ulong GetTimer(void);
void PauseTimer(flag);
void DoSetVariableW(int, int);
int NGetVariableW(int);
void DoOperationW(int, CONST char *, int, int, int);
void SystemHook(int);
void HourglassCursor(size_t *, flag);

// Function hooks from wdialog.cpp

flag FFileOpen(int, CONST char *, FILE *);
flag FFileSave(int, CONST char *);

// Function hooks from wdriver.cpp

flag DoCommandW(int);

/* daedalus.h */
