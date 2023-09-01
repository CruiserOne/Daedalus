/*
** Daedalus (Version 3.4) File: command.cpp
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
** This file contains tables of all commands and actions, and the code to
** execute macros and script files.
**
** Created: 11/26/2001.
** Last code change: 8/29/2023.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "resource.h"
#include "util.h"
#include "graphics.h"
#include "color.h"
#include "threed.h"
#include "maze.h"
#include "draw.h"
#include "daedalus.h"


/*
******************************************************************************
** Action Tables
******************************************************************************
*/

#define R1 fCmtRedraw
#define R2 fCmtDirtyView
#define HG fCmtHourglass
#define B1 fCmtBitmapBefore
#define B2 fCmtBitmapAfter
#define C1 fCmtColmapBefore
#define C2 fCmtColmapAfter
#define NT fCmtRequireTemp
#define NX fCmtRequireExtra
#define NC fCmtRequireColor
#define SP fCmtSpree
#define M2 fCmtSet2D
#define M3 fCmtSet3D
#define MZ fCmtMaze | R2 | HG | B2 | SP
#define CZ fCmtMaze | R2 | HG | SP
#define ME fCmtMazeEntrance
#define MC fCmtMazeCenter
#define SV fCmtSolve | R2 | HG
#define NK NT | NC
#define SZ fCmtString


// Commands

CONST CMD rgcmd[ccmd] = {
{cmd2nd,                   "2ndSet",          "g",    R1},
{cmd2ndAnd,                "2ndAnd",          "",     R2 | HG |      NT},
{cmd2ndBlock,              "2ndBlock",        "",     R2 | HG},
{cmd2ndBox,                "2ndBox",          "",     R2},
{cmd2ndCircle,             "2ndCircle",       "",     R2},
{cmd2ndDisk,               "2ndDisk",         "",     R2 | HG},
{cmd2ndGet,                "2ndGet",          "ag",        HG},
{cmd2ndLine,               "2ndLine",         "",     R2},
{cmd2ndOr,                 "2ndOr",           "",     R2 | HG |      NT},
{cmd2ndPut,                "2ndPut",          "aG",   R2 | HG |      NT},
{cmd2ndXor,                "2ndXor",          "",     R2 | HG |      NT},
{cmd3D,                    "3DBitmap",        "j",    R1},
{cmdAbout,                 "About",           "a0",   0},
{cmdAccentBoundary,        "Accent",          "",     R2 | HG},
{cmdAddDetachment,         "AddPassage",      "<",    R2 |      B1 | SP},
{cmdAddIn,                 "AddEntrance",     "",     R2 |      B1 | SP},
{cmdAddIsolation,          "AddWall",         ">",    R2 |      B1 | SP},
{cmdAddOut,                "AddExit",         "",     R2 |      B1 | SP},
{cmdAddX,                  "InsCol",          "",     R2 | HG},
{cmdAddX2,                 "InsCols",         "",     R2 | HG},
{cmdAddY,                  "InsRow",          "",     R2 | HG},
{cmdAddY2,                 "InsRows",         "",     R2 | HG},
{cmdAllClear,              "AllClear",        "Del",  R2},
{cmdAllInvert,             "AllInvert",       "cDel", R2 |      SP},
{cmdAllRandom,             "AllRandom",       "",     R2 | HG |      SP},
{cmdAllSet,                "AllSet",          "sDel", R2},
{cmdAnalyze,               "AnalyzePassage",  "ap",        HG | B1},
{cmdAnalyze2,              "AnalyzeWall",     "aP",        HG | B1 | M2},
{cmdBias,                  "DlgBias",         "az",   0},
{cmdCellDec1,              "Viewport-1",      "{",    R1},
{cmdCellDec10,             "Viewport-10",     "[",    R1},
{cmdCellInc1,              "Viewport+1",      "}",    R1},
{cmdCellInc10,             "Viewport+10",     "]",    R1},
{cmdClarify3D,             "Clarify3D",       "",     R2 | HG},
{cmdClarify4D,             "Clarify4D",       "",     R2 | HG},
{cmdClarifyWeave,          "ClarifyWeave",    "",     R2 | HG | M2},
{cmdCollapse,              "CollapseToSet",   "",     R2 | HG},
{cmdColmap,                "ShowColor",       "k",    0},
{cmdColmapAnd,             "ColorAnd",        "",     R2 | HG | C1 | NC},
{cmdColmapAntialias2,      "Antialias2",      "",     R2 | HG | C2},
{cmdColmapAntialias3,      "Antialias3",      "",     R2 | HG | C2},
{cmdColmapAntialias4,      "Antialias4",      "",     R2 | HG | C2},
{cmdColmapAntialias5,      "Antialias5",      "",     R2 | HG | C2},
{cmdColmapAntialias6,      "Antialias6",      "",     R2 | HG | C2},
{cmdColmapAntialias7,      "Antialias7",      "",     R2 | HG | C2},
{cmdColmapAntialias8,      "Antialias8",      "",     R2 | HG | C2},
{cmdColmapBlend,           "Blend2",          "",     R2 | HG | C2},
{cmdColmapBlend2,          "Blend3",          "",     R2 | HG | C2},
{cmdColmapBrightness,      "DlgBrightness",   "aK",   R2 |      C1},
{cmdColmapDel,             "ColorDelete",     "",     R1 |      B1},
{cmdColmapDistance,        "GraphDistance",   "",     R2 | HG | C2},
{cmdColmapDistance2,       "GraphWalls",      "",     R2 | HG | C2 | M2},
{cmdColmapDotAdd,          "ColorDotAdd",     "",     R2 | HG | C1},
{cmdColmapDotAnd,          "ColorDotAnd",     "",     R2 | HG | C1},
{cmdColmapDotBlend,        "ColorDotBlend",   "",     R2 | HG | C1},
{cmdColmapDotMax,          "ColorDotMax",     "",     R2 | HG | C1},
{cmdColmapDotMin,          "ColorDotMin",     "",     R2 | HG | C1},
{cmdColmapDotMul,          "ColorDotMul",     "",     R2 | HG | C1},
{cmdColmapDotOr,           "ColorDotOr",      "",     R2 | HG | C1},
{cmdColmapDotSet,          "ColorDotSet",     "",     R2 |      C1},
{cmdColmapDotSub,          "ColorDotSub",     "",     R2 | HG | C1},
{cmdColmapDotXor,          "ColorDotXor",     "",     R2 | HG | C1},
{cmdColmapGet,             "ColorGet",        "Quo",  R2 | HG | C2},
{cmdColmapMono,            "ColorGrayscale",  "",     R2 | HG | C1 | NC},
{cmdColmapOr,              "ColorOr",         "",     R2 | HG | C1 | NC},
{cmdColmapPut,             "ColorPut",        "",     R2 | HG | B2 | NC},
{cmdColmapPutDither,       "ColorDither",     "",     R2 | HG | B2 | NC},
{cmdColmapPutNearest,      "ColorNearest",    "",     R2 | HG | B2 | NC},
{cmdColmapReplace,         "DlgReplace",      "ak",   R2 | HG | C1},
{cmdColmapXor,             "ColorXor",        "",     R2 | HG | C1 | NC},
{cmdCommand,               "DlgCommand",      "Ent",  0},
{cmdCopyBitmap,            "CopyBitmap",      "cc",        HG},
{cmdCopyPicture,           "CopyPicture",     "",          HG},
{cmdCopyText,              "CopyText",        "",          HG},
{cmdCorner,                "SmoothCorner",    "",     R2 | HG | B1},
{cmdCount,                 "CountPixels",     "ac",        HG},
{cmdCountPossible,         "CountPossible",   "",                    M2},
{cmdCrackBlindAlley,       "NoBlindAlley",    "",     R2 | HG | B1},
{cmdCrackBlock,            "NoDeadEnd",       "",     R2 | HG | B1},
{cmdCrackBoundary,         "NoBoundary",      "",     R2 | HG | B1},
{cmdCrackCell,             "NoCell",          "",     R2 | HG | B1},
{cmdCrackDetach,           "NoDetachment",    "",     R2 | HG | B1 | M2},
{cmdCrackEntrance,         "NoEntrance",      "",     R2 | HG | B1 | M2},
{cmdCrackIsolation,        "NoIsolation",     "",     R2 | HG | B1 | M2},
{cmdCrackPassage,          "NoPassage",       "",     R2 | HG | B1},
{cmdCrackPole,             "ConnectPole",     "",     R2 | HG | B1},
{cmdCrackRoom,             "NoRoom",          "",     R2 | HG | B1},
{cmdCrackTube,             "NoTube",          "",     R2 | HG | B1},
{cmdCreate3D,              "3D",              "Q",    MZ | ME | M3},
{cmdCreate4D,              "4D",              "R",    MZ | ME | M2},
{cmdCreateAldous,          "AldousBroder",    "aa",   MZ | ME | M2},
{cmdCreateArrow,           "Arrow",           "A",    CZ | ME | M2},
{cmdCreateBinary,          "Binary",          "ay",   MZ | ME},
{cmdCreateBraid,           "Braid",           "B",    MZ | ME | M2},
{cmdCreateCavern,          "Cavern",          "av",   MZ | ME | M2},
{cmdCreateCrack,           "Crack",           "K",    MZ | ME | M2},
{cmdCreateDelta,           "Delta",           "Y",    MZ | ME | M2},
{cmdCreateDiagonal,        "Diagonal",        "D",    MZ | ME | M2},
{cmdCreateDivision,        "Division",        "aj",   MZ | ME},
{cmdCreateEller,           "Eller",           "ae",   MZ | ME | M2},
{cmdCreateForest,          "Forest",          "aT",   MZ | ME | M2},
{cmdCreateFractal,         "Fractal",         "O",    MZ | ME},
{cmdCreateFractal2,        "Fractal2",        "ax",   CZ | ME | M2},
{cmdCreateGamma,           "Gamma",           "aq",   MZ | ME | M2},
{cmdCreateHilbert,         "Hilbert",         "aH",   MZ},
{cmdCreateHyper,           "Hyper",           "ah",   MZ |      M3},
{cmdCreateInfinite0,       "InfiniteStart",   "ai",   MZ | ME | M2},
{cmdCreateInfinite1,       "Infinite",        "I",    MZ      | M2},
{cmdCreateInfinite2,       "InfiniteEnd",     "aI",   MZ      | M2},
{cmdCreateInfinite3,       "InfiniteRestart", "",     MZ      | M2},
{cmdCreateKruskal,         "Kruskal",         "al",   MZ | ME | M2},
{cmdCreateLabyrinth,       "Labyrinth",       "C",    MZ | ME | M2},
{cmdCreateOmicron,         "Omicron",         "",     MZ | ME | M2},
{cmdCreatePatchOverview,   "MakePatch",       "",     HG},
{cmdCreatePerfect,         "Perfect",         "P",    MZ | ME | M2},
{cmdCreatePlanair,         "Planair",         "J",    MZ      | M2},
{cmdCreatePrim,            "Prim",            "am",   MZ | ME | M2},
{cmdCreatePrim2,           "Prim2",           "aM",   MZ | ME | M2},
{cmdCreateRandom,          "Random",          "an",   MZ | ME | M2},
{cmdCreateRecursive,       "Recursive",       "ab",   MZ | ME | M2},
{cmdCreateSegment,         "Segment",         "L",    MZ | ME | M2},
{cmdCreateSidewinder,      "Sidewinder",      "as",   MZ | ME},
{cmdCreateSigma,           "Sigma",           "X",    MZ | ME | M2},
{cmdCreateSpiral,          "Spiral",          "S",    MZ | ME | M2},
{cmdCreateTheta,           "Theta",           "W",    MZ | ME | M2},
{cmdCreateTilt,            "Tilt",            "at",   MZ      | M2},
{cmdCreateTree,            "Tree",            "T",    MZ | ME | M2},
{cmdCreateUnicursal,       "Unicursal",       "U",    MZ | ME | M2},
{cmdCreateUpsilon,         "Upsilon",         "au",   MZ | ME | M2},
{cmdCreateWeave,           "Weave",           "V",    MZ | ME | M2},
{cmdCreateWilson,          "Wilson",          "aw",   MZ | ME | M2},
{cmdCreateWireOverview,    "MakeWire",        "",     HG},
{cmdCreateZeta,            "Zeta",            "Z",    MZ | ME | M2},
{cmdCubemapFlip,           "DlgFlip3D",       "",               M3},
{cmdDeletePole,            "NoPole",          "",     R2 | HG | B1},
{cmdDelX,                  "DelCol",          "",     R2 | HG},
{cmdDelX2,                 "DelCols",         "",     R2 | HG},
{cmdDelY,                  "DelRow",          "",     R2 | HG},
{cmdDelY2,                 "DelRows",         "",     R2 | HG},
{cmdDlgColor,              "DlgColors",       "ck",   0},
{cmdDlgCreate,             "DlgCreate",       "M",    0},
{cmdDlgDisplay,            "DlgDisplay",      "Tab",  0},
{cmdDlgDot,                "DlgDot",          "v",    0},
{cmdDlgDraw,               "DlgDraw",         "ad",   0},
{cmdDlgDraw2,              "DlgDraw2",        "aD",   0},
{cmdDlgEvent,              "DlgEvent",        "",     0},
{cmdDlgFile,               "DlgFile",         "af",   0},
{cmdDlgInside,             "DlgInside",       "q",    0},
{cmdDlgLabyrinth,          "DlgLabyrinth",    "cl",   0},
{cmdDlgMaze,               "DlgMaze",         "cm",   0},
{cmdDlgRandom,             "DlgRandom",       "ar",   0},
{cmdDlgSize,               "DlgSize",         "s",    0},
{cmdDlgSizeMaze,           "DlgMazeSize",     "m",    0},
{cmdDlgZoom,               "DlgZoom",         "z",    0},
{cmdDotBig,                "DotBig",          "b",    0},
{cmdDotBoth,               "DotDragTwo",      "f",    0},
{cmdDotDrag,               "DotDrag",         "a",    0},
{cmdDotOn,                 "DotErase",        "c",    0},
{cmdDotSet,                "DotSet",          "h",    R1},
{cmdDotShow,               "DotShow",         ".",    R1           | SP},
{cmdDotTwo,                "DotTwo",          "e",    0},
{cmdDotWall,               "DotWall",         ",",    0},
{cmdDotZap,                "ZapWall",         "cz",   R1 |      B1 | SP},
{cmdDotZapTrans,           "ZapTrans",        "cx",   R1 |           SP},
{cmdDotZapUntrans,         "ZapUntrans",      "cy",   R1 |           SP},
{cmdDrawDel,               "ExtraDelete",     "",     R1},
{cmdDrawGet,               "ExtraGet",        "\\",   R1},
{cmdDrawPut,               "ExtraPut",        "|",    R2 |           NX},
{cmdDrawSwap,              "ExtraSwap",       "-",    R2 |           NX | SP},
{cmdEraseCorner,           "NoBottomRight",   "'",    R2 |      B1 | SP},
{cmdExit,                  "Exit",            "Esc",  0},
{cmdExpandSet,             "ExpandSet",       "",     R2 | HG | B1 | M2},
{cmdFlipX,                 "FlipHorizontal",  "",     R2 | HG |      M2},
{cmdFlipY,                 "FlipVertical",    "",     R2 | HG |      M2},
{cmdFlipZ,                 "Transpose",       "",     R2 | HG |      M2},
{cmdFloodDot,              "Fill",            "`",    R2 | HG},
{cmdFloodDot2,             "Flood",           "~",    R2 | HG},
{cmdFloodPassage,          "FloodPassage",    "",     SV | M2},
{cmdFloodWallL,            "FloodWallLeft",   "",     SV | M2},
{cmdFloodWallR,            "FloodWallRight",  "",     SV | M2},
{cmdGo2nd,                 "Go2nd",           "cg",   R1 | MC},
{cmdGoCorner1,             "GoLL",            "c1",   R1 | MC},
{cmdGoCorner3,             "GoLR",            "c3",   R1 | MC},
{cmdGoCorner7,             "GoUL",            "c7",   R1 | MC},
{cmdGoCorner9,             "GoUR",            "c9",   R1 | MC},
{cmdGoIn,                  "GoEntrance",      "c8",   R1 | MC},
{cmdGoInY,                 "GoEntrance2",     "c4",   R1 | MC},
{cmdGoMiddle,              "GoMiddle",        "c5",   R1 | MC},
{cmdGoOut,                 "GoExit",          "c2",   R1 | MC},
{cmdGoOutY,                "GoExit2",         "c6",   R1 | MC},
{cmdGoRandom,              "GoRandom",        "c0",   R1 | MC | SP},
{cmdGroundVariable,        "GroundVariable",  "",     R2 | HG |      M2},
{cmdInsideMap,             "InsideMap",       ":",    R1},
{cmdInsideView,            "Inside",          "i",    R1},
{cmdJump1,                 "JumpDL",          "!",    SP},
{cmdJump2,                 "JumpD",           "@",    SP},
{cmdJump3,                 "JumpDR",          "#",    SP},
{cmdJump4,                 "JumpL",           "$",    SP},
{cmdJump6,                 "JumpR",           "^",    SP},
{cmdJump7,                 "JumpUL",          "&",    SP},
{cmdJump8,                 "JumpU",           "*",    SP},
{cmdJump9,                 "JumpUR",          "(",    SP},
{cmdLabyrinthChartres,     "Chartres",        "",     MZ | ME | M2},
{cmdLabyrinthChartresF,    "FlatChartres",    "",     MZ | ME | M2},
{cmdLabyrinthChartresR,    "ChartresReplica", "",     MZ | ME | M2},
{cmdLabyrinthClassical,    "Classical",       "",     MZ | ME | M2},
{cmdLabyrinthClassicalF,   "FlatClassical",   "",     MZ | ME | M2},
{cmdLabyrinthCretan,       "Cretan",          "",     MZ | ME | M2},
{cmdLabyrinthCustom,       "Custom",          "",     MZ | ME | M2},
{cmdLabyrinthMan,          "ManInTheMaze",    "",     MZ | ME | M2},
{cmdLife,                  "Life",            "aL",   R2 | HG | SP | M2},
{cmdMacro01,               "Macro1",          "F1",   SP},
{cmdMacro02,               "Macro2",          "F2",   SP},
{cmdMacro03,               "Macro3",          "F3",   SP},
{cmdMacro04,               "Macro4",          "F4",   SP},
{cmdMacro05,               "Macro5",          "F5",   SP},
{cmdMacro06,               "Macro6",          "F6",   SP},
{cmdMacro07,               "Macro7",          "F7",   SP},
{cmdMacro08,               "Macro8",          "F8",   SP},
{cmdMacro09,               "Macro9",          "F9",   SP},
{cmdMacro10,               "Macro10",         "F10",  SP},
{cmdMacro11,               "Macro11",         "F11",  SP},
{cmdMacro12,               "Macro12",         "F12",  SP},
{cmdMacro13,               "Macro13",         "sF1",  SP},
{cmdMacro14,               "Macro14",         "sF2",  SP},
{cmdMacro15,               "Macro15",         "sF3",  SP},
{cmdMacro16,               "Macro16",         "sF4",  SP},
{cmdMacro17,               "Macro17",         "sF5",  SP},
{cmdMacro18,               "Macro18",         "sF6",  SP},
{cmdMacro19,               "Macro19",         "sF7",  SP},
{cmdMacro20,               "Macro20",         "sF8",  SP},
{cmdMacro21,               "Macro21",         "sF9",  SP},
{cmdMacro22,               "Macro22",         "sF10", SP},
{cmdMacro23,               "Macro23",         "sF11", SP},
{cmdMacro24,               "Macro24",         "sF12", SP},
{cmdMacro25,               "Macro25",         "cF1",  SP},
{cmdMacro26,               "Macro26",         "cF2",  SP},
{cmdMacro27,               "Macro27",         "cF3",  SP},
{cmdMacro28,               "Macro28",         "cF4",  SP},
{cmdMacro29,               "Macro29",         "cF5",  SP},
{cmdMacro30,               "Macro30",         "cF6",  SP},
{cmdMacro31,               "Macro31",         "cF7",  SP},
{cmdMacro32,               "Macro32",         "cF8",  SP},
{cmdMacro33,               "Macro33",         "cF9",  SP},
{cmdMacro34,               "Macro34",         "cF10", SP},
{cmdMacro35,               "Macro35",         "cF11", SP},
{cmdMacro36,               "Macro36",         "cF12", SP},
{cmdMacro37,               "Macro37",         "aF1",  SP},
{cmdMacro38,               "Macro38",         "aF2",  SP},
{cmdMacro39,               "Macro39",         "aF3",  SP},
{cmdMacro40,               "Macro40",         "aF4",  SP},
{cmdMacro41,               "Macro41",         "aF5",  SP},
{cmdMacro42,               "Macro42",         "aF6",  SP},
{cmdMacro43,               "Macro43",         "aF7",  SP},
{cmdMacro44,               "Macro44",         "aF8",  SP},
{cmdMacro45,               "Macro45",         "aF9",  SP},
{cmdMacro46,               "Macro46",         "aF10", SP},
{cmdMacro47,               "Macro47",         "aF11", SP},
{cmdMacro48,               "Macro48",         "aF12", SP},
{cmdMark,                  "MarkX",           "x",    R1},
{cmdMarkBlindAlley,        "MarkBlindAlley",  "",     SV | M2},
{cmdMarkBlock,             "MarkDeadEnd",     "G",    SV | SP | M2},
{cmdMarkBlock2,            "FillDeadEnd",     "",     SV | SP | M2},
{cmdMarkCollision,         "MarkCollision",   "",     SV | SP | M2},
{cmdMarkCuldsac,           "MarkCulDeSac",    "",     SV | SP | M2},
{cmdMessage,               "IgnoreMessage",   "?",    0},
{cmdMove1,                 "MoveDL",          "1",    SP},
{cmdMove2,                 "MoveD",           "2",    SP},
{cmdMove3,                 "MoveDR",          "3",    SP},
{cmdMove4,                 "MoveL",           "4",    SP},
{cmdMove6,                 "MoveR",           "6",    SP},
{cmdMove7,                 "MoveUL",          "7",    SP},
{cmdMove8,                 "MoveU",           "8",    SP},
{cmdMove9,                 "MoveUR",          "9",    SP},
{cmdMoveAround,            "Around",          "ca",   SP},
{cmdMoveBack,              "MoveBack",        "cb",   SP},
{cmdMoveD,                 "MoveLower",       "d",    SP},
{cmdMoveEast,              "MoveEast",        "ce",   SP},
{cmdMoveForward,           "MoveForward",     "cf",   SP},
{cmdMoveLeft,              "Left",            "l",    SP},
{cmdMoveNorth,             "MoveNorth",       "cn",   SP},
{cmdMoveRandom,            "MoveRandom",      "0",    SP},
{cmdMoveRight,             "Right",           "r",    SP},
{cmdMoveSouth,             "MoveSouth",       "cs",   SP},
{cmdMoveU,                 "MoveRaise",       "u",    SP},
{cmdMoveWest,              "MoveWest",        "cw",   SP},
{cmdNormalize,             "NormalizePassage","",     R2 | HG | B1},
{cmdNormalize2,            "NormalizeWall",   "",     R2 | HG | B1},
{cmdOpen,                  "DlgOpen",         "o",    R2},
{cmdOpen3D,                "DlgOpen3D",       "",     R2 |      B2 | M3},
{cmdOpenBitmap,            "DlgOpenBitmap",   "ao",   R2},
{cmdOpenColmapTarga,       "DlgOpenTarga",    "",     R2 |      C2},
{cmdOpenPatch,             "DlgOpenPatch",    "",     0},
{cmdOpenScript,            "DlgOpenScript",   "co",   0},
{cmdOpenText,              "DlgOpenText",     "",     R2 |      B2},
{cmdOpenWire,              "DlgOpenWire",     "",     0},
{cmdOpenXbm,               "DlgOpenX11",      "",     R2 |      B2},
{cmdPartBraid,             "PartialBraid",    "aB",   R2 | HG | B1 | M2},
{cmdPartCavern,            "PartialCavern",   "",     R2 | HG | B1 | M2},
{cmdPartCrack,             "PartialCrack",    "cK",   R2 | HG | B1 | M2},
{cmdPartKruskal,           "PartialKruskal",  "",     R2 | HG | B1 | M2},
{cmdPartPerfect,           "PartialPerfect",  "cP",   R2 | HG | B1 | M2},
{cmdPartPrim,              "PartialPrim",     "",     R2 | HG | B1 | M2},
{cmdPartRecursive,         "PartialRecursive","",     R2 | HG | B1 | M2},
{cmdPartTree,              "PartialTree",     "",     R2 | HG | B1 | M2},
{cmdPartUnicursal,         "PartialUnicursal","cU",   R2 | HG | B1 | M2},
{cmdPartWeave,             "PartialWeave",    "cV",   R2 | HG | B1 | M2},
{cmdPartZeta,              "PartialZeta",     "",     R2 | HG | B1 | M2},
{cmdPaste,                 "Paste",           "cv",   R2 | HG},
{cmdPatchToWire,           "PatchToWire",     "",          HG},
{cmdPolishMaze,            "Polish",          "/",    R2},
{cmdPrint,                 "Print",           "",     0},
{cmdPrintSetup,            "PrintSetup",      "",     0},
{cmdRandomize,             "Randomize",       "cr",   R1},
{cmdRedrawNow,             "Update",          "n",    SP},
{cmdRenderAerial,          "Aerial",          "",     R2 | HG |      M2},
{cmdRenderOverview,        "Overview",        "",     R2 | HG},
{cmdRenderPatchPerspective,"PatchPerspective","",     R2 | HG |      M2},
{cmdRenderPyramid,         "Pyramid",         "",     R2 | HG |      M2},
{cmdRenderWirePerspective, "WirePerspective", "",     R2 | HG |      M2},
{cmdRepaintNow,            "Redraw",          "Spc",  SP},
{cmdRepeat,                "Repeat",          ";",    0},
{cmdRoomThin,              "RoomThinned",     "",     R2 | HG | B1 | M2},
{cmdRotateA,               "RotateAcross",    "",     R2 | HG |      M2},
{cmdRotateL,               "RotateLeft",      "",     R2 | HG |      M2},
{cmdRotateR,               "RotateRight",     "",     R2 | HG |      M2},
{cmdSave3DN,               "DlgSave3D",       "",     B1 | M3},
{cmdSave3DS,               "DlgSave3DSuper",  "",     B1 | M3},
{cmdSaveBitmap,            "DlgSaveBitmap",   "w",    0},
{cmdSaveColmapTarga,       "DlgSaveTarga",    "",     C1},
{cmdSaveDOS,               "DlgSaveDOSBlock", "",     B1},
{cmdSaveDOS2,              "DlgSaveDOSLine",  "",     B1},
{cmdSaveDOS3,              "DlgSaveDOSLine2", "",     B1},
{cmdSavePatch,             "DlgSavePatch",    "",     0},
{cmdSavePicture,           "DlgSavePicture",  "",     0},
{cmdSaveText,              "DlgSaveText",     "",     0},
{cmdSaveVector,            "DlgSaveVector",   "",     0},
{cmdSaveWallCenter,        "SaveWallCenter",  "",     0},
{cmdSaveWallFill,          "SaveWallFill",    "",     0},
{cmdSaveWallFit,           "SaveWallFit",     "",     0},
{cmdSaveWallStretch,       "SaveWallStretch", "",     0},
{cmdSaveWallTile,          "SaveWallTile",    "",     0},
{cmdSaveWire,              "DlgSaveWire",     "",     0},
{cmdSaveXbmC,              "DlgSaveX11Comp",  "",     B1},
{cmdSaveXbmN,              "DlgSaveX11",      "",     B1},
{cmdSaveXbmS,              "DlgSaveX11Super", "",     B1},
{cmdScript01,              "Script1",         "a1",   0},
{cmdScript02,              "Script2",         "a2",   0},
{cmdScript03,              "Script3",         "a3",   0},
{cmdScript04,              "Script4",         "a4",   0},
{cmdScript05,              "Script5",         "a5",   0},
{cmdScript06,              "Script6",         "a6",   0},
{cmdScript07,              "Script7",         "a7",   0},
{cmdScript08,              "Script8",         "a8",   0},
{cmdScript09,              "Script9",         "a9",   0},
{cmdScript10,              "Script10",        "a)",   0},
{cmdScript11,              "Script11",        "a!",   0},
{cmdScript12,              "Script12",        "a@",   0},
{cmdScript13,              "Script13",        "a#",   0},
{cmdScript14,              "Script14",        "a$",   0},
{cmdScript15,              "Script15",        "a%",   0},
{cmdScript16,              "Script16",        "a^",   0},
{cmdScript17,              "Script17",        "a&",   0},
{cmdScript18,              "Script18",        "a*",   0},
{cmdScript19,              "Script19",        "a(",   0},
{cmdScript21,              "Script21",        "ac1",  0},
{cmdScript22,              "Script22",        "ac2",  0},
{cmdScript23,              "Script23",        "ac3",  0},
{cmdScript24,              "Script24",        "ac4",  0},
{cmdScript25,              "Script25",        "ac5",  0},
{cmdScript26,              "Script26",        "ac6",  0},
{cmdScript27,              "Script27",        "ac7",  0},
{cmdScript28,              "Script28",        "ac8",  0},
{cmdScript29,              "Script29",        "ac9",  0},
{cmdScript30,              "Script30",        "ac0",  0},
{cmdScrollDown,            "ScrollDown",      "aPgDn",R2},
{cmdScrollEnd,             "ScrollEnd",       "aEnd", R2},
{cmdScrollHome,            "ScrollHome",      "aHome",R2},
{cmdScrollUp,              "ScrollUp",        "aPgUp",R2},
{cmdSetupAll,              "SetupAll",        "",     HG},
{cmdSetupDesktop,          "SetupDesktop",    "",     HG},
{cmdSetupExtension,        "SetupExtension",  "",     HG},
{cmdSetupUser,             "SetupUser",       "",     HG},
{cmdShellChange,           "HelpChange",      "",     0},
{cmdShellHelp,             "Documentation",   ")",    0},
{cmdShellLicense,          "HelpLicense",     "",     0},
{cmdShellScript,           "HelpScript",      "",     0},
{cmdShellWeb,              "Web",             "",     0},
{cmdShellWeb2,             "Web2",            "",     0},
{cmdShowBottleneck,        "Bottleneck",      "",     SV | M2},
{cmdSize01,                "SizeA",           "cA",   R2 | HG},
{cmdSize02,                "SizeB",           "cB",   R2 | HG},
{cmdSize03,                "SizeD",           "cD",   R2 | HG},
{cmdSize04,                "SizeE",           "cE",   R2 | HG},
{cmdSize05,                "SizeF",           "cF",   R2 | HG},
{cmdSize06,                "SizeH",           "cH",   R2 | HG},
{cmdSize07,                "SizeI",           "cI",   R2 | HG},
{cmdSize08,                "SizeJ",           "cJ",   R2 | HG},
{cmdSize09,                "SizeS",           "cS",   R2 | HG},
{cmdSize10,                "SizeT",           "cT",   R2 | HG},
{cmdSize11,                "SizeX",           "cX",   R2 | HG},
{cmdSize12,                "SizeY",           "cY",   R2 | HG},
{cmdSize13,                "SizeZ",           "cZ",   R2 | HG},
{cmdSize14,                "Size\\",          "c\\",  R2 | HG},
{cmdSize15,                "Size]",           "c]",   R2 | HG},
{cmdSize16,                "Size!",           "c!",   R2 | HG},
{cmdSize17,                "Size^",           "c^",   R2 | HG},
{cmdSize18,                "Size#",           "c#",   R2 | HG},
{cmdSize19,                "Size%",           "c%",   R2 | HG},
{cmdSizeWindow,            "SizeWindow",      "cTab", R2 | HG},
{cmdSlide,                 "Slide",           "",     R2 | HG |           SP},
{cmdSmooth,                "SmoothZoom",      "",     R2 | HG | B1 | M2},
{cmdSolve4D,               "DeadEnd4D",       "",     SV | M2},
{cmdSolveBlindAlley,       "BlindAlley",      "",     SV | M2},
{cmdSolveBlock,            "DeadEnd",         "F",    SV},
{cmdSolveChain,            "Chain",           "",     SV | M2},
{cmdSolveCollision,        "Collision",       "",     SV | M2},
{cmdSolveCuldsac,          "CulDeSac",        "",     SV | M2},
{cmdSolveFollowL,          "FollowLeft",      "",     SV | M2},
{cmdSolveFollowR,          "FollowRight",     "",     SV | M2},
{cmdSolveFractal2,         "SolveFractal2",   "",     R2 | HG | M2},
{cmdSolveHyper,            "SolveHyper",      "",     SV | M3},
{cmdSolvePledgeL,          "PledgeLeft",      "",     SV | M2},
{cmdSolvePledgeR,          "PledgeRight",     "",     SV | M2},
{cmdSolveRecursive,        "SolveRecursive",  "",     SV | M2},
{cmdSolveShortest,         "Shortest",        "",     SV | M2},
{cmdSolveShortest2,        "Shortests",       "",     SV | M2},
{cmdSolveTremaux,          "Tremaux",         "",     R2 | HG | C2 | M2},
{cmdSolveWeave,            "DeadEndWeave",    "",     SV | M2},
{cmdSpree,                 "Autorepeat",      "N",    0},
{cmdSymmetric,             "Symmetric",       "",     R2 | HG | B1 | M2},
{cmdTempAdd,               "ColorTempAdd",    "",     R2 | HG | C1 | NK | SP},
{cmdTempAlpha,             "ColorTempAlpha",  "",     R2 | HG | C1 | NK | SP},
{cmdTempAnd,               "TempAnd",         "",     R2 |           NT},
{cmdTempBlend,             "Blend",           "",     R2 | HG |      NT | SP},
{cmdTempDel,               "TempDelete",      "",     R1},
{cmdTempGet,               "TempGet",         "=",    R1},
{cmdTempOr,                "TempOr",          "",     R2 |           NT},
{cmdTempPut,               "TempPut",         "+",    R2 |           NT},
{cmdTempSub,               "ColorTempSub",    "",     R2 | HG | C1 | NK | SP},
{cmdTempSwap,              "TempSwap",        "_",    R2 |           NT | SP},
{cmdTempTessellate,        "Tessellate",      "",     R2 |           NT},
{cmdTempXor,               "TempXor",         "",     R2 |           NT | SP},
{cmdTextureBackground,     "TextureBack",     "",     R2},
{cmdTextureBlock1,         "TextureColorB",   "",     R1 | HG},
{cmdTextureBlock2,         "TextureOverlayB", "",     R1 | HG},
{cmdTextureBlock3,         "TextureBlock",    "",     R1 | HG},
{cmdTextureCeiling,        "TextureCeiling",  "",     R1 | HG},
{cmdTextureDel,            "TextureDelete",   "",     R1},
{cmdTextureGround1,        "TextureColorG",   "",     R1 | HG},
{cmdTextureGround2,        "TextureOverlayG", "",     R1 | HG},
{cmdTextureGround3,        "TextureGround",   "",     R1 | HG},
{cmdTextureWall1,          "TextureColor",    "",     R1 | HG},
{cmdTextureWall2,          "TextureOverlay",  "",     R1 | HG},
{cmdTextureWall3,          "TextureWall",     "",     R1 | HG},
{cmdThicken,               "Thicken",         "",     R2 | HG |      SP | M2},
{cmdThinner,               "Thinner",         "",     R2 | HG |           M2},
{cmdTimeGet,               "TimerQuery",      "t",    0},
{cmdTimePause,             "TimerPause",      "p",    0},
{cmdTimeReset,             "TimerReset",      "ct",   0},
{cmdTweakEndpoint,         "TweakEndpoint",   "E",    R2 | HG | B1 | M2},
{cmdTweakPassage,          "TweakPassage",    "H",    R2 | HG | B1 | M2},
{cmdUnmark,                "EraseX",          "y",    R1},
{cmdUnsetup,               "UnsetupExtension","",     HG},
{cmdWallVariable,          "WallVariable",    "",     R2 | HG | C2},
{cmdWeave3D,               "Weave3D",         "",     R2 | HG | B1 | M3},
{cmdWeaveInside,           "WeaveInside",     "",     R2 | HG | C2 | M2},
{cmdWindowBitmap,          "WindowBitmap",    "csTab",0},
{cmdWindowFull,            "WindowFull",      "sTab", 0},
};


// Operations

enum _operationindex {
  oprOpen,
  oprOpenBitmap,
  oprOpenText,
  oprOpenXbm,
  oprOpen3D,
  oprOpenDB,
  oprOpenScript,
  oprOpenTarga,
  oprOpenPaint,
  oprOpenWire,
  oprOpenPatch,
  oprDlgDB,
  oprDlgPaint,
  oprEmbedXbm,
  oprEmbed3D,
  oprEmbedDW,
  oprEmbedDP,
  oprEmbedDB,
  oprSaveBitmap,
  oprSaveText,
  oprSaveDOS,
  oprSaveDOS2,
  oprSaveDOS3,
  oprSaveXbmN,
  oprSaveXbmC,
  oprSaveXbmS,
  oprSave3DN,
  oprSave3DC,
  oprSave3DS,
  oprSaveDB,
  oprSaveTarga,
  oprSaveOverview,
  oprSavePatch,
  oprSavePicture,
  oprSaveVector,
  oprSaveSolids,
  oprSize,
  oprSizeC,
  oprZoom,
  oprFlip3D,
  oprSizeMaze,
  oprSizeMazeC,
  oprBias,
  oprReplace,
  oprBrightness,

  oprIf,
  oprIfElse,
  oprSwitch,
  oprDoCount,
  oprWhile,
  oprDoWhile,
  oprFor,
  oprForStep,
  oprComment,
  oprContinue,
  oprBreak,
  oprReturn,
  oprHalt,
  oprRestart,
  oprDo,
  oprDoWait,

  oprMacroDefine,
  oprMacro,
  oprMacroEmbed,
  oprDoEmbed,
  oprConstDefine,
  oprClearEvents,
  oprVarSet,
  oprVarSets,
  oprVarSwap,
  oprLocal,
  oprStrSet,
  oprStrSets,
  oprStrSwap,
  oprStrAppend,
  oprStrChar,
  oprStrNum,
  oprStrReal,
  oprMaskCopy,
  oprMaskSwap,
  oprMaskDel,
  oprTextureCopy,
  oprTextureSwap,
  oprTextureDel,
  oprMoveCloud,
  oprMessage,
  oprMessageAsk,
  oprMessageIns,
  oprStrGet,
  oprStrGet2,
  oprStrGet3,
  oprDisableAll,
  oprEnableAll,
  oprDisable,
  oprEnable,
  oprOnCmdAll,
  oprOnCmd,
  oprDoCmd,
  oprReset,
  oprResetIns,
  oprTitle,
  oprBeep,
  oprBeep2,
  oprSound,
  oprDelay,
  oprSet,
  oprSetT,
  oprSetE,
  oprSetA,
  oprSetC,
  oprSetCT,
  oprSetCE,
  oprSetCA,
  oprSetX,
  oprSetY,
  oprSet3,
  oprSet3T,
  oprSet3E,
  oprSet3A,
  oprSet3C,
  oprSet3CT,
  oprSet3CE,
  oprSet3CA,
  oprSet3X,
  oprSet3Y,
  oprZapTexture,
  oprMark2,
  oprUnmark2,
  oprTurtle,
  oprTextDraw,
  oprTextFont,
  oprPerimeter,
  oprThicken2,
  oprBlendN,
  oprConvex,
  oprAllMoire,
  opr2ndLine2,
  opr2ndPut2,
  oprIsland,
  oprIslands,
  opr4D3D,
  oprND,
  oprSolveND,
  oprDrawND,
  oprMoveND,
  oprFractal,
  oprFractal3,
  oprVirtPerfect,
  oprVirtAldous,
  oprVirtRecurs,
  oprDrawLevel,
  oprCustomInc,
  oprDungeon,
  oprZoomSprite,
  oprDrawSeen,
  oprPalette,
  oprSymmetricX,
  oprSymmetricY,
  oprSymmetricZ,
  oprZoomPic,
  oprOverview2,
  oprAltitude,
  oprGetWire,
  oprSetWire,
  oprGetPatch,
  oprSetPatch,
  oprGetStar,
  oprSetStar,
  oprStereogram,
  oprSetLife,
  oprEvolution,
  oprMandelbrot,
  oprMandelbrotG,
  oprContrast,
  oprHunger,
  oprSystem,
  oprSetup,

  oprFileClose,
  oprFileWrite,
  oprFileByte,
  oprFileStr = copr-1,
};

CONST OPR rgopr[copr] = {
{oprOpen,        "Open",          1, SZ | R2},
{oprOpenBitmap,  "OpenBitmap",    1, SZ | R2},
{oprOpenText,    "OpenText",      1, SZ | R2 |      B2},
{oprOpenXbm,     "OpenX11",       1, SZ | R2 |      B2},
{oprOpen3D,      "Open3D",        1, SZ | R2 |      B2 | M3},
{oprOpenDB,      "OpenDB",        1, SZ | R2},
{oprOpenScript,  "OpenScript",    1, SZ},
{oprOpenTarga,   "OpenTarga",     1, SZ | R2 |      C2},
{oprOpenPaint,   "OpenPaint",     1, SZ | R2 |      C2},
{oprOpenWire,    "OpenWire",      1, SZ},
{oprOpenPatch,   "OpenPatch",     1, SZ},
{oprDlgDB,       "DlgOpenDB",     0, R2},
{oprDlgPaint,    "DlgOpenPaint",  0, R2 |      C2},
{oprEmbedXbm,    "EmbedX11",      0, R2 |      B2},
{oprEmbed3D,     "Embed3D",       0, R2 |      B2 | M3},
{oprEmbedDW,     "EmbedDW",       0, 0},
{oprEmbedDP,     "EmbedDP",       0, 0},
{oprEmbedDB,     "EmbedDB",       0, R2},
{oprSaveBitmap,  "SaveBitmap",    1, SZ},
{oprSaveText,    "SaveText",      1, SZ | B1},
{oprSaveDOS,     "SaveDOSBlock",  1, SZ | B1},
{oprSaveDOS2,    "SaveDOSLine",   1, SZ | B1},
{oprSaveDOS3,    "SaveDOSLine2",  1, SZ | B1},
{oprSaveXbmN,    "SaveX11",       1, SZ | B1},
{oprSaveXbmC,    "SaveX11Comp",   1, SZ | B1},
{oprSaveXbmS,    "SaveX11Super",  1, SZ | B1},
{oprSave3DN,     "Save3D",        1, SZ | B1 | M3},
{oprSave3DC,     "Save3DComp",    1, SZ | B1 | M3},
{oprSave3DS,     "Save3DSuper",   1, SZ | B1 | M3},
{oprSaveDB,      "SaveDB",        1, SZ},
{oprSaveTarga,   "SaveTarga",     1, SZ | C1},
{oprSaveOverview,"SaveWire",      1, SZ},
{oprSavePatch,   "SavePatch",     1, SZ},
{oprSavePicture, "SavePicture",   1, SZ},
{oprSaveVector,  "SaveVector",    1, SZ},
{oprSaveSolids,  "SaveSolids",    1, SZ},
{oprSize,        "Size",          4, R2      | B1},
{oprSizeC,       "SizeC",         4, R2      | C1},
{oprZoom,        "Zoom",          4, R2 | HG},
{oprFlip3D,      "Flip3D",        2, R2 | HG |      M3},
{oprSizeMaze,    "MazeSize",      3, R2 |      B1},
{oprSizeMazeC,   "MazeSizeC",     3, R2 |      C1},
{oprBias,        "Bias",          4, R2 | HG},
{oprReplace,     "Replace",       3, R2 | HG | C1},
{oprBrightness,  "Brightness",    2, R2 | HG | C1},

{oprIf,          "If",            2, 0},
{oprIfElse,      "IfElse",        3, 0},
{oprSwitch,      "SwitchSgn",     4, 0},
{oprDoCount,     "DoCount",       2, 0},
{oprWhile,       "While",         2, 0},
{oprDoWhile,     "DoWhile",       2, 0},
{oprFor,         "For",           4, 0},
{oprForStep,     "ForStep",       5, 0},
{oprComment,     "//",            0, 0},
{oprContinue,    "Continue",      0, 0},
{oprBreak,       "Break",         0, 0},
{oprReturn,      "Return",        0, 0},
{oprHalt,        "Halt",          0, 0},
{oprRestart,     "Restart",       0, 0},
{oprDo,          "Do",            1, SZ},
{oprDoWait,      "DoWait",        1, SZ | HG},

{oprMacroDefine, "DefineMacro",   3, 0},
{oprMacro,       "Macro",         1, 0},
{oprMacroEmbed,  "EmbedMacro",    2, 0},
{oprDoEmbed,     "Embed",         0, 0},
{oprConstDefine, "DefineConst",   1, 0},
{oprClearEvents, "ClearEvents",   0, 0},
{oprVarSet,      "SetVar",        2, 0},
{oprVarSets,     "SetVars",       3, 0},
{oprVarSwap,     "SwapVar",       2, 0},
{oprLocal,       "Local",         1, 0},
{oprStrSet,      "SetString",     2, 0},
{oprStrSets,     "SetStrings",    3, 0},
{oprStrSwap,     "SwapString",    2, 0},
{oprStrAppend,   "AppendString",  2, 0},
{oprStrChar,     "SetChar",       3, 0},
{oprStrNum,      "SetStringNum",  3, 0},
{oprStrReal,     "SetStringReal", 3, 0},
{oprMaskCopy,    "CopyMask",      2, R2},
{oprMaskSwap,    "SwapMask",      2, R2},
{oprMaskDel,     "DeleteMask",    1, R2},
{oprTextureCopy, "CopyTexture",   2, R2},
{oprTextureSwap, "SwapTexture",   2, R2},
{oprTextureDel,  "DeleteTexture", 1, R2},
{oprMoveCloud,   "MoveCloud",     2, R1},
{oprMessage,     "Message",       1, 0},
{oprMessageAsk,  "MessageAsk",    2, 0},
{oprMessageIns,  "MessageInside", 2, 0},
{oprStrGet,      "GetString",     2, 0},
{oprStrGet2,     "GetString2",    3, 0},
{oprStrGet3,     "GetString3",    4, 0},
{oprDisableAll,  "DisableAll",    0, 0},
{oprEnableAll,   "EnableAll",     0, 0},
{oprDisable,     "Disable",       1, 0},
{oprEnable,      "Enable",        1, 0},
{oprOnCmdAll,    "OnCommandAll",  1, 0},
{oprOnCmd,       "OnCommand",     2, 0},
{oprDoCmd,       "DoCommand",     1, 0},
{oprReset,       "ResetProgram",  0, R1},
{oprResetIns,    "ResetInside",   0, R1},
{oprTitle,       "WindowTitle",   2, 0},
{oprBeep,        "Beep",          0, 0},
{oprBeep2,       "Beep2",         2, 0},
{oprSound,       "Sound",         1, 0},
{oprDelay,       "Delay",         1, 0},
{oprSet,         "Set",           3, R2},
{oprSetT,        "SetT",          3, R1},
{oprSetE,        "SetE",          3, R1},
{oprSetA,        "SetA",          4, R1},
{oprSetC,        "SetC",          3, R2},
{oprSetCT,       "SetCT",         3, R1},
{oprSetCE,       "SetCE",         3, 0},
{oprSetCA,       "SetCA",         4, R1},
{oprSetX,        "SetX",          4, R1},
{oprSetY,        "SetY",          4, R1},
{oprSet3,        "Set3",          4, R2},
{oprSet3T,       "Set3T",         4, R1},
{oprSet3E,       "Set3E",         4, R1},
{oprSet3A,       "Set3A",         5, R1},
{oprSet3C,       "Set3C",         4, R2},
{oprSet3CT,      "Set3CT",        4, R1},
{oprSet3CE,      "Set3CE",        4, 0},
{oprSet3CA,      "Set3CA",        5, R1},
{oprSet3X,       "Set3X",         5, R1},
{oprSet3Y,       "Set3Y",         5, R1},
{oprZapTexture,  "ZapTexture",    1, R1},
{oprMark2,       "MarkX2",        0, R1},
{oprUnmark2,     "EraseX2",       0, R1},
{oprTurtle,      "Turtle",        1, R2 | HG | B1 | SZ},
{oprTextDraw,    "TextDraw",      3, R2 | HG |      SZ},
{oprTextFont,    "TextFont",      3, SZ},
{oprPerimeter,   "Perimeter",     0, 0},
{oprThicken2,    "Thicken2",      1, R2 | HG | B1},
{oprBlendN,      "BlendN",        0, R2 | HG},
{oprConvex,      "Convex",        0, R2 | HG | B1},
{oprAllMoire,    "AllMoire",      0, R2 | HG | B2},
{opr2ndLine2,    "2ndLineUntil",  1, R2 |      B2},
{opr2ndPut2,     "2ndPutStretch", 4, R2 | HG | C2 | NT | NC},
{oprIsland,      "NoIsland",      2, R2 | HG | B1},
{oprIslands,     "NoIslands",     0, R2 | HG | B1},
{opr4D3D,        "4D3D",          3, R2 | HG | B1 |      M3},
{oprND,          "ND",            0, MZ |                M2},
{oprSolveND,     "DeadEndND",     0, R2 | HG | B1 |      M2},
{oprDrawND,      "DrawND",        5, R2 |                M2},
{oprMoveND,      "MoveND",        4, 0},
{oprFractal,     "FractalPart",   4, R2 | HG | B1 |      M2},
{oprFractal3,    "FractalPart3",  5, R2 | HG | B1 |      M3},
{oprVirtPerfect, "VirtualPerfect",1, R2 | HG | B1 |      M2},
{oprVirtAldous,  "VirtualAldous", 1, R2 | HG | B1 |      M2},
{oprVirtRecurs,  "VirtualRecurs", 2, R2 | HG | B1 |      M2},
{oprDrawLevel,   "DrawLevel",     7, R2 | HG | B1 | NT},
{oprCustomInc,   "CustomIterate", 0, 0},
{oprDungeon,     "Dungeon",       5, R2 | HG},
{oprZoomSprite,  "ZoomSprite",    0, R2 | HG      | NT},
{oprDrawSeen,    "DrawSeen",      1, R2 |                M2},
{oprPalette,     "Palette",       0, R2 | HG | C1 | NT},
{oprSymmetricX,  "SymmetricX",    0, R2 | HG | B1 |      M2},
{oprSymmetricY,  "SymmetricY",    0, R2 | HG | B1 |      M2},
{oprSymmetricZ,  "SymmetricZ",    0, R2 | HG | B1 |      M2},
{oprZoomPic,     "ZoomBiasPic",   0, R2 | HG | B1 |      M2},
{oprOverview2,   "Overview2",     0, R2 | HG},
{oprAltitude,    "Altitude",      2, R2 | HG |      NC | M2},
{oprGetWire,     "GetWireframe",  2, 0},
{oprSetWire,     "SetWireframe",  2, 0},
{oprGetPatch,    "GetPatch",      2, 0},
{oprSetPatch,    "SetPatch",      2, 0},
{oprGetStar,     "GetStar",       3, 0},
{oprSetStar,     "SetStar",       5, R2},
{oprStereogram,  "Stereogram",    2, R2 | HG},
{oprSetLife,     "SetLife",       1, 0},
{oprEvolution,   "Evolution",     2, R2 | HG | C1},
{oprMandelbrot,  "Mandelbrot",    5, R2 | HG | C2},
{oprMandelbrotG, "MandelbrotGet", 0, 0},
{oprContrast,    "Contrast",      0, R2 | HG},
{oprHunger,      "Hunger",        1, 0},
{oprSystem,      "System",        1, 0},
{oprSetup,       "Setup",         0, 0},

{oprFileClose, "FileClose",     1, 0},
{oprFileWrite, "FileWrite",     2, 0},
{oprFileByte,  "FileWriteByte", 2, 0},
{oprFileStr,   "FileReadStr",   2, 0},
};


// Variables

enum _variableindex {
  varInfGen = 0,
  varInfCutoff,
  varInfNewFile,
  varInfEller,
  varInfFile,
  varInfFileOld,
  varInfFileDos,
  varInfStart,
  varCutoffX,
  varCutoffY,
  varCutoffX3D,
  varTextClip,
  varLineChar,
  varTextTab,

  varEventEdge,
  varEventWall,
  varEventMove,
  varEventMove2,
  varEventFollow,
  varEventInside,
  varEventInside2,
  varEventCommand,
  varEventLeft,
  varEventRight,
  varEventMiddle,
  varEventPrev,
  varEventNext,
  varEventMouse,
  varEventQuit,

  varRedrawWhat,
  varRedrawHow,
  varRedrawX,
  varRedrawY,
  varSizeX,
  varSizeY,
  varPositionX,
  varPositionY,
  varScrollX,
  varScrollY,
  varDrawable,
  varWindowTop,
  varWindowMenu,
  varWindowScroll,
  varColor,
  varNoDirty,
  varPartUpdate,
  varTraceDot,
  varDisplayDelay,
  varErrorCheck,
  varHourglass,
  varAutorepeat,
  varTimerDelay,
  varTimePause,
  varSkipMessage,
  varSound,

  varKvBorder,
  varKvPassage,
  varKvWall,
  varKvDot,
  varKvMap,
  varKvInWall,
  varKvSky,
  varKvGround,
  varKvLine,
  varKv3DWall,
  varKv3DCeil,
  varKvMountain,
  varKvCloud,
  varKvFog,
  varRndSpree,

  varRndSeed,
  varRndBias,
  varRndRun,

  varX,
  varY,
  varZ,
  varDir,
  varX2,
  varY2,
  varZ2,
  varDir2,
  var3D,
  varDot,
  varCircle,
  varArrow,
  varDotSize,
  varWall,
  varNoCorner,
  varGlance,
  varChaseMouse,
  varNoDiagonal,
  varFollow,
  varRadar,
  varMove,
  varTwo,
  varDrag,
  varSet,
  varBig,
  varBoth,
  varOnTemp,
  varEdge,

  varInside,
  varCompass,
  varCompassAng,
  varLocation,
  varMap,
  varSealed,
  varMountain,
  varPeakHeight,
  varCloud,
  varSunMoon,
  varRainbow,
  varStar,
  varStarCount,
  varMeteor,
  varNarrow,
  varCellSize,
  varNarrowSize,
  varWallHeight,
  varViewHeight,
  varStepHeight,
  varTexture,
  varLighting,
  varFogLength,
  varClipPlane,
  varViewSpan,
  varStereo,
  varTrans,
  varFrameMove,
  varFrameTurn,
  varFrameZ,
  varSpeedMove,
  varSpeedTurn,
  varSpeedZ,
  varOffsetX,
  varOffsetY,
  varOffsetZ,
  varOffsetD,
  varZSmooth,

  var3W,
  var3X,
  var3Y,
  var3Z,

  varBlendType,
  varBlendType2,
  varRainStart,
  varRainDist,
  varBlend1,
  varBlend2,

  varPolish,
  varSection,
  varStartGoto,
  varPoleBlock,
  varSolvePixel,
  varSolveDot,
  varRandomPath,
  varShortCount,
  varTweak,
  varMazeCount,
  varWeaveOn,
  varWeaveOff,
  varWeaveRail,
  varStartPos,

  varLabyrinth,
  varCircuitShape,
  varClassicSize,
  varClassicCirc,
  varCustomPaths,
  varAutoiterate,
  varNoRedundant,
  varAsymmetry,
  varPartitionX,
  varPartitionY,
  varCustomAlign,
  varCustomCenter,
  varLabyrinthArc,

  varRiver,
  varRiverEdge,
  varRiverFlow,
  varOmega,
  varOmega2,
  varOmegaIn,
  varOmegaOut,
  varOmegaDraw,
  varPlanair,
  varSegment,
  varTreeWall,
  varTreeRandom,
  varTreeRiver,
  varForsInit,
  varForsAdd,
  varSpiral,
  varSpiralWall,
  varRandomAdd,
  varFractalX,
  varFractalY,
  varFractalZ,
  varFractalI,
  varCrackOff,
  varCrackLength,
  varCrackPass,
  varCrackSector,
  varBraid,
  varSparse,
  varKruskalPic,
  varWeaveCorner,
  varTiltDiamond,
  varTiltSize,

  varXSize,
  varYSize,
  varZSize,
  varXBias,
  varYBias,
  varXStart,
  varYStart,
  varZStart,
  varArrowKeys,
  varXVanish,
  varYVanish,
  varTheta,
  varPhi,
  varMotion,
  varRotation,
  varViewRight,
  varMergeBlocks,

  varReflect,
  varShadeSky,
  varShadeGround,
  varDrawEdges,
  varShadeObj,
  varTouchUps,
  varBorder,
  varHorizon,
  varDistance,
  varXScale,
  varYScale,
  varZScale,
  varKvEdge,
  varKvSkyHi,
  varKvSkyLo,
  varKvGroundHi,
  varKvGroundLo,
  varKvObject,
  varXLight,
  varYLight,
  varZLight,

  varScreenSaver,
  varTitleMessage,
  varCopyMessage,
  varSkipMessage2,
  varXTurtle,
  varYTurtle,
  varTurtleS,
  varDot2,
  varNoMouseDrag,
  varAutorepeat2,
  varFogFloor,
  varKvDot2,
  varKvSun,
  varKvMoon,
  varKvWall2,
  varKvSky2,
  varKvGround2,
  varKv3DCeil2,
  varKvMountain2,
  varKvCloud2,
  varRainbowSize,
  varGroundZ,
  varHorizonY,
  varSunMoon2,
  varSunMoonY,
  varStarColor,
  varStarSize,
  varSkyAll,
  varSky3D,
  varShowDelay,
  varDelay,
  varNoCompass,
  varNoLocation,
  varNoSubmerge,
  varStepDown,
  varRedrawAfter,
  varMarkColor,
  varMarkAll,
  varMarkBlock,
  varMark2,
  varMarkElev,
  varWallVar,
  varFogLit,
  varTextureBack,
  varTextureBack2,
  varTextureWall,
  varTextureGround,
  varTextureVar,
  varTextureElev,
  varTextureDual,
  varTextureDual2,
  varTextureBlend,
  varMarkElevX1,
  varMarkElevY1,
  varMarkElevX2,
  varMarkElevY2,
  varMazeCellMax,
  varHuntType,
  varFractalDepth,
  varFractalLength,
  varFractalType,
  varStretch,
  varGraphNumber,
  varGrayscale,
  varArrowFree,
  varTransparency,
  varFogLength2,
  varLineWidth,
  varLineSort,
  varLineDistance,
  varFaceOrigin,
  varStereo3D,
  varWireCount,
  varPatchCount,
  varMandelbrot,
  varSoundDelay,
  varFileLock,
  varRndOld,
  varNoExit,
  varAlloc,
  varAllocTotal,
  varAllocSize = cvar-1,
};

CONST VAR rgvar[cvar] = {
{varInfGen,     "nCurrentGeneration",      0},
{varInfCutoff,  "nGenerationCutoff",       0},
{varInfNewFile, "nNewFileCutoff",          0},
{varInfEller,   "fUseEllersAlgorithm",     0},
{varInfFile,    "nInfiniteFile",           0},
{varInfFileOld, "fSaveInfiniteToFile",     0},
{varInfFileDos, "fSaveInfiniteAsDOSText",  0},
{varInfStart,   "fDontAutostartInfinite",  0},
{varCutoffX,    "n2DBitmapXCutoff",        0},
{varCutoffY,    "n2DBitmapYCutoff",        0},
{varCutoffX3D,  "n3DBitmapXCutoff",        0},
{varTextClip,   "fSaveTextBitmapsClipped", 0},
{varLineChar,   "fLineCharsInTextBitmaps", 0},
{varTextTab,    "fSaveTextBitmapsTabbed",  0},

{varEventEdge,    "nOnMoveOffBitmap",    0},
{varEventWall,    "nOnMoveIntoWall",     0},
{varEventMove,    "nOnMoveToNewCell",    0},
{varEventMove2,   "nAfterMoveToNewCell", 0},
{varEventFollow,  "nOnFollowToNewCell",  0},
{varEventInside,  "nAfterRedrawInside",  0},
{varEventInside2, "nOnRedrawInside",     0},
{varEventCommand, "nOnRunCommand",       0},
{varEventLeft,    "nOnLeftClick",        0},
{varEventRight,   "nOnRightClick",       0},
{varEventMiddle,  "nOnMiddleClick",      0},
{varEventPrev,    "nOnPrevClick",        0},
{varEventNext,    "nOnNextClick",        0},
{varEventMouse,   "nOnMouseMove",        0},
{varEventQuit,    "nOnProgramExit",      0},

{varRedrawWhat,   "nWhatToDraw",                R1},
{varRedrawHow,    "nHowToDrawIt",               R1},
{varRedrawX,      "nHorizontalPixelValue",      R1},
{varRedrawY,      "nVerticalPixelValue",        R1},
{varSizeX,        "nWindowHorizontalSize",      R1},
{varSizeY,        "nWindowVerticalSize",        R1},
{varPositionX,    "nWindowHorizontalOffset",    R1},
{varPositionY,    "nWindowVerticalOffset",      R1},
{varScrollX,      "nHorizontalScroll",          R1},
{varScrollY,      "nVerticalScroll",            R1},
{varDrawable,     "fSizeIsDrawableArea",        R1},
{varWindowTop,    "fWindowAlwaysOnTop",         R1},
{varWindowMenu,   "fHideMenu",                  0},
{varWindowScroll, "fHideScrollBars",            0},
{varColor,        "fShowColorBitmap",           0},
{varNoDirty,      "fRedrawWhenBitmapEdited",    0},
{varPartUpdate,   "fAllowPartialScreenUpdates", 0},
{varTraceDot,     "fShowIndividualPixelEdits",  0},
{varDisplayDelay, "nDisplayDelay",              0},
{varErrorCheck,   "fErrorCheckPixelOperations", 0},
{varHourglass,    "fHourglassCursorOnRedraw",   0},
{varAutorepeat,   "fAutorepeatLastCommand",     0},
{varTimerDelay,   "nRepeatDelayInMsec",         0},
{varTimePause,    "fPauseTimer",                0},
{varSkipMessage,  "fSkipMessageDisplay",        0},
{varSound,        "fAllowSoundPlaying",         0},

{varKvBorder,   "kBorder",                        R1},
{varKvPassage,  "kPassage",                       R1},
{varKvWall,     "kWall",                          R1},
{varKvDot,      "kDot",                           R1},
{varKvMap,      "kOverlay",                       R1},
{varKvInWall,   "kInsideWall",                    R1},
{varKvSky,      "kSky",                           R1},
{varKvGround,   "kGround",                        R1},
{varKvLine,     "kLine",                          R1},
{varKv3DWall,   "k3DWall",                        R1},
{varKv3DCeil,   "k3DCeiling",                     R1},
{varKvMountain, "kMountain",                      R1},
{varKvCloud,    "kCloud",                         R1},
{varKvFog,      "kFog",                           R1},
{varRndSpree,   "fAutorepeatRandomizesWallColor", 0},

{varRndSeed, "nRandomSeed", 0},
{varRndBias, "nRandomBias", 0},
{varRndRun,  "nRandomRun",  0},

{varX,          "nX",               R1},
{varY,          "nY",               R1},
{varZ,          "nZ",               R1},
{varDir,        "nDir",             R1},
{varX2,         "nX2",              0},
{varY2,         "nY2",              0},
{varZ2,         "nZ2",              0},
{varDir2,       "nDir2",            0},
{var3D,         "f3D",              R1},
{varDot,        "fShowDot",         R1},
{varCircle,     "fDotIsCircle",     R1},
{varArrow,      "fDotShowsArrow",   R1},
{varDotSize,    "nDotSize",         R1},
{varWall,       "fWallsImpassable", 0},
{varNoCorner,   "fNoCornerHopping", 0},
{varGlance,     "fGlancingBlows",   0},
{varChaseMouse, "fChaseMousePoint", 0},
{varNoDiagonal, "fNoMouseDiagonal", 0},
{varFollow,     "fFollowPassages",  0},
{varRadar,      "nRadarLength",     0},
{varMove,       "nMoveCount",       0},
{varTwo,        "fMoveByTwo",       0},
{varDrag,       "fDragMoveDot",     0},
{varSet,        "fDragIsErase",     0},
{varBig,        "fDragBigDot",      0},
{varBoth,       "fDragByTwo",       0},
{varOnTemp,     "fDoDragOnTemp",    0},
{varEdge,       "nEdgeBehavior",    R1},

{varInside,     "nInside",               R1},
{varCompass,    "fCompass",              R1},
{varCompassAng, "fCompassIsExact",       R1},
{varLocation,   "fCellLocation",         R1},
{varMap,        "fMapOverlay",           R1},
{varSealed,     "fSealedEntrances",      R1},
{varMountain,   "fMountains",            R1},
{varPeakHeight, "nPeakHeight",           R1},
{varCloud,      "nCloudCount",           R1},
{varSunMoon,    "fSunAndMoon",           R1},
{varRainbow,    "fRainbow",              R1},
{varStar,       "fStars",                R1},
{varStarCount,  "nStarCount",            R1},
{varMeteor,     "rMeteorRate",           R1},
{varNarrow,     "fNarrowWalls",          R1},
{varCellSize,   "nCellSize",             R1},
{varNarrowSize, "nNarrowCellSize",       R1},
{varWallHeight, "nWallHeight",           R1},
{varViewHeight, "nViewHeight",           R1},
{varStepHeight, "nStepHeight",           0},
{varTexture,    "fAllowTextureMapping",  R1},
{varLighting,   "rLightFactor",          R1},
{varFogLength,  "nFogDistance",          R1},
{varClipPlane,  "nClippingPlane",        R1},
{varViewSpan,   "rViewingSpan",          R1},
{varStereo,     "nStereoWidth",          R1},
{varTrans,      "nSemitransparentWalls", R1},
{varFrameMove,  "nMotionFrames",         0},
{varFrameTurn,  "nRotationFrames",       0},
{varFrameZ,     "nUpDownFrames",         0},
{varSpeedMove,  "nMotionVelocity",       0},
{varSpeedTurn,  "nRotationVelocity",     0},
{varSpeedZ,     "nUpDownVelocity",       0},
{varOffsetX,    "nHorizontalOffset",     R1},
{varOffsetY,    "nVerticalOffset",       R1},
{varOffsetZ,    "nUpDownOffset",         R1},
{varOffsetD,    "nDirectionOffset",      R1},
{varZSmooth,    "fUpDownSmoothNotFree",  0},

{var3W, "Size3W", R1},
{var3X, "Size3X", R1},
{var3Y, "Size3Y", R1},
{var3Z, "Size3Z", R1},

{varBlendType,  "fRainbowBlend",       0},
{varBlendType2, "fStraightColorBlend", 0}, // Redundant, for old scripts
{varRainStart,  "nRainbowStart",       0},
{varRainDist,   "nRainbowDistance",    0},
{varBlend1,     "kBlend1",             0},
{varBlend2,     "kBlend2",             0},

{varPolish,     "fCreateMazesPolished",              0},
{varSection,    "fApplyCommandsToRectangleSection",  0},
{varStartGoto,  "fTeleportToEntranceOnMazeCreation", 0},
{varPoleBlock,  "fConnectPolesNeverAddsDeadEnds",    0},
{varSolvePixel, "fSolveFillersCheckEveryPixel",      0},
{varSolveDot,   "fSolveFillersConsiderDotsAsExits",  0},
{varRandomPath, "fFindAPathFindsRandomPath",         0},
{varShortCount, "fShowCountOfShortestSolutions",     0},
{varTweak,      "nTweakPassagesChances",             0},
{varMazeCount,  "nTotalMazesCreated",                0},
{varWeaveOn,    "nClarifyWeaveWallBias",             0},
{varWeaveOff,   "nClarifyWeavePassageBias",          0},
{varWeaveRail,  "nClarifyWeaveRailingBias",          0},
{varStartPos,   "nEntrancePositioning",              0},

{varLabyrinth,    "nLabyrinthType",           0},
{varCircuitShape, "nCircuitShape",            0},
{varClassicSize,  "nClassicalSize",           0},
{varClassicCirc,  "fClassicalSizeIsCircuits", 0},
{varCustomPaths,  "sCustomPaths",             0},
{varAutoiterate,  "fCustomAutoiterate",       0},
{varNoRedundant,  "nFanfoldsAllowed",         0},
{varAsymmetry,    "nAsymmetryAllowed",        0},
{varPartitionX,   "nCircuitPartitioning",     0},
{varPartitionY,   "nRadiusPartitioning",      0},
{varCustomAlign,  "nCustomAlign",             0},
{varCustomCenter, "nCustomCenterSize",        0},
{varLabyrinthArc, "fLabyrinthRoundedEdges",   0},

{varRiver,       "fCreateWithRiver",         0},
{varRiverEdge,   "fHaveRiverAtEdges",        0},
{varRiverFlow,   "fNoRiverFlowback",         0},
{varOmega,       "nOmegaDimensions",         0},
{varOmega2,      "nOmegaInnerDimensions",    0},
{varOmegaIn,     "nOmegaEntranceLocation",   0},
{varOmegaOut,    "nOmegaExitLocation",       0},
{varOmegaDraw,   "nOmegaDraw",               0},
{varPlanair,     "sPlanair",                 0},
{varSegment,     "sSegment",                 0},
{varTreeWall,    "fCreateWithWallAdding",    0},
{varTreeRandom,  "fTreeValueIsRandomChance", 0},
{varTreeRiver,   "nTreeRiverValue",          0},
{varForsInit,    "nForestInitial",           0},
{varForsAdd,     "nForestAddition",          0},
{varSpiral,      "nMaxSpirals",              0},
{varSpiralWall,  "nMaxSpiralWalls",          0},
{varRandomAdd,   "nRandomAddition",          0},
{varFractalX,    "nFractalX",                0},
{varFractalY,    "nFractalY",                0},
{varFractalZ,    "nNestingLevel",            0},
{varFractalI,    "fAllowIsolationInFractal", 0},
{varCrackOff,    "fCrackLinesMayHeadOff",    0},
{varCrackLength, "nCrackLineLength",         0},
{varCrackPass,   "nCrackPassLimit",          0},
{varCrackSector, "nCrackSector",             0},
{varBraid,       "nBraidMethod",             0},
{varSparse,      "nCavernSparseness",        0},
{varKruskalPic,  "fKruskalBasedOnPicture",   0},
{varWeaveCorner, "fWeaveCrossingsMayCorner", 0},
{varTiltDiamond, "fTiltLinesMakeNoDiamonds", 0},
{varTiltSize,    "nTiltCellSize",            0},

{varXSize,       "nHorizontalSize",       0},
{varYSize,       "nVerticalSize",         0},
{varZSize,       "nDepthSize",            0},
{varXBias,       "nHorizontalSizeBias",   0},
{varYBias,       "nVerticalSizeBias",     0},
{varXStart,      "nXStartingLocation",    0},
{varYStart,      "nYStartingLocation",    0},
{varZStart,      "nZStartingLocation",    0},
{varArrowKeys,   "fMoveWithArrowKeys",    0},
{varXVanish,     "nXVanishingPoint",      0},
{varYVanish,     "nYVanishingPoint",      0},
{varTheta,       "nViewingAngleTheta",    0},
{varPhi,         "nVerticalPitchPhi",     0},
{varMotion,      "nDrawMotionVelocity",   0},
{varRotation,    "nDrawRotationVelocity", 0},
{varViewRight,   "fViewFromRight",        0},
{varMergeBlocks, "fMergeBlocksTogether",  0},

{varReflect,     "fReflectCoordinates", 0},
{varShadeSky,    "fShadeSky",           0},
{varShadeGround, "fShadeGround",        0},
{varDrawEdges,   "fDrawEdges",          0},
{varShadeObj,    "fShadeObjects",       0},
{varTouchUps,    "fDoTouchUps",         0},
{varBorder,      "nBorderWidth",        0},
{varHorizon,     "nHorizonWidth",       0},
{varDistance,    "nDistanceScale",      0},
{varXScale,      "nXScale",             0},
{varYScale,      "nYScale",             0},
{varZScale,      "nZScale",             0},
{varKvEdge,      "kEdge",               0},
{varKvSkyHi,     "kSkyHigh",            0},
{varKvSkyLo,     "kSkyLow",             0},
{varKvGroundHi,  "kGroundHigh",         0},
{varKvGroundLo,  "kGroundLow",          0},
{varKvObject,    "kObject",             0},
{varXLight,      "nXLightVector",       0},
{varYLight,      "nYLightVector",       0},
{varZLight,      "nZLightVector",       0},

{varScreenSaver,   "fScreenSaver",    0},
{varTitleMessage,  "nMessageTitle",   0},
{varCopyMessage,   "fMessageCopy",    0},
{varSkipMessage2,  "nSkipMessage",    0},
{varXTurtle,       "nXTurtle",        0},
{varYTurtle,       "nYTurtle",        0},
{varTurtleS,       "nTurtleS",        0},
{varDot2,          "fShowDot2",       R1},
{varNoMouseDrag,   "fNoMouseDrag",    0},
{varAutorepeat2,   "nAutorepeat",     0},
{varFogFloor,      "fFogGround",      R1},
{varKvDot2,        "kDot2",           R1},
{varKvSun,         "kSun",            R1},
{varKvMoon,        "kMoon",           R1},
{varKvWall2,       "kWall2",          R1},
{varKvSky2,        "kSky2",           R1},
{varKvGround2,     "kGround2",        R1},
{varKv3DCeil2,     "k3DCeiling2",     R1},
{varKvMountain2,   "kMountain2",      R1},
{varKvCloud2,      "kCloud2",         R1},
{varRainbowSize,   "nRainbow",        R1},
{varGroundZ,       "nGround",         R1},
{varHorizonY,      "nHorizon",        R1},
{varSunMoon2,      "nSunMoon",        R1},
{varSunMoonY,      "nSunMoonY",       R1},
{varStarColor,     "nStarColor",      0},
{varStarSize,      "nStarSize",       R1},
{varSkyAll,        "fSkyAll",         R1},
{varSky3D,         "fSky3D",          R1},
{varShowDelay,     "fFrameDelay",     R1},
{varDelay,         "nFrameDelay",     R1},
{varNoCompass,     "fNoCompass",      R1},
{varNoLocation,    "fNoLocation",     R1},
{varNoSubmerge,    "fNoSubmerge",     R1},
{varStepDown,      "nStepDown",       0},
{varRedrawAfter,   "fRedrawAfter",    0},
{varMarkColor,     "fMarkColor",      R1},
{varMarkAll,       "fMarkAll",        R1},
{varMarkBlock,     "fMarkBlock",      R1},
{varMark2,         "nSkyMark",        R1},
{varMarkElev,      "nMarkElev",       R1},
{varWallVar,       "nWallVariable",   R1},
{varFogLit,        "nFogLit",         R1},
{varTextureBack,   "nTexture",        R1},
{varTextureBack2,  "nTexture3",       R1},
{varTextureWall,   "nTextureWall",    R1},
{varTextureGround, "nTextureGround",  R1},
{varTextureVar,    "nTextureBlock",   R1},
{varTextureElev,   "nTextureElev",    R1},
{varTextureDual,   "fTextureDual",    R1},
{varTextureDual2,  "fTextureDual2",   R1},
{varTextureBlend,  "fTextureBlend",   R1},
{varMarkElevX1,    "nMarkElevX1",     0},
{varMarkElevY1,    "nMarkElevY1",     0},
{varMarkElevX2,    "nMarkElevX2",     0},
{varMarkElevY2,    "nMarkElevY2",     0},
{varMazeCellMax,   "nMazeCellMax",    0},
{varHuntType,      "nHuntType",       0},
{varFractalDepth,  "nFractalDepth",   0},
{varFractalLength, "nFractalLength",  0},
{varFractalType,   "nFractalType",    0},
{varStretch,       "nStretch",        R1},
{varGraphNumber,   "fGraphNumber",    0},
{varGrayscale,     "nGrayscale",      0},
{varArrowFree,     "fArrowKeysFree",  0},
{varTransparency,  "rTransparency",   R1},
{varFogLength2,    "nFogDistance2",   0},
{varLineWidth,     "nWireWidth",      R1},
{varLineSort,      "fWireSort",       R1},
{varLineDistance,  "nWireDistance",   R1},
{varFaceOrigin,    "nDrawFaceOrigin", R1},
{varStereo3D,      "fStereo3D",       R1},
{varWireCount,     "nWireframeSize",  0},
{varPatchCount,    "nPatchSize",      0},
{varMandelbrot,    "fMandelbrotShip", 0},
{varSoundDelay,    "nSoundDelay",     0},
{varFileLock,      "nFileLock",       0},
{varRndOld,        "fRndOld",         0},
{varNoExit,        "fNoExit",         0},
{varAlloc,         "nAllocations",    0},
{varAllocTotal,    "nAllocsTotal",    0},
{varAllocSize,     "nAllocsSize",     0},
};


// Functions

enum _functionindex {
  funFalse = 0,
  funTrue,
  funOff,
  funOn,
  funAdd,
  funSub,
  funMul,
  funDiv,
  funMod,
  funNeg,
  funInc,
  funDec,
  funPow,
  funEqu,
  funNeq,
  funLt,
  funGt,
  funLte,
  funGte,
  funNot,
  funAnd,
  funOr,
  funXor,
  funInv,
  funShftL,
  funShftR,
  funAndL,
  funOrL,
  funOdd,
  funAbs,
  funSgn,
  funMin,
  funMax,
  funTween,
  funRnd,
  funIf,
  funSqr,
  funDist,
  funLn,
  funSin,
  funCos,
  funTan,
  funAsin,
  funAcos,
  funAtan,
  funAng,
  funSin2,
  funCos2,
  funTan2,
  funAsin2,
  funAcos2,
  funAtan2,
  funAng2,

  funLen,
  funChar,
  funNum,
  funReal,
  funEquS,
  funNeqS,
  funLtS,
  funGtS,
  funLteS,
  funGteS,
  funEquSI,
  funNeqSI,
  funLtSI,
  funGtSI,
  funLteSI,
  funGteSI,
  funInCh,

  funBX,
  funBY,
  funCX,
  funCY,
  funGet,
  funGetT,
  funGetE,
  funGetA,
  funGetC,
  funGetCT,
  funGetCE,
  funGetCA,
  funGetX,
  funGetH,
  funGetM,
  funGet3,
  funGet3T,
  funGet3E,
  funGet3A,
  funGet3C,
  funGet3CT,
  funGet3CE,
  funGet3CA,
  funGet3X,
  funGet3H,
  funGet3M,
  funPixel,
  funRGB,
  funRGBR,
  funRGBG,
  funRGBB,
  funGray,
  funLight,
  funDark,
  funShade,
  funBlend,
  funBlend2,
  funHue,
  funHSL,
  funDlg,
  funNWSE,
  funUD,
  funUDU,
  funUDD,
  funTimer,
  funTimer2,
  funVar,
  funCmd,
  funEval,
  funEvent,
  funVer,

  funFileOpen,
  funFileNum,
  funFileByte,
  funFileEnd = cfun-1,
};

CONST FUN rgfun[cfun] = {
{funFalse, "False", 0},
{funTrue,  "True",  0},
{funOff,   "Off",   0},
{funOn,    "On",    0},
{funAdd,   "Add",   2},
{funSub,   "Sub",   2},
{funMul,   "Mul",   2},
{funDiv,   "Div",   2},
{funMod,   "Mod",   2},
{funNeg,   "Neg",   1},
{funInc,   "Inc",   1},
{funDec,   "Dec",   1},
{funPow,   "Pow",   2},
{funEqu,   "Equ",   2},
{funNeq,   "Neq",   2},
{funLt,    "Lt",    2},
{funGt,    "Gt",    2},
{funLte,   "Lte",   2},
{funGte,   "Gte",   2},
{funNot,   "Not",   1},
{funAnd,   "And",   2},
{funOr,    "Or",    2},
{funXor,   "Xor",   2},
{funInv,   "Inv",   1},
{funShftL, "<<",    2},
{funShftR, ">>",    2},
{funAndL,  "&&",    2},
{funOrL,   "||",    2},
{funOdd,   "Odd",   1},
{funAbs,   "Abs",   1},
{funSgn,   "Sgn",   1},
{funMin,   "Min",   2},
{funMax,   "Max",   2},
{funTween, "Tween", 3},
{funRnd,   "Rnd",   2},
{funIf,    "?:",    3},
{funSqr,   "Sqr",   1},
{funDist,  "Dist",  4},
{funLn,    "Ln",    2},
{funSin,   "Sin",   2},
{funCos,   "Cos",   2},
{funTan,   "Tan",   2},
{funAsin,  "Asin",  2},
{funAcos,  "Acos",  2},
{funAtan,  "Atan",  2},
{funAng,   "Ang",   2},
{funSin2,  "Sin2",  3},
{funCos2,  "Cos2",  3},
{funTan2,  "Tan2",  3},
{funAsin2, "Asin2", 3},
{funAcos2, "Acos2", 3},
{funAtan2, "Atan2", 3},
{funAng2,  "Ang2",  3},

{funLen,   "Len",     1},
{funChar,  "Char",    2},
{funNum,   "Num",     1},
{funReal,  "Real",    2},
{funEquS,  "EquStr",  2},
{funNeqS,  "NeqStr",  2},
{funLtS,   "LtStr",   2},
{funGtS,   "GtStr",   2},
{funLteS,  "LteStr",  2},
{funGteS,  "GteStr",  2},
{funEquSI, "EquStrI", 2},
{funNeqSI, "NeqStrI", 2},
{funLtSI,  "LtStrI",  2},
{funGtSI,  "GtStrI",  2},
{funLteSI, "LteStrI", 2},
{funGteSI, "GteStrI", 2},
{funInCh,  "InChar",  2},

{funBX,     "SizeX",   0},
{funBY,     "SizeY",   0},
{funCX,     "SizeCX",  0},
{funCY,     "SizeCY",  0},
{funGet,    "Get",     2},
{funGetT,   "GetT",    2},
{funGetE,   "GetE",    2},
{funGetA,   "GetA",    3},
{funGetC,   "GetC",    2},
{funGetCT,  "GetCT",   2},
{funGetCE,  "GetCE",   2},
{funGetCA,  "GetCA",   3},
{funGetX,   "GetX",    3},
{funGetH,   "GetH",    2},
{funGetM,   "GetM",    2},
{funGet3,   "Get3",    3},
{funGet3T,  "Get3T",   3},
{funGet3E,  "Get3E",   3},
{funGet3A,  "Get3A",   4},
{funGet3C,  "Get3C",   3},
{funGet3CT, "Get3CT",  3},
{funGet3CE, "Get3CE",  3},
{funGet3CA, "Get3CA",  4},
{funGet3X,  "Get3X",   4},
{funGet3H,  "Get3H",   3},
{funGet3M,  "Get3M",   3},
{funPixel,  "Pixels",  0},
{funRGB,    "Rgb",     3},
{funRGBR,   "RgbR",    1},
{funRGBG,   "RgbG",    1},
{funRGBB,   "RgbB",    1},
{funGray,   "GrayN",   1},
{funLight,  "Light",   1},
{funDark,   "Dark",    1},
{funShade,  "Shade",   2},
{funBlend,  "Blend",   2},
{funBlend2, "Blend2",  4},
{funHue,    "Hue",     1},
{funHSL,    "HSL",     3},
{funDlg,    "Dlg",     0},
{funNWSE,   "NWSE",    4},
{funUD,     "UD",      2},
{funUDU,    "UdU",     1},
{funUDD,    "UdD",     1},
{funTimer,  "Timer",   0},
{funTimer2, "Timer2",  0},
{funVar,    "Var",     1},
{funCmd,    "Command", 1},
{funEval,   "Eval",    1},
{funEvent,  "Event",   1},
{funVer,    "Version", 0},

{funFileOpen, "FileOpen",     2},
{funFileNum,  "FileReadNum",  1},
{funFileByte, "FileReadByte", 1},
{funFileEnd,  "FileEnd",      1},
};


/*
******************************************************************************
** Action Processing
******************************************************************************
*/

// Return a command index for a string. Try looking in both the case
// insensitive command list and the case sensitive command abbreviation list.

int CmdFromRgch(CONST char *pch, int cch)
{
  int icmd;

  icmd = ILookupTrie(ws.rgsTrieCmd, pch, cch, fTrue);
  if (icmd >= 0)
    return icmd;
  return ILookupTrie(ws.rgsTrieAbbrev, pch, cch, fFalse);
}


// Set a variable to either a string or numeric value.

void DoSetVariable(int ivar, CONST char *sz, int cch, long l)
{
  int n = (int)l;
  flag f = (n != 0);
  char szT[cchSzMax];

  switch (ivar) {
  case varInfGen:     ms.nInfGen        = n; break;
  case varInfCutoff:  ms.nInfCutoff     = n; break;
  case varInfNewFile: ms.nInfFileCutoff = n; break;
  case varInfEller:   ms.fInfEller      = f; break;
  case varInfFile:    ms.nInfFile       = n; break;
  case varInfFileOld: ms.nInfFile       = f; break;    // Version 1.2-3.2
  case varInfFileDos: ms.nInfFile       = f*2; break;  // Version 1.2-3.2
  case varInfStart:   ms.fInfAutoStart  = f; break;
  case varCutoffX:    ws.xCutoff        = n; break;
  case varCutoffY:    ws.yCutoff        = n; break;
  case varCutoffX3D:  ms.xCutoff        = n; break;
  case varTextClip:   ws.fTextClip      = f; break;
  case varLineChar:   ws.fLineChar      = f; break;
  case varTextTab:    ws.fTextTab       = f; break;

  case varEventEdge:    ws.iEventEdge    = n; break;
  case varEventWall:    ws.iEventWall    = n; break;
  case varEventMove:    ws.iEventMove    = n; break;
  case varEventMove2:   ws.iEventMove2   = n; break;
  case varEventFollow:  ws.iEventFollow  = n; break;
  case varEventInside:  ws.iEventInside  = n; break;
  case varEventInside2: ws.iEventInside2 = n; break;
  case varEventCommand: ws.iEventCommand = n; break;
  case varEventLeft:    ws.iEventLeft    = n; break;
  case varEventRight:   ws.iEventRight   = n; break;
  case varEventMiddle:  ws.iEventMiddle  = n; break;
  case varEventPrev:    ws.iEventPrev    = n; break;
  case varEventNext:    ws.iEventNext    = n; break;
  case varEventMouse:   ws.iEventMouse   = n; break;
  case varEventQuit:    ws.iEventQuit    = n; break;

  case varRedrawWhat: bm.nWhat        = n; break;
  case varRedrawHow:  bm.nHow         = n; break;
  case varRedrawX:    bm.xCell        = n; break;
  case varRedrawY:    bm.yCell        = n; break;
  case varScrollX:      DoSetVariableW(vosScrollX, n); break;
  case varScrollY:      DoSetVariableW(vosScrollY, n); break;
  case varColor:        FShowColmap(f);       break;
  case varNoDirty:      ws.fNoDirty      = f; break;
  case varPartUpdate:   ws.fAllowUpdate  = f; break;
  case varTraceDot:     gs.fTraceDot     = f; break;
  case varDisplayDelay: ws.nDisplayDelay = n; break;
  case varErrorCheck:   gs.fErrorCheck   = f; break;
  case varHourglass:    ws.fHourglass    = f; break;
  case varAutorepeat:   ws.fSpree        = f; ws.fMenuDirty = fTrue; break;
  case varTimerDelay:   DoSetVariableW(vosEnsureTimer, n);  break;
  case varTimePause:    DoSetVariableW(vosPauseTimer, n);
    ws.fMenuDirty = fTrue; break;
  case varSkipMessage:  ws.fIgnorePrint  = f; ws.fMenuDirty = fTrue; break;
  case varSound:        ws.fSound        = f; break;
  case varSizeX:        DoSetVariableW(vosSizeX, n);        break;
  case varSizeY:        DoSetVariableW(vosSizeY, n);        break;
  case varPositionX:    DoSetVariableW(vosPositionX, n);    break;
  case varPositionY:    DoSetVariableW(vosPositionY, n);    break;
  case varDrawable:     DoSetVariableW(vosDrawable, n);     break;
  case varWindowTop:    DoSetVariableW(vosWindowTop, n);    break;
  case varWindowMenu:   DoSetVariableW(vosWindowMenu, n);   break;
  case varWindowScroll: DoSetVariableW(vosWindowScroll, n); break;

  case varKvBorder:   dr.kvEdge       = n; break;
  case varKvPassage:  dr.kvOff        = n; break;
  case varKvWall:     dr.kvOn         = n; break;
  case varKvDot:      dr.kvDot        = n; break;
  case varKvMap:      dr.kvMap        = n; break;
  case varKvInWall:   dr.kvInWall     = n; break;
  case varKvSky:      dr.kvInSky      = n; break;
  case varKvGround:   dr.kvInDirt     = n; break;
  case varKvLine:     dr.kvInEdge     = n; break;
  case varKv3DWall:   dr.kvIn3D       = n; break;
  case varKv3DCeil:   dr.kvInCeil     = n; break;
  case varKvMountain: dr.kvInMtn      = n; break;
  case varKvCloud:    dr.kvInCld      = n; break;
  case varKvFog:      dr.kvInFog      = n; break;
  case varRndSpree:   dr.fSpreeRandom = f; break;

  case varRndSeed: ms.nRndSeed = n; InitRndL(ms.nRndSeed); break;
  case varRndBias: ms.nRndBias = n; break;
  case varRndRun:  ms.nRndRun  = n; break;

  case varX:          dr.x            = n; break;
  case varY:          dr.y            = n; break;
  case varZ:          dr.z            = n; break;
  case varDir:        dr.dir          = n; break;
  case varX2:         dr.x2           = n; break;
  case varY2:         dr.y2           = n; break;
  case varZ2:         dr.z2           = n; break;
  case varDir2:       dr.dir2         = n; break;
  case var3D:         dr.f3D          = f; ws.fMenuDirty = fTrue; break;
  case varDot:        dr.fDot         = f; ws.fMenuDirty = fTrue; break;
  case varCircle:     dr.fCircle      = f; break;
  case varArrow:      dr.fArrow       = f; break;
  case varDotSize:    dr.nDotSize     = n; break;
  case varWall:       dr.fWall        = f; ws.fMenuDirty = fTrue; break;
  case varNoCorner:   dr.fNoCorner    = f; break;
  case varGlance:     dr.fGlance      = f; break;
  case varChaseMouse: dr.fFollowMouse = f; break;
  case varNoDiagonal: dr.fNoDiagonal  = f; break;
  case varFollow:     dr.fFollow      = f; break;
  case varRadar:      ms.nRadar       = n; break;
  case varMove:       dr.cMove        = n; break;
  case varTwo:        dr.fTwo         = f;  ws.fMenuDirty = fTrue; break;
  case varDrag:       dr.fDrag        = f;  ws.fMenuDirty = fTrue; break;
  case varSet:        dr.fSet         = !f; ws.fMenuDirty = fTrue; break;
  case varBig:        dr.fBig         = f;  ws.fMenuDirty = fTrue; break;
  case varBoth:       dr.fBoth        = f;  ws.fMenuDirty = fTrue; break;
  case varOnTemp:     dr.fOnTemp      = f; break;
  case varEdge:       dr.nEdge        = n; break;

  case varInside:
    dr.fInside = n > nInsideNone;
    if (dr.fInside)
      dr.nInside = n;
    ws.fMenuDirty = fTrue;
    break;
  case varCompass:    dr.fCompass      = f; break;
  case varCompassAng: dr.fCompassAngle = f; break;
  case varLocation:   dr.fLocation     = f; break;
  case varMap:        dr.fMap          = f; break;
  case varSealed:     dr.fSealEntrance = f; break;
  case varMountain:   dr.fMountain     = f; break;
  case varPeakHeight: dr.nPeakHeight   = n; break;
  case varCloud:      dr.cCloud        = n; break;
  case varSunMoon:    dr.fSunMoon      = f; break;
  case varRainbow:    dr.fRainbow      = f; break;
  case varStar:       ds.fStar         = f; break;
  case varStarCount:  ds.cStar         = n; break;
  case varMeteor:     dr.nMeteor       = n; break;
  case varNarrow:     dr.fNarrow       = f; break;
  case varCellSize:   dr.zCell         = n; break;
  case varNarrowSize: dr.zCellNarrow   = n; break;
  case varWallHeight: dr.zWall         = n; break;
  case varViewHeight: dr.zElev         = n; break;
  case varStepHeight: dr.zStep         = n; break;
  case varTexture:    dr.fTexture      = f; break;
  case varLighting:   dr.rLightFactor  = (real)n / 100.0; break;
  case varFogLength:  dr.nFog          = n; break;
  case varClipPlane:  dr.nClip         = n; break;
  case varViewSpan:   dr.dInside       = (real)n / 100.0; break;
  case varStereo:     ds.nStereo       = n; break;
  case varTrans:      dr.nTrans        = n; break;
  case varFrameMove:  dr.nFrameXY      = n; break;
  case varFrameTurn:  dr.nFrameD       = n; break;
  case varFrameZ:     dr.nFrameZ       = n; break;
  case varSpeedMove:  dr.nSpeedXY      = n; break;
  case varSpeedTurn:  dr.nSpeedD       = n; break;
  case varSpeedZ:     dr.nSpeedZ       = n; break;
  case varOffsetX:    dr.nOffsetX      = n; break;
  case varOffsetY:    dr.nOffsetY      = n; break;
  case varOffsetZ:    dr.nOffsetZ      = n; break;
  case varOffsetD:    dr.nOffsetD      = n; break;
  case varZSmooth:    dr.f3DNotFree    = f; break;

  case var3W: BFocus().m_w3 = n; break;
  case var3X: BFocus().m_x3 = n; break;
  case var3Y: BFocus().m_y3 = n; break;
  case var3Z: BFocus().m_z3 = n; break;

  case varBlendType:  cs.fRainbowBlend    = f; break;
  case varBlendType2: cs.fRainbowBlend    = f; break;  // Bug: Should be !f
  case varRainStart:  cs.nRainbowStart    = n; break;
  case varRainDist:   cs.nRainbowDistance = n; break;
  case varBlend1:     cs.kvBlend1         = n; break;
  case varBlend2:     cs.kvBlend2         = n; break;

  case varPolish:     dr.fPolishMaze       = f; ws.fMenuDirty = fTrue; break;
  case varSection:    ms.fSection          = f; break;
  case varStartGoto:  ms.fTeleportEntrance = f; break;
  case varPoleBlock:  ms.fPoleNoDeadEnd    = f; break;
  case varSolvePixel: ms.fSolveEveryPixel  = f; break;
  case varSolveDot:   ms.fSolveDotExit     = f; break;
  case varRandomPath: ms.fRandomPath       = f; break;
  case varShortCount: ms.fCountShortest    = f; break;
  case varTweak:      ms.nTweakPassage     = n; break;
  case varMazeCount:  ms.cMaze             = n; break;
  case varWeaveOn:    ms.nWeaveOn          = n; break;
  case varWeaveOff:   ms.nWeaveOff         = n; break;
  case varWeaveRail:  ms.nWeaveRail        = n; break;
  case varStartPos:   ms.nEntrancePos      = n; break;

  case varLabyrinth:    ms.nLabyrinth     = n; break;
  case varCircuitShape: ms.lcs            = n; break;
  case varClassicSize:  ms.nClassical     = n; break;
  case varClassicCirc:  ms.fClassicalCirc = f; break;
  case varCustomPaths:  FCopyRgchToSzCustom(sz, cch, ms.szCustom); break;
  case varAutoiterate:  ms.fCustomAuto    = f; break;
  case varNoRedundant:  ms.nCustomFold    = n; break;
  case varAsymmetry:    ms.nCustomAsym    = n; break;
  case varPartitionX:   ms.nCustomPartX   = n; break;
  case varPartitionY:   ms.nCustomPartY   = n; break;
  case varCustomAlign:  ms.lcc            = n; break;
  case varCustomCenter: ms.zCustomMiddle  = n; break;
  case varLabyrinthArc: ms.lcs            = n + 1; break;  // Version 1.1-2.2

  case varRiver:        ms.fRiver        = f; break;
  case varRiverEdge:    ms.fRiverEdge    = f; break;
  case varRiverFlow:    ms.fRiverFlow    = f; break;
  case varOmega:        ms.omega         = n; break;
  case varOmega2:       ms.omega2        = n; break;
  case varOmegaIn:      ms.omegas        = n; break;
  case varOmegaOut:     ms.omegaf        = n; break;
  case varOmegaDraw:    ms.nOmegaDraw    = n; break;
  case varPlanair:      CopyRgchToSz(sz, cch, ms.szPlanair, cchSzMax); break;
  case varSegment:
    CopyRgchToSz(sz, cch, szT, cchSzMax); SegmentParseSz(szT); break;
  case varTreeWall:     ms.fTreeWall     = f; break;
  case varTreeRandom:   ms.fTreeRandom   = f; break;
  case varTreeRiver:    ms.nTreeRiver    = n; break;
  case varForsInit:     ms.nForsInit     = n; break;
  case varForsAdd:      ms.nForsAdd      = n; break;
  case varSpiral:       ms.cSpiral       = n; break;
  case varSpiralWall:   ms.cSpiralWall   = n; break;
  case varRandomAdd:    ms.cRandomAdd    = n; break;
  case varFractalX:     ms.xFractal      = n; break;
  case varFractalY:     ms.yFractal      = n; break;
  case varFractalZ:     ms.zFractal      = n; break;
  case varFractalI:     ms.fFractalI     = f; break;
  case varCrackOff:     ms.fCrackOff     = f; break;
  case varCrackLength:  ms.nCrackLength  = n; break;
  case varCrackPass:    ms.nCrackPass    = n; break;
  case varCrackSector:  ms.nCrackSector  = n; break;
  case varBraid:        ms.nBraid        = n; break;
  case varSparse:       ms.nSparse       = n; break;
  case varKruskalPic:   ms.fKruskalPic   = f; break;
  case varWeaveCorner:  ms.fWeaveCorner  = f; break;
  case varTiltDiamond:  ms.fTiltDiamond  = f; break;
  case varTiltSize:     ms.nTiltSize     = n; break;

  case varXSize:       ds.horsiz  = n; break;
  case varYSize:       ds.versiz  = n; break;
  case varZSize:       ds.depsiz  = n; break;
  case varXBias:       ds.horbias = n; break;
  case varYBias:       ds.verbias = n; break;
  case varXStart:      ds.hormin  = n; break;
  case varYStart:      ds.vermin  = n; break;
  case varZStart:      ds.depmin  = n; break;
  case varArrowKeys:   ds.fArrow  = f; break;
  case varXVanish:     ds.horv    = n; break;
  case varYVanish:     ds.verv    = n; break;
  case varTheta:       ds.theta   = n; break;
  case varPhi:         ds.phi     = n; break;
  case varMotion:      ds.speedm  = n; break;
  case varRotation:    ds.speedr  = n; break;
  case varViewRight:   ds.fRight  = f; break;
  case varMergeBlocks: ds.fMerge  = f; break;

  case varReflect:     ds.fReflect     = f; break;
  case varShadeSky:    ds.fSkyShade    = f; break;
  case varShadeGround: ds.fGroundShade = f; break;
  case varDrawEdges:   ds.fEdges       = f; break;
  case varShadeObj:    ds.fShading     = f; break;
  case varTouchUps:    ds.fTouch       = f; break;
  case varBorder:      ds.nBorder      = n; break;
  case varHorizon:     ds.nHoriz       = n; break;
  case varDistance:    ds.rScale       = (real)n; break;
  case varXScale:      ds.rxScale      = (real)n; break;
  case varYScale:      ds.ryScale      = (real)n; break;
  case varZScale:      ds.rzScale      = (real)n; break;
  case varKvEdge:      ds.kvTrim       = n; break;
  case varKvSkyHi:     ds.kvSkyHi      = n; break;
  case varKvSkyLo:     ds.kvSkyLo      = n; break;
  case varKvGroundHi:  ds.kvGroundHi   = n; break;
  case varKvGroundLo:  ds.kvGroundLo   = n; break;
  case varKvObject:    ds.kvObject     = n; break;
  case varXLight:      ds.vLight.m_x   = (real)n; break;
  case varYLight:      ds.vLight.m_y   = (real)n; break;
  case varZLight:      ds.vLight.m_z   = (real)n; break;

  case varScreenSaver:   ws.fSaverRun     = f; break;
  case varTitleMessage:  ws.nTitleMessage = n; break;
  case varCopyMessage:   ws.fCopyMessage  = f; break;
  case varSkipMessage2:  ws.nIgnorePrint  = n; break;
  case varXTurtle:       gs.turtlex       = n; break;
  case varYTurtle:       gs.turtley       = n; break;
  case varTurtleS:       gs.turtles       = n; break;
  case varDot2:          dr.fDot2         = f; break;
  case varNoMouseDrag:   dr.fNoMouseDrag  = f; break;
  case varAutorepeat2:   ws.wCmdSpree     = n + icmdBase; break;
  case varFogFloor:      dr.fFogFloor     = f; break;
  case varKvDot2:        dr.kvDot2        = n; break;
  case varKvSun:         dr.kvSun         = n; break;
  case varKvMoon:        dr.kvMoon        = n; break;
  case varKvWall2:       dr.kvInWall2     = n; break;
  case varKvSky2:        dr.kvInSky2      = n; break;
  case varKvGround2:     dr.kvInDirt2     = n; break;
  case varKv3DCeil2:     dr.kvInCeil2     = n; break;
  case varKvMountain2:   dr.kvInMtn2      = n; break;
  case varKvCloud2:      dr.kvInCld2      = n; break;
  case varRainbowSize:   dr.nRainbow      = n; break;
  case varGroundZ:       dr.zGround       = n; break;
  case varHorizonY:      dr.yHorizon      = n; break;
  case varSunMoon2:      dr.nSunMoon      = n; break;
  case varSunMoonY:      dr.ySunMoon      = n; break;
  case varStarColor:     ds.lStarColor    = n; break;
  case varStarSize:      ds.nStarSize     = n; break;
  case varSkyAll:        ds.fSkyAll       = f; break;
  case varSky3D:         dr.fSky3D        = f; break;
  case varShowDelay:     dr.fDelay        = f; break;
  case varDelay:         dr.nDelay        = n; break;
  case varNoCompass:     dr.fNoCompass    = f; break;
  case varNoLocation:    dr.fNoLocation   = f; break;
  case varNoSubmerge:    dr.fNoSubmerge   = f; break;
  case varStepDown:      dr.zStep2        = n; break;
  case varRedrawAfter:   dr.fRedrawAfter  = f; break;
  case varMarkColor:     dr.fMarkColor    = f; break;
  case varMarkAll:       dr.fMarkAll      = f; break;
  case varMarkBlock:     dr.fMarkBlock    = f; break;
  case varMark2:         dr.nMarkSky      = n; break;
  case varMarkElev:      dr.nMarkElev     = n; break;
  case varWallVar:       dr.nWallVar      = n; break;
  case varFogLit:        dr.nFogLit       = n; break;
  case varTextureBack:   dr.nTexture      = n; break;
  case varTextureBack2:  dr.nTexture2     = n; break;
  case varTextureWall:   dr.nTextureWall  = n; break;
  case varTextureGround: dr.nTextureDirt  = n; break;
  case varTextureVar:    dr.nTextureVar   = n; break;
  case varTextureElev:   dr.nTextureElev  = n; break;
  case varTextureDual:   dr.fTextureDual  = f; break;
  case varTextureDual2:  dr.fTextureDual2 = f; break;
  case varTextureBlend:  dr.fTextureBlend = f; break;
  case varMarkElevX1:    dr.nMarkElevX1   = n; break;
  case varMarkElevY1:    dr.nMarkElevY1   = n; break;
  case varMarkElevX2:    dr.nMarkElevX2   = n; break;
  case varMarkElevY2:    dr.nMarkElevY2   = n; break;
  case varMazeCellMax:   ms.nCellMax      = n; break;
  case varHuntType:      ms.nHuntType     = n; break;
  case varFractalDepth:  ms.nFractalD     = n; break;
  case varFractalLength: ms.nFractalL     = n; break;
  case varFractalType:   ms.nFractalT     = n; break;
  case varStretch:       dr.nStretchMode  = n; break;
  case varGraphNumber:   cs.fGraphNumber  = f; break;
  case varGrayscale:     cs.lGrayscale    = l; break;
  case varArrowFree:     ds.fArrowFree    = n; break;
  case varTransparency:  ds.nTrans        = n; break;
  case varFogLength2:    ds.nFog          = n; break;
  case varLineWidth:     ds.nWireWidth    = n; break;
  case varLineSort:      ds.fWireSort     = f; break;
  case varLineDistance:  ds.nWireDistance = n; break;
  case varFaceOrigin:    ds.nFaceOrigin   = n; break;
  case varStereo3D:      ds.fStereo3D     = f; break;
  case varWireCount:
    if (FSetWireSize(&bm.coor, bm.ccoor, n)) bm.ccoor = n; break;
  case varPatchCount:
    if (FSetPatchSize(&bm.patch, bm.cpatch, n)) bm.cpatch = n; break;
  case varMandelbrot:    cs.fMandelbrot   = f; break;
  case varSoundDelay:    ws.nSoundDelay   = n; break;
  case varFileLock:      ws.nFileLock     = n; break;
  case varRndOld:        us.fRndOld       = f; break;
  case varNoExit:        ws.fNoExit       = f; break;
  case varAlloc:         us.cAlloc        = n; break;
  case varAllocTotal:    us.cAllocTotal   = n; break;
  case varAllocSize:     us.cAllocSize    = n; break;

  default:
    PrintSzN_E("Setting variable %d is undefined.", ivar);
  }

  // Some variables directly affect the timer or the display.
  if (FWantTimer())
    DoSetVariableW(vosEnsureTimer, ws.nTimerDelay);
  if ((rgvar[ivar].grf & fCmtRedraw) > 0 && ws.fNoDirty)
    SystemHook(hosRedraw);
}


// Get the value of a variable, which will be stored in either a string or
// numeric return parameter.

void GetVariable(int ivar, char **psz, int *pcch, long *pl)
{
  int n = 0;

  switch (ivar) {
  case varInfGen:     n = ms.nInfGen;        break;
  case varInfCutoff:  n = ms.nInfCutoff;     break;
  case varInfNewFile: n = ms.nInfFileCutoff; break;
  case varInfEller:   n = ms.fInfEller;      break;
  case varInfFile:    n = ms.nInfFile;       break;
  case varInfFileOld: n = ms.nInfFile != 0;  break;  // Version 1.2-3.2
  case varInfFileDos: n = ms.nInfFile == 2;  break;  // Version 1.2-3.2
  case varInfStart:   n = ms.fInfAutoStart;  break;
  case varCutoffX:    n = ws.xCutoff;        break;
  case varCutoffY:    n = ws.yCutoff;        break;
  case varCutoffX3D:  n = ms.xCutoff;        break;
  case varTextClip:   n = ws.fTextClip;      break;
  case varLineChar:   n = ws.fLineChar;      break;
  case varTextTab:    n = ws.fTextTab;       break;

  case varEventEdge:    n = ws.iEventEdge;    break;
  case varEventWall:    n = ws.iEventWall;    break;
  case varEventMove:    n = ws.iEventMove;    break;
  case varEventMove2:   n = ws.iEventMove2;   break;
  case varEventFollow:  n = ws.iEventFollow;  break;
  case varEventInside:  n = ws.iEventInside;  break;
  case varEventInside2: n = ws.iEventInside2; break;
  case varEventCommand: n = ws.iEventCommand; break;
  case varEventLeft:    n = ws.iEventLeft;    break;
  case varEventRight:   n = ws.iEventRight;   break;
  case varEventMiddle:  n = ws.iEventMiddle;  break;
  case varEventPrev:    n = ws.iEventPrev;    break;
  case varEventNext:    n = ws.iEventNext;    break;
  case varEventMouse:   n = ws.iEventMouse;   break;
  case varEventQuit:    n = ws.iEventQuit;    break;

  case varRedrawWhat:   n = bm.nWhat;         break;
  case varRedrawHow:    n = bm.nHow;          break;
  case varRedrawX:      n = bm.xCell;         break;
  case varRedrawY:      n = bm.yCell;         break;
  case varSizeX:        n = NGetVariableW(vosSizeX);     break;
  case varSizeY:        n = NGetVariableW(vosSizeY);     break;
  case varPositionX:    n = NGetVariableW(vosPositionX); break;
  case varPositionY:    n = NGetVariableW(vosPositionY); break;
  case varScrollX:      n = ws.xScroll;       break;
  case varScrollY:      n = ws.yScroll;       break;
  case varDrawable:     n = ws.fResizeClient; break;
  case varWindowTop:    n = ws.fWindowTop;    break;
  case varWindowMenu:   n = ws.fWindowMenu;   break;
  case varWindowScroll: n = ws.fWindowScroll; break;
  case varColor:        n = bm.fColor;        break;
  case varNoDirty:      n = ws.fNoDirty;      break;
  case varPartUpdate:   n = ws.fAllowUpdate;  break;
  case varTraceDot:     n = gs.fTraceDot;     break;
  case varDisplayDelay: n = ws.nDisplayDelay; break;
  case varErrorCheck:   n = gs.fErrorCheck;   break;
  case varHourglass:    n = ws.fHourglass;    break;
  case varAutorepeat:   n = ws.fSpree;        break;
  case varTimerDelay:   n = ws.nTimerDelay;   break;
  case varTimePause:    n = ws.fTimePause;    break;
  case varSkipMessage:  n = ws.fIgnorePrint;  break;
  case varSound:        n = ws.fSound;        break;

  case varKvBorder:   n = dr.kvEdge;       break;
  case varKvPassage:  n = dr.kvOff;        break;
  case varKvWall:     n = dr.kvOn;         break;
  case varKvDot:      n = dr.kvDot;        break;
  case varKvMap:      n = dr.kvMap;        break;
  case varKvInWall:   n = dr.kvInWall;     break;
  case varKvSky:      n = dr.kvInSky;      break;
  case varKvGround:   n = dr.kvInDirt;     break;
  case varKvLine:     n = dr.kvInEdge;     break;
  case varKv3DWall:   n = dr.kvIn3D;       break;
  case varKv3DCeil:   n = dr.kvInCeil;     break;
  case varKvMountain: n = dr.kvInMtn;      break;
  case varKvCloud:    n = dr.kvInCld;      break;
  case varKvFog:      n = dr.kvInFog;      break;
  case varRndSpree:   n = dr.fSpreeRandom; break;

  case varRndSeed: n = ms.nRndSeed; break;
  case varRndBias: n = ms.nRndBias; break;
  case varRndRun:  n = ms.nRndRun;  break;

  case varX:          n = dr.x;            break;
  case varY:          n = dr.y;            break;
  case varZ:          n = dr.z;            break;
  case varDir:        n = dr.dir;          break;
  case varX2:         n = dr.x2;           break;
  case varY2:         n = dr.y2;           break;
  case varZ2:         n = dr.z2;           break;
  case varDir2:       n = dr.dir2;         break;
  case var3D:         n = dr.f3D;          break;
  case varDot:        n = dr.fDot;         break;
  case varCircle:     n = dr.fCircle;      break;
  case varArrow:      n = dr.fArrow;       break;
  case varDotSize:    n = dr.nDotSize;     break;
  case varWall:       n = dr.fWall;        break;
  case varNoCorner:   n = dr.fNoCorner;    break;
  case varGlance:     n = dr.fGlance;      break;
  case varChaseMouse: n = dr.fFollowMouse; break;
  case varNoDiagonal: n = dr.fNoDiagonal;  break;
  case varFollow:     n = dr.fFollow;      break;
  case varRadar:      n = ms.nRadar;       break;
  case varMove:       n = dr.cMove;        break;
  case varTwo:        n = dr.fTwo;         break;
  case varDrag:       n = dr.fDrag;        break;
  case varSet:        n = !dr.fSet;        break;
  case varBig:        n = dr.fBig;         break;
  case varBoth:       n = dr.fBoth;        break;
  case varOnTemp:     n = dr.fOnTemp;      break;
  case varEdge:       n = dr.nEdge;        break;

  case varInside:     n = dr.fInside ? dr.nInside : 0; break;
  case varCompass:    n = dr.fCompass;      break;
  case varCompassAng: n = dr.fCompassAngle; break;
  case varLocation:   n = dr.fLocation;     break;
  case varMap:        n = dr.fMap;          break;
  case varSealed:     n = dr.fSealEntrance; break;
  case varMountain:   n = dr.fMountain;     break;
  case varPeakHeight: n = dr.nPeakHeight;   break;
  case varCloud:      n = dr.cCloud;        break;
  case varSunMoon:    n = dr.fSunMoon;      break;
  case varRainbow:    n = dr.fRainbow;      break;
  case varStar:       n = ds.fStar;         break;
  case varStarCount:  n = ds.cStar;         break;
  case varMeteor:     n = dr.nMeteor;       break;
  case varNarrow:     n = dr.fNarrow;       break;
  case varCellSize:   n = dr.zCell;         break;
  case varNarrowSize: n = dr.zCellNarrow;   break;
  case varWallHeight: n = dr.zWall;         break;
  case varViewHeight: n = dr.zElev;         break;
  case varStepHeight: n = dr.zStep;         break;
  case varTexture:    n = dr.fTexture;      break;
  case varLighting:   n = (int)(dr.rLightFactor * 100.0); break;
  case varFogLength:  n = dr.nFog;          break;
  case varClipPlane:  n = dr.nClip;         break;
  case varViewSpan:   n = (int)(dr.dInside * 100.0);      break;
  case varStereo:     n = ds.nStereo;       break;
  case varTrans:      n = dr.nTrans;        break;
  case varFrameMove:  n = dr.nFrameXY;      break;
  case varFrameTurn:  n = dr.nFrameD;       break;
  case varFrameZ:     n = dr.nFrameZ;       break;
  case varSpeedMove:  n = dr.nSpeedXY;      break;
  case varSpeedTurn:  n = dr.nSpeedD;       break;
  case varSpeedZ:     n = dr.nSpeedZ;       break;
  case varOffsetX:    n = dr.nOffsetX;      break;
  case varOffsetY:    n = dr.nOffsetY;      break;
  case varOffsetZ:    n = dr.nOffsetZ;      break;
  case varOffsetD:    n = dr.nOffsetD;      break;
  case varZSmooth:    n = dr.f3DNotFree;    break;

  case var3W: n = BFocus().m_w3; break;
  case var3X: n = BFocus().m_x3; break;
  case var3Y: n = BFocus().m_y3; break;
  case var3Z: n = BFocus().m_z3; break;

  case varBlendType:  n = cs.fRainbowBlend;    break;
  case varBlendType2: n = cs.fRainbowBlend;    break;  // Bug: Should be !f
  case varRainStart:  n = cs.nRainbowStart;    break;
  case varRainDist:   n = cs.nRainbowDistance; break;
  case varBlend1:     n = cs.kvBlend1;         break;
  case varBlend2:     n = cs.kvBlend2;         break;

  case varPolish:     n = dr.fPolishMaze;       break;
  case varSection:    n = ms.fSection;          break;
  case varStartGoto:  n = ms.fTeleportEntrance; break;
  case varPoleBlock:  n = ms.fPoleNoDeadEnd;    break;
  case varSolvePixel: n = ms.fSolveEveryPixel;  break;
  case varSolveDot:   n = ms.fSolveDotExit;     break;
  case varRandomPath: n = ms.fRandomPath;       break;
  case varShortCount: n = ms.fCountShortest;    break;
  case varTweak:      n = ms.nTweakPassage;     break;
  case varMazeCount:  n = ms.cMaze;             break;
  case varWeaveOn:    n = ms.nWeaveOn;          break;
  case varWeaveOff:   n = ms.nWeaveOff;         break;
  case varWeaveRail:  n = ms.nWeaveRail;        break;
  case varStartPos:   n = ms.nEntrancePos;      break;

  case varLabyrinth:    n = ms.nLabyrinth;     break;
  case varCircuitShape: n = ms.lcs;            break;
  case varClassicSize:  n = ms.nClassical;     break;
  case varClassicCirc:  n = ms.fClassicalCirc; break;
  case varCustomPaths: *psz = ms.szCustom; *pcch = CchSz(ms.szCustom); break;
  case varAutoiterate:  n = ms.fCustomAuto;    break;
  case varNoRedundant:  n = ms.nCustomFold;    break;
  case varAsymmetry:    n = ms.nCustomAsym;    break;
  case varPartitionX:   n = ms.nCustomPartX;   break;
  case varPartitionY:   n = ms.nCustomPartY;   break;
  case varCustomAlign:  n = ms.lcc;            break;
  case varCustomCenter: n = ms.zCustomMiddle;  break;
  case varLabyrinthArc: n = ms.lcs - 1;        break;  // Version 2.2

  case varRiver:        n = ms.fRiver;        break;
  case varRiverEdge:    n = ms.fRiverEdge;    break;
  case varRiverFlow:    n = ms.fRiverFlow;    break;
  case varOmega:        n = ms.omega;         break;
  case varOmega2:       n = ms.omega2;        break;
  case varOmegaIn:      n = ms.omegas;        break;
  case varOmegaOut:     n = ms.omegaf;        break;
  case varOmegaDraw:    n = ms.nOmegaDraw;    break;
  case varPlanair: *psz = ms.szPlanair; *pcch = CchSz(ms.szPlanair); break;
  case varSegment: *psz = NULL; *pcch = 0; break;  // Can't be queried.
  case varTreeWall:     n = ms.fTreeWall;     break;
  case varTreeRandom:   n = ms.fTreeRandom;   break;
  case varTreeRiver:    n = ms.nTreeRiver;    break;
  case varForsInit:     n = ms.nForsInit;     break;
  case varForsAdd:      n = ms.nForsAdd;      break;
  case varSpiral:       n = ms.cSpiral;       break;
  case varSpiralWall:   n = ms.cSpiralWall;   break;
  case varRandomAdd:    n = ms.cRandomAdd;    break;
  case varFractalX:     n = ms.xFractal;      break;
  case varFractalY:     n = ms.yFractal;      break;
  case varFractalZ:     n = ms.zFractal;      break;
  case varFractalI:     n = ms.fFractalI;     break;
  case varCrackOff:     n = ms.fCrackOff;     break;
  case varCrackLength:  n = ms.nCrackLength;  break;
  case varCrackPass:    n = ms.nCrackPass;    break;
  case varCrackSector:  n = ms.nCrackSector;  break;
  case varBraid:        n = ms.nBraid;        break;
  case varSparse:       n = ms.nSparse;       break;
  case varKruskalPic:   n = ms.fKruskalPic;   break;
  case varWeaveCorner:  n = ms.fWeaveCorner;  break;
  case varTiltDiamond:  n = ms.fTiltDiamond;  break;
  case varTiltSize:     n = ms.nTiltSize;     break;

  case varXSize:       n = ds.horsiz;  break;
  case varYSize:       n = ds.versiz;  break;
  case varZSize:       n = ds.depsiz;  break;
  case varXBias:       n = ds.horbias; break;
  case varYBias:       n = ds.verbias; break;
  case varXStart:      n = ds.hormin;  break;
  case varYStart:      n = ds.vermin;  break;
  case varZStart:      n = ds.depmin;  break;
  case varArrowKeys:   n = ds.fArrow;  break;
  case varXVanish:     n = ds.horv;    break;
  case varYVanish:     n = ds.verv;    break;
  case varTheta:       n = ds.theta;   break;
  case varPhi:         n = ds.phi;     break;
  case varMotion:      n = ds.speedm;  break;
  case varRotation:    n = ds.speedr;  break;
  case varViewRight:   n = ds.fRight;  break;
  case varMergeBlocks: n = ds.fMerge;  break;

  case varReflect:     n = ds.fReflect;     break;
  case varShadeSky:    n = ds.fSkyShade;    break;
  case varShadeGround: n = ds.fGroundShade; break;
  case varDrawEdges:   n = ds.fEdges;       break;
  case varShadeObj:    n = ds.fShading;     break;
  case varTouchUps:    n = ds.fTouch;       break;
  case varBorder:      n = ds.nBorder;      break;
  case varHorizon:     n = ds.nHoriz;       break;
  case varDistance:    n = (int)ds.rScale;  break;
  case varXScale:      n = (int)ds.rxScale; break;
  case varYScale:      n = (int)ds.ryScale; break;
  case varZScale:      n = (int)ds.rzScale; break;
  case varKvEdge:      n = ds.kvTrim;       break;
  case varKvSkyHi:     n = ds.kvSkyHi;      break;
  case varKvSkyLo:     n = ds.kvSkyLo;      break;
  case varKvGroundHi:  n = ds.kvGroundHi;   break;
  case varKvGroundLo:  n = ds.kvGroundLo;   break;
  case varKvObject:    n = ds.kvObject;     break;
  case varXLight:      n = (int)ds.vLight.m_x; break;
  case varYLight:      n = (int)ds.vLight.m_y; break;
  case varZLight:      n = (int)ds.vLight.m_z; break;

  case varScreenSaver:   n = ws.fSaverRun;     break;
  case varTitleMessage:  n = ws.nTitleMessage; break;
  case varCopyMessage:   n = ws.fCopyMessage;  break;
  case varSkipMessage2:  n = ws.nIgnorePrint;  break;
  case varXTurtle:       n = gs.turtlex;       break;
  case varYTurtle:       n = gs.turtley;       break;
  case varTurtleS:       n = gs.turtles;       break;
  case varDot2:          n = dr.fDot2;         break;
  case varNoMouseDrag:   n = dr.fNoMouseDrag;  break;
  case varAutorepeat2:   n = ws.wCmdSpree - icmdBase; break;
  case varFogFloor:      n = dr.fFogFloor;     break;
  case varKvDot2:        n = dr.kvDot2;        break;
  case varKvSun:         n = dr.kvSun;         break;
  case varKvMoon:        n = dr.kvMoon;        break;
  case varKvWall2:       n = dr.kvInWall2;     break;
  case varKvSky2:        n = dr.kvInSky2;      break;
  case varKvGround2:     n = dr.kvInDirt2;     break;
  case varKv3DCeil2:     n = dr.kvInCeil2;     break;
  case varKvMountain2:   n = dr.kvInMtn2;      break;
  case varKvCloud2:      n = dr.kvInCld2;      break;
  case varRainbowSize:   n = dr.nRainbow;      break;
  case varGroundZ:       n = dr.zGround;       break;
  case varHorizonY:      n = dr.yHorizon;      break;
  case varSunMoon2:      n = dr.nSunMoon;      break;
  case varSunMoonY:      n = dr.ySunMoon;      break;
  case varStarColor:     n = ds.lStarColor;    break;
  case varStarSize:      n = ds.nStarSize;     break;
  case varSkyAll:        n = ds.fSkyAll;       break;
  case varSky3D:         n = dr.fSky3D;        break;
  case varShowDelay:     n = dr.fDelay;        break;
  case varDelay:         n = dr.nDelay;        break;
  case varNoCompass:     n = dr.fNoCompass;    break;
  case varNoLocation:    n = dr.fNoLocation;   break;
  case varNoSubmerge:    n = dr.fNoSubmerge;   break;
  case varStepDown:      n = dr.zStep2;        break;
  case varRedrawAfter:   n = dr.fRedrawAfter;  break;
  case varMarkColor:     n = dr.fMarkColor;    break;
  case varMarkAll:       n = dr.fMarkAll;      break;
  case varMarkBlock:     n = dr.fMarkBlock;    break;
  case varMark2:         n = dr.nMarkSky;      break;
  case varMarkElev:      n = dr.nMarkElev;     break;
  case varWallVar:       n = dr.nWallVar;      break;
  case varFogLit:        n = dr.nFogLit;       break;
  case varTextureBack:   n = dr.nTexture;      break;
  case varTextureBack2:  n = dr.nTexture2;     break;
  case varTextureWall:   n = dr.nTextureWall;  break;
  case varTextureGround: n = dr.nTextureDirt;  break;
  case varTextureVar:    n = dr.nTextureVar;   break;
  case varTextureElev:   n = dr.nTextureElev;  break;
  case varTextureDual:   n = dr.fTextureDual;  break;
  case varTextureDual2:  n = dr.fTextureDual2; break;
  case varTextureBlend:  n = dr.fTextureBlend; break;
  case varMarkElevX1:    n = dr.nMarkElevX1;   break;
  case varMarkElevY1:    n = dr.nMarkElevY1;   break;
  case varMarkElevX2:    n = dr.nMarkElevX2;   break;
  case varMarkElevY2:    n = dr.nMarkElevY2;   break;
  case varMazeCellMax:   n = ms.nCellMax;      break;
  case varHuntType:      n = ms.nHuntType;     break;
  case varFractalDepth:  n = ms.nFractalD;     break;
  case varFractalLength: n = ms.nFractalL;     break;
  case varFractalType:   n = ms.nFractalT;     break;
  case varStretch:       n = dr.nStretchMode;  break;
  case varGraphNumber:   n = cs.fGraphNumber;  break;
  case varGrayscale:     n = cs.lGrayscale;    break;
  case varArrowFree:     n = ds.fArrowFree;    break;
  case varTransparency:  n = ds.nTrans;        break;
  case varFogLength2:    n = ds.nFog;          break;
  case varLineWidth:     n = ds.nWireWidth;    break;
  case varLineSort:      n = ds.fWireSort;     break;
  case varLineDistance:  n = ds.nWireDistance; break;
  case varFaceOrigin:    n = ds.nFaceOrigin;   break;
  case varStereo3D:      n = ds.fStereo3D;     break;
  case varWireCount:     n = bm.ccoor;         break;
  case varPatchCount:    n = bm.cpatch;        break;
  case varMandelbrot:    n = cs.fMandelbrot;   break;
  case varSoundDelay:    n = ws.nSoundDelay;   break;
  case varFileLock:      n = ws.nFileLock;     break;
  case varRndOld:        n = us.fRndOld;       break;
  case varNoExit:        n = ws.fNoExit;       break;
  case varAlloc:         n = us.cAlloc;        break;
  case varAllocTotal:    n = us.cAllocTotal;   break;
  case varAllocSize:     n = us.cAllocSize;    break;

  default:
    PrintSzN_E("Getting variable %d is undefined.", ivar);
  }

  *pl = n;
}


// Evaluate a function, returning the number it evaluates to, given a list of
// string and numeric parameters to the function.

long EvalFunction(int ifun, char *rgsz[], CONST int *rgcch, CONST long *rgl)
{
  int n = 0, n1, n2, n3, n4;
  char sz[cchSzMax];

  n1 = (int)rgl[0]; n2 = (int)rgl[1]; n3 = (int)rgl[2]; n4 = (int)rgl[3];

  switch (ifun) {
  case funFalse: n = fFalse; break;
  case funTrue:  n = fTrue;  break;
  case funOff:   n = fFalse; break;
  case funOn:    n = fTrue;  break;
  case funAdd:   n = n1 + n2;  break;
  case funSub:   n = n1 - n2;  break;
  case funMul:   n = n1 * n2;  break;
  case funDiv:   n = LDiv(n1, n2); break;
  case funMod:   n = LMod(n1, n2); break;
  case funNeg:   n = -n1;      break;
  case funInc:   n = n1 + 1;   break;
  case funDec:   n = n1 - 1;   break;
  case funPow:   n = LPower(n1, n2); break;
  case funEqu:   n = n1 == n2; break;
  case funNeq:   n = n1 != n2; break;
  case funLt:    n = n1 < n2;  break;
  case funGt:    n = n1 > n2;  break;
  case funLte:   n = n1 <= n2; break;
  case funGte:   n = n1 >= n2; break;
  case funNot:   n = !n1;      break;
  case funAnd:   n = n1 & n2;  break;
  case funOr:    n = n1 | n2;  break;
  case funXor:   n = n1 ^ n2;  break;
  case funInv:   n = ~n1;      break;
  case funShftL: n = n1 << n2; break;
  case funShftR: n = n1 >> n2; break;
  case funAndL:  n = n1 && n2; break;
  case funOrL:   n = n1 || n2; break;
  case funOdd:   n = FOdd(n1); break;
  case funAbs:   n = NAbs(n1); break;
  case funSgn:   n = NSgn(n1); break;
  case funMin:   n = Min(n1, n2); break;
  case funMax:   n = Max(n1, n2); break;
  case funTween: n = FBetween(n1, n2, n3); break;
  case funRnd:   n = Rnd(n1, n2); break;
  case funIf:    n = n1 ? n2 : n3; break;
  case funSqr:   n = NSqr(NAbs(n1)); break;
  case funDist:  n = (int)LengthN(n1, n2, n3, n4); break;
  case funLn:    n = (int)((real)n1 * log((real)n2)); break;
  case funSin:   n = (int)(((real)n1 + rRound) * RSinD((real)n2)); break;
  case funCos:   n = (int)(((real)n1 + rRound) * RCosD((real)n2)); break;
  case funTan:   n = (int)(((real)n1 + rRound) * RTanD((real)n2)); break;
  case funAsin:  n = (int)(RAsinD((real)n1 / (real)n2) + rRound); break;
  case funAcos:  n = (int)(RAcosD((real)n1 / (real)n2) + rRound); break;
  case funAtan:  n = (int)(RAtnD ((real)n1 / (real)n2) + rRound); break;
  case funAng:   n = (int)GetAngle(0, 0, n1, n2); break;
  case funSin2:  n = (int)(((real)n1 + rRound) *
    RSinD((real)n2 / (real)n3 * rDegMax)); break;
  case funCos2:  n = (int)(((real)n1 + rRound) *
    RCosD((real)n2 / (real)n3 * rDegMax)); break;
  case funTan2:  n = (int)(((real)n1 + rRound) *
    RTanD((real)n2 / (real)n3 * rDegMax)); break;
  case funAsin2: n = (int)(RAsinD((real)n2 / (real)n3) /
    rDegMax * (real)n1 + rRound); break;
  case funAcos2: n = (int)(RAcosD((real)n2 / (real)n3) /
    rDegMax * (real)n1 + rRound); break;
  case funAtan2: n = (int)(RAtnD ((real)n2 / (real)n3) /
    rDegMax * (real)n1 + rRound); break;
  case funAng2:  n = (int)(GetAngle(0, 0, n1, n2) * (real)n3 / rDegMax);
    break;

  case funLen:   n = rgcch[0]; break;
  case funChar:  n = n2 >= rgcch[0] ? -1 : rgsz[0][n2]; break;
  case funNum:   n = LFromRgch(rgsz[0], rgcch[0]); break;
  case funReal:  n = LFromRgch2(rgsz[0], rgcch[0], n2); break;
  case funEquS:  n = rgcch[0] == rgcch[1] &&
    CompareRgch(rgsz[0], rgcch[0], rgsz[1], rgcch[1]) == 0; break;
  case funNeqS:  n = rgcch[0] != rgcch[1] ||
    CompareRgch(rgsz[0], rgcch[0], rgsz[1], rgcch[1]) != 0; break;
  case funLtS:
    n = CompareRgch(rgsz[0], rgcch[0], rgsz[1], rgcch[1]) < 0; break;
  case funGtS:
    n = CompareRgch(rgsz[0], rgcch[0], rgsz[1], rgcch[1]) > 0; break;
  case funLteS:
    n = CompareRgch(rgsz[0], rgcch[0], rgsz[1], rgcch[1]) <= 0; break;
  case funGteS:
    n = CompareRgch(rgsz[0], rgcch[0], rgsz[1], rgcch[1]) >= 0; break;
  case funEquSI: n = rgcch[0] == rgcch[1] &&
    CompareRgchI(rgsz[0], rgcch[0], rgsz[1], rgcch[1]) == 0; break;
  case funNeqSI: n = rgcch[0] != rgcch[1] ||
    CompareRgchI(rgsz[0], rgcch[0], rgsz[1], rgcch[1]) != 0; break;
  case funLtSI:
    n = CompareRgchI(rgsz[0], rgcch[0], rgsz[1], rgcch[1]) < 0; break;
  case funGtSI:
    n = CompareRgchI(rgsz[0], rgcch[0], rgsz[1], rgcch[1]) > 0; break;
  case funLteSI:
    n = CompareRgchI(rgsz[0], rgcch[0], rgsz[1], rgcch[1]) <= 0; break;
  case funGteSI:
    n = CompareRgchI(rgsz[0], rgcch[0], rgsz[1], rgcch[1]) >= 0; break;
  case funInCh:
    if (n2 == ~0)
      n2 = rgsz[1][0];
    n = FindCh(rgsz[0], rgcch[0], n2); break;

  case funBX:     n = bm.b.m_x; break;
  case funBY:     n = bm.b.m_y; break;
  case funCX:     n = bm.k.FNull() ? 0 : bm.k.m_x; break;
  case funCY:     n = bm.k.FNull() ? 0 : bm.k.m_y; break;
  case funGet:    n = bm.b.Get(n1, n2); break;
  case funGetT:   n = bm.b2.GetSafe(n1, n2); break;
  case funGetE:   n = bm.b3.GetSafe(n1, n2); break;
  case funGetA:   n = ws.rgbMask == NULL || ws.cbMask <= n1 ? fOff :
    ws.rgbMask[n1].GetSafe(n2, n3); break;
  case funGetC:   n = bm.k.GetSafe(n1, n2);  break;
  case funGetCT:  n = bm.k2.GetSafe(n1, n2); break;
  case funGetCE:  n = bm.k3.GetSafe(n1, n2); break;
  case funGetCA:  n = ws.rgcTexture == NULL || ws.ccTexture <= n1 ? kvBlack :
    ws.rgcTexture[n1].GetSafe(n2, n3); break;
  case funGetX:   n = ws.rgcTexture == NULL || ws.rgcTexture[0].FNull() ? 0 :
    (n3 < 0 ? ws.rgcTexture[0].Get(n1, n2) :
    GetT(ws.rgcTexture[0], n1, n2, n3)); break;
  case funGetH:   n = dr.nWallVar >= 0 && ws.cbMask > dr.nWallVar &&
    ws.ccTexture > dr.nWallVar && !ws.rgbMask[dr.nWallVar].FNull() &&
    !ws.rgcTexture[dr.nWallVar].FNull() && ws.rgbMask[dr.nWallVar].Get(
    n1, n2) ? ZEye(UdU(ws.rgcTexture[dr.nWallVar].Get(n1, n2))) : 0; break;
  case funGetM:   n = dr.nMarkElev < 0 || ws.rgcTexture == NULL ||
    ws.ccTexture <= dr.nMarkElev ? kvBlack :
    ws.rgcTexture[dr.nMarkElev].GetSafe(n1, n2); break;
  case funGet3:   n = bm.b.Get3(n1, n2, n3); break;
  case funGet3T:  n = bm.b2.Get3Safe(n1, n2, n3); break;
  case funGet3E:  n = bm.b3.Get3Safe(n1, n2, n3); break;
  case funGet3A:  n = ws.rgbMask == NULL || ws.cbMask <= n1 ? fOff :
    ws.rgbMask[n1].Get3Safe(n2, n3, n4); break;
  case funGet3C:  n = bm.k.Get3Safe(n1, n2, n3);  break;
  case funGet3CT: n = bm.k2.Get3Safe(n1, n2, n3); break;
  case funGet3CE: n = bm.k3.Get3Safe(n1, n2, n3); break;
  case funGet3CA: n = ws.rgcTexture == NULL || ws.ccTexture <= n1 ? kvBlack :
    ws.rgcTexture[n1].Get3Safe(n2, n3, n4); break;
  case funGet3X:  n = ws.rgcTexture == NULL || ws.rgcTexture[0].FNull() ? 0 :
    (n4 < 0 ? ws.rgcTexture[0].Get3(n1, n2, n3) :
    GetT3(ws.rgcTexture[0], n1, n2, n3, n4)); break;
  case funGet3H:  n = dr.nWallVar >= 0 && ws.cbMask > dr.nWallVar &&
    ws.ccTexture > dr.nWallVar && !ws.rgbMask[dr.nWallVar].FNull() &&
    !ws.rgcTexture[dr.nWallVar].FNull() && ws.rgbMask[dr.nWallVar].Get3(
    n1, n2, n3) ? ZEye(UdU(ws.rgcTexture[dr.nWallVar].Get3(n1, n2, n3))) : 0;
    break;
  case funGet3M:  n = dr.nMarkElev < 0 || ws.rgcTexture == NULL ||
    ws.ccTexture <= dr.nMarkElev ? kvBlack :
    ws.rgcTexture[dr.nMarkElev].Get3Safe(n1, n2, n3); break;
  case funPixel:  n = bm.b.BitmapCount();             break;
  case funRGB:    n = Rgb(n1, n2, n3);                break;
  case funRGBR:   n = RgbR(n1);                       break;
  case funRGBG:   n = RgbG(n1);                       break;
  case funRGBB:   n = RgbB(n1);                       break;
  case funGray:   n = GrayN(n1);                      break;
  case funLight:  n = KvShade(n1, 0.5);               break;
  case funDark:   n = KvShade(n1, -0.5);              break;
  case funShade:  n = KvShade(n1, (real)n2 / 100.0);  break;
  case funBlend:  n = KvBlend(n1, n2);                break;
  case funBlend2: n = KvBlendN(n1, n2, n3, n4);       break;
  case funHue:    n = HueD(n1);                       break;
  case funHSL:    n = Hsl(n1, n2, n3);                break;
  case funDlg:    n = KvDialog();                     break;
  case funNWSE:   n = NWSE(n1, n2, n3, n4);           break;
  case funUD:     n = UD(n1, n2);                     break;
  case funUDU:    n = UdU(n1);                        break;
  case funUDD:    n = UdD(n1);                        break;
  case funTimer:  n = (NGetVariableW(vosGetTimer) + 500) / 1000; break;
  case funTimer2: n = NGetVariableW(vosGetTimer);     break;
  case funVar:    n = LVar(n1);                       break;
  case funCmd:    n = CmdFromRgch(rgsz[0], rgcch[0]); break;
  case funEval:
    CopyRgchToSz(rgsz[0], rgcch[0], sz, cchSzMax);
    PchGetParameter(sz, NULL, NULL, (long *)&n, 1);
    break;
  case funEvent:  n = NGetVariableW(n1 ? vosEventMouse : vosEventKey); break;
  case funVer:    n = 3400;                           break;  // Daedalus 3.4

  case funFileOpen:
    CopyRgchToSz(rgsz[0], rgcch[0], sz, cchSzMax);
    n = (int)(size_t)FileOpen(sz, n2 == 0 ? "rb" : (n2 == 1 ? "wb" : "ab"));
    break;
  case funFileNum:  fscanf((FILE *)(size_t)n1, "%ld", &n); break;
  case funFileByte: n = getc((FILE *)(size_t)n1);          break;
  case funFileEnd:  n = feof((FILE *)(size_t)n1) != 0;     break;
  default:
    PrintSzN_E("Function %d is undefined.", ifun);
  }

  return n;
}


// Read a parameter to an action from a command line, given the current
// position into the command line string. Return the evaluation of the
// parameter in either a string or numeric return variable, or null on error.
// Also update the command line position to point after the parameter.

char *PchGetParameter(char *pchCur, char **psz, int *pcch, long *pl,
  int iParam)
{
  char sz[cchSzMax*2], szT[cchSzMax], ch1, ch2;
  char *rgsz[4], *pchParam, *pchT;
  long rgl[4];
  int rgcch[4], ivar, ifun, iParamT, cch, i;
  flag fQuote;

  // Skip whitespace.
  while (*pchCur == ' ')
    pchCur++;
  if (*pchCur == chNull) {
    if (iParam < 0)
      PrintSz_W("Couldn't get variable value due to end of line.\n");
    else
      PrintSzN_W("Couldn't get parameter %d due to end of line.\n", iParam+1);
    return NULL;
  }

  // Get parameter string.
  for (pchParam = pchCur, ch1 = *pchCur;; pchCur++) {
    while (*pchCur && *pchCur != ' ')
      pchCur++;
    if (*pchCur == chNull)
      break;
    if (pchParam[1] > ' ') {
      ch2 = *(pchCur-1);
      if (ch1 == '"' && ch2 != '"')
        continue;
      if (ch1 == '\'' && ch2 != '\'')
        continue;
      if (ch1 == '{' && ch2 != '}')
        continue;
      if (ch1 == '(' && ch2 != ')')
        continue;
    }
    break;
  }
  cch = PD(pchCur - pchParam);
  ch2 = *(pchCur-1);

  // Check for a real string. String constants are surrounded with double
  // quotes, single quotes, curly braces, or parentheses. Either way, all
  // parameters are terminated with either a space or end of string.
  fQuote = cch >= 2 &&
    ((ch1 == '"' && ch2 == '"') || (ch1 == '\'' && ch2 == '\'') ||
    (ch1 == '{' && ch2 == '}') || (ch1 == '(' && ch2 == ')'));
  if (fQuote) {
    pchParam++;
    cch -= 2;
  }
  if (psz != NULL)
    *psz = pchParam;
  if (pcch != NULL)
    *pcch = cch;
  *pl = ~0;

  // Evaluate the parameter. First check for numeric constant.
  if (FDigitCh(*pchParam) ||
    ((*pchParam == '-' || *pchParam == '#') && cch > 1)) {
    *pl = LFromRgch(pchParam, cch);
    goto LDone;
  }

  // Check for custom numeric or custom string variable.
  ch1 = pchParam[0];
  if ((ch1 == '@' || ch1 == '$') && !fQuote) {
    if (cch < 2)
      goto LBad;
    i = ChCap(pchParam[1]);
    if (cch == 2 && FCapCh(i)) {
      ivar = i - '@';
      if (ch1 == '$')
        ivar = LVar(ivar);
    } else if (FDigitCh(pchParam[1])) {
      ivar = 0;
      for (i = 1; i < cch; i++) {
        if (!FDigitCh(pchParam[i]))
          goto LBad;
        ivar = ivar * 10 + (pchParam[i] - '0');
      }
    } else {
      if (ws.rgsTrieConst == NULL)
        goto LBad;
      ivar = ILookupTrie(ws.rgsTrieConst, pchParam+1, cch-1, fFalse);
      if (ivar < 0)
        goto LBad;
      ivar = ws.rgnTrieConst[ivar];
    }
    if (ch1 == '@')
      *pl = LVar(ivar);
    else {
      if (FSzVar(ivar)) {
        *psz = ws.rgszVar[ivar];
        *pcch = CchSz(ws.rgszVar[ivar]);
      } else {
        *psz = NULL;
        *pcch = 0;
      }
    }
    goto LDone;
  }

  // Check for custom constant or function
  if ((ch1 == '%' || ch1 == '*') && !fQuote) {
    if (ws.rgsTrieConst == NULL)
      goto LBad;
    ivar = ILookupTrie(ws.rgsTrieConst, pchParam+1, cch-1, fFalse);
    if (ivar < 0)
      goto LBad;
    if (ch1 == '%')
      *pl = ws.rgnTrieConst[ivar];
    else {
      RunMacro(ws.rgnTrieConst[ivar]);
      *pl = ws.rglVar[iLetterZ];
    }
    goto LDone;
  }

  // Check for variable.
  ivar = ILookupTrie(ws.rgsTrieVar, pchParam, cch, fTrue);
  if (ivar >= 0) {
    GetVariable(ivar, psz, pcch, pl);
    goto LDone;
  }

  // Check for function.
  ifun = ILookupTrie(ws.rgsTrieFun, pchParam, cch, fTrue);
  if (ifun >= 0) {

    // Recursively get the parameters to the function.
    for (iParamT = 0; iParamT < rgfun[ifun].nParam; iParamT++) {
      pchCur = PchGetParameter(pchCur, &rgsz[iParamT], &rgcch[iParamT],
        &rgl[iParamT], iParamT);
      if (pchCur == NULL)
        return NULL;
    }
    *pl = EvalFunction(ifun, rgsz, rgcch, rgl);
    goto LDone;
  }

  // Check for color constant.
  i = ILookupTrie(ws.rgsTrieKv, pchParam, cch, fTrue);
  if (i >= 0) {
    *pl = rgkv[i];
    goto LDone;
  }

  if (!fQuote) {
LBad:
    CopyRgchToSz(pchParam, cch, szT, cchSzMax);
    sprintf(S(sz), "Unknown parameter: '%s'\nContext: ", szT);
    for (pchParam = sz; *pchParam; pchParam++)
      ;
    pchT = pchCur;
    while (*pchT && pchParam - sz < cchSzMax)
      *pchParam++ = *pchT++;
    *pchParam = chNull;
    PrintSz_W(sz);
    return NULL;
  }
LDone:
  return pchCur;
}


#define nRetHalt 10000

// Execute an operation given a list of parameters.
// Return values: -1 = Caller should restart command line, 0 = Success, 1+ =
// Caller(s) should break out of command line the specified number of times.

int DoOperation(int iopr, char **rgsz, CONST int *rgcch, CONST long *rgl,
  FILE *file)
{
  size_t cursorPrev = NULL;
  CMap3 *pbSrc, *pbDst, *bT;
  STAR *pstar;
  char sz[cchSzOpr], szT[cchSzMax], sz3[cchSzDef];
  char *pch, *pchT;
  int nRet = 0, n1, n2, n3, n4, n5, n6, n7, x, y, cch, i;
  long l;
  real rx, ry;
  KV kv1, kv2;

  // Do things before the operation runs based on operation flags.
  if ((rgopr[iopr].grf & fCmtRequireTemp) > 0 &&
    (bm.fColor ? bm.k2.FNull() : bm.b2.FNull()))
    return nRet;
  if ((rgopr[iopr].grf & fCmtRequireExtra) > 0 &&
    (bm.fColor ? bm.k3.FNull() : bm.b3.FNull()))
    return nRet;
  if ((rgopr[iopr].grf & fCmtRequireColor) > 0 && bm.k.FNull())
    return nRet;
  if ((rgopr[iopr].grf & fCmtHourglass) > 0 &&
    (ws.fHourglass || iopr == oprDoWait))
    HourglassCursor(&cursorPrev, fTrue);
  if (rgopr[iopr].grf & fCmtSet2D) {
    if (dr.f3D) {
      dr.f3D = fFalse;
      SystemHook(hos3DOff);
    }
  } else if (rgopr[iopr].grf & fCmtSet3D) {
    if (!dr.f3D) {
      dr.f3D = fTrue;
      SystemHook(hos3DOn);
    }
  }
  if (rgopr[iopr].grf & fCmtBitmapBefore)
    FShowColmap(fFalse);
  else if (rgopr[iopr].grf & fCmtColmapBefore)
    FShowColmap(fTrue);
  if (rgopr[iopr].grf & fCmtString)
    CopyRgchToSz(rgsz[0], rgcch[0], sz, cchSzOpr);

  n1 = (int)rgl[0]; n2 = (int)rgl[1]; n3 = (int)rgl[2]; n4 = (int)rgl[3];
  n5 = (int)rgl[4]; n6 = (int)rgl[5]; n7 = (int)rgl[6];
  switch (iopr) {

  // Dialog operations

  case oprOpen:
    FFileOpen(cmdOpen, sz, NULL);
    break;
  case oprOpenBitmap:
    FFileOpen(cmdOpenBitmap, sz, NULL);
    break;
  case oprOpenText:
    FFileOpen(cmdOpenText, sz, NULL);
    break;
  case oprOpenXbm:
    FFileOpen(cmdOpenXbm, sz, NULL);
    break;
  case oprOpen3D:
    FFileOpen(cmdOpen3D, sz, NULL);
    break;
  case oprOpenDB:
    FFileOpen(cmdOpenDB, sz, NULL);
    break;
  case oprOpenScript:
    FFileOpen(cmdOpenScript, sz, NULL);
    break;
  case oprOpenTarga:
    FFileOpen(cmdOpenColmapTarga, sz, NULL);
    break;
  case oprOpenPaint:
    FFileOpen(cmdOpenColmapPaint, sz, NULL);
    break;
  case oprOpenWire:
    FFileOpen(cmdOpenWire, sz, NULL);
    break;
  case oprOpenPatch:
    FFileOpen(cmdOpenPatch, sz, NULL);
    break;

  case oprDlgDB:
    FFileOpen(cmdOpenDB, NULL, NULL);
    break;
  case oprDlgPaint:
    FFileOpen(cmdOpenColmapPaint, NULL, NULL);
    break;

  case oprEmbedXbm:
    FFileOpen(cmdOpenXbm, NULL, file);
    break;
  case oprEmbed3D:
    FFileOpen(cmdOpen3D, NULL, file);
    break;
  case oprEmbedDW:
    FFileOpen(cmdOpenWire, NULL, file);
    break;
  case oprEmbedDP:
    FFileOpen(cmdOpenPatch, NULL, file);
    break;
  case oprEmbedDB:
    FFileOpen(cmdOpenDB, NULL, file);
    break;

  case oprSaveBitmap:
    FFileSave(cmdSaveBitmap, sz);
    break;
  case oprSaveText:
    FFileSave(cmdSaveText, sz);
    break;
  case oprSaveDOS:
    FFileSave(cmdSaveDOS, sz);
    break;
  case oprSaveDOS2:
    FFileSave(cmdSaveDOS2, sz);
    break;
  case oprSaveDOS3:
    FFileSave(cmdSaveDOS3, sz);
    break;
  case oprSaveXbmN:
    FFileSave(cmdSaveXbmN, sz);
    break;
  case oprSaveXbmC:
    FFileSave(cmdSaveXbmC, sz);
    break;
  case oprSaveXbmS:
    FFileSave(cmdSaveXbmS, sz);
    break;
  case oprSave3DN:
    FFileSave(cmdSave3DN, sz);
    break;
  case oprSave3DC:
    FFileSave(cmdSave3DC, sz);
    break;
  case oprSave3DS:
    FFileSave(cmdSave3DS, sz);
    break;
  case oprSaveDB:
    FFileSave(cmdSaveDB, sz);
    break;
  case oprSaveTarga:
    FFileSave(cmdSaveColmapTarga, sz);
    break;
  case oprSaveOverview:
    FFileSave(cmdSaveWire, sz);
    break;
  case oprSavePatch:
    FFileSave(cmdSavePatch, sz);
    break;
  case oprSavePicture:
    FFileSave(cmdSavePicture, sz);
    break;
  case oprSaveVector:
    FFileSave(cmdSaveVector, sz);
    break;
  case oprSaveSolids:
    CreateSolids(sz);
    break;

  case oprSize:
  case oprSizeC:
    DoSize(n1, n2, n3, n4);
    break;
  case oprZoom:
    if (!n3) {
      rx = (real)n1; ry = (real)n2;
      if (rx < 0.0)
        rx = -1.0 / rx;
      if (ry < 0.0)
        ry = -1.0 / ry;
      if (n4 < 2) {
        if (!bm.fColor)
          bm.b.FBitmapZoomBy(rx, ry, n4);
        else
          bm.k.FBitmapZoomBy(rx, ry, n4);
      } else {
        if (!bm.fColor)
          bm.b.FBitmapTessellateBy(rx, ry);
        else
          bm.k.FBitmapTessellateBy(rx, ry);
      }
    } else {
      x = n1; y = n2;
      if (x < 0)
        x = NAbs(bm.b.m_x + x);
      if (y < 0)
        y = NAbs(bm.b.m_y + y);
      if (n4 < 2) {
        if (!bm.fColor)
          bm.b.FBitmapZoomTo(x, y, n4);
        else
          bm.k.FBitmapZoomTo(x, y, n4);
      } else {
        if (!bm.fColor)
          bm.b.FBitmapTessellateTo(x, y);
        else
          bm.k.FBitmapTessellateTo(x, y);
      }
    }
    break;
  case oprFlip3D:
    PbFocus()->FCubeFlip2(n1, n2);
    break;
  case oprSizeMaze:
  case oprSizeMazeC:
    i = n3 >= 2;
    x = ((n1 + 1) << 1) - i;
    y = ((n2 + 1) << 1) - i;
    DoSize(x, y, fFalse, FOdd(n3));
    break;
  case oprBias:
    if (!bm.fColor)
      bm.b.FBitmapBias(n1, n2, n3, n4);
    else
      bm.k.FBitmapBias(n1, n2, n3, n4);
    break;
  case oprReplace:
    kv1 = n1; kv2 = n2;
    if (kv2 == ~0) {
      CopyRgchToSz(rgsz[1], rgcch[1], sz, cchSzOpr);
      kv2 = ParseColor(sz, !n3);
    }
    if (!n3)
      bm.k.ColmapReplace(kv1, kv2, dr.x, dr.y, dr.x2, dr.y2);
    else
      bm.k.ColmapExchange(kv1, kv2);
    break;
  case oprBrightness:
    if (n2 < 4)
      bm.k.ColmapBrightness(n2, (real)n1 / 100.0, n1);
    else
      bm.k.FColmapTransform(n2 - 4, n1);
    break;

  // Control flow operations

  case oprIf:
    if (n1) {
      CopyRgchToSz(rgsz[1], rgcch[1], sz, cchSzOpr);
      nRet = RunCommandLine(sz, file);
    }
    break;
  case oprIfElse:
    i = 2 - (n1 != 0);
    CopyRgchToSz(rgsz[i], rgcch[i], sz, cchSzOpr);
    nRet = RunCommandLine(sz, file);
    break;
  case oprSwitch:
    i = NSgn(n1) + 2;
    CopyRgchToSz(rgsz[i], rgcch[i], sz, cchSzOpr);
    nRet = RunCommandLine(sz, file);
    break;
  case oprDoCount:
    CopyRgchToSz(rgsz[1], rgcch[1], sz, cchSzOpr);
    for (i = 0; i < n1; i++) {
      nRet = RunCommandLine(sz, file);
      if (nRet > 0) {
        nRet--;
        break;
      }
    }
    break;
  case oprWhile:
    CopyRgchToSz(rgsz[1], rgcch[1], sz, cchSzOpr);
    CopyRgchToSz(rgsz[0], rgcch[0], szT, cchSzMax);
    loop {
      pchT = szT;
      if (PchGetParameter(pchT, &pch, &cch, &l, 1) == NULL)
        break;
      if (!l)
        break;
      nRet = RunCommandLine(sz, file);
      if (nRet > 0) {
        nRet--;
        break;
      }
    }
    break;
  case oprDoWhile:
    CopyRgchToSz(rgsz[1], rgcch[1], sz, cchSzOpr);
    CopyRgchToSz(rgsz[0], rgcch[0], szT, cchSzMax);
    loop {
      nRet = RunCommandLine(sz, file);
      if (nRet > 0) {
        nRet--;
        break;
      }
      pchT = szT;
      if (PchGetParameter(pchT, &pch, &cch, &l, 1) == NULL)
        break;
      if (!l)
        break;
    }
    break;
  case oprFor:
    CopyRgchToSz(rgsz[3], rgcch[3], sz, cchSzOpr);
    if (n1 == ~0)
      n1 = ChCap(rgsz[0][0]) - '@';
    if (!FEnsureLVar(n1 + 1))
      break;
    for (ws.rglVar[n1] = n2; ws.rglVar[n1] <= n3; ws.rglVar[n1]++) {
      nRet = RunCommandLine(sz, file);
      if (nRet > 0) {
        nRet--;
        break;
      }
    }
    break;
  case oprForStep:
    CopyRgchToSz(rgsz[4], rgcch[4], sz, cchSzOpr);
    if (n1 == ~0)
      n1 = ChCap(rgsz[0][0]) - '@';
    if (!FEnsureLVar(n1 + 1))
      break;
    for (ws.rglVar[n1] = n2; ws.rglVar[n1] >= Min(n2, n3) &&
      ws.rglVar[n1] <= Max(n2, n3); ws.rglVar[n1] += n4) {
      nRet = RunCommandLine(sz, file);
      if (nRet > 0) {
        nRet--;
        break;
      }
    }
    break;
  case oprComment:
    nRet = 1;
    break;
  case oprContinue:
    nRet = 2;
    break;
  case oprBreak:
    nRet = 3;
    break;
  case oprReturn:
    ws.fReturning = fTrue;
    // Fall through
  case oprHalt:
    nRet = nRetHalt;
    break;
  case oprRestart:
    nRet = -1;
    break;
  case oprDo:
  case oprDoWait:
    nRet = RunCommandLine(sz, file);
    break;

  // Daedalus specific operations

  case oprMacroDefine:
    CopyRgchToSz(rgsz[2], rgcch[2], sz, cchSzOpr);
    DoDefineMacro(n1, rgsz[1], rgcch[1], sz);
    break;
  case oprMacro:
    nRet = RunMacro(n1);
    if (nRet > 0)
      nRet--;
    break;
  case oprMacroEmbed:
    CopyRgchToSz(rgsz[1], rgcch[1], sz, cchSzOpr);
    pch = ReadEmbedLines(file);
    if (pch != NULL) {
      DoDefineMacro(n1, pch, CchSz(pch), sz);
      DeallocateP(pch);
    }
    break;
  case oprDoEmbed:
    pch = ReadEmbedLines(file);
    if (pch != NULL) {
      RunCommandLine(pch, file);
      DeallocateP(pch);
    }
    break;
  case oprConstDefine:
    if (!FEnsureLVar(cLetter))
      break;
    i = FCreateConstTrie(rgsz[0], rgcch[0]);
    ws.rglVar[iLetterZ] = !i;
    break;
  case oprClearEvents:
    ws.iEventEdge = ws.iEventWall = ws.iEventMove = ws.iEventMove2 =
      ws.iEventFollow = ws.iEventInside =
      ws.iEventLeft = ws.iEventRight = ws.iEventCommand = ws.iEventQuit =
      ws.iEventInside2 =
      ws.iEventMiddle = ws.iEventPrev = ws.iEventNext = ws.iEventMouse = 0;
    dr.fRedrawAfter = fFalse;
    break;
  case oprVarSet:
    if (n1 == ~0)
      n1 = ChCap(rgsz[0][0]) - '@';
    if (!FEnsureLVar(n1 + 1))
      break;
    ws.rglVar[n1] = n2;
    break;
  case oprVarSets:
    if (n1 == ~0)
      n1 = ChCap(rgsz[0][0]) - '@';
    if (!FEnsureLVar(n1 + n2))
      break;
    pch = rgsz[2];
    for (i = n1; i < n1 + n2; i++) {
      for (pchT = pch; pchT - rgsz[2] < rgcch[2] && *pchT != ' '; pchT++)
        ;
      cch = PD(pchT - pch);
      if (*pch == '@')
        l = ws.rglVar[ChCap(pch[1]) - '@'];
      else {
        x = ILookupTrie(ws.rgsTrieKv, pch, cch, fTrue);
        l = x >= 0 ? rgkv[x] : LFromRgch(pch, cch);
      }
      ws.rglVar[i] = l;
      for (pch += cch; pch - rgsz[2] < rgcch[2] && *pch == ' '; pch++)
        ;
    }
    break;
  case oprVarSwap:
    if (n1 == ~0)
      n1 = ChCap(rgsz[0][0]) - '@';
    if (n2 == ~0)
      n2 = ChCap(rgsz[1][0]) - '@';
    if (!FEnsureLVar(Max(n1, n2) + 1))
      break;
    l = ws.rglVar[n1];
    ws.rglVar[n1] = ws.rglVar[n2];
    ws.rglVar[n2] = l;
    break;
  case oprLocal:
    if (!FEnsureLVar(cLetter))
      break;
    if (ws.rglLocal != NULL)
      for (pch = rgsz[0]; pch - rgsz[0] < rgcch[0]; pch++) {
        n1 = ChCap(*pch) - '@';
        if (FBetween(n1, 1, cLetter)) {
          ws.rglLocal[n1] = ws.rglVar[n1];
          ws.rglLocal[0] |= (1 << n1);
        }
      }
    break;
  case oprStrSet:
    if (n1 == ~0)
      n1 = ChCap(rgsz[0][0]) - '@';
    FormatRgchToSz(rgsz[1], rgcch[1], sz, cchSzOpr);
    SetSzVar(sz, -1, n1);
    break;
  case oprStrSets:
    if (n1 == ~0)
      n1 = ChCap(rgsz[0][0]) - '@';
    if (!FEnsureSzVar(n1 + n2 + 1))
      break;
    pch = rgsz[2] + 1;
    for (i = n1; i < n1 + n2; i++) {
      for (pchT = pch; pchT - rgsz[2] < rgcch[2] && *pchT != rgsz[2][0];
        pchT++)
        ;
      cch = NMax(PD(pchT - pch), 0);
      if (!SetSzVar(pch, cch, i))
        break;
      pch += cch + 1;
    }
    break;
  case oprStrSwap:
    if (n1 == ~0)
      n1 = ChCap(rgsz[0][0]) - '@';
    if (n2 == ~0)
      n2 = ChCap(rgsz[1][0]) - '@';
    if (!FEnsureLVar(Max(n1, n2) + 1))
      break;
    pchT = ws.rgszVar[n1];
    ws.rgszVar[n1] = ws.rgszVar[n2];
    ws.rgszVar[n2] = pchT;
    break;
  case oprStrAppend:
    if (n1 == ~0)
      n1 = ChCap(rgsz[0][0]) - '@';
    if (!FEnsureSzVar(n1 + 1))
      break;
    if (ws.rgszVar[n1] == NULL) {
      SetSzVar(rgsz[1], rgcch[1], n1);
      break;
    }
    cch = CchSz(ws.rgszVar[n1]);
    pch = RgAllocate(cch + rgcch[1] + 1, char);
    if (pch == NULL)
      break;
    CopyRgb(ws.rgszVar[n1], pch, cch);
    CopyRgb(rgsz[1], pch + cch, rgcch[1]);
    pch[cch + rgcch[1]] = chNull;
    DeallocateP(ws.rgszVar[n1]);
    ws.rgszVar[n1] = pch;
    break;
  case oprStrChar:
    if (n1 == ~0)
      n1 = ChCap(rgsz[0][0]) - '@';
    if (!FEnsureSzVar(n1 + 1))
      break;
    if (ws.rgszVar[n1] != NULL && CchSz(ws.rgszVar[n1]) > n2)
      ws.rgszVar[n1][n2] = n3 != ~0 ? n3 : rgsz[2][0];
    break;
  case oprStrNum:
    if (n1 == ~0)
      n1 = ChCap(rgsz[0][0]) - '@';
    sprintf(S(szT), "%%0%dd", n3);
    sprintf(S(sz), szT, n2);
    SetSzVar(sz, -1, n1);
    break;
  case oprStrReal:
    if (n1 == ~0)
      n1 = ChCap(rgsz[0][0]) - '@';
    sprintf(S(szT), "%%s%%d.%%0%dd", n3);
    l = LPower(10, n3);
    sprintf(S(sz), szT, n2 < 0 ? "-" : "", NAbs(n2) / l, NAbs(n2) % l);
    SetSzVar(sz, -1, n1);
    break;
  case oprMaskCopy:
  case oprMaskSwap:
    if (BitmapGetMask(Max(n1, n2)) == NULL)
      break;
    pbSrc = BitmapGetMask(n1);
    pbDst = BitmapGetMask(n2);
    if (iopr == oprMaskSwap &&
      (n1 != -1 || !pbDst->FNull()) && (n2 != -1 || !pbSrc->FNull())) {
      if (pbSrc != pbDst)
        pbSrc->SwapWith(*pbDst);
      break;
    }
    if (!pbSrc->FNull())
      pbDst->FBitmapCopy(*pbSrc);
    break;
  case oprMaskDel:
    pbSrc = BitmapGetMask(n1);
    if (pbSrc != NULL && n1 != -1)
      pbSrc->Free();
    break;
  case oprTextureCopy:
  case oprTextureSwap:
    if (ColmapGetTexture(Max(n1, n2)) == NULL)
      break;
    pbSrc = ColmapGetTexture(n1);
    pbDst = ColmapGetTexture(n2);
    if (iopr == oprTextureSwap) {
      if (pbSrc != pbDst)
        pbSrc->SwapWith(*pbDst);
      if (bm.k.FNull())
        FShowColmap(fFalse);
      break;
    }
    if (!pbSrc->FNull())
      pbDst->FBitmapCopy(*pbSrc);
    break;
  case oprTextureDel:
    pbSrc = ColmapGetTexture(n1);
    if (pbSrc != NULL && n1 != -1)
      pbSrc->Free();
    break;
  case oprMoveCloud:
    FMoveCloud(n1, n2);
    break;
  case oprMessage:
    FormatRgchToSz(rgsz[0], rgcch[0], sz, cchSzOpr);
    PrintSz_N(sz);
    break;
  case oprMessageAsk:
    FormatRgchToSz(rgsz[0], rgcch[0], sz, cchSzOpr);
    i = PrintSzCore(sz, -n2);
    SetMacroReturn(i);
    break;
  case oprMessageIns:
    DoOperationW(oosMessageIns, rgsz[0], rgcch[0], n1, n2);
    break;
  case oprStrGet:
    if (!FEnsureLVar(cLetter))
      break;
    if (n2 == ~0)
      n2 = ChCap(rgsz[1][0]) - '@';
    FormatRgchToSz(rgsz[0], rgcch[0], sz, cchSzOpr);
    ws.szDialog = sz;
    ws.iszDialog = n2;
    SystemHook(hosDlgString);
    ws.rglVar[iLetterZ] = (ws.iszDialog != n2);
    break;
  case oprStrGet2:
    if (!FEnsureLVar(cLetter))
      break;
    if (n3 == ~0)
      n3 = ChCap(rgsz[1][0]) - '@';
    FormatRgchToSz(rgsz[0], rgcch[0], sz,  cchSzOpr);
    FormatRgchToSz(rgsz[1], rgcch[1], szT, cchSzMax);
    ws.szDialog = sz;
    ws.szDialog2 = szT;
    ws.iszDialog = n3;
    SystemHook(hosDlgString2);
    ws.rglVar[iLetterZ] = (ws.iszDialog != n3);
    break;
  case oprStrGet3:
    if (!FEnsureLVar(cLetter))
      break;
    if (n4 == ~0)
      n4 = ChCap(rgsz[1][0]) - '@';
    FormatRgchToSz(rgsz[0], rgcch[0], sz,  cchSzOpr);
    FormatRgchToSz(rgsz[1], rgcch[1], szT, cchSzMax);
    FormatRgchToSz(rgsz[2], rgcch[2], sz3, cchSzDef);
    ws.szDialog = sz;
    ws.szDialog2 = szT;
    ws.szDialog3 = sz3;
    ws.iszDialog = n4;
    SystemHook(hosDlgString3);
    ws.rglVar[iLetterZ] = (ws.iszDialog != n4);
    break;
  case oprEnableAll:
    if (ws.rgbCmdMacro != NULL)
      for (i = 0; i < ccmd; i++)
        if (ws.rgbCmdMacro[i] == 255)
          ws.rgbCmdMacro[i] = 0;
    break;
  case oprDisableAll:
  case oprOnCmdAll:
    if (iopr == oprDisableAll)
      n1 = 255;
    if (ws.rgbCmdMacro == NULL)
      ws.rgbCmdMacro = RgAllocate(ccmd, byte);
    if (ws.rgbCmdMacro != NULL)
      for (i = 0; i < ccmd; i++)
        ws.rgbCmdMacro[i] = n1;
    break;
  case oprEnable:
  case oprDisable:
  case oprOnCmd:
  case oprDoCmd:
    if (n1 == ~0 && rgsz[0][0] != '0')
      n1 = CmdFromRgch(rgsz[0], rgcch[0]);
    if (iopr == oprDoCmd)
      DoCommand(icmdBase + n1);
    else {
      if (ws.rgbCmdMacro == NULL) {
        ws.rgbCmdMacro = RgAllocate(ccmd, byte);
        if (ws.rgbCmdMacro != NULL)
          for (i = 0; i < ccmd; i++)
            ws.rgbCmdMacro[i] = 0;
      }
      if (iopr == oprEnable)
        n2 = 0;
      else if (iopr == oprDisable)
        n2 = 255;
      ws.rgbCmdMacro[n1] = n2;
    }
    break;
  case oprReset:
    DoOperation(oprClearEvents, rgsz, rgcch, rgl, file);
    DeallocateTextures();
    if (ws.rgbCmdMacro != NULL) {
      DeallocateP(ws.rgbCmdMacro);
      ws.rgbCmdMacro = NULL;
    }
    bm.b.Turtle("_");
    ws.fSpree = dr.fDot2 = dr.fSky3D = dr.fNoCompass = dr.fNoLocation =
      dr.fPolishMaze = dr.fFogFloor = dr.fMarkColor = dr.fMarkAll =
      dr.fMarkBlock = dr.fTextureDual = dr.fTextureDual2 = dr.fTextureBlend =
      fFalse;
    ms.nCellMax = ws.nTitleMessage = dr.zStep =
      dr.kvInWall2 = dr.kvInSky2 = dr.kvInDirt2 = dr.kvInCeil2 = dr.kvInMtn2 =
      dr.kvInCld2 = dr.nMarkSky = dr.nWallVar = dr.nFogLit = -1;
    dr.ySunMoon = 333;
    dr.kvOff = kvBlack; dr.kvOn = kvWhite;
    break;
  case oprResetIns:
    ResetInside();
    break;
  case oprTitle:
    DoOperationW(oosTitle, rgsz[0], rgcch[0], ~0, n2);
    break;
  case oprBeep:
    DoOperationW(oosBeep, NULL, 0, ~0, ~0);
    break;
  case oprBeep2:
    DoOperationW(oosBeep2, NULL, 0, n1, n2);
    break;
  case oprSound:
    DoOperationW(oosSound, rgsz[0], rgcch[0], ~0, ~0);
    break;
  case oprDelay:
    DoOperationW(oosDelay, NULL, 0, n1, ~0);
    break;
  case oprSet:
    bm.b.Set(n1, n2, n3);
    break;
  case oprSetT:
    if (!bm.b2.FNull())
      bm.b2.Set(n1, n2, n3);
    break;
  case oprSetE:
    if (!bm.b3.FNull())
      bm.b3.Set(n1, n2, n3);
    break;
  case oprSetA:
    bT = BitmapGetMask(n1);
    if (bT == NULL)
      break;
    bT->Set(n2, n3, n4);
    break;
  case oprSetC:
    if (!bm.k.FNull())
      bm.k.Set(n1, n2, n3);
    break;
  case oprSetCT:
    if (!bm.k2.FNull())
      bm.k2.Set(n1, n2, n3);
    break;
  case oprSetCE:
    if (!bm.k3.FNull())
      bm.k3.Set(n1, n2, n3);
    break;
  case oprSetCA:
    bT = ColmapGetTexture(n1);
    if (bT == NULL)
      break;
    bT->Set(n2, n3, n4);
    break;
  case oprSetX:
    bT = ColmapGetTexture(dr.nTextureWall);
    if (bT == NULL)
      break;
    if (n3 >= 0)
      l = SetT(*bT, n1, n2, n3, n4);
    else if (n3 == -1)
      l = NWSE(n4, n4, n4, n4);
    else
      l = UD(n4, n4);
    bT->Set(n1, n2, l);
    break;
  case oprSetY:
    bT = ColmapGetTexture(dr.nTextureWall);
    if (bT == NULL)
      break;
    if (n3 >= 0)
      l = SetU(*bT, n1, n2, n3, n4);
    else if (n3 == -1)
      l = UD(n4, n4);
    else
      l = NWSE(n4, n4, n4, n4);
    bT->Set(n1, n2, l);
    break;
  case oprSet3:
    bm.b.Set3(n1, n2, n3, n4);
    break;
  case oprSet3T:
    if (!bm.b2.FNull())
      bm.b2.Set3(n1, n2, n3, n4);
    break;
  case oprSet3E:
    if (!bm.b3.FNull())
      bm.b3.Set3(n1, n2, n3, n4);
    break;
  case oprSet3A:
    bT = BitmapGetMask(n1);
    if (bT == NULL)
      break;
    bT->Set3(n2, n3, n4, n5);
    break;
  case oprSet3C:
    if (!bm.k.FNull())
      bm.k.Set3(n1, n2, n3, n4);
    break;
  case oprSet3CT:
    if (!bm.k2.FNull())
      bm.k2.Set3(n1, n2, n3, n4);
    break;
  case oprSet3CE:
    if (!bm.k3.FNull())
      bm.k3.Set3(n1, n2, n3, n4);
    break;
  case oprSet3CA:
    bT = ColmapGetTexture(n1);
    if (bT == NULL)
      break;
    bT->Set3(n2, n3, n4, n5);
    break;
  case oprSet3X:
    bT = ColmapGetTexture(dr.nTextureWall);
    if (bT == NULL)
      break;
    if (n4 >= 0)
      l = SetT3(*bT, n1, n2, n3, n4, n5);
    else if (n4 == -1)
      l = NWSE(n5, n5, n5, n5);
    else
      l = UD(n5, n5);
    bT->Set3(n1, n2, n3, l);
    break;
  case oprSet3Y:
    bT = ColmapGetTexture(dr.nTextureWall);
    if (bT == NULL)
      break;
    if (n4 >= 0)
      l = SetU3(*bT, n1, n2, n3, n4, n5);
    else if (n4 == -1)
      l = NWSE(n5, n5, n5, n5);
    else
      l = UD(n5, n5);
    bT->Set3(n1, n2, n3, l);
    break;
  case oprZapTexture:
    DotZap(cmdZapTexture, n1);
    break;
  case oprMark2:
  case oprUnmark2:
    if (dr.nMarkSky < 0)
      dr.nMarkSky = Max(ws.ccTexture, ws.cbMask);
    bT = BitmapGetMask(dr.nMarkSky);
    if (bT->FNull()) {
      if (bT->FBitmapCopy(bm.b))
        bT->BitmapOff();
    }
    if (!bT->FNull() &&
      bT->FLegal(bT->X2(dr.x, dr.z), bT->Y2(dr.y, dr.z))) {
      bT->Set3(dr.x, dr.y, dr.z, iopr == oprMark2);
    }
    break;
  case oprTurtle:
    if (!bm.b.Turtle(sz))
      nRet = nRetHalt;
    break;
  case oprTextDraw:
    DoOperationW(oosDrawText, sz, CchSz(sz), n2, n3);
    break;
  case oprTextFont:
    CopyRgchToSz(sz, CchSz(sz), ws.szFontName, cchSzDef);
    ws.nFontSize = n2; ws.nFontWeight = n3;
    break;
  case oprPerimeter:
    l = bm.b.AnalyzePerimeter(dr.x, dr.y, !dr.fNoCorner);
    SetMacroReturn(l);
    break;
  case oprThicken2:
    l = bm.b.BitmapThicken2(n1, !dr.fNoCorner);
    SetMacroReturn(l);
    break;
  case oprBlendN:
    bm.k.ColmapBlendBitmaps(bm.k2);
    break;
  case oprConvex:
    bm.b.FBitmapConvex(xl, yl, xh, yh);
    break;
  case oprAllMoire:
    bm.b.BitmapMoire();
    break;
  case opr2ndLine2:
    i = bm.b.FLineUntil(dr.x, dr.y, dr.x2, dr.y2, &x, &y, dr.fSet, fTrue, n1);
    if (FEnsureLVar(cLetter)) {
      ws.rglVar[iLetterX] = x; ws.rglVar[iLetterY] = y;
      ws.rglVar[iLetterZ] = i;
    }
    break;
  case opr2ndPut2:
    bm.k.ColmapStretchToEdge(bm.k2, dr.x, dr.y, n1, n2, n3, n4, dr.x2, dr.y2);
    break;
  case oprIsland:
    l = bm.b.DoCrackIsland(n1, n2, !dr.fNoCorner);
    SetMacroReturn(l);
    break;
  case oprIslands:
    l = bm.b.DoCrackIslands(!dr.fNoCorner);
    SetMacroReturn(l);
    break;
  case opr4D3D:
    bm.b.FConvert4DTo3D(bm.b3, bm.k, dr.kvIn3D, dr.kvInWall, n1, n2, n3);
    if (dr.nTrans == nTransNone)
      dr.nTrans = nTransDefault;
    break;
  case oprND:
    bm.b.CreateMazeND();
    break;
  case oprSolveND:
    l = bm.b.SolveMazeND();
    SetMacroReturn(l);
    break;
  case oprDrawND:
    if (!bm.b2.FNull())
      bm.b2.RenderND(BFocus(),
        &n1, &n2, &n3, &n4, &ws.rglVar[n3], &ws.rglVar[n4], n5,
        1 | (!dr.fBig << 1) | (dr.fDrag << 2));
    dr.x = n1; dr.y = n2;
    dr.x2 = n3; dr.y2 = n4;
    break;
  case oprMoveND:
    l = bm.b2.MoveND(&ws.rglVar[n1], &ws.rglVar[n2], &n3, &n4, fTrue);
    dr.x = n3; dr.y = n4;
    SetMacroReturn(l);
    break;
  case oprFractal:
    if (FEnsureLVar(cLetter))
      bm.b.FractalPartial(n1, n2, n3, n4,
        &ws.rglVar[iLetterX], &ws.rglVar[iLetterY]);
    break;
  case oprFractal3:
    if (FEnsureLVar(cLetter))
      bm.b.FractalPartial3D(n1, n2, n3, n4, n5,
        &ws.rglVar[iLetterX], &ws.rglVar[iLetterY]);
    break;
  case oprVirtPerfect:
  case oprVirtAldous:
  case oprVirtRecurs:
    if (iopr == oprVirtPerfect)
      bm.b.PerfectPartial(n1, dr.x, dr.y, &x, &y, &i);
    else if (iopr == oprVirtAldous)
      bm.b.AldousBroderPartial(n1, dr.x, dr.y, &x, &y, &i);
    else
      bm.b.RecursivePartial(n1, dr.x, dr.y, n2, &x, &y, &cch, &i, bm.b2);
    dr.x = x; dr.y = y;
    if (FEnsureLVar(cLetter)) {
      ws.rglVar[iLetterZ] = i;
      if (iopr == oprVirtRecurs)
        ws.rglVar[iLetterY] = cch;
    }
    break;
  case oprDrawLevel:
    bm.b.BlockMoveMaze(bm.b2, bm.b2.X2(0, n1), bm.b2.Y2(0, n1),
      bm.b2.X2(0, n1) + bm.b2.m_x3, bm.b2.Y2(0, n1) + bm.b2.m_y3,
      n2, n3, n4, n5, n6, n7);
    break;
  case oprCustomInc:
    ms.nLabyrinth = 5;
    if (FEnsureLVar(cLetter))
      ws.rglVar[iLetterZ] = !FComputeNextCustom(ms.szCustom, fFalse);
    break;
  case oprDungeon:
    bm.b.CreateMazeDungeon(bm.k, bm.b2, n1, n2, n3, n4, dr.f3D, n5, &x, &y);
    dr.x = x; dr.y = y;
    break;
  case oprZoomSprite:
    if (!bm.fColor)
      bm.b.FZoomSprite(NULL, bm.b2);
    else
      bm.k.FZoomSprite(&bm.b, bm.k2);
    break;
  case oprDrawSeen:
    if (!bm.fColor)
      bm.b.BitmapSeen(dr.x, dr.y, n1);
    else
      bm.k.DrawSeen(bm.b, bm.k2, dr.x, dr.y, n1, !dr.fNoCorner);
    break;
  case oprPalette:
    bm.k.ColmapPalette(bm.k2);
    break;
  case oprSymmetricX:
  case oprSymmetricY:
  case oprSymmetricZ:
    l = bm.b.DoMakeSymmetric(1 + (iopr - oprSymmetricX),
      ws.wCmdSpree == cmdCreateBraid);
    SetMacroReturn(l);
    PrintSzL("Asymmetries in Maze: %ld\n", l);
    break;
  case oprZoomPic:
    bm.b.MazeZoomWithPicture(&bm.k);
    break;
  case oprOverview2:
    if (!dr.f3D)
      DrawOverview2(bm.b);
    else
      DrawOverviewCube2(bm.b);
    break;
  case oprAltitude:
    if (!bm.fColor)
      DrawOverviewAltitude(bm.b, bm.k, n1, n2);
    else
      DrawOverviewAltitude(bm.k, bm.k2, n1, n2);
    break;
  case oprGetWire:
    if (n1 < bm.ccoor && FEnsureLVar(n2 + 6)) {
      ws.rglVar[n2]   = (int)bm.coor[n1].x1;
      ws.rglVar[n2+1] = (int)bm.coor[n1].y1;
      ws.rglVar[n2+2] = (int)bm.coor[n1].z1;
      ws.rglVar[n2+3] = (int)bm.coor[n1].x2;
      ws.rglVar[n2+4] = (int)bm.coor[n1].y2;
      ws.rglVar[n2+5] = (int)bm.coor[n1].z2;
      ws.rglVar[n2+6] = (int)bm.coor[n1].kv;
    }
    break;
  case oprSetWire:
    if (n1 < bm.ccoor && FEnsureLVar(n2 + 6)) {
      bm.coor[n1].x1 = (real)ws.rglVar[n2];
      bm.coor[n1].y1 = (real)ws.rglVar[n2+1];
      bm.coor[n1].z1 = (real)ws.rglVar[n2+2];
      bm.coor[n1].x2 = (real)ws.rglVar[n2+3];
      bm.coor[n1].y2 = (real)ws.rglVar[n2+4];
      bm.coor[n1].z2 = (real)ws.rglVar[n2+5];
      bm.coor[n1].kv =       ws.rglVar[n2+6];
    }
    break;
  case oprGetPatch:
    if (n1 < bm.cpatch && FEnsureLVar(n2 + 18)) {
      y = n2 + 4*4;
      for (i = 0; i < bm.patch[n1].cpt; i++) {
        x = n2 + i*4;
        ws.rglVar[x]   = (int)bm.patch[n1].p[i].x;
        ws.rglVar[x+1] = (int)bm.patch[n1].p[i].y;
        ws.rglVar[x+2] = (int)bm.patch[n1].p[i].z;
        ws.rglVar[x+3] =      bm.patch[n1].p[i].fLine;
      }
      ws.rglVar[y]   = bm.patch[n1].cpt;
      ws.rglVar[y+1] = bm.patch[n1].kv;
      ws.rglVar[y+2] = bm.patch[n1].nTrans;
    }
    break;
  case oprSetPatch:
    if (n1 < bm.cpatch && FEnsureLVar(n2 + 18)) {
      y = n2 + 4*4;
      for (i = 0; i < ws.rglVar[y]; i++) {
        x = n2 + i*4;
        bm.patch[n1].p[i].x     = (real)ws.rglVar[x];
        bm.patch[n1].p[i].y     = (real)ws.rglVar[x+1];
        bm.patch[n1].p[i].z     = (real)ws.rglVar[x+2];
        bm.patch[n1].p[i].fLine =       ws.rglVar[x+3] != 0;
      }
      bm.patch[n1].cpt    = (ws.rglVar[y] <= 3 ? 3 : 4);
      bm.patch[n1].kv     =        ws.rglVar[y+1];
      bm.patch[n1].nTrans = (short)ws.rglVar[y+2];
    }
    break;
  case oprGetStar:
    pstar = !n1 ? dr.rgstar : ds.rgstar;
    if (pstar != NULL && FBetween(n2, 0, istarMax-1) &&
      FEnsureLVar(n3 + 2)) {
      ws.rglVar[n3]   = pstar[n2].x;
      ws.rglVar[n3+1] = pstar[n2].y;
      ws.rglVar[n3+2] = pstar[n2].kv;
    }
    break;
  case oprSetStar:
    if (dr.rgstar == NULL)
      FCreateInsideStars(NULL, 0.0, 0, fFalse);
    pstar = !n1 ? dr.rgstar : ds.rgstar;
    if (pstar != NULL && FBetween(n2, 0, istarMax-1)) {
      pstar[n2].x = n3;
      pstar[n2].y = n4;
      pstar[n2].kv = ParseColor(rgsz[4], fFalse);
    }
    break;
  case oprStereogram:
    if (!bm.fColor)
      bm.b.FStereogram(bm.b2, n1, n2);
    else
      bm.k.FColmapStereogram(bm.b, bm.k2, n1, n2);
    break;
  case oprSetLife:
    l = FSetLife(rgsz[0], rgcch[0]);
    SetMacroReturn(l);
    break;
  case oprEvolution:
    if (!(ms.nInfCutoff > 0 && ms.nInfGen >= ms.nInfCutoff)) {
      l = bm.k.Evolution(bm.k2, n1, n2, dr.nEdge == nEdgeTorus,
        !dr.fNoCorner);
      SetMacroReturn(l);
      if (n2) {
        ms.nInfGen++;
        PrintSzL("Pixels changed: %ld\n", l);
      }
    }
    break;
  case oprMandelbrot:
    bm.k.Mandelbrot((real)n1 / 1000000000.0, (real)n2 / 1000000000.0,
      (real)n3 / 1000000000.0, (real)n4 / 1000000000.0, n5, cs.fMandelbrot);
    break;
  case oprMandelbrotG:
    l = bm.k.ColmapFind(1);
    if (FEnsureLVar(cLetter)) {
      ws.rglVar[iLetterX] = WLo(l);
      ws.rglVar[iLetterY] = WHi(l);
    }
    break;
  case oprContrast:
    if (!bm.fColor)
      bm.b.FBitmapAccentContrast(!dr.fNoCorner);
    else
      bm.k.FColmapAccentContrast(!dr.fNoCorner);
    break;
  case oprHunger:
    FHungerGame(n1);
    break;
  case oprSystem:
    DoOperationW(oosSystem, NULL, 0, n1, ~0);
    break;
  case oprSetup:
    DoCommand(cmdSetupDesktop);
    DoCommand(cmdSetupUser);
    DoCommand(cmdSetupExtension);
    break;

  case oprFileClose:
    fclose((FILE *)(size_t)n1);
    break;
  case oprFileWrite:
    if (n2 == ~0 && rgsz[1][0] != chNull) {
      FormatRgchToSz(rgsz[1], rgcch[1], sz, cchSzOpr);
      WriteSz((FILE *)(size_t)n1, sz);
    } else
      fprintf((FILE *)(size_t)n1, "%ld", n2);
    break;
  case oprFileByte:
    putc(n2, (FILE *)(size_t)n1);
    break;
  case oprFileStr:
    if (n2 == ~0)
      n2 = ChCap(rgsz[1][0]) - '@';
    fscanf((FILE *)(size_t)n1, "%s", S(sz));
    SetSzVar(sz, -1, n2);
    break;
  default:
    PrintSzN_E("Operation %d is undefined.", iopr);
  }

  // Update things after the operation runs based on operation flags.
  if (rgopr[iopr].grf & fCmtBitmapAfter)
    FShowColmap(fFalse);
  else if (rgopr[iopr].grf & fCmtColmapAfter)
    FShowColmap(fTrue);
  if (rgopr[iopr].grf & fCmtMaze)
    ms.cMaze++;
  if (rgopr[iopr].grf & fCmtDirtyView)
    SystemHook(hosDirtyView);
  if ((rgopr[iopr].grf & fCmtRedraw) > 0 && ws.fNoDirty)
    SystemHook(hosRedraw);
  if ((rgopr[iopr].grf & fCmtHourglass) > 0 && cursorPrev != NULL)
    HourglassCursor(&cursorPrev, fFalse);
  return nRet;
}


#define OsDoCommand fDidRedraw = DoCommandW(wCmd)
#define OsDoDialog(dlg) OsDoCommand
#define CheckMenuW(wCmd, f) DoOperationW(oosCheck, NULL, 0, wCmd, f);

// Execute a command. This gets run when a menu option is manually selected,
// and when a command is automatically invoked via scripting.

flag DoCommand(int wCmd)
{
  size_t cursorPrev = NULL;
  char sz[cchSzDef], szT[cchSzDef], *pch;
  int icmd = wCmd - icmdBase, grf, i, j, x, y;
  CMaz bT;
  long l, m;
  ulong ul;
  time_t lTime;
  flag fDidRedraw = fFalse;

  Assert(FBetween(icmd, 0, ccmd-1));

  // Do or check things before the command runs based on command flags.
  grf = rgcmd[icmd].grf;
  if ((grf & fCmtRequireTemp) > 0 &&
    (bm.fColor ? bm.k2.FNull() : bm.b2.FNull())) {
    pch = "temporary";
    goto LBitmapError;
  }
  if ((grf & fCmtRequireExtra) > 0 &&
    (bm.fColor ? bm.k3.FNull() : bm.b3.FNull())) {
    pch = "extra";
    goto LBitmapError;
  }
  if ((grf & fCmtRequireColor) > 0 && bm.k.FNull()) {
    pch = "color";
LBitmapError:
    sprintf(S(sz), "The %s command requires a non-empty %s bitmap.\n",
      rgcmd[icmd].szName, pch);
    PrintSz_W(sz);
    goto LDone;
  }
  if ((grf & fCmtHourglass) > 0 && ws.fHourglass)
    HourglassCursor(&cursorPrev, fTrue);
  if (grf & fCmtSet2D) {
    if (dr.f3D) {
      dr.f3D = fFalse;
      CheckMenuW(cmd3D, fFalse);
    }
  } else if (grf & fCmtSet3D) {
    if (!dr.f3D) {
      dr.f3D = fTrue;
      CheckMenuW(cmd3D, fTrue);
    }
  }
  if (grf & fCmtBitmapBefore)
    FShowColmap(fFalse);
  else if (grf & fCmtColmapBefore) {
    if (!FShowColmap(fTrue))
      goto LDone;
  }
  if (grf & fCmtMaze) {
    ms.cRunRnd = 0;
    PolishMaze(wCmd, 0);
  } else if ((grf & fCmtSolve) > 0 && bm.fColor)
    bT.FBitmapCopy(bm.b);

  switch (wCmd) {

  // File menu

  case cmdOpen:
  case cmdOpenBitmap:
  case cmdOpenText:
  case cmdOpenXbm:
  case cmdOpen3D:
  case cmdOpenScript:
  case cmdOpenColmapTarga:
  case cmdOpenWire:
  case cmdOpenPatch:
    FFileOpen(wCmd, NULL, NULL);
    break;

  case cmdSaveColmapTarga:
  case cmdSaveBitmap:
  case cmdSaveText:
  case cmdSaveDOS:
  case cmdSaveDOS2:
  case cmdSaveDOS3:
  case cmdSaveXbmN:
  case cmdSaveXbmC:
  case cmdSaveXbmS:
  case cmdSave3DN:
  case cmdSave3DS:
  case cmdSaveWallCenter:
  case cmdSaveWallTile:
  case cmdSaveWallStretch:
  case cmdSaveWallFit:
  case cmdSaveWallFill:
  case cmdSaveWire:
  case cmdSavePicture:
  case cmdSaveVector:
  case cmdSavePatch:
    FFileSave(wCmd, NULL);
    break;

  case cmdPrint:
    OsDoCommand;
    break;

  case cmdPrintSetup:
    OsDoCommand;
    break;

  case cmdDlgFile:
    OsDoDialog(dlgFile);
    break;

  case cmdShellHelp:
    OsDoDialog(dlgHelp);
    break;

  case cmdShellScript:
  case cmdShellChange:
  case cmdShellLicense:
  case cmdShellWeb:
  case cmdShellWeb2:
    OsDoCommand;
    break;

  case cmdSetupAll:
  case cmdSetupUser:
  case cmdSetupDesktop:
  case cmdSetupExtension:
  case cmdUnsetup:
    OsDoCommand;
    break;

  case cmdAbout:
    OsDoDialog(dlgAbout);
    break;

  case cmdExit:
    if (ws.fNoExit)
      PrintSz_W("Program exit has been disabled.");
    else
      ws.fQuitting = fTrue;
    break;

  // Edit menu

  case cmdRepeat:
    DoCommand(ws.wCmdRepeat);
    break;

  case cmdSpree:
    OsDoCommand;
    break;

  case cmdCommand:
    OsDoDialog(dlgCommand);
    break;

  case cmdDlgEvent:
    OsDoDialog(dlgEvent);
    break;

  case cmdCopyBitmap:
  case cmdCopyText:
  case cmdCopyPicture:
    OsDoCommand;
    break;

  case cmdPaste:
    OsDoCommand;
    break;

  case cmdDlgDisplay:
    OsDoDialog(dlgDisplay);
    break;

  case cmdCellDec1:
    bm.xCell = Max(bm.xCell-1, 1); bm.yCell = Max(bm.yCell-1, 1);
    bm.nWhat = 2;
    break;

  case cmdCellInc1:
    bm.xCell++; bm.yCell++;
    bm.nWhat = 2;
    break;

  case cmdCellDec10:
    bm.xCell = Max(bm.xCell-10, 1); bm.yCell = Max(bm.yCell-10, 1);
    bm.nWhat = 2;
    break;

  case cmdCellInc10:
    bm.xCell += 10; bm.yCell += 10;
    bm.nWhat = 2;
    break;

  case cmdWindowFull:
    OsDoCommand;
    break;

  case cmdWindowBitmap:
    OsDoCommand;
    break;

  case cmdSizeWindow:
    BFocus().FBitmapResizeTo(ws.xpClient, ws.ypClient);
    break;

  case cmdRedrawNow:
    OsDoCommand;
    break;

  case cmdRepaintNow:
    OsDoCommand;
    break;

  case cmdScrollUp:
    OsDoCommand;
    break;

  case cmdScrollDown:
    OsDoCommand;
    break;

  case cmdScrollHome:
    OsDoCommand;
    break;

  case cmdScrollEnd:
    OsDoCommand;
    break;

  case cmdDlgColor:
    OsDoDialog(dlgColor);
    break;

  case cmdMessage:
    inv(ws.fIgnorePrint);
    CheckMenuW(cmdMessage, ws.fIgnorePrint);
    break;

  case cmdTimeGet:
    OsDoCommand;
    break;

  case cmdTimeReset:
    OsDoCommand;
    break;

  case cmdTimePause:
    OsDoCommand;
    break;

  case cmdDlgRandom:
    OsDoDialog(dlgRandom);
    break;

  case cmdRandomize:
    time(&lTime);
    InitRndL((long)lTime ^ NGetVariableW(vosGetTick));
    ResetInside();
    break;

  // Dot menu

  case cmdDlgDot:
    OsDoDialog(dlgDot);
    break;

  case cmdMove2:
  case cmdMove4:
  case cmdMove6:
  case cmdMove8:
  case cmdMove1:
  case cmdMove3:
  case cmdMove7:
  case cmdMove9:
  case cmdMoveU:
  case cmdMoveD:
    if (dr.fInside && dr.nInside == nInsideVeryFree && !ws.fInSpree &&
      (!dr.f3DNotFree || (wCmd != cmdMoveU && wCmd != cmdMoveD)))
      break;
    if (ds.fArrow && ds.fArrowFree && !dr.fInside && !ws.fInSpree)
      break;
  case cmdMoveForward:
  case cmdMoveBack:
  case cmdMoveAround:
  case cmdMoveLeft:
  case cmdMoveRight:
  case cmdMoveRandom:
  case cmdMoveNorth:
  case cmdMoveSouth:
  case cmdMoveWest:
  case cmdMoveEast:
    DotMove(wCmd);
    break;

  case cmdJump4:
  case cmdJump6:
    if (dr.fInside && dr.nInside == nInsideVeryFree && !ws.fInSpree)
      break;
  case cmdJump2:
  case cmdJump8:
    if (ds.fArrow && ds.fArrowFree && !dr.fInside && !ws.fInSpree)
      break;
  case cmdJump1:
  case cmdJump3:
  case cmdJump7:
  case cmdJump9:
    DotJump(wCmd);
    break;

  case cmdGoCorner7:
    dr.x = dr.y = dr.z = 0;
    break;

  case cmdGoCorner9:
    dr.x = (!bm.fColor ? bm.b.m_x : bm.k.m_x)-1; dr.y = dr.z = 0;
    break;

  case cmdGoCorner1:
    dr.x = dr.z = 0; dr.y = (!bm.fColor ? bm.b.m_y : bm.k.m_y)-1;
    break;

  case cmdGoCorner3:
    dr.x = (!bm.fColor ? bm.b.m_x : bm.k.m_x)-1;
    dr.y = (!bm.fColor ? bm.b.m_y : bm.k.m_y)-1; dr.z = 0;
    break;

  case cmdGoMiddle:
    if (!dr.f3D) {
      dr.x = (!bm.fColor ? bm.b.m_x : bm.k.m_x) >> 1;
      dr.y = (!bm.fColor ? bm.b.m_y : bm.k.m_y) >> 1; dr.z = 0;
    } else {
      dr.x = bm.b.m_x3 >> 1; dr.y = bm.b.m_y3 >> 1; dr.z = bm.b.m_z3 >> 1;
    }
    break;

  case cmdGoRandom:
    if (!dr.f3D) {
      if (ms.fSolveEveryPixel) {
        dr.x = Rnd(0, (!bm.fColor ? bm.b.m_x : bm.k.m_x)-1);
        dr.y = Rnd(0, (!bm.fColor ? bm.b.m_y : bm.k.m_y)-1);
      } else {
        dr.x = Rnd(0, Odd(!bm.fColor ? bm.b.m_x : bm.k.m_x)-2) | 1;
        dr.y = Rnd(0, Odd(!bm.fColor ? bm.b.m_y : bm.k.m_y)-2) | 1;
      }
      dr.z = 0;
    } else {
      if (ms.fSolveEveryPixel) {
        dr.x = Rnd(0, bm.b.m_x3-1); dr.y = Rnd(0, bm.b.m_y3-1);
        dr.z = Rnd(0, bm.b.m_z3-1);
      } else {
        dr.x = Rnd(0, Odd(bm.b.m_x3)-2) | 1;
        dr.y = Rnd(0, Odd(bm.b.m_y3)-2) | 1;
        dr.z = Rnd(0, Odd(bm.b.m_z3)) & ~1;
      }
    }
    dr.dir = Rnd(0, DIRS1);
    break;

  case cmdGo2nd:
    dr.x = dr.x2; dr.y = dr.y2; dr.z = dr.z2;
    break;

  case cmdGoIn:
    x = xl; y = yl;
    bm.b.FFindPassage(&x, &y, fFalse);
    dr.x = x; dr.y = y; dr.z = dr.dir = 0;
    break;

  case cmdGoOut:
    x = xl; y = yh - !bm.b.Get(xl, yh);
    bm.b.FFindPassage(&x, &y, fFalse);
    dr.x = x; dr.y = y; dr.z = dr.dir = 0;
    break;

  case cmdGoInY:
    x = xl; y = yl;
    bm.b.FFindPassage(&x, &y, fTrue);
    dr.x = x; dr.y = y; dr.z = 0; dr.dir = 1;
    break;

  case cmdGoOutY:
    x = xh - !bm.b.Get(xh, yl); y = yl;
    bm.b.FFindPassage(&x, &y, fTrue);
    dr.x = x; dr.y = y; dr.z = 0; dr.dir = 1;
    break;

  case cmdDotShow:
    inv(dr.fDot);
    CheckMenuW(cmdDotShow, dr.fDot);
    break;

  case cmdDotWall:
    inv(dr.fWall);
    CheckMenuW(cmdDotWall, dr.fWall);
    break;

  case cmdDotTwo:
    inv(dr.fTwo);
    CheckMenuW(cmdDotTwo, dr.fTwo);
    break;

  case cmdDotDrag:
    inv(dr.fDrag);
    CheckMenuW(cmdDotDrag, dr.fDrag);
    break;

  case cmdDotOn:
    inv(dr.fSet);
    CheckMenuW(cmdDotOn, !dr.fSet);
    break;

  case cmdDotBig:
    inv(dr.fBig);
    CheckMenuW(cmdDotBig, dr.fBig);
    break;

  case cmdDotBoth:
    inv(dr.fBoth);
    CheckMenuW(cmdDotBoth, dr.fBoth);
    break;

  case cmdDotSet:
    BitmapDot(bm.b.X2(dr.x, dr.z), bm.b.Y2(dr.y, dr.z));
    break;

  case cmdDotZap:
  case cmdDotZapTrans:
  case cmdDotZapUntrans:
    DotZap(wCmd, 0);
    break;

  case cmd2nd:
    dr.x2 = dr.x; dr.y2 = dr.y; dr.z2 = dr.z;
    break;

  case cmd2ndLine:
    if (!bm.fColor)
      bm.b.Line(dr.x, dr.y, dr.x2, dr.y2, dr.fSet);
    else
      bm.k.Line(dr.x, dr.y, dr.x2, dr.y2, kvSet);
    break;

  case cmd2ndBlock:
    if (!dr.f3D) {
      if (!bm.fColor)
        bm.b.Block(dr.x, dr.y, dr.x2, dr.y2, dr.fSet);
      else
        bm.k.Block(dr.x, dr.y, dr.x2, dr.y2, kvSet);
    } else {
      if (!bm.fColor)
        bm.b.CubeBlock(dr.x, dr.y, dr.z, dr.x2, dr.y2, dr.z2, dr.fSet);
      else
        bm.k.CubeBlock(dr.x, dr.y, dr.z, dr.x2, dr.y2, dr.z2, kvSet);
    }
    break;

  case cmd2ndBox:
    if (!bm.fColor)
      bm.b.Edge(dr.x, dr.y, dr.x2, dr.y2, dr.fSet);
    else
      bm.k.Edge(dr.x, dr.y, dr.x2, dr.y2, kvSet);
    break;

  case cmd2ndDisk:
    if (!bm.fColor)
      bm.b.Disk(dr.x, dr.y, dr.x2, dr.y2, dr.fSet);
    else
      bm.k.Disk(dr.x, dr.y, dr.x2, dr.y2, kvSet);
    break;

  case cmd2ndCircle:
    if (!bm.fColor)
      bm.b.Circle(dr.x, dr.y, dr.x2, dr.y2, dr.fSet);
    else
      bm.k.Circle(dr.x, dr.y, dr.x2, dr.y2, kvSet);
    break;

  case cmd2ndGet:
    if (!bm.fColor) {
      if (bm.b2.FBitmapSizeSet(
        NAbs(dr.x2 - dr.x) + 1, NAbs(dr.y2 - dr.y) + 1))
        bm.b2.BlockMove(bm.b, dr.x, dr.y, dr.x2, dr.y2, 0, 0);
    } else {
      if (bm.k2.FBitmapSizeSet(
        NAbs(dr.x2 - dr.x) + 1, NAbs(dr.y2 - dr.y) + 1))
        bm.k2.BlockMove(bm.k, dr.x, dr.y, dr.x2, dr.y2, 0, 0);
    }
    break;

  case cmd2ndPut:
    if (!bm.fColor)
      bm.b.BlockMove(bm.b2, 0, 0, bm.b2.m_x-1, bm.b2.m_y-1, dr.x, dr.y);
    else
      bm.k.BlockMove(bm.k2, 0, 0, bm.k2.m_x-1, bm.k2.m_y-1, dr.x, dr.y);
    break;

  case cmd2ndOr:
    if (!bm.fColor)
      bm.b.BlockPutOrAnd(bm.b2, dr.x, dr.y, rgfOr);
    else
      bm.k.BlockPut(bm.k2, dr.x, dr.y, 1);
    break;

  case cmd2ndAnd:
    if (!bm.fColor)
      bm.b.BlockPutOrAnd(bm.b2, dr.x, dr.y, rgfAnd);
    else
      bm.k.BlockPut(bm.k2, dr.x, dr.y, 0);
    break;

  case cmd2ndXor:
    if (!bm.fColor)
      bm.b.BlockPutOrAnd(bm.b2, dr.x, dr.y, rgfXor);
    else
      bm.k.BlockPut(bm.k2, dr.x, dr.y, 2);
    break;

  case cmdInsideView:
    inv(dr.fInside);
    DoSetVariableW(vosEnsureTimer, ws.nTimerDelay);
    CheckMenuW(cmdInsideView, dr.fInside);
    break;

  case cmdInsideMap:
    if (!dr.fInside) {
      dr.fInside = fTrue;
      DoSetVariableW(vosEnsureTimer, ws.nTimerDelay);
      CheckMenuW(cmdInsideView, fTrue);
    }
    inv(dr.fMap);
    CheckMenuW(cmdInsideMap, dr.fMap);
    if (dr.fMap) {
      dr.fDot = dr.fCircle = fTrue;
      CheckMenuW(cmdDotShow, fTrue);
    }
    break;

  case cmdMark:
  case cmdUnmark:
    if (bm.b2.FNull()) {
      if (bm.b2.FBitmapCopy(bm.b))
        bm.b2.BitmapOff();
    }
    if (!bm.b2.FNull() &&
      bm.b2.FLegal(bm.b2.X2(dr.x, dr.z), bm.b2.Y2(dr.y, dr.z))) {
      bm.b2.Set3(dr.x, dr.y, dr.z, wCmd == cmdMark);
    }
    break;

  case cmdDlgInside:
    OsDoDialog(dlgInside);
    break;

  // Bitmap menu

  case cmdDlgSize:
    OsDoDialog(dlgSize);
    break;

  case cmdSize01:
  case cmdSize02:
  case cmdSize03:
  case cmdSize04:
  case cmdSize05:
  case cmdSize06:
  case cmdSize07:
  case cmdSize08:
  case cmdSize09:
  case cmdSize10:
  case cmdSize11:
  case cmdSize12:
  case cmdSize13:
  case cmdSize14:
  case cmdSize15:
  case cmdSize16:
  case cmdSize17:
  case cmdSize18:
  case cmdSize19:
    i = wCmd - cmdSize01;
    BFocus().FBitmapResizeTo(rgptSize[i].x, rgptSize[i].y);
    break;

  case cmdAllClear:
    if (!bm.fColor)
      bm.b.BitmapOff();
    else
      bm.k.BitmapSet(dr.kvOff);
    break;

  case cmdAllSet:
    if (!bm.fColor)
      bm.b.BitmapOn();
    else
      bm.k.BitmapSet(dr.kvOn);
    break;

  case cmdAllInvert:
    if (!bm.fColor)
      bm.b.BitmapReverse();
    else
      bm.k.ColmapOrAndKv(kvWhite, 2);
    break;

  case cmdAllRandom:
    if (!bm.fColor) {
      if (FBetween(ms.nRndBias, -50, 50))
        bm.b.BitmapRandom(50 + ms.nRndBias, 100);
      else
        bm.b.BitmapRandom(ms.nRndBias < 0 ? 1 : ms.nRndBias-1,
          NAbs(ms.nRndBias));
    } else {
      bm.k.BitmapSet(kvBlack);
      bm.k.ColmapReplacePatternSimple(kvBlack, crpRandom);
    }
    break;

  case cmdDlgZoom:
    OsDoDialog(dlgZoom);
    break;

  case cmdFlipX:
    BFocus().BitmapFlipX();
    break;

  case cmdFlipY:
    BFocus().BitmapFlipY();
    break;

  case cmdFlipZ:
    BFocus().FBitmapTranspose();
    break;

  case cmdRotateR:
    BFocus().BitmapRotateR();
    break;

  case cmdRotateL:
    BFocus().BitmapRotateL();
    break;

  case cmdRotateA:
    BFocus().BitmapRotateA();
    break;

  case cmdCubemapFlip:
    OsDoDialog(dlgCubeFlip);
    break;

  case cmdTempGet:
    if (!bm.fColor)
      bm.b2.FBitmapCopy(bm.b);
    else
      bm.k2.FBitmapCopy(bm.k);
    break;

  case cmdTempPut:
    if (!bm.fColor)
      bm.b.FBitmapCopy(bm.b2);
    else
      bm.k.FBitmapCopy(bm.k2);
    break;

  case cmdTempSwap:
    if (!bm.fColor)
      bm.b.SwapWith(bm.b2);
    else
      bm.k.SwapWith(bm.k2);
    break;

  case cmdTempOr:
    if (!bm.fColor)
      bm.b.BitmapOr(bm.b2);
    else
      bm.k.ColmapOrAnd(bm.k2, 1);
    break;

  case cmdTempAnd:
    if (!bm.fColor)
      bm.b.BitmapAnd(bm.b2);
    else
      bm.k.ColmapOrAnd(bm.k2, 0);
    break;

  case cmdTempXor:
    if (!bm.fColor)
      bm.b.BitmapXor(bm.b2);
    else
      bm.k.ColmapOrAnd(bm.k2, 2);
    break;

  case cmdTempBlend:
    if (!bm.fColor)
      bm.b.BitmapBlend(bm.b2);
    else
      bm.k.ColmapOrAnd(bm.k2, 5 + cs.fGraphNumber*3);
    break;

  case cmdTempTessellate:
    if (!bm.fColor)
      bm.b.BitmapTessellate(bm.b2);
    else
      bm.k.BitmapTessellate(bm.k2);
    break;

  case cmdTempDel:
    if (!bm.fColor)
      bm.b2.Free();
    else
      bm.k2.Free();
    break;

  case cmdDrawGet:
    if (!bm.fColor)
      bm.b3.FBitmapCopy(bm.b);
    else
      bm.k3.FBitmapCopy(bm.k);
    break;

  case cmdDrawPut:
    if (!bm.fColor)
      bm.b.FBitmapCopy(bm.b3);
    else
      bm.k.FBitmapCopy(bm.k3);
    break;

  case cmdDrawSwap:
    if (!bm.fColor)
      bm.b.SwapWith(bm.b3);
    else
      bm.k.SwapWith(bm.k3);
    break;

  case cmdDrawDel:
    if (!bm.fColor)
      bm.b3.Free();
    else
      bm.k3.Free();
    break;

  case cmdCollapse:
    if (!dr.f3D)
      BFocus().FBitmapCollapse();
    else
      BFocus().FCubeCollapse();
    break;

  case cmdSmooth:
    l = bm.b.BitmapSmooth(!dr.fNoCorner);
    SetMacroReturn(l);
    PrintSzL("Pixels smoothed: %ld\n", l);
    break;

  case cmdCorner:
    l = bm.b.BitmapSmoothCorner(dr.fSet);
    SetMacroReturn(l);
    PrintSzL("Corners smoothed: %ld\n", l);
    break;

  case cmdThicken:
    if (!bm.fColor)
      bm.b.BitmapThicken();
    else
      bm.k.FColmapBlur(dr.nEdge == nEdgeTorus);
    break;

  case cmdThinner:
    if (!bm.fColor) {
      l = bm.b.BitmapThinner(!dr.fNoCorner);
      SetMacroReturn(l);
      PrintSzL("Pixels thinned: %ld\n", l);
    } else
      bm.k.ColmapContrast(cs.fRainbowBlend);
    break;

  case cmdAccentBoundary:
    BFocus().FBitmapAccentBoundary();
    break;

  case cmdFloodDot:
    if (!bm.fColor) {
      if (!dr.f3D)
        bm.b.FFillReverse(dr.x, dr.y);
      else
        bm.b.FCubeFill(dr.x, dr.y, dr.z, !bm.b.Get3(dr.x, dr.y, dr.z));
    } else
      bm.k.FFill(dr.x, dr.y, dr.kvDot, -1, fFalse);
    break;

  case cmdFloodDot2:
    if (!bm.fColor)
      bm.b.FFloodReverse(dr.x, dr.y);
    else
      bm.k.FFill(dr.x, dr.y, dr.kvDot, -1, fTrue);
    break;

  case cmdSlide:
    if (!bm.fColor)
      bm.b.FBitmapSlide(dr.x - (bm.b.m_x >> 1), dr.y - (bm.b.m_y >> 1));
    else
      bm.k.FBitmapSlide(dr.x - (bm.k.m_x >> 1), dr.y - (bm.k.m_y >> 1));
    break;

  case cmdAddX:
    BFocus().FBitmapEditRowColumn(dr.x, dr.x, fTrue, fTrue);
    break;

  case cmdDelX:
    BFocus().FBitmapEditRowColumn(dr.x, dr.x, fTrue, fFalse);
    break;

  case cmdAddY:
    BFocus().FBitmapEditRowColumn(dr.y, dr.y, fFalse, fTrue);
    break;

  case cmdDelY:
    BFocus().FBitmapEditRowColumn(dr.y, dr.y, fFalse, fFalse);
    break;

  case cmdAddX2:
    BFocus().FBitmapEditRowColumn(dr.x, dr.x2, fTrue, fTrue);
    break;

  case cmdDelX2:
    BFocus().FBitmapEditRowColumn(dr.x, dr.x2, fTrue, fFalse);
    break;

  case cmdAddY2:
    BFocus().FBitmapEditRowColumn(dr.y, dr.y2, fFalse, fTrue);
    break;

  case cmdDelY2:
    BFocus().FBitmapEditRowColumn(dr.y, dr.y2, fFalse, fFalse);
    break;

  case cmd3D:
    inv(dr.f3D);
    CheckMenuW(cmd3D, dr.f3D);
    break;

  case cmdCount:
    if (!bm.fColor) {
      l = (long)bm.b.m_x*bm.b.m_y;
      m = bm.b.BitmapCount();
      sprintf(S(sz),
        "Total pixels: %ld\nOn: %ld (%.2f%%)\nOff: %ld (%.2f%%)\n",
        l, m, l ? (real)m/(real)l*100.0 : 0.0, l-m,
        l ? (real)(l-m)/(real)l*100.0 : 0.0); PrintSz_N(sz);
    } else
      m = bm.k.BitmapCount();
    SetMacroReturn(m);
    break;

  // Color menu

  case cmdColmap:
    FShowColmap(!bm.fColor);
    break;

  case cmdColmapGet:
    bm.k.FColmapGetFromBitmap(bm.b, dr.kvOff, dr.kvOn);
    break;

  case cmdColmapPut:
  case cmdColmapPutNearest:
  case cmdColmapPutDither:
    bm.k.FColmapPutToBitmap(bm.b,
      (wCmd == cmdColmapPutNearest) + (wCmd == cmdColmapPutDither)*2);
    break;

  case cmdColmapOr:
  case cmdColmapAnd:
  case cmdColmapXor:
    bm.k.ColmapOrAndFromBitmap(bm.b, dr.kvOff, dr.kvOn,
      (wCmd == cmdColmapOr) + (wCmd == cmdColmapXor)*2);
    break;

  case cmdColmapBlend:
    bm.k.FColmapBlendFromBitmap(&bm.b, &bm.b2, NULL);
    break;

  case cmdColmapBlend2:
    bm.k.FColmapBlendFromBitmap(&bm.b, &bm.b2, &bm.b3);
    break;

  case cmdColmapDistance:
    if (!dr.f3D)
      l = bm.k.ColmapGraphDistance(bm.b, bm.b2, dr.kvOff, dr.kvOn,
        dr.x, dr.y, !dr.fNoCorner);
    else
      l = bm.k.ColmapGraphDistance3(bm.b, bm.b2, dr.kvOff, dr.kvOn,
        dr.x, dr.y, dr.z);
    SetMacroReturn(l);
    if (l >= 0)
      PrintSzL("Maximum pixel distance from source: %ld\n", l);
    break;

  case cmdColmapDistance2:
    l = bm.k.ColmapGraphDistance2(bm.b, dr.kvOff, dr.kvOn, !dr.fNoCorner);
    SetMacroReturn(l);
    if (l >= 0)
      PrintSzL("Maximum pixel distance along wall: %ld\n", l);
    break;

  case cmdColmapAntialias2:
  case cmdColmapAntialias3:
  case cmdColmapAntialias4:
  case cmdColmapAntialias5:
  case cmdColmapAntialias6:
  case cmdColmapAntialias7:
  case cmdColmapAntialias8:
    bm.k.FColmapAntialias(bm.b, dr.kvOff, dr.kvOn,
      wCmd - cmdColmapAntialias2 + 2);
    break;

  case cmdColmapDotSet:
    bm.k.BitmapSet(dr.kvDot);
    break;

  case cmdColmapDotOr:
    bm.k.ColmapOrAndKv(dr.kvDot, 1);
    break;

  case cmdColmapDotAnd:
    bm.k.ColmapOrAndKv(dr.kvDot, 0);
    break;

  case cmdColmapDotXor:
    bm.k.ColmapOrAndKv(dr.kvDot, 2);
    break;

  case cmdColmapDotAdd:
    bm.k.ColmapOrAndKv(dr.kvDot, 3 + cs.fGraphNumber*6);
    break;

  case cmdColmapDotSub:
    bm.k.ColmapOrAndKv(dr.kvDot, 4 + cs.fGraphNumber*6);
    break;

  case cmdColmapDotMul:
    bm.k.ColmapOrAndKv(dr.kvDot, 8 + cs.fGraphNumber*6);
    break;

  case cmdColmapDotMin:
    bm.k.ColmapOrAndKv(dr.kvDot, 6 + cs.fGraphNumber*6);
    break;

  case cmdColmapDotMax:
    bm.k.ColmapOrAndKv(dr.kvDot, 7 + cs.fGraphNumber*6);
    break;

  case cmdColmapDotBlend:
    bm.k.ColmapOrAndKv(dr.kvDot, 5 + cs.fGraphNumber*6);
    break;

  case cmdTempAdd:
  case cmdTempSub:
    if (!bm.k2.FNull())
      bm.k.ColmapOrAnd(bm.k2, 3 + (wCmd == cmdTempSub) + cs.fGraphNumber*3);
    break;

  case cmdTempAlpha:
    if (!bm.k2.FNull() && !bm.k3.FNull())
      bm.k.ColmapAlpha(bm.k2, bm.k3);
    break;

  case cmdColmapDel:
    if (!bm.k.FNull()) {
      FShowColmap(fFalse);
      bm.k.Free();
    }
    break;

  case cmdColmapMono:
    bm.k.ColmapGrayscale();
    break;

  case cmdTextureBackground:
    FTextureBackground();
    dr.fInside = dr.fTexture = fTrue;
    break;

  case cmdTextureWall1:
  case cmdTextureWall2:
  case cmdTextureWall3:
    FTextureWall(wCmd - cmdTextureWall1 + 1, !dr.fSet);
    dr.fInside = dr.fTexture = fTrue;
    break;

  case cmdTextureGround1:
  case cmdTextureGround2:
  case cmdTextureGround3:
    FTextureGround(fFalse, wCmd - cmdTextureGround1 + 1, !dr.fSet);
    dr.fInside = dr.fTexture = fTrue;
    break;

  case cmdTextureBlock1:
  case cmdTextureBlock2:
  case cmdTextureBlock3:
    FTextureGround(fTrue, wCmd - cmdTextureBlock1 + 1, !dr.fSet);
    dr.fInside = dr.fTexture = fTrue;
    break;

  case cmdTextureCeiling:
    FTextureCeiling();
    dr.fInside = dr.fTexture = fTrue;
    break;

  case cmdWallVariable:
    FWallVariable(cs.nReplacePattern);
    dr.fWall = dr.fInside = fTrue;
    dr.zStep = dr.zWall >> 1;
    dr.nClip = Max(dr.nClip, 300);
    if (dr.nInside != nInsideFree && dr.nInside != nInsideVeryFree)
      dr.nInside = nInsideVeryFree;
    DoSetVariableW(vosEnsureTimer, ws.nTimerDelay);
    CenterWithinCell();
    break;

  case cmdGroundVariable:
    FGroundVariable();
    dr.fWall = dr.fInside = dr.fMarkAll = dr.fMarkBlock = fTrue;
    dr.zStep = dr.zWall - 1;
    dr.nClip = Max(dr.nClip, 300);
    if (dr.nInside != nInsideFree && dr.nInside != nInsideVeryFree)
      dr.nInside = nInsideVeryFree;
    DoSetVariableW(vosEnsureTimer, ws.nTimerDelay);
    CenterWithinCell();
    break;

  case cmdTextureDel:
    DeallocateTextures();
    break;

  case cmdColmapBrightness:
    OsDoDialog(dlgBright);
    break;

  case cmdColmapReplace:
    OsDoDialog(dlgColorReplace);
    break;

  // Maze menu

  case cmdDlgSizeMaze:
    OsDoDialog(dlgSizeMaze);
    break;

  case cmdBias:
    OsDoDialog(dlgBias);
    break;

  case cmdAddIn:
    bm.b.AddEntranceExit(fFalse);
    break;

  case cmdAddOut:
    bm.b.AddEntranceExit(fTrue);
    break;

  case cmdAddIsolation:
  case cmdAddDetachment:
    bm.b.MakeIsolationDetachment(wCmd == cmdAddDetachment);
    break;

  case cmdCrackIsolation:
    l = bm.b.DoCrackIsolations();
    SetMacroReturn(l);
    PrintSzL("Isolations cracked: %ld\n", l);
    break;

  case cmdCrackDetach:
    l = bm.b.DoConnectDetachments();
    SetMacroReturn(l);
    PrintSzL("Detachments connected: %ld\n", l);
    break;

  case cmdCrackBlock:
    l = bm.b.DoCrackDeadEnds();
    SetMacroReturn(l);
    PrintSzL("Dead ends cracked: %ld\n", l);
    break;

  case cmdCrackEntrance:
    l = bm.b.DoCrackEntrances();
    SetMacroReturn(l);
    PrintSzL("Entrances sealed: %ld\n", l);
    break;

  case cmdCrackPole:
    l = bm.b.DoConnectPoles(!ms.fPoleNoDeadEnd);
    SetMacroReturn(l);
    PrintSzL("Poles connected: %ld\n", l);
    break;

  case cmdDeletePole:
    l = bm.b.DoDeletePoles(ms.fSolveEveryPixel);
    SetMacroReturn(l);
    PrintSzL("Poles removed: %ld\n", l);
    break;

  case cmdCrackCell:
    l = bm.b.DoSetAllCellsToPoles();
    SetMacroReturn(l);
    PrintSzL("Cells cracked: %ld\n", l);
    break;

  case cmdCrackBlindAlley:
    l = bm.b.DoCrackBlindAlleys(bm.b2);
    SetMacroReturn(l);
    PrintSzL("Blind alleys cracked: %ld\n", l);
    break;

  case cmdCrackPassage:
    l = bm.b.DoCrackPassages();
    SetMacroReturn(l);
    PrintSzL("Passages filled: %ld\n", l);
    break;

  case cmdCrackRoom:
    l = bm.b.DoCrackRooms();
    SetMacroReturn(l);
    PrintSzL("Open cells filled: %ld\n", l);
    break;

  case cmdCrackBoundary:
    l = bm.b.DoCrackBoundaryWall(!dr.fNoCorner, dr.nEdge);
    SetMacroReturn(l);
    PrintSzL("Boundary wall cells cracked: %ld\n", l);
    break;

  case cmdCrackTube:
    l = bm.b.DoCrackTubes();
    SetMacroReturn(l);
    PrintSzL("Tube cells cracked: %ld\n", l);
    break;

  case cmdNormalize:
    if (FOdd(xh - xl) || FOdd(yh - yl))
      bm.b.BlockOuter(fOff);
    l = bm.b.MazeNormalize(fOff);
    SetMacroReturn(l);
    PrintSzL("Cells cleared: %ld\n", l);
    break;

  case cmdNormalize2:
    l = bm.b.MazeNormalize(fOn);
    SetMacroReturn(l);
    PrintSzL("Poles set: %ld\n", l);
    break;

  case cmdEraseCorner:
    if (!bm.b.FZero()) {
      i = !bm.b.Get(xh, yh);
      bm.b.BlockOuter(i);
    }
    break;

  case cmdSymmetric:
    l = bm.b.DoMakeSymmetric(0, ws.wCmdSpree == cmdCreateBraid);
    SetMacroReturn(l);
    PrintSzL("Asymmetries in Maze: %ld\n", l);
    break;

  case cmdExpandSet:
    l = bm.b.MazeZoomAndExpandSetCells();
    SetMacroReturn(l);
    PrintSzL("Cells expanded: %ld\n", l);
    break;

  case cmdRoomThin:
    l = bm.b.MazeRoomifyThinnedCells();
    SetMacroReturn(l);
    PrintSzL("Cells roomified: %ld\n", l);
    break;

  case cmdTweakEndpoint:
    l = bm.b.MazeTweakEndpoints();
    SetMacroReturn(l);
    PrintSzL("Endpoints tweaked: %ld\n", l);
    break;

  case cmdTweakPassage:
    l = bm.b.MazeTweakPassages();
    SetMacroReturn(l);
    PrintSzL("Passages tweaked: %ld\n", l);
    break;

  case cmdWeave3D:
    bm.b.ConvertWeaveTo3D(dr.nTrans != nTransNone ? &bm.b3 : NULL);
    break;

  case cmdWeaveInside:
    if (dr.nWallVar < 0) {
      dr.nWallVar = Max(ws.ccTexture, ws.cbMask);
      dr.nWallVar = Max(dr.nWallVar, 1);
    }
    i = dr.zCell / dr.zCellNarrow; i = Max(i, 2);
    bm.b.ConvertWeaveToInside(bm.k, *ColmapGetTexture(dr.nWallVar),
      *BitmapGetMask(dr.nWallVar), i, dr.kvInWall, dr.kvIn3D, dr.kvInCeil);
    dr.fWall = dr.fOnTemp = dr.fInside = fTrue;
    dr.fNarrow = fFalse;
    if (dr.nInside != nInsideFree && dr.nInside != nInsideVeryFree)
      dr.nInside = nInsideVeryFree;
    DoSetVariableW(vosEnsureTimer, ws.nTimerDelay);
    dr.zWall = i * 100;
    dr.nSpeedXY = dr.zCell >> 1;
    dr.nSpeedZ = 8 * i;
    dr.zStep = (dr.zWall * 3 >> 1) / i;
    dr.nClip = Max(dr.nClip, 400);
    bm.b.SetXyh();
    x = y = 0;
    bm.b.FFindPassage(&x, &y, fFalse);
    dr.x = x + (i >> 1); dr.y = y; dr.z = 0; dr.dir = 2;
    break;

  case cmdClarifyWeave:
    bm.b.FClarifyWeave(ms.nWeaveOn, ms.nWeaveOff, ms.nWeaveRail);
    break;

  case cmdClarify3D:
    if (!bm.fColor)
      bm.b.FClarify3D(bm.b, fOff, fOn);
    else
      bm.b.FClarify3D(bm.k, dr.kvOff, dr.kvOn);
    break;

  case cmdClarify4D:
    if (!bm.fColor)
      bm.b.FClarify4D(bm.b, fOff, fOn);
    else
      bm.b.FClarify4D(bm.k, dr.kvOff, dr.kvOn);
    break;

  case cmdCountPossible:
    i = Max(bm.b.m_x-1, 0) >> 1; j = Max(bm.b.m_y-1, 0) >> 1;
    ul = MazeCountPossible(i, j);
    SetMacroReturn(ul);
    if (ul != ~0L)
      sprintf(S(szT), "%lu", ul);
    else
      sprintf(S(szT), "Billions");
    sprintf(S(sz), "Total possible %d by %d Mazes: %s\n", i, j, szT);
    PrintSz_N(sz);
    break;

  case cmdAnalyze:
    if (!dr.f3D)
      bm.b.MazeAnalyze();
    else
      bm.b.MazeAnalyze3D();
    break;

  case cmdAnalyze2:
    bm.b.MazeAnalyze2();
    break;

  case cmdDlgMaze:
    OsDoDialog(dlgMaze);
    break;

  // Create menu

  case cmdCreatePerfect:
    bm.b.CreateMazePerfect();
    break;

  case cmdCreateBraid:
    if (ms.nBraid <= 0)
      bm.b.CreateMazeBraid();
    else if (ms.nBraid == 1)
      bm.b.CreateMazeBraidEller();
    else
      bm.b.CreateMazeBraidTilt();
    break;

  case cmdCreateUnicursal:
    bm.b.CreateMazeUnicursal();
    break;

  case cmdCreateRecursive:
    bm.b.CreateMazeRecursive();
    break;

  case cmdCreatePrim:
    bm.b.CreateMazePrim();
    break;

  case cmdCreatePrim2:
    bm.b.CreateMazePrim2(dr.x, dr.y);
    break;

  case cmdCreateKruskal:
    bm.b.CreateMazeKruskal(fTrue, NULL, ms.fKruskalPic ? &bm.k : NULL);
    break;

  case cmdCreateAldous:
    bm.b.CreateMazeAldousBroder();
    break;

  case cmdCreateWilson:
    bm.b.CreateMazeWilson();
    break;

  case cmdCreateEller:
    bm.b.CreateMazeEller();
    break;

  case cmdCreateTree:
    bm.b.CreateMazeTree();
    break;

  case cmdCreateForest:
    bm.b.CreateMazeForest();
    break;

  case cmdCreateDivision:
    if (!dr.f3D)
      bm.b.CreateMazeDivision();
    else
      bm.b.CreateMazeDivision3D();
    break;

  case cmdCreateBinary:
    if (!dr.f3D)
      bm.b.CreateMazeBinary();
    else
      bm.b.CreateMazeBinary3D();
    break;

  case cmdCreateSidewinder:
    if (!dr.f3D)
      bm.b.CreateMazeSidewinder();
    else
      bm.b.CreateMazeSidewinder3D();
    break;

  case cmdCreateRandom:
    i = Rnd(0, 8);
    switch (i) {
    case 0: bm.b.CreateMazeRecursive(); break;
    case 1: bm.b.CreateMazePrim();      break;
    case 2: bm.b.CreateMazePrim2(dr.x, dr.y);          break;
    case 3: bm.b.CreateMazeKruskal(fTrue, NULL, NULL); break;
    case 4: bm.b.CreateMazeWilson();    break;
    case 5: bm.b.CreateMazeEller();     break;
    case 6: bm.b.CreateMazeTree();      break;
    case 7: bm.b.CreateMazeForest();    break;
    case 8: bm.b.CreateMazePerfect();   break;
    }
    break;

  case cmdCreateSpiral:
    bm.b.CreateMazeSpiral();
    break;

  case cmdCreateDiagonal:
    bm.b.CreateMazeDiagonal();
    break;

  case cmdCreateSegment:
    bm.b.CreateMazeSegment();
    break;

  case cmdCreateFractal:
    if (!dr.f3D)
      bm.b.CreateMazeFractal();
    else
      bm.b.CreateMazeFractal3D();
    break;

  case cmdCreateWeave:
    bm.b.CreateMazeWeave();
    break;

  case cmdCreate3D:
    bm.b.CreateMaze3D();
    break;

  case cmdCreate4D:
    bm.b.CreateMaze4D();
    break;

  case cmdCreatePlanair:
    bm.b.CreateMazePlanair();
    break;

  case cmdCreateHyper:
    bm.b.CreateMazeHyper();
    break;

  case cmdCreateCrack:
    l = bm.b.CreateMazeCrack(fTrue);
    SetMacroReturn(l);
    break;

  case cmdCreateCavern:
    bm.b.CreateMazeCavern();
    break;

  case cmdCreateArrow:
    bm.b.CreateMazeArrow(!bm.fColor ? NULL : &bm.k);
    break;

  case cmdCreateHilbert:
    bm.b.CreateMazeHilbert(dr.f3D);
    break;

  case cmdCreateTilt:
    bm.b.CreateMazeTilt();
    break;

  case cmdCreateFractal2:
    if (bm.k.FNull())
      bm.k.FColmapGetFromBitmap(bm.b, dr.kvOff, dr.kvOn);
    i = bm.b.CreateMazeFractal2(&bm.k);
    SetMacroReturn(i);
    break;

  case cmdCreateInfinite0:
  case cmdCreateInfinite1:
  case cmdCreateInfinite2:
  case cmdCreateInfinite3:
    bm.b.CreateMazeInfinite(wCmd - cmdCreateInfinite0);
    break;

  case cmdCreateGamma:
    bm.b.CreateMazeGamma();
    break;

  case cmdCreateTheta:
    bm.b.CreateMazeTheta();
    break;

  case cmdCreateDelta:
    bm.b.CreateMazeDelta();
    break;

  case cmdCreateSigma:
    bm.b.CreateMazeSigma();
    break;

  case cmdCreateUpsilon:
    bm.b.CreateMazeUpsilon();
    break;

  case cmdCreateOmicron:
    bm.b.CreateMazeOmicron();
    break;

  case cmdCreateZeta:
    bm.b.CreateMazeZeta();
    break;

  case cmdCreateLabyrinth:
    bm.b.CreateLabyrinth();
    break;

  case cmdDlgLabyrinth:
    OsDoDialog(dlgLabyrinth);
    break;

  case cmdLabyrinthClassical:
    ms.nLabyrinth = ltClassical;
    if (ms.lcs == lcsRound || ms.lcs == lcsSquare)
      ms.lcs = lcsCircle;
    bm.b.CreateLabyrinth();
    break;

  case cmdLabyrinthChartres:
    ms.nLabyrinth = ltChartres;
    if (ms.lcs == lcsFlat)
      ms.lcs = lcsCircle;
    bm.b.CreateLabyrinth();
    break;

  case cmdLabyrinthCretan:
    ms.nLabyrinth = ltCretan;
    bm.b.CreateLabyrinth();
    break;

  case cmdLabyrinthMan:
    ms.nLabyrinth = ltManInMaze;
    bm.b.CreateLabyrinth();
    break;

  case cmdLabyrinthClassicalF:
    ms.nLabyrinth = ltClassical;
    if (ms.lcs == lcsCircle || ms.lcs == lcsFlat)
      ms.lcs = lcsRound;
    bm.b.CreateLabyrinth();
    break;

  case cmdLabyrinthChartresF:
    ms.nLabyrinth = ltChartres; ms.lcs = lcsFlat;
    bm.b.CreateLabyrinth();
    break;

  case cmdLabyrinthChartresR:
    ms.nLabyrinth = ltChartresR;
    if (ms.lcs == lcsFlat)
      ms.lcs = lcsCircle;
    bm.b.CreateLabyrinthChartres();
    break;

  case cmdLabyrinthCustom:
    ms.nLabyrinth = ltCustom;
    bm.b.CreateLabyrinthCustom();
    break;

  case cmdPartPerfect:
    bm.b.PerfectGenerate(fFalse, dr.x, dr.y);
    ms.cMaze++;
    break;

  case cmdPartBraid:
    l = bm.b.BraidConnectWalls();
    SetMacroReturn(l);
    PrintSzL("Walls added: %ld\n", l);
    break;

  case cmdPartUnicursal:
    bm.b.UnicursalApply(fFalse);
    break;

  case cmdPartRecursive:
    bm.b.RecursiveGenerate(dr.x, dr.y);
    ms.cMaze++;
    break;

  case cmdPartPrim:
    bm.b.PrimGenerate(fFalse, fFalse, dr.x >> 1, dr.y >> 1);
    ms.cMaze++;
    break;

  case cmdPartKruskal:
    bm.b.CreateMazeKruskal(fFalse, &bm.k, ms.fKruskalPic ? &bm.k2 : NULL);
    ms.cMaze++;
    break;

  case cmdPartTree:
    bm.b.TreeGenerate(fFalse, dr.x, dr.y);
    ms.cMaze++;
    break;

  case cmdPartWeave:
    bm.b.WeaveGenerate(fFalse, dr.x, dr.y);
    ms.cMaze++;
    break;

  case cmdPartCrack:
    l = bm.b.CreateMazeCrack(fFalse);
    SetMacroReturn(l);
    ms.cMaze++;
    break;

  case cmdPartCavern:
    bm.b.CavernGenerate(fFalse, dr.x, dr.y);
    ms.cMaze++;
    break;

  case cmdPartZeta:
    bm.b.ZetaGenerate(fFalse, dr.x, dr.y);
    ms.cMaze++;
    break;

  case cmdPolishMaze:
    if (!dr.fPolishMaze) {
      dr.fPolishMaze = fTrue;
      if (dr.kvOn == kvWhite && dr.kvOff == kvBlack) {
        dr.kvOff = kvWhite; dr.kvOn = kvBlack;
      }
      if (dr.kvEdge == kvGray)
        dr.kvEdge = kvWhite;
      if (rgcmd[ws.wCmdSpree - icmdBase].grf & fCmtMaze)
        PolishMaze(ws.wCmdSpree, 1);
      if (ws.xScroll == 0 && ws.yScroll == 0) {
        DoSetVariableW(vosScrollX, nScrollPage);
        DoSetVariableW(vosScrollY, nScrollPage);
      }
    } else {
      if (dr.kvOn == kvBlack && dr.kvOff == kvWhite) {
        dr.kvOff = kvBlack; dr.kvOn = kvWhite;
      }
      if (dr.kvEdge == kvWhite)
        dr.kvEdge = kvGray;
      if (rgcmd[ws.wCmdSpree - icmdBase].grf & fCmtMaze)
        PolishMaze(ws.wCmdSpree, 2);
      if (ws.xScroll == nScrollPage && ws.yScroll == nScrollPage) {
        DoSetVariableW(vosScrollX, 0);
        DoSetVariableW(vosScrollY, 0);
      }
      dr.fPolishMaze = fFalse;
    }
    CheckMenuW(cmdPolishMaze, dr.fPolishMaze);
    break;

  case cmdDlgCreate:
    OsDoDialog(dlgCreate);
    break;

  // Solve menu

  case cmdSolveBlock:
    if (!dr.f3D) {
      l = bm.b.SolveMazeFillDeadEnds(dr.x, dr.y, dr.x2, dr.y2);
      PrintSzL("Total dead ends filled: %ld\n", l);
    } else {
      l = bm.b.SolveMaze3D();
      PrintSzL("Total 3D dead ends filled: %ld\n", l);
    }
    SetMacroReturn(l);
    break;

  case cmdSolveCuldsac:
    l = bm.b.SolveMazeFillCulDeSacs(dr.x, dr.y, dr.x2, dr.y2);
    SetMacroReturn(l);
    PrintSzL("Total cul-de-sacs filled: %ld\n", l);
    break;

  case cmdSolveBlindAlley:
    l = bm.b.SolveMazeFillBlindAlleys(dr.x, dr.y, dr.x2, dr.y2);
    SetMacroReturn(l);
    PrintSzL("Blind alleys filled: %ld\n", l);
    break;

  case cmdSolveCollision:
    l = bm.b.SolveMazeFillCollisions(dr.x, dr.y, dr.x2, dr.y2);
    SetMacroReturn(l);
    PrintSzL("Total collisions filled: %ld\n", l);
    break;

  case cmdSolveRecursive:
    l = bm.b.SolveMazeRecursive(dr.x, dr.y, dr.x2, dr.y2, !dr.fNoCorner);
    SetMacroReturn(l);
    if (l <= 0)
      PrintSz_W("No solution found.");
    else
      PrintSzL("Length of this solution: %ld\n", l);
    break;

  case cmdSolveShortest:
    l = bm.b.SolveMazeShortest(dr.x, dr.y, dr.x2, dr.y2, !dr.fNoCorner);
    SetMacroReturn(l);
    if (l <= 0)
      PrintSz_W("No solution found.");
    else
      PrintSzL("Length of shortest solution: %ld\n", l);
    break;

  case cmdSolveShortest2:
    l = bm.b.SolveMazeShortest2(dr.x, dr.y, dr.x2, dr.y2, !dr.fNoCorner);
    SetMacroReturn(l);
    if (!ms.fCountShortest || l > 0)
      sprintf(S(szT), "%ld", l);
    else
      sprintf(S(szT), "Billions");
    sprintf(S(sz), "%s of shortest solutions: %s\n",
      !ms.fCountShortest ? "Length" : "Number", szT);
    PrintSz(sz);
    break;

  case cmdSolveFollowL:
  case cmdSolveFollowR:
    l = bm.b.SolveMazeFollowWall(dr.x, dr.y, dr.dir, dr.x2, dr.y2,
      wCmd == cmdSolveFollowR);
    SetMacroReturn(l);
    PrintSzL("Points traversed: %ld\n", l);
    break;

  case cmdSolvePledgeL:
  case cmdSolvePledgeR:
    l = bm.b.SolveMazePledge(dr.x, dr.y, dr.dir, dr.x2, dr.y2,
      wCmd == cmdSolvePledgeR);
    SetMacroReturn(l);
    PrintSzL("Points traversed: %ld\n", l);
    break;

  case cmdSolveChain:
    l = bm.b.SolveMazeChain(dr.x, dr.y, dr.x2, dr.y2);
    SetMacroReturn(l);
    PrintSzL("Points traversed: %ld\n", l);
    break;

  case cmdSolveWeave:
    l = bm.b.SolveMazeWeave();
    SetMacroReturn(l);
    PrintSzL("Total weave dead ends filled: %ld\n", l);
    break;

  case cmdSolve4D:
    l = bm.b.SolveMaze4D();
    SetMacroReturn(l);
    PrintSzL("Total 4D dead ends filled: %ld\n", l);
    break;

  case cmdSolveHyper:
    l = bm.b.SolveMazeHyper();
    SetMacroReturn(l);
    PrintSzL("Total hyper cells filled: %ld\n", l);
    break;

  case cmdSolveFractal2:
    i = bm.b.SolveMazeFractal2();
    SetMacroReturn(i);
    break;

  case cmdSolveTremaux:
    l = bm.b.SolveMazeTremaux(bm.k, dr.x, dr.y, dr.x2, dr.y2,
      dr.kvOff, dr.kvOn, dr.kvDot, dr.kvMap);
    SetMacroReturn(l);
    if (l <= 0)
      PrintSz_W("No solution found.");
    else
      PrintSzL("Points traversed: %ld\n", l);
    break;

  case cmdMarkBlock:
    l = bm.b.DoMarkDeadEnds(dr.x, dr.y, dr.x2, dr.y2);
    SetMacroReturn(l);
    PrintSzL("Dead ends marked: %ld\n", l);
    break;

  case cmdMarkCuldsac:
    l = bm.b.DoMarkCulDeSacs(dr.x, dr.y, dr.x2, dr.y2);
    SetMacroReturn(l);
    PrintSzL("Cul-de-sacs marked: %ld\n", l);
    break;

  case cmdMarkBlindAlley:
    l = bm.b.DoMarkBlindAlleys();
    SetMacroReturn(l);
    PrintSzL("Blind alley cells filled: %ld\n", l);
    break;

  case cmdMarkCollision:
    l = bm.b.DoMarkCollisions(dr.x, dr.y);
    SetMacroReturn(l);
    PrintSzL("Collisions marked: %ld\n", l);
    break;

  case cmdMarkBlock2:
    l = bm.b.DoFillDeadEnds(dr.x, dr.y, dr.x2, dr.y2);
    SetMacroReturn(l);
    PrintSzL("Dead ends filled: %ld\n", l);
    break;

  case cmdShowBottleneck:
    bm.b.DoShowBottlenecks(dr.x, dr.y, dr.dir, dr.x2, dr.y2);
    break;

  case cmdFloodPassage:
    bm.b.FFillReverse(xh, yh);
    break;

  case cmdFloodWallL:
    bm.b.FFillReverse(xl, yl);
    break;

  case cmdFloodWallR:
    bm.b.FFillReverse(xh-1, yl);
    break;

  // Draw menu

  case cmdDlgDraw:
    OsDoDialog(dlgDraw);
    break;

  case cmdDlgDraw2:
    OsDoDialog(dlgDraw2);
    break;

  case cmdRenderOverview:
    if (!dr.f3D)
      DrawOverview(BFocus(), bm.b);
    else
      DrawOverviewCube(BFocus(), bm.b);
    break;

  case cmdRenderPyramid:
    DrawPyramid(bm.b, !bm.fColor ? NULL : &bm.k);
    break;

  case cmdLife:
    if (!(ms.nInfCutoff > 0 && ms.nInfGen >= ms.nInfCutoff)) {
      if (!bm.fColor)
        l = bm.b.LifeGenerate(dr.nEdge == nEdgeTorus);
      else
        l = bm.k.ColmapLifeGenerate(bm.k2, dr.nEdge == nEdgeTorus);
      SetMacroReturn(l);
      ms.nInfGen++;
      PrintSzL("Cells changed: %ld\n", l);
    }
    break;

  case cmdCreateWireOverview:
    bm.ccoor = CreateWireframe(bm.b, &bm.coor, dr.f3D,
      bm.k.FNull() ? NULL : &bm.k);
    SetMacroReturn(bm.ccoor);
    PrintSzL("Total number of lines generated: %ld\n", bm.ccoor);
    break;

  case cmdRenderWirePerspective:
    FRenderPerspectiveWire(BFocus(), bm.coor, bm.ccoor);
    ds.fDidPatch = fFalse;
    break;

  case cmdRenderAerial:
    FRenderAerial(BFocus(), bm.coor, bm.ccoor);
    ds.fDidPatch = fFalse;
    break;

  case cmdCreatePatchOverview:
    bm.cpatch = CreatePatches(bm.b, &bm.patch, dr.f3D,
      bm.k.FNull() ? NULL : &bm.k);
    SetMacroReturn(bm.cpatch);
    PrintSzL("Total number of patches generated: %ld\n", bm.cpatch);
    break;

  case cmdRenderPatchPerspective:
    FRenderPerspectivePatch(BFocus(), bm.patch, bm.cpatch);
    ds.fDidPatch = fTrue;
    break;

  case cmdPatchToWire:
    bm.ccoor = ConvertPatchToWire(&bm.coor, bm.patch, bm.cpatch);
    SetMacroReturn(bm.ccoor);
    PrintSzL("Total number of lines generated: %ld\n", bm.ccoor);
    break;

  default:
    // Run Macro submenus
    if (FBetween(wCmd, cmdMacro01, cmdMacro48)) {
      i = wCmd - cmdMacro01 + 1;
      if (i >= ws.cszMacro || ws.rgszMacro[i] == NULL) {
        if (i == 1/*F1*/)
          DoCommand(cmdShellHelp);
        else if (i == 40/*Alt+F4*/)
          DoCommand(cmdExit);
        else if (!ws.fIgnorePrint)
          PrintSzN_W("Macro number %d is not defined.\n", i);
        break;
      }
      RunMacro(i);
      break;
    } else if (FBetween(wCmd, cmdScript01, cmdScriptLast)) {
      i = wCmd - cmdScript01;
      if (!FBootExternal(rgszScript[i], fTrue)) {
        sprintf(S(sz), "Script file '%s' not found.", rgszScript[i]);
        PrintSz_E(sz);
      }
      break;
    }
    PrintSzN_E("Command %d is unimplemented.", icmd);
    return fFalse;
  }

  // Update things after the command runs based on command flags.
  if (grf & fCmtBitmapAfter)
    FShowColmap(fFalse);
  else if (grf & fCmtColmapAfter)
    FShowColmap(fTrue);
  if (grf & fCmtSpree)
    if (!ws.fSpree)
      ws.wCmdSpree = wCmd;
  if (grf & fCmtMazeEntrance)
    if (ms.fTeleportEntrance) {
      dr.x = ms.xEntrance; dr.y = ms.yEntrance; dr.z = 0; dr.dir = 0;
      CenterWithinCell();
    }
  if (grf & fCmtMazeCenter)
    CenterWithinCell();
  if (grf & fCmtMaze) {
    ms.cMaze++;
    PolishMaze(wCmd, 1);
  } else if ((grf & fCmtSolve) > 0 && bm.fColor && !bT.FNull()) {
    if (bm.k.FColmapBlendFromBitmap(&bm.b, &bT, NULL)) {
      if (dr.kvDot != kvRed)
        bm.k.ColmapReplaceSimple(kvRed, dr.kvDot);
      if (dr.kvMap != kvCyan)
        bm.k.ColmapReplaceSimple(kvCyan, dr.kvMap);
    }
    bm.b.FBitmapCopy(bT);
  }
  if (grf & fCmtDirtyView)
    SystemHook(hosDirtyView);
  if ((grf & fCmtRedraw) > 0 && ws.fNoDirty) {
    fDidRedraw = fTrue;
    SystemHook(hosRedraw);
  }
  if ((grf & fCmtHourglass) > 0 && cursorPrev != NULL)
    HourglassCursor(&cursorPrev, fFalse);

LDone:
  if (wCmd != cmdRepeat)
    ws.wCmdRepeat = wCmd;
  return fDidRedraw;
}


/*
******************************************************************************
** Command Line Processing
******************************************************************************
*/

// Create trie tree lookup tables for the various action lists. This work is
// only done once, right before the first lookup is needed.

flag FCreateTries()
{
  char *rgsz[iActionMax];
  int csMax = 16000, isz, cs;

  ws.rgsTrieAlloc = RgAllocate(csMax, short);
  if (ws.rgsTrieAlloc == NULL)
    return fFalse;

  ws.rgsTrieCmd = ws.rgsTrieAlloc;
  Assert(ccmd <= iActionMax);
  for (isz = 0; isz < ccmd; isz++)
    rgsz[isz] = rgcmd[isz].szName;
  cs = CsCreateTrie((CONST uchar **)rgsz, ccmd, ws.rgsTrieCmd, csMax, fTrue);
  csMax -= cs;

  ws.rgsTrieAbbrev = ws.rgsTrieCmd + cs;
  Assert(ccmd <= iActionMax);
  for (isz = 0; isz < ccmd; isz++)
    rgsz[isz] = rgcmd[isz].szAbbrev;
  cs = CsCreateTrie((CONST uchar **)rgsz, ccmd, ws.rgsTrieAbbrev, csMax,
    fFalse);
  csMax -= cs;

  ws.rgsTrieOpr = ws.rgsTrieAbbrev + cs;
  Assert(copr <= iActionMax);
  for (isz = 0; isz < copr; isz++)
    rgsz[isz] = rgopr[isz].szName;
  cs = CsCreateTrie((CONST uchar **)rgsz, copr, ws.rgsTrieOpr, csMax, fTrue);
  csMax -= cs;

  ws.rgsTrieVar = ws.rgsTrieOpr + cs;
  Assert(cvar <= iActionMax);
  for (isz = 0; isz < cvar; isz++)
    rgsz[isz] = rgvar[isz].szName;
  cs = CsCreateTrie((CONST uchar **)rgsz, cvar, ws.rgsTrieVar, csMax, fTrue);
  csMax -= cs;

  ws.rgsTrieFun = ws.rgsTrieVar + cs;
  Assert(cfun <= iActionMax);
  for (isz = 0; isz < cfun; isz++)
    rgsz[isz] = rgfun[isz].szName;
  cs = CsCreateTrie((CONST uchar **)rgsz, cfun, ws.rgsTrieFun, csMax, fTrue);
  csMax -= cs;

  ws.rgsTrieKv = ws.rgsTrieFun + cs;
  cs = CsCreateTrie((CONST uchar **)rgszColor, cColor, ws.rgsTrieKv, csMax,
    fTrue);
  csMax -= cs;

#ifdef ASSERT
  // Sanity check to ensure the tries are valid. For each string, look it up
  // in its trie and see if it returns the expected index.

  for (isz = 0; isz < ccmd; isz++) {
    cs = ILookupTrieSz(ws.rgsTrieCmd, rgcmd[isz].szName, fTrue);
    if (cs != isz) {
      PrintSzNN_E("Command string %d maps to trie value %d.", isz, cs);
      break;
    }
  }
  for (isz = 0; isz < ccmd; isz++) {
    cs = ILookupTrieSz(ws.rgsTrieAbbrev, rgcmd[isz].szAbbrev, fFalse);
    if (rgcmd[isz].szAbbrev[0] != chNull && cs != isz) {
      PrintSzNN_E("Command abbreviation %d maps to trie value %d.", isz, cs);
      break;
    }
  }
  for (isz = 0; isz < copr; isz++) {
    cs = ILookupTrieSz(ws.rgsTrieOpr, rgopr[isz].szName, fTrue);
    if (cs != isz) {
      PrintSzNN_E("Operation string %d maps to trie value %d.", isz, cs);
      break;
    }
  }
  for (isz = 0; isz < cvar; isz++) {
    cs = ILookupTrieSz(ws.rgsTrieVar, rgvar[isz].szName, fTrue);
    if (cs != isz) {
      PrintSzNN_E("Variable string %d maps to trie value %d.", isz, cs);
      break;
    }
  }
  for (isz = 0; isz < cfun; isz++) {
    cs = ILookupTrieSz(ws.rgsTrieFun, rgfun[isz].szName, fTrue);
    if (cs != isz) {
      PrintSzNN_E("Function string %d maps to trie value %d.", isz, cs);
      break;
    }
  }
  for (isz = 0; isz < cColor; isz++) {
    cs = ILookupTrieSz(ws.rgsTrieKv, rgszColor[isz], fTrue);
    if (cs != isz) {
      PrintSzNN_E("Color string %d maps to trie value %d.", isz, cs);
      break;
    }
  }
#endif
  return fTrue;
}


// Function hook to parse an arbitrary expression, like in dialog fields.

long ParseExpression(char *sz)
{
  long l = 0; // If parsing fails, assume 0.

  // Ensure the lookup tables have been created.
  if (ws.rgsTrieAlloc == NULL && !FCreateTries())
    return l;

  PchGetParameter(sz, NULL, NULL, &l, 1);
  return l;
}


// Run a command line, executing each action in it in sequence.
// Return values: 0 = Success, 1+ = Caller(s) should break.

int RunCommandLine(char *szLine, FILE *file)
{
  char szCmd[cchSzDef], szT[cchSzDef*2];
  char *pchCur, *pchCmd, ch;
  int cchCmd, icmd, iopr, ivar;
  char *rgsz[7], *sz;
  int rgcch[7], cch, iParam, n, nRet;
  long rglLocal[iLetterZ+1], rgl[7], l;

  if (szLine == NULL)
    return 0;
  rglLocal[0] = 0;
  ws.rglLocal = rglLocal;
  ws.nMacroDepth++;
  if (ws.nMacroDepth > ws.nMacroDepthMax)
    ws.nMacroDepthMax = ws.nMacroDepth;
  if (ws.nMacroDepth > 100) {
    PrintSzN_E("Command lines too heavily nested!\n"
      "Nesting level reached: %d", ws.nMacroDepth-1);
    goto LError;
  }
LRestart:
  pchCur = szLine;
  while (*pchCur) {
    while (*pchCur == ' ')
      pchCur++;
    pchCmd = pchCur;
    while (*pchCur && *pchCur != ' ')
      pchCur++;
    cchCmd = PD(pchCur - pchCmd);
    if (cchCmd > 0) {

      // Ensure the lookup tables have been created.
      if (ws.rgsTrieAlloc == NULL && !FCreateTries())
        goto LError;

      // Check for custom numeric or string variable assignment.
      ch = pchCmd[0];
      if ((ch == '@' || ch == '$') && cchCmd > 1) {
        n = ChCap(pchCmd[1]);
        if (cchCmd == 2 && FCapCh(n)) {
          ivar = n - '@';
          if (ch == '$')
            ivar = LVar(ivar);
        } else if (FDigitCh(pchCmd[1])) {
          ivar = 0;
          for (n = 1; n < cchCmd; n++) {
            if (!FDigitCh(pchCmd[n]))
              goto LUnknown;
            ivar = ivar * 10 + (pchCmd[n] - '0');
          }
        } else {
          if (ws.rgsTrieConst == NULL)
            goto LUnknown;
          ivar = ILookupTrie(ws.rgsTrieConst, pchCmd+1, cchCmd-1, fFalse);
          if (ivar < 0)
            goto LUnknown;
          ivar = ws.rgnTrieConst[ivar];
        }
        pchCur = PchGetParameter(pchCur, &sz, &cch, &l, -1);
        if (pchCur == NULL)
          goto LError;
        if (ch == '@') {
          if (!FEnsureLVar(ivar + 1))
            goto LError;
          ws.rglVar[ivar] = l;
        } else {
          if (!FEnsureSzVar(ivar + 1))
            goto LError;
          SetSzVar(sz, cch, ivar);
        }
        goto LAfter;
      }

      // Check for custom constant macro
      if (ch == '*' && cchCmd > 1) {
        if (ws.rgsTrieConst == NULL)
          goto LUnknown;
        n = ILookupTrie(ws.rgsTrieConst, pchCmd+1, cchCmd-1, fFalse);
        if (n < 0)
          goto LUnknown;
        nRet = RunMacro(ws.rgnTrieConst[n]);
        if (nRet > 1) {
          nRet--;
          goto LReturn;
        }
        goto LAfter;
      }

      // Check for command.
      if ((icmd = CmdFromRgch(pchCmd, cchCmd)) >= 0) {
        DoCommand(rgcmd[icmd].wCmd);
        goto LAfter;
      }

      // Check for operation.
      iopr = ILookupTrie(ws.rgsTrieOpr, pchCmd, cchCmd, fTrue);
      if (iopr >= 0) {
        for (iParam = 0; iParam < rgopr[iopr].nParam; iParam++) {
          pchCur = PchGetParameter(pchCur, &rgsz[iParam], &rgcch[iParam],
            &rgl[iParam], iParam);
          if (pchCur == NULL)
            goto LError;
        }
        n = DoOperation(iopr, rgsz, rgcch, rgl, file);
        if (n == -1)
          goto LRestart;
        else if (n > 0) {
          nRet = n-1;
          goto LReturn;
        }
        goto LAfter;
      }

      // Check for variable assignment.
      ivar = ILookupTrie(ws.rgsTrieVar, pchCmd, cchCmd, fTrue);
      if (ivar >= 0) {
        pchCur = PchGetParameter(pchCur, &sz, &cch, &l, -1);
        if (pchCur == NULL)
          goto LError;
        DoSetVariable(ivar, sz, cch, l);
        goto LAfter;
      }

      // Check for filename on command line that started the program itself.
      if (ws.fStarting && pchCmd == szLine) {
        FFileOpen(cmdOpen, szLine, NULL);
        SystemHook(hosDirtyView);
        goto LHalt;
      }

LUnknown:
      CopyRgchToSz(pchCmd, cchCmd, szCmd, cchSzDef);
      sprintf(S(szT), "Unknown action: '%s'\nContext: ", szCmd);
      for (pchCmd = szT; *pchCmd; pchCmd++)
        ;
      pchCur = szLine;
      while (*pchCur && pchCmd - szT < cchSzDef*2)
        *pchCmd++ = *pchCur++;
      *pchCmd = chNull;
      PrintSz_W(szT);
      goto LError;
    }
LAfter:
    if (ws.fQuitting)
      goto LHalt;
  }
  nRet = 0;
  goto LReturn;

LError:
  ws.fSpree = fFalse;
LHalt:
  nRet = nRetHalt;
LReturn:
  ws.nMacroDepth--;
  if (rglLocal[0]) {
    for (ivar = 1; ivar <= cLetter; ivar++)
      if (rglLocal[0] & (1 << ivar))
        ws.rglVar[ivar] = rglLocal[ivar];
  }
  ws.rglLocal = NULL;
  return nRet;
}


// Run a macro, treating its string as a command line.

int RunMacro(int iMacro)
{
  int nRet;

  if (iMacro >= ws.cszMacro)
    return 0;
  nRet = RunCommandLine(ws.rgszMacro[iMacro], NULL);
  if (ws.fReturning) {
    ws.fReturning = fFalse;
    return 0;
  }
  return nRet;
}


// Run a sequence of command lines. Special variables allow embedded files
// within the strings to work, and be still treated as a stream of file data.

void RunCommandLines(CONST char *rgsz[])
{
  char szLine[cchSzOpr];
  int i;

  for (i = 0; rgsz[i] != NULL; i++) {
    ws.rgszStartup = &rgsz[i + 1];
    ws.iszStartup = ws.ichStartup = 0;
    CopySz(rgsz[i], szLine, cchSzOpr);
    if (RunCommandLine(szLine, NULL) > 1)
      break;
    if (ws.ichStartup > 0)
      ws.iszStartup++;
    i += ws.iszStartup;
  }
  ws.rgszStartup = NULL;
  ws.fReturning = fFalse;
}


// Read a Daedalus script file, running each command line in it in sequence.

flag FReadScript(FILE *file)
{
  char szLine[cchSzOpr], ch, ch2;
  int i;

  ch = getbyte(); ch2 = getbyte();
  if (ch != 'D' || ch2 != 'S') {
    PrintSz_W("This file does not look like a Daedalus script file.\n");
    return fFalse;
  }

  loop {
    while (!feof(file) && (ch = getbyte()) < ' ')
      ;
    if (feof(file))
      break;
    for (szLine[0] = ch, i = 1; i < cchSzOpr-1 && !feof(file) &&
      (ch = getbyte()) >= ' '; i++)
      szLine[i] = ch;
    skiplf();
    szLine[i] = chNull;
    i = RunCommandLine(szLine, file);
    if (i > 1)
      break;
  }
  ws.fReturning = fFalse;
  return fTrue;
}


// Read a sequence of strings from a file, concatenating them into a single
// string. Implements the Embed and EmbedMacro operations.

char *ReadEmbedLines(FILE *file)
{
  char szLine[cchSzOpr], ch, *sz = NULL, *szNew = NULL;
  int cch = 0, i, j, fSpace;

  if (file == NULL && ws.rgszStartup == NULL)
    return NULL;
  loop {
    for (i = 0; i < cchSzOpr && (file == NULL || !feof(file)) &&
      (ch = getbyte()) >= ' '; i++)
      szLine[i] = ch;
    skiplf();
    if (i <= 0) // Stop reading at the next blank line
      break;
    for (j = 0; szLine[j] == ' '; j++) // Drop initial spaces in each line
      ;
    if (szLine[j] == '/' && szLine[j+1] == '/') // Skip comment lines
      continue;
    ch = (sz == NULL ? '{' : sz[cch-1]);
    fSpace = (ch != '{' && ch != '(' && szLine[j] != '}' && szLine[j] != ')');
    i -= j;
    szNew = RgAllocate(cch + fSpace + i + 1, char);
    if (szNew == NULL)
      break;
    CopyRgb(sz, szNew, cch);
    if (fSpace)
      szNew[cch++] = ' '; // Usually insert a space between lines
    CopyRgb(szLine + j, szNew + cch, i);
    cch += i;
    szNew[cch] = chNull;
    if (sz != NULL)
      DeallocateP(sz);
    sz = szNew;
  }
  return sz;
}

/* command.cpp */
