/*
** Daedalus (Version 3.4) File: graphics.cpp
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
** This file contains general graphics routines, that operate on a monochrome
** bitmap, unrelated to Mazes.
**
** Converted from Modula2: 3/8/1991.
** Last code change: 8/29/2023.
*/

#include <stdio.h>
#include <math.h>
#include "util.h"
#include "graphics.h"


// Offset table for orthogonal and diagonal directions.
CONST int xoff[DIRS2] = { 0,-1, 0, 1, -1, -1, 1, 1};
CONST int yoff[DIRS2] = {-1, 0, 1, 0, -1, 1, 1, -1};

CONST int xoff2[DIRS2] = { 0,-2, 0, 2, -2, -2, 2, 2};
CONST int yoff2[DIRS2] = {-2, 0, 2, 0, -2, 2, 2, -2};

// Simple truth table for logical operators, given the number of true bits.
CONST int rgfOr[3]  = {0, 1, 1};
CONST int rgfAnd[3] = {0, 0, 1};
CONST int rgfXor[3] = {0, 1, 0};

// Dither table giving the order to set points in a 4x4 pixel patch.
CONST int rgnDitherPoint[16] =
  {6, 14,  7, 15,
   9,  1, 12,  4,
   8, 16,  5, 13,
  11,  3, 10,  2};

// Bit flags for all combinations of a consecutive group of <= 3 neighbors.
CONST int rggrfNeighbor[25] = {0x00,
  0x01, 0x10, 0x02, 0x20, 0x04, 0x40, 0x08, 0x80,
  0x11, 0x12, 0x22, 0x24, 0x44, 0x48, 0x88, 0x81,
  0x13, 0x32, 0x26, 0x64, 0x4C, 0xC8, 0x89, 0x91};

// Turtle font for height 5 variable width characters.
CONST char *szTurtleCh1[128-32] = {
  "BR4", "D2BD2D0BU4BR2", "DBR2UBR2", "BRD4BR2U4BFL4BD2R4BU3BR2",
  "NR2D2R2D2LNLU4BR3", "D0BR2DG2DBR2D0BU4BR2", "BFD2LUR2BE2", "NDBR2",
  "BRGD2FBU4BR2", "FD2GBU4BR3", "BFDNLNDNRNHNGNFEBUBR2", "BFD2BHR2BE2",
  "BD4EBU3BR2", "BD2R2BE2", "BD4D0BU4BR2", "BD4UE2UBR2", // Symbols

  "BRGD2FEU2HBR3", "RD4NRLBE4", "R2D2L2D2NR2BE4", "R2D2NL2D2L2BE4",
  "D2R2ND2U2BR2", "NR2D2RFGLBE4", "NR2D4R2U2NL2BE2", "R2DG2DBE4",
  "D4R2U2NL2U2L2BR4", "ND2R2D2NL2D2L2BE4", // Digits

  "BDD0BD2D0BU3BR2", "BFD0BD2GBU4BR3", "BR2G2F2BU4BR2", "BDR2BD2L2BU3BR4",
  "F2G2BE4", "R2D2L2BD2D0BE4", "BF2U2L2D4NR2BE4", // Symbols

  "NR2D2ND2R2ND2U2BR2", "ND4RFGNLFGLBE4", "ND4R2DBD2DL2BE4", "ND4RFD2GLBE4",
  "NR2D2NR2D2NR2BE4", "NR2D2NR2D2BE4", "NR2D4R2U2BE2", "D2ND2R2ND2U2BR2",
  "RNRD4NRLBE4", "BD3DR2U4BR2", "D2ND2RFDBU3NGUBR2", "D4NR2BE4",
  "ND4RFND3ERND4BR2", "ND4R2ND4BR2", "D4R2U4L2BR4", "D2ND2R2U2L2BR4",
  "D4REU3NL2BD4HBE3", "D2ND2RFDBU3NGHLBR4", "NR2D2R2D2L2BE4", "RND4RBR2",
  "D4R2U4BR2", "D3FEU3BR2", "D4RENU3FRU4BR2", "DF2DBL2UE2UBR2",
  "D2RND2RU2BR2", "R2DG2DNR2BE4", // Upper Case Letters

  "NR2D4NR2BE4", "DF2DBU4BR2", "R2D4L2BE4", "BDEFBUBR2",
  "BD4NR2BE4", "NFBR3", // Symbols

  "", "", "", "", "", "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "", "", "", "", "", "", // Lower Case Letters

  "BR2LDGFDRBU4BR2", "ND4BR2", "RDFGDLBE4", "BDEDEBR2",
  "BRGFEHBR3"}; // Symbols

// Turtle font for 5x9 characters.
CONST char *szTurtleCh2[256-32] = {"",
  "BR2D4BD2D0", "BRD2BR2U2", "BD2R4BD2L4BFU4BR2D4", "BR2D6BENL3EHL2HER3",
  "RDLNUBR4G4BR4DLUR", "BD2NF4UEFDG2DFRE2", "BR2DG", "BR3G2D2F2", "BRF2D2G2",
  "BD2FNGRNU2ND2RNEF", "BD3R2NU2ND2R2", "BD5BR2DG", "BD3R4", "BD6BRRULD",
  "BD5E4", // Special Characters

  "BDD4NE4FR2EU4HL2G", "BFED6NLR", "BDER2FDG4R4", "BDER2FDGNLFDGL2H",
  "D3R3NU3ND3R", "NR4D3R3FDGL2H", "BR3NFL2GD4FR2EUHL3", "R4DG4D",
  "BDDFNR2GDFR2EUHEUHL2G", "BD5FR2EU4HL2GDFR3", // Numbers

  "BR2BD2D0BD2D0", "BR2BD2D0BD2G", "BR3G3F3", "BD2R4BD2L4", "BRF3G3",
  "BDER2FDGLDBD2D0", "BF2DFEU2HL2GD4FR2", // Special Characters

  "BD6U4E2F2D2NL4D2", "D6R3EUHNL3EUHL3", "BR3NFL2GD4FR2E", "D6R2E2U2H2L2",
  "NR4D3NR3D3R4", "NR4D3NR3D3", "BR3NFL2GD4FR2EU2L2", "D3ND3R4NU3D3",
  "BRRNRD6NLR", "BD4DFR2EU5", "D3ND3RNE3F3", "D6R4", "ND6F2NDE2D6",
  "ND6F4ND2U4", "BDD4FR2EU4HL2G", "R3FDGL3NU3D3", "BDD4FRENHNFEU3HL2G",
  "ND6R3FDGL2NLF3", "BR3NFL2GDFR2FDGL2H", "R2NR2D6", "D5FR2EU5",
  "D2FDFNDEUEU2", "D6E2NUF2U6", "DF4DBL4UE4U", "D2FRND3REU2",
  "R4DG4DR4", // Upper Case Letters

  "BR3L2D6R2", "BDF4", "BRR2D6L2", "BD2E2F2", "BD6R4", "BR2DF", // Symbols

  "BF4G2LHU2ER2FD3", "D5NDFR2EU2HL2G", "BF4BUHL2GD2FR2E", "BR4D5NDGL2HU2ER2F",
  "BD4R4UHL2GD2FR3", "BD3RNR3ND3U2ERF", "BD8R3EU4HL2GD2FR2E", "D3ND3ER2FD3",
  "BR2D0BD2D4", "BR3D0BD2D5GLH", "D4ND2REREBD4HLH", "BR2D6",
  "BD2DND3EFNDEFD3", "BD2DND3ER2FD3", "BD3D2FR2EU2HL2G", "BD2DND5ER2FD2GL2H",
  "BR4BD8U5HL2GD2FR2E", "BD2DND3ER2F", "BD6R3EHL2HER3", "BR2D2NL2NR2D4",
  "BD2D3FRE2NU2D2", "BD2DFDFEUEU", "BD2D3FENUFEU3", "BD2F2NG2NE2F2",
  "BD2D3FR2ENU3D2GL3", "BD2R4G4R4", // Lower Case Letters

  "BR3GDGFDF", "BR2D2BD2D2", "BRFDFGDG", "BFEFE", "", // Symbols

  "BR3NFLGDNRNLD2NRNLDFRE", "BR2FGHE", "BR2BD4DG", "BR4LGD2NRNLD2GL",
  "BD6EBFE", "BD6D0BR2D0BR2D0", "BR2DNL2NR2D4", "BR2DNL2NR2D4NL2NR2D",
  "BR2NGF", "RDLNUBR4G2L2BD2RDLUBR3RDLU", "BD5FR2EUHL2HUER2NFBU2GH", "BF2FG",
  "BRNRGD4FRNR2U3NR2U3R2", "", "R4DG4DR4BU8BLGH", "", // 80-8F

  "", "BR3GD", "BR2DG", "BFNDEBFNED", "BD2EUBR2DG", "BD3BRRDLU", "BD3BRR2",
  "BD3R4", "BFEFE", "RND2RBRND2FNDED2", "BD6R3EHL2HENR3BU3FE", "BF2GF",
  "BD3NED2FRNR2U2R2UHLNLD2", "", "BD6NR4E4L4BU3BRFE",
  "D2FRND3REU2BUBHD0BL2D0", // 90-9F

  "", "BR2D0BD2D4", "BR2DND6NLRFBL4NED3FR2E", "BD6RNR3U3NLNR2U2ERF",
  "BDFEFNEFGNFGHNGHE", "F2NE2DNL2NR2D2NL2NR2D", "BR2D2BD2D2",
  "BD6R3EHNL2EHL2GFBU2HER3", "BRD0BR2D0", "BD2ER2FD2GL2HU2BR3LGFR",
  "BF4U2NUG2LHU2ER2FBD5L4", "BD3NFEBR3GF", "BD2R4D", "BD3BRR2",
  "BD2ER2FD2GL2HU2BRND2R2DLNLF", "BRR2", // A0-AF

  "BR2FGHE", "BR2D2NL2NR2D2BG2R4", "BRR2DL2DR2", "BRR2DNLDL2", "BR3G",
  "BD3D2ND3FRE2NUD2", "BR4ND6L2ND6LGDFR", "BR2BD3D0", "BR2BD5DL", "BRRD2NLR",
  "BDD2FR2EU2HL2G", "BD2FGBR3EH", "RND2BR3BDG4BDBR4UNULU",
  "RND2BR3BDG4BDBR4LURUL", "RD2LBURBR3G4BDBR4UNULU",
  "BR2D0BD2DLGDFR2E", // B0-BF

  "BD6U4E2F2D2NL4D2BU7BLH", "BD6U4E2F2D2NL4D2BU7BL3E",
  "BD6U4E2F2D2NL4D2BU7BLHG", "BD6U4E2F2D2NL4D2BU8GHG",
  "BD6U4E2F2D2NL4D2BU8BLD0BL2D0", "BD6U4E2F2D2NL4D2BU8BL2D0",
  "BD6U2NR2U2E2NR2D3NR2D3R2", "BR3NFL2GD4FR2EBG2DL", "NR4D3NR3D3R4BU7BLH",
  "NR4D3NR3D3R4BU7BL3E", "NR4D3NR3D3R4BU7BLHG", "NR4D3NR3D3R4BU8BLD0BL2D0",
  "BRRNRD6NLRBU7H", "BRRNRD6NLRBU7BL2E", "BRRNRD6NLRBU7HG",
  "BRRNRD6NLRBU8D0BL2D0", // C0-CF

  "BD3RNRD3RE2U2H2LD3", "ND6F4ND2U4BU2GHG", "BRGD4FR2EU4HNL2BUH",
  "BRGD4FR2EU4HL2BUE", "BRGD4FR2EU4HL2BUEF", "BRGD4FR2EU4HL2BUEFE",
  "BRGD4FR2EU4HL2BU2D0BR2D0", "BDF2NE2NG2F2", "BRGD4FR2EU4HNL2BRGDG2DG",
  "D5FR2EU5BHBLH", "D5FR2EU5BHBLE", "D5FR2EU5BHHG", "D5FR2EU5BHBUD0BL2D0",
  "D2FRND3REU2BHBLE", "D5NDR3EU2HL3", "BD6U5ERFDGF2GL", // D0-DF

  "BF4G2LHU2ER2FD3BU6BLH", "BF4G2LHU2ER2FD3BU6BL2E", "BF4G2LHU2ER2FD3BU6BLHG",
  "BF4G2LHU2ER2FD3BU7GHG", "BF4G2LHU2ER2FD3BU6BLD0BL2D0",
  "BF4G2LHU2ER2FD3BU6BL2D0", "BD2R3FDL3GDR2NR2U4", "BF4BUHL2GD2FR2EBG3RU2",
  "BD4R4UHL2GD2FR3BU6BL2H", "BD4R4UHL2GD2FR3BU7BLG", "BD4R4UHL2GD2FR3BU6BLHG",
  "BD4R4UHL2GD2FR3BU6BLD0BL2D0", "BEFBD2D4", "BR2NEBD2D4", "BRENFBD3D4",
  "BRD0BR2D0BLBD2D4", // E0-EF

  "BR3DNLNRD2FDGL2HUER2", "BD2DND3ER2FD3BU7GHG", "BD3D2FR2EU2HL2NGBU3F",
  "BD3D2FR2EU2HL2NGBUBEE", "BD3D2FR2EU2HL2NGBU2EF", "BD3D2FR2EU2HL2NGBU2EFE",
  "BD3D2FR2EU2HL2NGBU2D0BR2D0", "BD3R4BH2D0BD4D0", "BD3D2FR2EU2HL2GD2BDE4",
  "BD2D3FRE2ND2U2BH2H", "BD2D3FRE2ND2U2BH2E", "BD2D3FRE2ND2U2BUBHHG",
  "BD2D3FRE2ND2U2BHBUD0BL2D0", "BD8R3EU2NU3GL2HU3BE2E", "D5ND3FR2EU2HL2G",
  "BD8R3EU2NU3GL2HU3BEBUD0BR2D0"}; // F0-FF

// Turtle font for 8x13 unicursal characters.
CONST char *szTurtleCh3[128-32] = {
  "R7", "R3ULU2RU2LU7R3D7LD2RD2LDR3", "RU8LU4R2D4R3U4R2D4LD8R",
  "R2U3RUL3UR2U3L2URU3RD3R3U3RD3RDL2D3R2DL3DRD3R2",
  "URDR2U5RD5R2U6L6U6R7D2LUL2D3LU3L2D4R6D7",
  "U3RURURURURURU2L6U2R2DR4URD5LDLDLDLDLDLD2R4U2R2DLDR",
  "R3U4LULULUR3U3RD3R3DL3D6R3", "R3U8LU2RU2R2D4LD8R3",
  "R5ULULU2LU4RU2RURURD2LDLD2LD2RD2RDRD2R",
  "RU2RURU2RU2LU2LULU2RDRDRD2RD4LD2LDLDR5",
  "R3U5LDL2URUR2UL2ULUR2DRU3RD3RUR2DLDL2DR2DRDL2ULD5R3",
  "R3U6L3UR3U3RD3R3DL3D6R3", "R2U3RURD2LD2R4", "R3U6L3UR7DL3D6R3",
  "R2U3R2D2LDR4", "U2RU2RU2RU2RU2RU2R2DLD2LD2LD2LD2LD2LDR6", // Symbols

  "R2ULULU9RUR5DRD9LDL3DR4", "RURDRU11L2UR3D12RURDR",
  "U5RUR5U5L5DLU2R7D7L5DLD4R6", "URDR5U6L4UR4U4L5DLU2R7D12",
  "R5U6L5U6RD5R4U5RD5RDLD6R", "URDR5U6L6U6R7DL6D4R6D7",
  "U12R7D2LUL5D5R6D5L6DR6", "U3RURURURURURU3L6UR7D5LDLDLDLDLDLD2R6",
  "U12R7D5L6DR6D5L6DR6", "U2RD2R5U7L6U5R7D12", // Digits

  "R3U2LU2RU4LU2R3D2LD4RD2LD2R3", "RU2RURU5LU2R3D2LD4RD2LDL2DR5",
  "R6ULULULULULULURURURURURUR2DLDLDLDLDLDRDRDRDRDRD2",
  "R3U4L3UR3U3L3UR7DL3D3R3DL3D4R3",
  "U2RURURURURULULULULULUR2DRDRDRDRDRDLDLDLDLDLDLDR6",
  "U3RU3RUR3URU2LUL3DLDLU2RUR5DRD4LDL3DLD2RD2L2DR6",
  "RULU10RUR5DRD5LDL3U3RD2RURU3LUL3DLD8RD2RURDRURDR", // Symbols

  "U11RUR5DRD5L6DR6D5", "U12R6DRD3LDL5DR5DRD3LDL5DR6",
  "U12R7D3LU2L5D11RURDRURDRU2RD2", "U12R5DRDRD7LDLDL4DR6",
  "U12R7DL6D4R5DL5D6RURDRURDRURD", "U12R7DL6D5R5DL5D5R6",
  "U12R7D3LU2L5D11RURDRURDRU5LUR2D6", "U12RD6R5U6RD12",
  "RURDRU11L2UR5DL2D11RURDR", "U2RD2RURDRURDRU12RD12",
  "U12RD6R2URURURU3RD4LDLDLDLDRDRDRDRD", "U12RD12RURDRURDRURD",
  "U12R3D11RU11R3D12", "U12R5DRDRD10", "U12R7D11L6DR6",
  "U12R6DRD4LDL5D6R6", "U12R7D10LULULULD2RDRDL4DR6",
  "U12R6DRD4LDL5DRDRDRDRDRDR", "URDR5U5L6U7R7D2LUL5D5R6D6",
  "R3U11L3UR7DL3D11R3", "U12RD12RURDRURDRU12RD12",
  "R3U2LULULU8RD7RDRDRURURU7RD8LDLDLD2R3", "U12RD12R2U11RD11R2U12RD12",
  "U3RURURU2LULULU3RD2RDRDRURURU2RD3LDLDLD2RDRDRD3",
  "R3U5L3U7RD6R5U6RD7L3D5R3",
  "U3RURURURURURU3L6UR7D5LDLDLDLDLDLD2R6", // Upper Case Letters

  "RU12R5DL4D11RURDRURDR", "R6ULU2LU2LU2LU2LU2LUR2D2RD2RD2RD2RD2RD2",
  "RURDRURDRU11L4UR5D12R", "RU10RURURDRDRD10R", "URDRURDRURDRURD",
  "R3U8LU4R2D2RD2LD8R3", // Symbols

  "", "", "", "", "", "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "", "", "", "", "", "", // Lower Case Letters

  "R5ULULU2LU2L2UR2U2RU2RUR2DLDLD2LD3RD2RDRD2R", "R3U12RD12R3",
  "RU2RURU2RU3LU2LULUR2DRD2RD2R2DL2D2LD2LDLDR5", "U10RURURDRDRURURD12",
  "R3U8LULU2RUR3DRD2LDLD8R3"}; // Symbols

// Turtle font for 8x14 characters.
CONST char *szTurtleCh4[256-32] = {"",
  "BR3D5BD3D", "BR2D3BR3U3", "BD3R6BD3L6BF2U7BR2D7",
  "BR3D9BH2BLFR4EU2HL4HUER4F", "RDLNUBR6G6BR6DLUR", "BD3NF6U2ERFD2G3D2FR2E3",
  "BR3DG", "BR4G3D3F3", "BR2F3D3G3", "BD2F2NG2RNU3ND3RNE2F2", "BD4R3NU3ND3R3",
  "BR3BD8DG", "BD4R6", "BD9BR2RULD", "BD7E6", // Special Characters

  "BD2D5NE5F2R2E2U5NLH2L2G2", "BR3NG2D9NL2R2", "BD2E2R2F2DG6R6",
  "BD2E2R2F2G2NLF2DG2L2H2", "D5R4NU5ND4R2", "NR6D4R4F2DG2L2H2",
  "BR4NF2L2G2D5F2R2E2UH2L2G2", "R6DG6D2", "BD2F2NR2G2DF2R2E2UH2E2H2L2G2",
  "BD7F2R2E2U5H2L2G2DF2R2E2", // Numbers

  "BR2BD2DRULBD4DRUL", "BR2BD2DRULBD4RDG", "BR4G4F4", "BD3R6BD3L6", "BRF4G4",
  "BD2E2R2F2DG2LD2BD2D0", "BF3RDLNUDFREU3H2L2G2D5F2R3E", // Special Characters

  "BD9U6E3F3D3NL6D3", "D9R4E2UH2NL4E2H2L4", "BR4NF2L2G2D5F2R2E2",
  "D9R3E3U3H3L3", "NR6D4NR5D5R6", "NR6D5NR5D4", "BR4NF2L2G2D5F2R2E2U2L3",
  "D4ND5R6NU4D5", "BRR2NR2D9NL2R2", "BD6DF2R2E2U7", "D4ND5R2NE4F4D", "D9R6",
  "ND9F3ND2E3D9", "ND9F2DFDFDF2U9", "BD2D5F2R2E2U5H2L2G2", "ND9R4F2DG2L4",
  "BD2D5F2R2ENFNH2EU5H2L2G2", "ND9R4F2DG2L2NL2F4", "BR4NF2L2G2F2R2F2DG2L2H2",
  "R3NR3D9", "D7F2R2E2U7", "D3FDFDFNDEUEUEU3", "D9E3NU2F3U9",
  "DF6D2BL6U2E6U", "D3F2RND4RE2U3", "R6DG6D2R6", // Upper Case Letters

  "BR4L3D9R3", "BDF6", "BRR3D9L3", "BD3E3F3", "BD9R6", "BR3DF", // Symbols

  "BF6G3LH2U2E2R3FD5", "D7ND2F2R2E2U2H2L2G2", "BR6BD4HL3G2D2F2R3E",
  "BR6D7ND2G2L2H2U2E2R2F2", "BD6R6UH2L2G2D2F2R3E", "BD4R2NR4ND5U3ER2F",
  "BD11FR3E2U5H2L2G2D2F2R2E2", "D5ND4E2R2F2D4", "BR3DBD3D5", "BR4DBD3D7GL2H",
  "D6ND3REREREBD6HLHLH", "BR3D9", "BD3DND5ERFND2ERFD5", "BD3D2ND4E2R2F2D4",
  "BD5D2F2R2E2U2H2L2G2", "BD3D2ND7E2R2F2D2G2L2H2",
  "BR6BD12U7NU2H2L2G2D2F2R2E2", "BD3D2ND4E2R2F2", "BD8FR4EUHL4HUER4F",
  "BR3D4NL3NR3D5", "BD3D4F2R2E2NU4D2", "BD3DFDFDFEUEUEU", "BD3D5FRENU2FREU5",
  "BD3F3NG3NE3F3", "BD3D4F2R2E2NU4D3G2L4", "BD3R6G6R6", // Lower Case Letters

  "BR4LGD2GFD2FR", "BR3D3BD3D3", "BRRFD2FGD2GL", "BFERFRE", "", // Symbols

  "BR5NFL2G2DNR2NLD2NR2NLD2F2R2E", "BR3FGHE", "BR3BD8DG", "BR5LGD3NR2NL2D4GL",
  "BD9BREBFE", "BD9BRD0BR2D0BR2D0", "BR3BDD2NL2NR2D5",
  "BR3BDD2NL2NR2D3NL2NR2D2", "BR3NGF", "BRFGHNEBR6GLG2LGBD3EFGHBR4EFGH",
  "BU3BR2FEBD3NF2L2G2F2R2F2DG2L2H2", "BR3BD2F2G2", "BR2NRG2D5F2RNR3U5NR2U4R3",
  "", "R6DG6D2R6BU12BL2GH", "", // 80-8F

  "", "BR4GD", "BR3DG", "BFNDEBR3GD", "BRBD2EUBR3DG", "BD4BR3RDLU", "BD4BRR4",
  "BD4R6", "BRBFEFE", "RND2RBR2ND2FNDED2", "BD8FR4EUHL4HUER4FBH4FE",
  "BR4BD2G2F2", "BD4NED4FREU4HLBR3NGRFD2L3BD2FRE", "", "BD3R6G6R6BU9BL2GH",
  "D3F2RND4RE2U3BH2D0BL2D0", // 90-9F

  "", "BR3DBD3D5", "BR3D2ND7NL2R2FBL6NED3FR4E", "BD9RNR5U4NLNR4U3E2R2F",
  "DFER2FEUBD3NHD2BD3UHNEGL2HGDBU3NFU2E", "F3NE3D2NL2NR2D2NL2NR2D2",
  "BR3D3BD3D3", "BD8FR4EUHNL4EUHL4GDFBU3HUER4F", "BR2D0BR2D0",
  "BD3E2R2F2D2G2L2H2U2BR4LGFR", "BRBF4U2NUG2LHU2ER2FBD5L4", "BD4NF2E2BR4G2F2",
  "BD4R6D2", "BD4BRR4", "BD2E2R2F2D2G2L2H2U2BR2ND2R2DLNLF", "BRR4", // A0-AF

  "BR3FGHE", "BR3D3NL3NR3D3BG3R6", "BRR3FGL2GDR4", "BRR3FGNL3FGL3", "BR4G",
  "BD3D4ND5F2RE2NU4DF", "BR6ND9L3ND9LG2D2F2R", "BR3BD4D0", "BR3BD8DL",
  "BRR2D4NL2R2", "BR3GD2FR2EU2HL2BLBD6R4", "BD2F2G2BR4E2H2",
  "RD3NLRBR4BU2G6BR6ND2NU2L2U2", "RD3NLRBR4BU2G6BR6BD2L2U2R2U2L2",
  "R2D2NL2D2L2BR6BU2G6BDBR6UNU2L2U2", "BR3D0BD2D2LG2DF2R2E2", // B0-BF

  "BD9U6E3F3D3NL6D3BU10BL2H", "BD9U6E3F3D3NL6D3BU10BL4E",
  "BD9U6E3F3D3NL6D3BU10BL2HG", "BD9U6E3F3D3NL6D3BU11BLGHG",
  "BD9U6E3F3D3NL6D3BU11BL2D0BL2D0", "BD9U6E3F3D3NL6D3BU11BL3D0",
  "BD9U3NR3U3E3NR3D4NR3D5R3", "BR4NF2L2G2D5F2R2E2BG3DL",
  "NR6D4NR5D5R6BU10BL3H", "NR6D4NR5D5R6BU10BL3E", "NR6D4NR5D5R6BU10BL2HG",
  "NR6D4NR5D5R6BU11BL2D0BL2D0", "BRR2NR2D9NL2R2BU10BL2H",
  "BRR2NR2D9NL2R2BU10BL2E", "BRR2NR2D9NL2R2BU10BLHG",
  "BRR2NR2D9NL2R2BU11BLD0BL2D0", // C0-CF

  "BD4RNRD5R2E3U3H3L2D4", "ND9F2DFDFDF2U9BU2BLGHG", "BR2G2D5F2R2E2U5H2L2BEH",
  "BR2G2D5F2R2E2U5H2L2BEE", "BR2G2D5F2R2E2U5H2L2BUEF",
  "BR2G2D5F2R2E2U5H2L2BUEFE", "BR2G2D5F2R2E2U5H2L2BU2D0BR2D0", "BDF3NE3NG3F3",
  "BR2G2D5F2R2E2U5H2L2BR4G2DGDGDG2", "D7F2R2E2U7BL3BUH", "D7F2R2E2U7BL3BUE",
  "D7F2R2E2U7BL2BUHG", "D7F2R2E2U7BH2D0BL2D0", "D3F2RND4RE2U3BL3BUE",
  "D7ND2R4E2UH2L4", "BD9U7E2R2FDG2DFRFDGL3", // D0-DF

  "BF6G3LH2U2E2R3FD5BU8BL3H", "BF6G3LH2U2E2R3FD5BU8BL3E",
  "BF6G3LH2U2E2R3FD5BU8BL2HG", "BF6G3LH2U2E2R3FD5BU9BLGHG",
  "BF6G3LH2U2E2R3FD5BU8BL2D0BL2D0", "BF6G3LH2U2E2R3FD5BU8BL3D0",
  "BD4ERFD4GLHUER5U2HLGD4FRE", "BR6BD4HL3G2D2F2R3EBG2NUDL",
  "BD6R6UH2L2G2D2F2R3EBU7BL3H", "BD6R6UH2L2G2D2F2R3EBU7BL3E",
  "BD6R6UH2L2G2D2F2R3EBU7BL2HG", "BD6R6UH2L2G2D2F2R3EBU7BL2D0BL2D0",
  "BR2FBD2D6", "BR4GBD2D6", "BR3NGNFBD3D6", "BR2BDD0BR2D0BLBD2D6", // E0-EF

  "BR4DNLNRD3FD2G2L2H2U2E2R2F", "BD3D2ND4E2R2F2D4BU9BLGHG",
  "BD5D2F2R2E2U2H2L2G2BE4BLH", "BD5D2F2R2E2U2H2L2G2BE4BLE",
  "BD5D2F2R2E2U2H2L2G2BE4HG", "BD5D2F2R2E2U2H2L2G2BE4NEHG",
  "BD5D2F2R2E2U2H2L2G2BE4D0BL2D0", "BD4R6BU3BL3D0BD6D0",
  "BD5D2F2R2E2U2H2L2NG2BR4G6", "BD3D4F2R2E2ND2U4BH2H",
  "BD3D4F2R2E2ND2U4BH2BUG", "BD3D4F2R2E2ND2U4BH2HG",
  "BD3D4F2R2E2ND2U4BH2D0BL2D0", "BD3D4F2R2E2NU4D3G2L4BU11BR3E",
  "D7ND5F2R2E2U2H2L2G2", "BD3D4F2R2E2NU4D3G2L4BU11BR2D0BR2D0"}; // F0-FF

GS gs = {
  // Display settings
  fFalse, fTrue, NULL,
  // Macro accessible only settings
  0, 0, 1, 0, fOn, fTrue, fFalse, 0x1F3, 0x008};


/*
******************************************************************************
** Bitmap Primitives
******************************************************************************
*/

// Allocate a new bitmap of a given size.

flag CMon::FAllocate(int x, int y, CONST CMap *pbOld)
{
  long cb;

  if (x < 0 || y < 0 || x > xBitmap || y > yBitmap) {
    PrintSzNN_E("Can't create bitmap larger than %d by %d!\n",
      xBitmap, yBitmap);
    return fFalse;
  }
  cb = CbBitmap(x, y);
  if (cb < 0) {
    PrintSzNN_E("Can't allocate bitmap of size %d by %d!\n", x, y);
    return fFalse;
  }
  m_rgb = (byte *)PAllocate(cb);
  if (m_rgb == NULL)
    return fFalse;
  m_x = x; m_y = y;
  m_clRow = CbBitmapRow(x) >> 2;
  m_cfPix = 1;

  // Inherit the client fields (i.e. 3D dimensions) from passed in bitmap.

  if (pbOld != NULL)
    Copy3(*pbOld);
  else {
    m_x3 = wClient1Def; m_y3 = wClient2Def;
    m_z3 = wClient3Def; m_w3 = wClient4Def;
  }
  return fTrue;
}


// Turn off a pixel in a bitmap.

void CMon::Set0(int x, int y)
{
  if (!FLegal(x, y))
    return;
  if (gs.fTraceDot && FVisible())
    ScreenDot(x, y, fOff, ~0);
  *_Pl(x, y) &= ~Lf(x);
}


// Turn on a pixel in a bitmap.

void CMon::Set1(int x, int y)
{
  if (!FLegal(x, y))
    return;
  if (gs.fTraceDot && FVisible())
    ScreenDot(x, y, fOn, ~0);
  *_Pl(x, y) |= Lf(x);
}


// Invert a pixel in a bitmap.

void CMon::Inv(int x, int y)
{
  if (!FLegal(x, y))
    return;
  if (gs.fTraceDot && FVisible())
    ScreenDot(x, y, !_Get(x, y), ~0);
  *_Pl(x, y) ^= Lf(x);
}


// Return the status of a pixel in a bitmap. Pixels not on the bitmap are
// assumed to be off.

KV CMon::Get(int x, int y) CONST
{
  if (!FLegal(x, y))
    return fOff;
  return _Get(x, y);
}


// Set the status of a pixel in a bitmap. Does nothing if an attempt is made
// to set a pixel not on the bitmap.

void CMon::Set(int x, int y, KV kv)
{
  if (!FLegal(x, y))
    return;
  if (gs.fTraceDot && FVisible())
    ScreenDot(x, y, kv, ~0);
  if (kv)
    *_Pl(x, y) |= Lf(x);
  else
    *_Pl(x, y) &= ~Lf(x);
}


// Force coordinates to be within the bounds of a bitmap, moving them to the
// edge of the bitmap if not within it already.

void CMap::Legalize(int *x, int *y) CONST
{
  if (*x < 0)
    *x = 0;
  else if (*x >= m_x)
    *x = m_x-1;
  if (*y < 0)
    *y = 0;
  else if (*y >= m_y)
    *y = m_y-1;
}


// Force coordinates to be within the bounds of a bitmap, wrapping them around
// to the other edge of the bitmap if not within it already.

void CMap::Legalize2(int *x, int *y) CONST
{
  while (*x < 0)
    *x += m_x;
  while (*x >= m_x)
    *x -= m_x;
  while (*y < 0)
    *y += m_y;
  while (*y >= m_y)
    *y -= m_y;
}


/*
******************************************************************************
** Bitmap Block Editing
******************************************************************************
*/

// Set all pixels within a rectangle in a bitmap.

void CMon::Block(int x1, int y1, int x2, int y2, KV o)
{
  int x, y;
  long il1, il2, il, l, l1, l2;

  Legalize(&x1, &y1); Legalize(&x2, &y2);
  SortN(&x1, &x2);
  SortN(&y1, &y2);

  // Fastest case: If the horizontal coordinates cover the whole width of the
  // bitmap, it covers consecutive memory addresses. Set 32 pixels at a time.
  if (x1 == 0 && x2 == m_x-1) {
    il1 = _Il(x1, y1);
    il2 = _Il(x2, y2);
    l = o ? dwSet : 0;
    while (il1 <= il2)
      *_Pl(il1++) = l;
    goto LDone;
  }

  // Fast case: Can still set 32 pixels at a time, just need to do each row
  // separately, and calculate the bit pattern for start and end of each row.
  il1 = _Il(x1, y1); il2 = _Il(x2, y1);
  l1 = ~((1L << (x1-1 & 31 ^ 7)) - 1 ^ (255L << (x1-1 & 24)));
  l2 = ((1L << (x2 & 31 ^ 7)) - 1 ^ (255L << (x2 & 24)));
  for (y = y1; y <= y2; y++) {
    for (il = il1; il <= il2; il++) {
      l = dwSet;
      if (il == il1 && (x1 & 31) > 0)
        l &= l1;
      if (il == il2)
        l &= l2;
      if (o)
        *_Pl(il) |= l;
      else
        *_Pl(il) &= ~l;
    }
    il1 += m_clRow; il2 += m_clRow;
  }

  // Need to update the screen if Show Pixel Edits is on.
LDone:
  if (gs.fTraceDot && FVisible())
    for (y = y1; y <= y2; y++)
      for (x = x1; x <= x2; x++)
        ScreenDot(x, y, o, ~0);
}


// Invert all pixels within a rectangle in a bitmap.

void CMap::BlockReverse(int x1, int y1, int x2, int y2)
{
  int x, y;

  Legalize(&x1, &y1); Legalize(&x2, &y2);
  SortN(&x1, &x2);
  SortN(&y1, &y2);
  for (y = y1; y <= y2; y++)
    for (x = x1; x <= x2; x++)
      Inv(x, y);
}


// Set pixels around the border of a rectangle in a bitmap.

void CMap::Box(int x1, int y1, int x2, int y2, int xsiz, int ysiz, KV kv)
{
  SortN(&x1, &x2);
  SortN(&y1, &y2);
  Block(x1, y1, x2, y1 + ysiz - 1, kv);
  if (y1 + ysiz < y2) {
    Block(x1, y1 + ysiz, x1 + xsiz - 1, y2 - ysiz, kv);
    Block(x2 - xsiz + 1, y1 + ysiz, x2, y2 - ysiz, kv);
  }
  Block(x1, y2 - ysiz + 1, x2, y2, kv);
}


// Copy pixels in a rectangle from one bitmap to another bitmap. If the
// bitmaps are the same, the rectangles should not overlap.

void CMon::BlockMove(CONST CMap &b, int x1, int y1, int x2, int y2,
  int x0, int y0)
{
  CONST CMon &b1 = dynamic_cast<CONST CMon &>(b);
  int x, y, xT, yT;
  long il1, il2, il0;

  Assert(!b.FColor());
  SortN(&x1, &x2);
  SortN(&y1, &y2);

  // Fast case: If source and destination bitmaps have the same width, and the
  // horizontal spans cover the whole bitmap width, and source and destination
  // rentangles are within the bounds of their bitmaps, both source and dest
  // rectangles cover consecutive memory addresses. Copy 32 pixels at a time.
  if (b1.m_x == m_x &&
    x1 == 0 && x2 == b1.m_x-1 && y1 >= 0 && y2 < b1.m_y &&
    x0 == 0 && y0 >= 0 && y0+(y2-y1) < m_y) {
    il1 = b1._Il(x1, y1);
    il2 = b1._Il(x2, y2);
    il0 = _Il(x0, y0);
    while (il1 <= il2)
      *_Pl(il0++) = *b1._Pl(il1++);
    return;
  }

  // Normal case: Copy one pixel at a time from source to dest rectangle.
  for (y = y1; y <= y2; y++)
    for (x = x1; x <= x2; x++) {
      xT = x0+(x-x1); yT = y0+(y-y1);
      if (FLegal(xT, yT))
        Set(xT, yT, b1.Get(x, y));
    }
}


// Copy pixels in a rectangle from one bitmap to another bitmap, combining
// them using the specified logical operator table.

void CMon::BlockMoveOrAnd(CONST CMon &b1, int x1, int y1, int x2, int y2,
  int x0, int y0, CONST int *rgf)
{
  int x, y, xT, yT;

  SortN(&x1, &x2);
  SortN(&y1, &y2);
  for (y = y1; y <= y2; y++)
    for (x = x1; x <= x2; x++) {
      xT = x0+(x-x1); yT = y0+(y-y1);
      if (FLegal(xT, yT))
        Set(xT, yT, rgf[b1.Get(x, y) + Get(xT, yT)]);
    }
}


/*
******************************************************************************
** Bitmap Line Drawing
******************************************************************************
*/

// Draw a horizontal line on a bitmap.

void CMon::LineX(int x1, int x2, int y, KV o)
{
  int x;
  long il1, il2, il, l;

  // Don't worry about pixels that are off the bitmap.
  if (y < 0 || y >= m_y)
    return;
  SortN(&x1, &x2);
  if (x1 < 0)
    x1 = 0;
  if (x2 >= m_x)
    x2 = m_x-1;

  // Quickly draw the line, setting up to 32 pixels at a time.
  il1 = _Il(x1, y); il2 = _Il(x2, y);
  for (il = il1; il <= il2; il++) {
    l = dwSet;
    if (il == il1 && (x1 & 31) > 0)
      l &= ~((1L << (x1-1 & 31 ^ 7)) - 1 ^ (255 << (x1-1 & 24)));
    if (il == il2)
      l &= ((1L << (x2 & 31 ^ 7)) - 1 ^ (255 << (x2 & 24)));
    if (o)
      *_Pl(il) |= l;
    else
      *_Pl(il) &= ~l;
  }

  // Need to update the screen if Show Pixel Edits is on.
  if (gs.fTraceDot && FVisible())
    for (x = x1; x <= x2; x++)
      ScreenDot(x, y, o, ~0);
}


// Draw a vertical line on a bitmap.

void CMon::LineY(int x, int y1, int y2, KV o)
{
  int y;
  long il, lf;

  // Don't worry about pixels that are off the bitmap.
  if (x < 0 || x >= m_x)
    return;
  SortN(&y1, &y2);
  if (y1 < 0)
    y1 = 0;
  if (y2 >= m_y)
    y2 = m_y-1;

  // Quickly draw the line, where things like the bit mask and memory address
  // only need to be calculated once.
  il = _Il(x, y1);
  if (o) {
    lf = Lf(x);
    for (y = y1; y <= y2; y++) {
      *_Pl(il) |= lf;
      il += m_clRow;
    }
  } else {
    lf = ~Lf(x);
    for (y = y1; y <= y2; y++) {
      *_Pl(il) &= lf;
      il += m_clRow;
    }
  }

  // Need to update the screen if Show Pixel Edits is on.
  if (gs.fTraceDot && FVisible())
    for (y = y1; y <= y2; y++)
      ScreenDot(x, y, o, ~0);
}


// Draw a diagonal line of the specified tilt on a bitmap.

void CMap::LineZ(int x, int y1, int y2, int dir, KV o)
{
  int i, j;

  if (NAbs(dir) <= 1) {
    for (i = 0; i <= y1 - y2; i++)
      Set(x + i*dir, y1 - i, o);
  } else {
    j = dir < 0 ? -1 : 1;
    for (i = 0; i <= y1 - y2; i++)
      LineX(x + i*dir, x + (i+1)*dir - j, y1 - i, o);
  }
}


// Draw a line between any two points on a bitmap. This is an implementation
// of Bresenham's algorithm.

void CMon::Line(int x1, int y1, int x2, int y2, KV o)
{
  int x = x1, y = y1, dx = x2 - x1, dy = y2 - y1, xInc, yInc, xInc2, yInc2,
    d, dInc, z, zMax;
  flag fAllLegal = FLegal(x1, y1) && FLegal(x2, y2);

  // Determine slope.
  if (NAbs(dx) >= NAbs(dy)) {
    xInc = NSgn(dx); yInc = 0;
    xInc2 = 0; yInc2 = NSgn(dy);
    zMax = NAbs(dx); dInc = NAbs(dy);
    d = zMax - (!FOdd(dx) && x1 > x2);
  } else {
    xInc = 0; yInc = NSgn(dy);
    xInc2 = NSgn(dx); yInc2 = 0;
    zMax = NAbs(dy); dInc = NAbs(dx);
    d = zMax - (!FOdd(dy) && y1 > y2);
  }
  d >>= 1;

  // Loop over long axis, adjusting short axis for slope as needed.
  for (z = 0; z <= zMax; z++) {
    if (fAllLegal || FLegal(x, y)) {
      if (o)
        *_Pl(x, y) |= Lf(x);
      else
        *_Pl(x, y) &= ~Lf(x);
    }
    x += xInc; y += yInc; d += dInc;
    if (d >= zMax) {
      x += xInc2; y += yInc2; d -= zMax;
    }
  }

  // Need to update the screen if Show Pixel Edits is on.
  if (gs.fTraceDot && FVisible())
    UpdateDisplay();
}


// Draw a line between any two points on a bitmap, stopping if a pixel already
// changed is reached.

flag CMon::FLineUntil(int x1, int y1, int x2, int y2, int *xEnd, int *yEnd,
  KV o, flag fDraw, flag fWall)
{
  int x = x1, y = y1, xLast = x1, yLast = y1, dx = x2 - x1, dy = y2 - y1,
    xInc, yInc, xInc2, yInc2, d, dInc, z, zMax;
  flag fAllLegal = FLegal(x1, y1) && FLegal(x2, y2);

  // Determine slope.
  if (NAbs(dx) >= NAbs(dy)) {
    xInc = NSgn(dx); yInc = 0;
    xInc2 = 0; yInc2 = NSgn(dy);
    zMax = NAbs(dx); dInc = NAbs(dy);
    d = zMax - (!FOdd(dx) && x1 > x2);
  } else {
    xInc = 0; yInc = NSgn(dy);
    xInc2 = NSgn(dx); yInc2 = 0;
    zMax = NAbs(dy); dInc = NAbs(dx);
    d = zMax - (!FOdd(dy) && y1 > y2);
  }
  d >>= 1;

  // Loop over long axis, adjusting short axis for slope as needed.
  for (z = 0;; z++) {
    if (fAllLegal || FLegal(x, y)) {
      if (_Get(x, y) != o) {
        if (fDraw)
          Set(x, y, o);
      } else
        break;
    }
    xLast = x; yLast = y;
    if (z >= zMax)
      break;
    x += xInc; y += yInc; d += dInc;
    if (d >= zMax) {
      x += xInc2; y += yInc2; d -= zMax;
    }
  }

  // Return either the last pixel before the barrier, or the barrier itself.
  if (fWall) {
    xLast = x; yLast = y;
  }
  if (xEnd != NULL)
    *xEnd = xLast;
  if (yEnd != NULL)
    *yEnd = yLast;

  // Need to update the screen if Show Pixel Edits is on.
  if (gs.fTraceDot && FVisible())
    UpdateDisplay();
  return xLast != x2 || yLast != y2;
}


// Calculate a line between two points, recording the horizontal coordinate at
// each row if it's smaller or larger than the current limits. This is used
// when drawing triangles and quadrilaterals.

void CMap::LineSetup(MM *xminmax, int x1, int y1, int x2, int y2) CONST
{
  int x = x1, y = y1, dx = x2 - x1, dy = y2 - y1, xInc, yInc, xInc2, yInc2,
    d, dInc, z, zMax;

  // Determine slope.
  if (NAbs(dx) >= NAbs(dy)) {
    xInc = NSgn(dx); yInc = 0;
    xInc2 = 0; yInc2 = NSgn(dy);
    zMax = NAbs(dx); dInc = NAbs(dy);
    d = zMax - (!FOdd(dx) && x1 > x2);
  } else {
    xInc = 0; yInc = NSgn(dy);
    xInc2 = NSgn(dx); yInc2 = 0;
    zMax = NAbs(dy); dInc = NAbs(dx);
    d = zMax - (!FOdd(dy) && y1 > y2);
  }
  d >>= 1;

  // Loop over long axis, adjusting short axis for slope as needed.
  for (z = 0; z <= zMax; z++) {
    if (y >= 0 && y < m_y) {
      if (x < xminmax[y].min)
        xminmax[y].min = x;
      if (x > xminmax[y].max)
        xminmax[y].max = x;
    }
    x += xInc; y += yInc; d += dInc;
    if (d >= zMax) {
      x += xInc2; y += yInc2; d -= zMax;
    }
  }
}


// Draw a filled in triangle on a bitmap, defined by three points.

flag CMon::FTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int k)
{
  MM rgmm[zBitmapStack], *pmm = NULL, *xminmax;
  int ymin, ymax, y, yT, xmax, x;

  if (m_y <= zBitmapStack)
    xminmax = rgmm;
  else {
    pmm = RgAllocate(m_y, MM);
    if (pmm == NULL)
      return fFalse;
    xminmax = pmm;
  }
  ymin = NMax(NMin(Min(y1, y2), y3), 0);
  ymax = NMin(NMax(Max(y1, y2), y3), m_y - 1);
  for (y = ymin; y <= ymax; y++) {
    xminmax[y].min = xBitmap;
    xminmax[y].max = -1;
  }
  LineSetup(xminmax, x1, y1, x2, y2);
  LineSetup(xminmax, x2, y2, x3, y3);
  LineSetup(xminmax, x3, y3, x1, y1);

  // Draw one horizontal line per row.
  if (k >= 0) {
    for (y = ymin; y <= ymax; y++)
      LineX(xminmax[y].min, xminmax[y].max, y, k);
  } else {
    neg(k);
    for (y = ymin; y <= ymax; y++) {
      yT = y & 3;
      xmax = Min(xminmax[y].max, m_x - 1);
      for (x = Max(xminmax[y].min, 0); x <= xmax; x++)
        Set(x, y, FDitherCore(k, x & 3, yT));
    }
  }
  if (pmm != NULL)
    DeallocateP(pmm);
  return fTrue;
}


// Draw a filled in four sided quadrilateral on a bitmap.

flag CMon::FQuadrilateral(int x1, int y1, int x2, int y2, int x3, int y3,
  int x4, int y4, int k)
{
  MM rgmm[zBitmapStack], *pmm = NULL, *xminmax;
  int ymin, ymax, y, yT, xmax, x;

  if (m_y <= zBitmapStack)
    xminmax = rgmm;
  else {
    pmm = RgAllocate(m_y, MM);
    if (pmm == NULL)
      return fFalse;
    xminmax = pmm;
  }
  ymin = NMax(NMin(NMin(Min(y1, y2), y3), y4), 0);
  ymax = NMin(NMax(NMax(Max(y1, y2), y3), y4), m_y - 1);
  for (y = ymin; y <= ymax; y++) {
    xminmax[y].min = xBitmap;
    xminmax[y].max = -1;
  }
  LineSetup(xminmax, x1, y1, x2, y2);
  LineSetup(xminmax, x2, y2, x3, y3);
  LineSetup(xminmax, x3, y3, x4, y4);
  LineSetup(xminmax, x4, y4, x1, y1);

  // Since one horizontal line is drawn per row, the quadrilateral should be
  // convex, or at least not concave any along the horizontal axis.
  if (k >= 0) {
    for (y = ymin; y <= ymax; y++)
      LineX(xminmax[y].min, xminmax[y].max, y, k);
  } else {
    neg(k);
    for (y = ymin; y <= ymax; y++) {
      yT = y & 3;
      xmax = Min(xminmax[y].max, m_x - 1);
      for (x = Max(xminmax[y].min, 0); x <= xmax; x++)
        Set(x, y, FDitherCore(k, x & 3, yT));
    }
  }
  if (pmm != NULL)
    DeallocateP(pmm);
  return fTrue;
}


/*
******************************************************************************
** Bitmap Curve Drawing
******************************************************************************
*/

// Draw a filled in circle or ellipse, within a rectangle on a bitmap.

void CMap::Disk(int x1, int y1, int x2, int y2, KV kv)
{
  int rx, ry, r, i, j, iT, jT, q, qLo, qHi, m1, m2, n1, n2;

  // This efficient algorithm has no calculations more complicated than
  // addition (and multiplication to proportion in the case of an ellipse).
  rx = NAbs(x2 - x1) >> 1; ry = NAbs(y2 - y1) >> 1;
  if (rx == 0 || ry == 0) {
    Block(x1, y1, x2, y2, kv);
    return;
  }
  r = Max(rx, ry);
  j = r; q = r+1; qLo = 1; qHi = r+r-1;
  m1 = (x1 + x2) >> 1; m2 = m1 + FOdd(x2 - x1);
  n1 = (y1 + y2) >> 1; n2 = n1 + FOdd(y2 - y1);
  for (i = 0; i <= r; i++) {
    iT = (rx <= ry) ? i : i * ry / rx;
    jT = (rx >= ry) ? j : j * rx / ry;
    LineX(m1 - jT, m2 + jT, n1 - iT, kv);
    LineX(m1 - jT, m2 + jT, n2 + iT, kv);
    q -= qLo;
    while (q < 0) {
      q += qHi;
      qHi -= 2;
      j--;
    }
    qLo += 2;
  }
}


// Draw an outline of a circle or ellipse, within a rectangle on a bitmap.

void CMap::Circle(int x1, int y1, int x2, int y2, KV kv)
{
  int rx, ry, r, imax, i, j, i1, i2, j1, j2, q, qLo, qHi, m1, m2, n1, n2;

  // This efficient algorithm has no calculations more complicated than
  // addition (and multiplication to proportion in the case of an ellipse).
  rx = NAbs(x2 - x1) >> 1; ry = NAbs(y2 - y1) >> 1;
  if (rx == 0 || ry == 0) {
    Block(x1, y1, x2, y2, kv);
    return;
  }
  r = Max(rx, ry);
  j = r; q = r+1; qLo = 1; qHi = r+r-1;
  m1 = (x1 + x2) >> 1; m2 = m1 + FOdd(x2 - x1);
  n1 = (y1 + y2) >> 1; n2 = n1 + FOdd(y2 - y1);
  imax = NMultDiv(r + 1, 708, 1000);
  for (i = 0; i <= imax; i++) {
    i1 = j2 = j; j1 = i2 = i;
    if (rx < ry) {
      i1 = i1 * rx / ry; i2 = i2 * rx / ry;
    } else if (ry < rx) {
      j1 = j1 * ry / rx; j2 = j2 * ry / rx;
    }
    Set(m1 - i1, n1 - j1, kv); Set(m1 - i2, n1 - j2, kv);
    Set(m2 + i1, n1 - j1, kv); Set(m2 + i2, n1 - j2, kv);
    Set(m1 - i1, n2 + j1, kv); Set(m1 - i2, n2 + j2, kv);
    Set(m2 + i1, n2 + j1, kv); Set(m2 + i2, n2 + j2, kv);
    q -= qLo;
    if (q < 0) {
      q += qHi;
      qHi -= 2;
      j--;
    }
    qLo += 2;
  }
}


// Draw one quadrant or 90 degrees of a circle.

void CMap::ArcQuadrant(int x, int y, int r, int d, KV kv)
{
  int imax = NMultDiv(r, 708, 1000), i, j = r, q = r+1, qLo = 1, qHi = r+r-1,
    xi, yi;

  // This efficient algorithm has no calculations more complicated than
  // addition (and multiplication for the sign to indicate the quadrant).
  if (r == 0) {
    Set(x, y, kv);
    return;
  }
  xi = d > 0 && d < 3 ? -1 : 1; yi = d < 2 ? 1 : -1;
  for (i = 0; i <= imax; i++) {
    q -= qLo;
    if (q < 0) {
      q += qHi;
      qHi -= 2;
      j--;
    }
    qLo += 2;
    Set(x + xi*j, y + yi*i, kv);
    Set(x + xi*i, y + yi*j, kv);
  }
}


// Draw a subsection of one quadrant or 90 degrees of a circle.

void CMap::ArcQuadrantSub(int x, int y, int r, int d, int dx, int dy,
  int *pdx, int *pdy, KV kv)
{
  int imax = NMultDiv(r, 708, 1000), i, j = r, q = r+1, qLo = 1, qHi = r+r-1,
    xi, yi;

  // Use the same efficient algorithm as in ArcQuadrant().
  if (r == 0) {
    Set(x, y, kv);
    return;
  }
  if (dx < 0)
    dx = 0;
  if (dy < 0)
    dy = 0;
  xi = d > 0 && d < 3 ? -1 : 1; yi = d < 2 ? 1 : -1;
  for (i = 0; i <= imax; i++) {
    q -= qLo;
    if (q < 0) {
      q += qHi;
      qHi -= 2;
      j--;
    }
    qLo += 2;
    if (i >= dy && j >= dx) {
      Set(x + xi*j, y + yi*i, kv);
      if (i == dy && pdx != NULL)
        *pdx = j;
      if (j == dx && pdy != NULL)
        *pdy = i;
    }
    if (i >= dx && j >= dy) {
      Set(x + xi*i, y + yi*j, kv);
      if (i == dx && pdy != NULL)
        *pdy = j;
      if (j == dy && pdx != NULL)
        *pdx = i;
    }
  }
}


// Draw an arc of an ellipse, specifically some subsection of sides of a dMax
// sized polygon.

void CMap::ArcPolygon(int x, int y, int rx, int ry, int d1, int d2,
  int dInc, int dMax, KV kv)
{
  int i, m, n, m0, n0;
  real rMax;

  if (dMax < 0) {
    dMax = 64;
    if (rx + ry > 384)
      dMax = 256;
    d1 = 0;
    d2 = dMax;
  }
  rMax = (real)dMax;
  for (i = d1; i <= d2; i += dInc) {
    AngleR(&m, &n, x, y, (real)rx, (real)ry, (real)i * rDegMax / rMax);
    if (i > d1)
      Line(m0, n0, m, n, kv);
    m0 = m; n0 = n;
  }
}


// Draw an arc of a circle.

void CMap::ArcReal(real x, real y, real r, real d1, real d2, KV kv)
{
  real h, v, d, dInc;
  int m, n, m0, n0, i, iMax;

  // Figure out how many lines should be drawn, based on the radius.
  iMax = (int)((d2 - d1) / rDegQuad * r + rRound);
  iMax = Max(iMax, 1);
  dInc = (d2 - d1) / (real)iMax;

  // Draw the sides as if this were a subsection of a polygon.
  for (i = 0; i <= iMax; i++) {
    d = (d1 + (real)i * dInc);
    h = x + r*RCosD(d) + rRound;
    v = y + r*RSinD(d) + rRound;
    m = (int)h; n = (int)v;
    if (i > 0)
      Line(m0, n0, m, n, kv);
    m0 = m; n0 = n;
  }
}


/*
******************************************************************************
** Bitmap Special Editing
******************************************************************************
*/

// Parse and return a number from a turtle subaction string, advancing the
// string index to after the number.

int TurtleNumber(CONST char *szCmd, int *index)
{
  int n;
  char ch;

  ch = szCmd[*index];
  if (!FDigitCh(ch))
    return 1;
  n = 0;
  loop {
    if (!FDigitCh(ch))
      return n;
    if (n > (lHighest - 9) / 10) {
      PrintSzN_E("Turtle number overflow: %d\n", n);
      return 0;
    }
    n = n*10 + (ch - '0');
    (*index)++;
    ch = szCmd[*index];
  }
}


// Draw a sequence of lines on the bitmap, based on subactions in the string.
// This is similar to the "draw" command in BASIC.

flag CMon::Turtle(CONST char *szCmd)
{
  int index, x, y, deltax, deltay, deltaz, relx, rely, i, temp;
  flag fBlank = fFalse, fNoupdate = fFalse;
  char szT[cchSzDef], cmd;
  uchar ch, chT;

  index = 0;
  loop {
    cmd = ChCap(szCmd[index]);
    index++;
    switch (cmd) {
    case chNull:
      goto LDone;

    // Blank: The next action will just update the pen position and not draw.
    case 'B':
      fBlank = fTrue;
      break;

    // No update: The next action will not update the pen position.
    case 'N':
      fNoupdate = fTrue;
      break;

    // Color: Set whether to draw using on or off pixels.
    case 'C':
      gs.turtlec = FOdd(TurtleNumber(szCmd, &index));
      break;

    // Scale: Set the scale to apply to all relative motion.
    case 'S':
      gs.turtles = TurtleNumber(szCmd, &index);
      break;

    // Angle: Set the rotation to apply to all relative motion.
    case 'A':
      gs.turtlea = TurtleNumber(szCmd, &index);
      break;

    // Move to the specified absolute or relative coordinates.
    case 'M':
      relx = rely = 0;
      if (szCmd[index] == '-') {
        index++;
        relx = -1;
      } else if (szCmd[index] == '+') {
        index++;
        relx = 1;
      }
      deltax = TurtleNumber(szCmd, &index);
      if (relx < 0)
        neg(deltax);
      if (szCmd[index] != ',') {
        PrintSz_E("Bad turtle coordinates.\n");
        return fFalse;
      }
      index++;
      if (szCmd[index] == '-') {
        index++;
        rely = -1;
      } else if (szCmd[index] == '+') {
        index++;
        rely = 1;
      }
      deltay = TurtleNumber(szCmd, &index);
      if (rely < 0)
        neg(deltay);
      if (relx != 0 && rely != 0) {
        switch (gs.turtlea) {
        case 1:
          temp   = deltax;
          deltax = -deltay;
          deltay = temp;
          break;
        case 2:
          neg(deltax);
          neg(deltay);
          break;
        case 3:
          temp   = deltax;
          deltax = deltay;
          deltay = -temp;
          break;
        }
      }
      x = gs.turtlex;
      y = gs.turtley;
      if (relx == 0)
        gs.turtlex = deltax;
      else
        gs.turtlex += deltax * gs.turtles;
      if (rely == 0)
        gs.turtley = deltay;
      else
        gs.turtley += deltay * gs.turtles;
      if (!fBlank)
        Line(x, y, gs.turtlex, gs.turtley, gs.turtlec);
      if (fNoupdate) {
        gs.turtlex = x;
        gs.turtley = y;
      }
      break;

    // Draw a line (or arc) from the current pen position.
    case 'U': case 'D': case 'L': case 'R':
    case 'E': case 'F': case 'G': case 'H':
    case 'O':
      switch (cmd) {
      case 'U': deltax =  0; deltay = -1; break;
      case 'D': deltax =  0; deltay = 1;  break;
      case 'L': deltax = -1; deltay = 0;  break;
      case 'R': deltax =  1; deltay = 0;  break;
      case 'E': deltax =  1; deltay = -1; break;
      case 'F': deltax =  1; deltay = 1;  break;
      case 'G': deltax = -1; deltay = 1;  break;
      case 'H': deltax = -1; deltay = -1; break;
      case 'O':
        ch = ChCap(szCmd[index]);
        index++;
        switch (ch) {
          case 'A': deltax =  1; deltay = -1; deltaz = fFalse; break;
          case 'B': deltax =  1; deltay = -1; deltaz = fTrue;  break;
          case 'C': deltax =  1; deltay = 1;  deltaz = fTrue;  break;
          case 'D': deltax =  1; deltay = 1;  deltaz = fFalse; break;
          case 'E': deltax = -1; deltay = 1;  deltaz = fFalse; break;
          case 'F': deltax = -1; deltay = 1;  deltaz = fTrue;  break;
          case 'G': deltax = -1; deltay = -1; deltaz = fTrue;  break;
          case 'H': deltax = -1; deltay = -1; deltaz = fFalse; break;
          default:
            gs.turtleo = (ch == '1'); continue;
        }
        break;
      }
      switch (gs.turtlea) {
      case 1:
        temp   = deltax;
        deltax = -deltay;
        deltay = temp;
        break;
      case 2:
        deltax = -deltax;
        deltay = -deltay;
        break;
      case 3:
        temp   = deltax;
        deltax = deltay;
        deltay = -temp;
        break;
      }
      temp = TurtleNumber(szCmd, &index) * gs.turtles;
      x = gs.turtlex;
      y = gs.turtley;
      if (fBlank) {
        gs.turtlex += deltax*temp;
        gs.turtley += deltay*temp;
      } else {
        Set(gs.turtlex, gs.turtley, gs.turtlec);
        if (cmd != 'O') {

          // Draw an orthogonal or diagonal line.
          for (i = 0; i < temp; i++) {
            gs.turtlex += deltax;
            gs.turtley += deltay;
            Set(gs.turtlex, gs.turtley, gs.turtlec);
          }
        } else {

          // Draw an arc quadrant.
          if (!gs.turtleo) {
            if (deltaz)
              for (i = 0; i < temp; i++) {
                gs.turtlex += deltax;
                Set(gs.turtlex, gs.turtley, gs.turtlec);
              }
            for (i = 0; i < temp; i++) {
              gs.turtley += deltay;
              Set(gs.turtlex, gs.turtley, gs.turtlec);
            }
            if (!deltaz)
              for (i = 0; i < temp; i++) {
                gs.turtlex += deltax;
                Set(gs.turtlex, gs.turtley, gs.turtlec);
              }
          } else {
            relx = gs.turtlex + (deltaz ? 0 : deltax*temp);
            rely = gs.turtley + (deltaz ? deltay*temp : 0);
            i = ((ch - 'A') >> 1) + gs.turtlea;
            if (!FOdd(ch - 'A'))
              i += 2;
            i &= 3;
            ArcQuadrant(relx, rely, temp, i, gs.turtlec);
            gs.turtlex += deltax*temp;
            gs.turtley += deltay*temp;
          }
        }
      }
      if (fNoupdate) {
        gs.turtlex = x;
        gs.turtley = y;
      }
      break;

    // Draw text or set the font to use.
    case 'T':
      chT = szCmd[index++];
      if (chT == chNull)
        goto LDone;
      if (FBetween(chT, '0', '3')) {
        gs.turtlet = (chT - '0');
        break;
      }
      if (ChCap(chT) == 'T')
        chT = chNull;
      x = gs.turtlex; y = gs.turtley;
      loop {
        ch = szCmd[index++];
        if (ch == chNull)
          goto LDone;
        if (ch == chT)
          break;
        if (ch == '\n') {
          i = gs.turtlet == 0 ? 6 : (gs.turtlet == 1 ? 10 : 16);
          i *= gs.turtles;
          gs.turtlex = x + xoff[gs.turtlea]*i;     // 0, -1, 0, 1
          gs.turtley = y + yoff[gs.turtlea ^ 2]*i; // 1, 0, -1, 0
          x = gs.turtlex; y = gs.turtley;
        }
        if (FBetween(ch, 32, gs.turtlet != 1 ? 127 : 255)) {
          switch (gs.turtlet) {
          case 0:
            ch = ChCap(ch);
            Turtle(szTurtleCh1[ch - 32]);
            break;
          case 1:
            x = gs.turtlex; y = gs.turtley;
            Turtle(szTurtleCh2[ch - 32]);
            i = 6 * gs.turtles;
            gs.turtlex = x + xoff[gs.turtlea ^ 3]*i; // 1, 0, -1, 0
            gs.turtley = y + yoff[gs.turtlea ^ 3]*i; // 0, 1, 0, -1
            break;
          case 2:
            ch = ChCap(ch);
            Turtle("R2");
            Turtle(szTurtleCh3[ch - 32]);
            Turtle("R2");
            break;
          case 3:
            x = gs.turtlex; y = gs.turtley;
            Turtle(szTurtleCh4[ch - 32]);
            i = 9 * gs.turtles;
            gs.turtlex = x + xoff[gs.turtlea ^ 3]*i; // 1, 0, -1, 0
            gs.turtley = y + yoff[gs.turtlea ^ 3]*i; // 0, 1, 0, -1
            break;
          }
        }
      }
      break;

    // Fill or flood the region the pen is over.
    case '`':
      FFill(gs.turtlex, gs.turtley, gs.turtlec);
      break;
    case '~':
      FFlood(gs.turtlex, gs.turtley, gs.turtlec);
      break;

    // Reset Turtle settings to their defaults.
    case '_':
      gs.turtlex = gs.turtley = gs.turtlea = 0;
      gs.turtles = 1;
      gs.turtlec = fOn;
      gs.turtleo = fTrue;
      gs.turtlet = fFalse;
      break;

    default:
      sprintf(S(szT), "Bad turtle subaction '%c'.\n", cmd); PrintSz_E(szT);
      return fFalse;
    }
    if (cmd != 'B' && cmd != 'N')
      fBlank = fNoupdate = fFalse;
  }
LDone:
  return fTrue;
}


// Flood an irregular shape in a bitmap with the given bit color.

flag CMon::FFillCore(int x, int y, KV o, flag fFlood)
{
  PT *rgpt;
  long ipt = 0;
  int xnew, ynew, d, dMax;

  if (!FLegalFill(x, y, o))
    return fTrue;
  if (F64K())
    return fFalse;
  rgpt = RgAllocate(m_x*m_y, PT);
  if (rgpt == NULL)
    return fFalse;
  dMax = DIRS + fFlood*DIRS;
LSet:
  Set(x, y, o);
  loop {
    for (d = 0; d < dMax; d++) {
      xnew = x + xoff[d]; ynew = y + yoff[d];
      if (FLegalFill(xnew, ynew, o)) {
        FillPush(x, y);
        x = xnew; y = ynew;
        goto LSet;
      }
    }
    if (ipt <= 0)
      break;
    FillPop(x, y);
  }
  DeallocateP(rgpt);
  return fTrue;
}


// Calculate and display the perimeter of a shape of on pixels.

long CMon::AnalyzePerimeter(int x0, int y0, flag fCorner) CONST
{
  long cLength = 0, cSide = 0;
  int x, y, x1, y1, x2, y2, d = 0, dOld = -1;
  char sz[cchSzMax];

  // Find an edge pixel of a shape.
  if (!FLegal(x0, y0) || !Get(x0, y0)) {
    if (!FBitmapFind(&x0, &y0, fOn))
      return 0;
  }
  while (Get(x0 + 1, y0))
    x0++;
  x = x0; y = y0;

  // Loop around the shape boundary, counting the number of segments.
  do {
    dOld = d;
    x1 = x + xoff[d]; y1 = y + yoff[d];
    x2 = x1 + xoff[d-1 & DIRS1]; y2 = y1 + yoff[d-1 & DIRS1];
    if (!fCorner) {
      if (Get(x1, y1)) {
        if (Get(x2, y2)) {
          x = x2; y = y2;
          d = d-1 & DIRS1;
        } else {
          x = x1; y = y1;
        }
      } else
        d = d+1 & DIRS1;
    } else {
      if (Get(x2, y2)) {
        x = x2; y = y2;
        d = d-1 & DIRS1;
      } else if (Get(x1, y1)) {
        x = x1; y = y1;
      } else
        d = d+1 & DIRS1;
    }
    cLength++;
    if (d != dOld)
      cSide++;
    dOld = d;
  } while (x != x0 || y != y0 || d != 0);

  cSide = cSide & ~1;
  sprintf(S(sz),
    "Perimeter length: %d\nPerimeter sides: %d\nAverage length: %.2lf\n",
    cLength, cSide, (real)cLength / (real)cSide);
  PrintSz_N(sz);
  return cLength;
}


/*
******************************************************************************
** Bitmap Transformations
******************************************************************************
*/

// Return the number of on pixels in a bitmap.

long CMon::BitmapCount() CONST
{
  int x, y;
  long count = 0;
  dword l;
  byte *pb = (byte *)&l;

  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x; x += 32) {
      l = _L(x, y);
      if (x + 32 > m_x) {
        SwapN(pb[0], pb[3]); SwapN(pb[1], pb[2]);
        // Ignore bits off the right edge of the bitmap.
        l &= ~((1L << (x + 32 - m_x)) - 1);
      }

      // Fast way of counting the number of on bits in a long. Add neighboring
      // bits together, then groups of four, then eight, etc.
      l = ((l & 0xaaaaaaaaL) >> 1) + (l & 0x55555555L);
      l = ((l & 0xccccccccL) >> 2) + (l & 0x33333333L);
      l = ((l & 0xf0f0f0f0L) >> 4) + (l & 0x0f0f0f0fL);
      l = ((l & 0xff00ff00L) >> 8) + (l & 0x00ff00ffL);
      l = (l >> 16)                + (l & 0x0000ffffL);
      count += l;
    }
  return count;
}


// Find the first on or off pixel in a bitmap, and return its coordinates.

flag CMon::FBitmapFind(int *x0, int *y0, bit o) CONST
{
  int x, y;

  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x; x++)
      if (_Get(x, y) == o) {
        *x0 = x; *y0 = y;
        return fTrue;
      }
  return fFalse;
}


// Return whether one monochrome bitmap is a proper subset of another.

flag CMon::FBitmapSubset(CONST CMon &b2) CONST
{
  int x, y;
  flag f = fFalse;

  // 2nd bitmap must exist and be the same size as the main bitmap.
  if (b2.FNull() || b2.m_x != m_x || b2.m_y != m_y)
    return fFalse;
  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x; x++)
      if (!b2.Get(x, y)) {
        // Reject 2nd bitmap if it contains an off pixel not off in main.
        if (Get(x, y))
          return fFalse;
      } else if (!Get(x, y)) {
        // 2nd bitmap must have at least one on pixel off in the main.
        f = fTrue;
      }
  return f;
}


// Set or clear all pixels in a bitmap.

void CMon::BitmapSet(KV o)
{
  dword l;
  long clBitmap, il;

  l = o ? dwSet : 0;
  clBitmap = CbBitmap(m_x, m_y) >> 2;
  // Set 32 pixels at a time.
  for (il = 0; il < clBitmap; il++)
    *_Pl(il) = l;
  if (gs.fTraceDot && FVisible())
    UpdateDisplay();
}


// Invert all pixels in a bitmap.

void CMon::BitmapReverse()
{
  long clBitmap, il;

  clBitmap = CbBitmap(m_x, m_y) >> 2;
  // Invert 32 pixels at a time.
  for (il = 0; il < clBitmap; il++)
    *_Pl(il) ^= dwSet;
}


// Set all pixels in a bitmap randomly, with on pixels at a given percentage.

void CMon::BitmapRandom(int nRatio, int nTotal)
{
  int x, y;

  BitmapOff();
  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x; x++)
      if (Rnd(0, nTotal-1) < nRatio)
        Set1(x, y);
}


// Draw a moire pattern in a bitmap, composed of on or off lines radiating
// from the center of the bitmap to the edges.

void CMon::BitmapMoire()
{
  int x0, y0, x, y;

  x0 = m_x >> 1; y0 = m_y >> 1;
  for (y = 0; y < m_y; y++) {
    Line(x0, y0, 0, y, FOdd(y));
    Line(x0, y0, m_x-1, y, FOdd(y) == FOdd(m_x));
  }
  for (x = 0; x < m_x; x++) {
    Line(x0, y0, x, 0, FOdd(x));
    Line(x0, y0, x, m_y-1, FOdd(x) == FOdd(m_y));
  }
}


// Copy the contents of one bitmap to another bitmap.

flag CMap::FBitmapCopy(CONST CMap &bSrc)
{
  long clBitmap, il;

  if (!FBitmapSizeSet(bSrc.m_x, bSrc.m_y))
    return fFalse;
  Copy3(bSrc);
  Assert(m_cfPix == bSrc.m_cfPix);
  clBitmap = m_y * m_clRow;
  for (il = 0; il < clBitmap; il++)
    *_Rgl(il) = *bSrc._Rgl(il);
  return fTrue;
}


// Copy pixels from one bitmap to another bitmap, combining them using the
// specified logical operator table.

void CMon::BitmapMoveOrAnd(CONST CMon &bSrc, CONST int *rgf)
{
  int x, y, x1, y1;

  x1 = Min(m_x, bSrc.m_x);
  y1 = Min(m_y, bSrc.m_y);
  for (y = 0; y < y1; y++)
    for (x = 0; x < x1; x++)
      Set(x, y, rgf[Get(x, y) + bSrc.Get(x, y)]);
}


// Logical "or" one bitmap into another bitmap.

void CMon::BitmapOr(CONST CMon &bSrc)
{
  long clBitmap, il;

  // Fast case: For bitmaps of the same size, combine 32 pixels at a time.
  if (m_x == bSrc.m_x && m_y == bSrc.m_y) {
    clBitmap = CbBitmap(m_x, m_y) >> 2;
    for (il = 0; il < clBitmap; il++)
      *_Pl(il) |= *bSrc._Pl(il);
    return;
  }

  BitmapMoveOrAnd(bSrc, rgfOr);
}


// Logical "and" one bitmap into another bitmap.

void CMon::BitmapAnd(CONST CMon &bSrc)
{
  long clBitmap, il;

  // Fast case: For bitmaps of the same size, combine 32 pixels at a time.
  if (m_x == bSrc.m_x && m_y == bSrc.m_y) {
    clBitmap = CbBitmap(m_x, m_y) >> 2;
    for (il = 0; il < clBitmap; il++)
      *_Pl(il) &= *bSrc._Pl(il);
    return;
  }

  BitmapMoveOrAnd(bSrc, rgfAnd);
}


// Logical "exclusive or" one bitmap into another bitmap.

void CMon::BitmapXor(CONST CMon &bSrc)
{
  long clBitmap, il;

  // Fast case: For bitmaps of the same size, combine 32 pixels at a time.
  if (m_x == bSrc.m_x && m_y == bSrc.m_y) {
    clBitmap = CbBitmap(m_x, m_y) >> 2;
    for (il = 0; il < clBitmap; il++)
      *_Pl(il) ^= *bSrc._Pl(il);
    return;
  }

  BitmapMoveOrAnd(bSrc, rgfXor);
}


// Blend one bitmap into another bitmap, where half the pixels come from one
// bitmap and half from the other, in a checkerboard pattern.

void CMon::BitmapBlend(CONST CMon &bSrc)
{
  int x, y;

  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x; x++)
      Set(x, y, (FOdd(x ^ y) ? bSrc : *this).Get(x, y));
}


// Copy one bitmap into another bitmap, tessellating or forming a grid of the
// source bitmap pattern within the bounds of the destination bitmap.

void CMap::BitmapTessellate(CONST CMap &bSrc)
{
  int x, y;

  for (y = 0; y < m_y; y += bSrc.m_y)
    for (x = 0; x < m_x; x += bSrc.m_x)
      BlockMove(bSrc, 0, 0, bSrc.m_x-1, bSrc.m_y-1, x, y);
}


// Resize a bitmap to the specified size, then tessellate the old pattern
// within the bounds of the new dimensions.

flag CMap::FBitmapTessellateTo(int x, int y)
{
  CMap *bNew;

  bNew = Create();
  if (bNew == NULL)
    return fFalse;
  if (!bNew->FAllocate(x, y, this))
    return fFalse;
  bNew->BitmapTessellate(*this);
  CopyFrom(*bNew);
  bNew->Destroy();
  return fTrue;
}


// Flip the pixels in a bitmap horizontally.

void CMon::BitmapFlipX()
{
  int x, y, o;
  long il, ilT;
  dword l;

  if ((m_x & 31) == 0) {
    // Fast case: For bitmaps with horizontal dimensions divisible by 32, flip
    // 32 bits at a time.
    for (y = 0; y < m_y; y++) {
      il = ilT = _Il(0, y);
      for (x = 0; x < m_clRow; x++) {
        l = *_Pl(il);

        // Fast way of flipping the bits in a long. Flip neighboring bits with
        // each other, then groups of four, then eight, etc.
        l = (l & 0xaaaaaaaaL) >> 1 | (l & 0x55555555L) << 1;
        l = (l & 0xccccccccL) >> 2 | (l & 0x33333333L) << 2;
        l = (l & 0xf0f0f0f0L) >> 4 | (l & 0x0f0f0f0fL) << 4;
        l = (l & 0xff00ff00L) >> 8 | (l & 0x00ff00ffL) << 8;
        l = l >> 16                | (l & 0x0000ffffL) << 16;

        *_Pl(il) = l;
        il++;
      }
      il--;
      while (ilT < il) {
        l = *_Pl(ilT);
        *_Pl(ilT) = *_Pl(il);
        *_Pl(il) = l;
        ilT++, il--;
      }
    }
  } else {
    // Normal case: Flip one bit at a time.
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x >> 1; x++) {
        o = Get(x, y);
        Set(x, y, Get(m_x-1-x, y));
        Set(m_x-1-x, y, o);
      }
  }
}


// Flip the pixels in a bitmap vertically.

void CMap::BitmapFlipY()
{
  int x, y;
  dword l, *pl1 = _Rgl(0), *pl2 = _Rgl((m_y-1) * m_clRow);

  // Swap one row at a time, 32 bits at a time.
  for (y = 0; y < m_y >> 1; y++) {
    for (x = 0; x < m_clRow; x++) {
      l = *pl1; *pl1 = *pl2; *pl2 = l;
      pl1++, pl2++;
    }
    pl2 -= (m_clRow << 1);
  }
}


// Transpose the pixels in a bitmap i.e. flip across the main diagonal.

flag CMap::FBitmapTranspose()
{
  CMap *bNew;
  int x, y;
  KV kv, kv2;
  flag fColor = FColor();

  if (m_x == m_y) {
    // For square bitmaps, just swap the pixels across the main diagonal.
    for (y = 1; y < m_y; y++)
      for (x = 0; x < y; x++) {
        kv = Get(x, y);
        kv2 = Get(y, x);
        if (kv != kv2) {
          Set(x, y, kv2);
          Set(y, x, kv);
        }
      }
  } else {
    // For rectangular bitmaps, create a new bitmap of the transposed size.
    bNew = Create();
    if (bNew == NULL)
      return fFalse;
    if (!bNew->FAllocate(m_y, m_x, this))
      return fFalse;
    if (!fColor)
      bNew->BitmapOff();
    for (y = 0; y < bNew->m_y; y++)
      for (x = 0; x < bNew->m_x; x++)
        if (fColor)
          bNew->Set(x, y, Get(y, x));
        else if (Get(y, x))
          bNew->Set1(x, y);
    CopyFrom(*bNew);
    bNew->Destroy();
  }
  return fTrue;
}


// Make a bitmap be a given size, reallocating or allocating in the first
// place if necessary. The contents of the bitmap are undefined.

flag CMon::FBitmapSizeSet(int xnew, int ynew)
{
  CMon bNew;

  if (!FNull()) {
    // Check if bitmap will have same dimensions, or same number of bytes.
    if (m_x == xnew && m_y == ynew)
      return fTrue;
    if (xnew > xBitmap || ynew > yBitmap) {
      PrintSzNN_E("Can't create bitmap larger than %d by %d!\n",
        xBitmap, yBitmap);
      return fFalse;
    }
    if (CbBitmap(m_x, m_y) == CbBitmap(xnew, ynew)) {
      m_x = xnew; m_y = ynew;
      m_clRow = CbBitmapRow(xnew) >> 2;
      return fTrue;
    }
  }
  if (!bNew.FAllocate(xnew, ynew, !FNull() ? this : NULL))
    return fFalse;
  CopyFrom(bNew);
  return fTrue;
}


// Resize a bitmap to be a given size, by either truncating or appending off
// pixels to its right and bottom edges.

flag CMap::FBitmapResizeTo(int xnew, int ynew)
{
  CMap *bNew;

  if (m_x == xnew && m_y == ynew)
    return fTrue;
  bNew = Create();
  if (!bNew->FAllocate(xnew, ynew, this))
    return fFalse;
  bNew->BitmapOff();
  if (!FZero())
    bNew->BlockMove(*this, 0, 0, m_x-1, m_y-1, 0, 0);
  CopyFrom(*bNew);
  bNew->Destroy();
  return fTrue;
}


// Resize a bitmap by a given offset, by either truncating or appending off
// pixels to its left and top edges.

flag CMap::FBitmapShiftBy(int xnew, int ynew)
{
  CMap *bNew;
  int x, y;

  if (xnew == 0 && ynew == 0)
    return fTrue;
  bNew = Create();
  if (!bNew->FAllocate(m_x + xnew, m_y + ynew, this))
    return fFalse;
  for (y = 0; y < bNew->m_y; y++)
    for (x = 0; x < bNew->m_x; x++)
      bNew->Set(x, y, Get(x - xnew, y - ynew));
  CopyFrom(*bNew);
  bNew->Destroy();
  return fTrue;
}


// Resize a bitmap to be a given size. The old contents of the bitmap are
// stretched or compacted to fit within the new dimensions.

flag CMon::FBitmapZoomTo(int xnew, int ynew, flag fPreserve)
{
  CMon bNew;
  int x, y, x1, y1, x2, y2;
  real deltax, deltay;

  if (!bNew.FAllocate(xnew, ynew, this))
    return fFalse;
  bNew.BitmapOff();
  if (FZero() || bNew.FZero())
    goto LDone;
  deltax = (real)m_x / (real)xnew;
  deltay = (real)m_y / (real)ynew;
  if (fPreserve && deltax >= 1.0 && deltay >= 1.0) {

    // Preserve on pixels when shrinking case: If the bitmap isn't growing in
    // either dimension, then there are always one or more source pixels to
    // map to a destination pixel. Make each destination pixel on if any of
    // the source pixels that map to it are on.
    for (y = 0; y < ynew; y++)
      for (x = 0; x < xnew; x++) {
        y2 = (int)((real)(y+1)*deltay);
        for (y1 = (int)((real)y*deltay); y1 < y2; y1++) {
          x2 = (int)((real)(x+1)*deltax);
          for (x1 = (int)((real)x*deltax); x1 < x2; x1++)
            if (Get(x1, y1)) {
              bNew.Set1(x, y);
              goto LNext;
            }
        }
LNext:
        ;
      }
  } else {

    // Normal case: Make each destination pixel a copy of the one source pixel
    // that best maps to it.
    for (y = 0; y < ynew; y++)
      for (x = 0; x < xnew; x++)
        if (Get((int)((real)x*deltax + rMinute),
          (int)((real)y*deltay + rMinute)))
          bNew.Set1(x, y);
  }
LDone:
  CopyFrom(bNew);
  return fTrue;
}


// Zoom a bitmap and its contents by the given factors. Each source pixel
// becomes an x by y block in the destination, where x and y can be different
// for even and odd numbered rows and columns.

flag CMon::FBitmapBias(int xbias1, int xbias2, int ybias1, int ybias2)
{
  CMon bNew;
  int x, y;

  if (!bNew.FAllocate((m_x >> 1)*(xbias1 + xbias2) + FOdd(m_x)*xbias1,
    (m_y >> 1)*(ybias1 + ybias2) + FOdd(m_y)*ybias1, this))
    return fFalse;
  bNew.BitmapOff();
  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x; x++)
      if (Get(x, y))
        bNew.Block((x >> 1)*(xbias1 + xbias2) + FOdd(x)*xbias1,
          (y >> 1)*(ybias1 + ybias2) + FOdd(y)*ybias1,
          ((x + 1) >> 1)*(xbias1 + xbias2) + FOdd(x + 1)*xbias1 - 1,
          ((y + 1) >> 1)*(ybias1 + ybias2) + FOdd(y + 1)*ybias1 - 1, fOn);
  CopyFrom(bNew);
  return fTrue;
}


// Resize a bitmap smaller, by trimming all blank rows and columns from its
// four edges.

flag CMap::FBitmapCollapse()
{
  int xnew, ynew, i;

  // Trim off pixels from the right and bottom edges.
  for (ynew = m_y-1; ynew >= 0; ynew--)
    for (i = 0; i < m_x; i++)
      if (Get(i, ynew))
        goto LNext1;
LNext1:
  for (xnew = m_x-1; xnew >= 0; xnew--)
    for (i = 0; i <= ynew; i++)
      if (Get(xnew, i))
        goto LNext2;
LNext2:
  if (!FBitmapResizeTo(xnew + 1, ynew + 1))
    return fFalse;

  // Trim off pixels from the left and top edges.
  for (ynew = 0; ynew < m_y; ynew++)
    for (i = 0; i < m_x; i++)
      if (Get(i, ynew))
        goto LNext3;
LNext3:
  for (xnew = 0; xnew < m_x; xnew++)
    for (i = ynew; i < m_y; i++)
      if (Get(xnew, i))
        goto LNext4;
LNext4:
  return FBitmapShiftBy(-xnew, -ynew);
}


// Zoom a bitmap and its contents 200%. Then smooth the result so it doesn't
// look blocky with just 2x2 blocks everywhere.

long CMon::BitmapSmooth(flag fAll)
{
  CMon b2;
  int x, y, x2, y2, grf;
  long count = 0;

  // Zoom the bitmap 200%.
  if (!b2.FAllocate(m_x << 1, m_y << 1, this))
    return -1;
  for (y = 0; y < b2.m_y; y++)
    for (x = 0; x < b2.m_x; x++)
      b2.Set(x, y, Get(x >> 1, y >> 1));

  // Fill in certain corners of the zoomed bitmap with on pixels.
  for (y = 0; y < m_y-1; y++)
    for (x = 0; x < m_x-1; x++) {

      // See what pixels are set in each 2x2 block.
      grf = (Get(x, y) << 3) + (Get(x+1, y) << 2) + (Get(x, y+1) << 1) +
        Get(x+1, y+1);
      x2 = (x << 1) + 1; y2 = (y << 1) + 1;

      // If two opposite pixels are set in a 2x2 block, set pixels inside the
      // "stairs" between the two blocks to form a thick diagonal line.
      if (grf == 6) {
        b2.Set1(x2, y2); b2.Set1(x2+1, y2+1);
        count++;
      } else if (grf == 9) {
        b2.Set1(x2+1, y2); b2.Set1(x2, y2+1);
        count++;
      } else if (fAll) {

        // If three pixels are set in a 2x2 block, set the pixel in the corner
        // if it looks like the end of a diagonal line, but leave sharp right
        // angle corners alone.
        if (grf == 7 && ((Get(x+2, y-1) && !Get(x+1, y-1)) ||
          (Get(x-1, y+2) && !Get(x-1, y+1)))) {
          b2.Set1(x2, y2);
          count++;
        } else if (grf == 11 && ((Get(x-1, y-1) && !Get(x, y-1)) ||
          (Get(x+2, y+2) && !Get(x+2, y+1)))) {
          b2.Set1(x2+1, y2);
          count++;
        } else if (grf == 13 && ((Get(x-1, y-1) && !Get(x-1, y)) ||
          (Get(x+2, y+2) && !Get(x+1, y+2)))) {
          b2.Set1(x2, y2+1);
          count++;
        } else if (grf == 14 && ((Get(x+2, y-1) && !Get(x+2, y)) ||
          (Get(x-1, y+2) && !Get(x, y+2)))) {
          b2.Set1(x2+1, y2+1);
          count++;
        }
      }
    }
  CopyFrom(b2);
  return count;
}


// Chisel off the corners of set content in a bitmap, by one pixel width.

long CMon::BitmapSmoothCorner(bit o)
{
  CMon bCopy;
  int x, y, i, j;
  long count = 0;

  if (!bCopy.FBitmapCopy(*this))
    return -1;
  j = 3 - (o << 1);
  for (y = -1; y <= m_y; y++)
    for (x = -1; x <= m_x; x++) {

      // For each 2x2 block in the bitmap, if only one pixel is set
      // appropriately, that means a corner or endpoint, so remove the pixel.
      i = bCopy.Get(x, y) + bCopy.Get(x+1, y) + bCopy.Get(x, y+1) +
        bCopy.Get(x+1, y+1);
      if (i == j) {
        Block(x, y, x+1, y+1, !o);
        count++;
      }
    }
  return count;
}


// Thicken all content in a bitmap.

long CMon::BitmapThicken()
{
  int x, y;
  long count = 0;

  // For each on pixel, expand it into a 2x2 block of on pixels.
  for (y = m_y-1; y >= 0; y--)
    for (x = m_x-1; x >= 0; x--)
      if (Get(x, y)) {
        Set1(x+1, y); Set1(x, y+1); Set1(x+1, y+1);
        count++;
      }
  return count;
}


// Thicken all content in a bitmap by a specified diameter.

long CMon::BitmapThicken2(int z, flag fCorner)
{
  CMon bCopy, bDot;
  char sz[cchSzDef];
  int x, y, x2, y2, h, f, i, iMax, grf;
  long count = 0;

  if (z <= 2)
    return BitmapThicken();
  if (!bCopy.FBitmapCopy(*this))
    return -1;

  // Compose a circle of on pixels to expand each bitmap pixel into.
  if (!bDot.FAllocate(z, z, this))
    return -1;
  bDot.BitmapOff();
  bDot.Turtle("S1A0C1O1");
  h = (z - 1) >> 1; f = !FOdd(z);
  sprintf(S(sz), "BM0,%dOA%dBR%dOC%dBD%dOE%dBL%dOG%dBM%d,%d`",
    h, h, f, h, f, h, f, h, h, h);
  bDot.Turtle(sz);

  // For each on pixel, expand it into a circle of on pixels.
  iMax = fCorner ? DIRS2 : DIRS;
  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x; x++)
      if (bCopy.Get(x, y)) {
        grf = 0;
        for (i = 0; i < iMax; i++)
          if (bCopy.Get(x + xoff[i], y + yoff[i]))
            grf |= (1 << i);

        if ((grf & NBinary(0,1,0,1,0,1,0,1)) == NBinary(0,1,0,1,0,1,0,1))
          continue;
        for (y2 = 0; y2 < bDot.m_y; y2++)
          for (x2 = 0; x2 < bDot.m_x; x2++)
            if (bDot.Get(x2, y2))
              Set1(x-h+x2, y-h+y2);

        // May want square instead of rounded endpoints.
        if (fCorner) {
          if ((grf & NBinary(1,1,1,0,0,0,1,1)) == 0)
            Block(x,   y,   x-h,   y-h,   fOn);
          if ((grf & NBinary(1,1,1,1,1,0,0,0)) == 0)
            Block(x,   y+f, x-h,   y+h+f, fOn);
          if ((grf & NBinary(0,0,1,1,1,1,1,0)) == 0)
            Block(x+f, y+f, x+h+f, y+h+f, fOn);
          if ((grf & NBinary(1,0,0,0,1,1,1,1)) == 0)
            Block(x+f, y,   x+h+f, y-h,   fOn);
        }
        count++;
      }
  return count;
}


// Return whether an on pixel can be turned off without significantly
// affecting the topology of the on pixels around it, i.e. don't split shapes,
// create holes, or shorten lines. Used by BitmapThinner().

flag CMon::FBitmapThinCore(int x, int y, flag fAll)
{
  flag rgf[DIRS2];
  int c[DIRS], c1 = 0, d;

  // Look at the pixel's eight neighbors. Count the number of on pixels and
  // the number of on in the three pixels closest to each of the four corners.
  for (d = 0; d < DIRS2; d++)
    rgf[d] = Get(x + xoff[d], y + yoff[d]);
  for (d = 0; d < DIRS; d++) {
    c[d] = rgf[d] + rgf[d+1 & DIRS1] + rgf[DIRS + d];
    c1 += rgf[d];
  }

  // If one corner is all on and the opposite corner all off, that means the
  // pixel in question is a corner and can be turned off. The pixel can also
  // be turned off if two adjacent corners are all on, and the edge opposite
  // them is off, meaning a small indentation can be made.
  for (d = 0; d < DIRS; d++)
    if (c[d] == 3 && ((c[d+2 & DIRS1] == 0) ||
      (fAll && c[d+1 & DIRS1] == 3 && c1 == 3))) {
      Set0(x, y);
      return fTrue;
    }
  return fFalse;
}


// Make all on pixel content in a bitmap as thin as possible, without
// splitting shapes or making anything shorter. After this most if not all
// content will be one pixel wide, with few or no 2x2 blocks anywhere.

long CMon::BitmapThinner(flag fCorner)
{
  int x, y, f, d, cd;
  long count = 0, cOld;
  flag rgf[DIRS2];

  // First file down all corners as much as possible, then allow making
  // indentations or cavities within edges.

  for (f = 0; f <= 1; f++) {

    // Check all pixels from upper left to lower right.
    cOld = count;
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++)
        if (Get(x, y))
          count += FBitmapThinCore(x, y, f);
    if (count == cOld)
      continue;

    // Check all pixels from lower right to upper left.
    cOld = count;
    for (y = m_y-1; y >= 0; y--)
      for (x = m_x-1; x >= 0; x--)
        if (Get(x, y))
          count += FBitmapThinCore(x, y, f);
    if (count == cOld)
      continue;

    // Check all pixels from upper right to lower left.
    cOld = count;
    for (x = m_x-1; x >= 0; x--)
      for (y = 0; y < m_y; y++)
        if (Get(x, y))
          count += FBitmapThinCore(x, y, f);
    if (count == cOld)
      continue;

    // Check all pixels from lower left to upper right.
    for (x = 0; x < m_x; x++)
      for (y = m_y-1; y >= 0; y--)
        if (Get(x, y))
          count += FBitmapThinCore(x, y, f);
  }

  // After all possible orthogonal adjustments made, allow removals that only
  // leave pixels connected diagonally. This will break all 2x2 blocks left,
  // except those like an "X" whose four corners are connected diagonally.

  if (fCorner) {
    for (y = 0; y < m_y; y++)
      for (x = 0; x < m_x; x++)
        if (Get(x, y)) {
          cd = 0;
          for (d = 0; d < DIRS2; d++) {
            f = Get(x + xoff[d], y + yoff[d]);
            rgf[d] = f;
            if (f)
              cd++;
          }
          if (cd < 3)
            continue;

          // Only delete this pixel if it won't cause a separation or a hole.
          // For each surrounding pixel, check if it's still connected to a
          // pixel counterclockwise. To delete there must be exactly one such
          // break. Less would make a hole, and more would make a separation.

          cd = 0;
          for (d = 0; d < DIRS2; d++)
            if (rgf[d] && (d < DIRS ? !rgf[d + 1 & DIRS1] && !rgf[DIRS + d] :
              !rgf[d + 1 & DIRS1]))
              cd++;
          if (cd == 1) {
            count++;
            Set0(x, y);
          }
        }
   }
  return count;
}


// Edit pixels in a bitmap so they're a single convex shape, with no concave
// inlets, as if a rubber band were wrapped around the on pixels.

flag CMon::FBitmapConvex(int x1, int y1, int x2, int y2)
{
  CMon bCopy;
  int x, y, m, n, m2, n2;

  // Determine the minimum bounding box containing on pixels.
  for (; x1 <= x2; x1++)
    for (y = y1; y <= y2; y++)
      if (Get(x1, y))
        goto LNext1;
  return fFalse;
LNext1:
  for (; x2 >= x1; x2--)
    for (y = y1; y <= y2; y++)
      if (Get(x2, y))
        goto LNext2;
  return fFalse;
LNext2:
  for (; y1 <= y2; y1++)
    for (x = x1; x <= x2; x++)
      if (Get(x, y1))
        goto LNext3;
  return fFalse;
LNext3:
  for (; y2 >= y1; y2--)
    for (x = x1; x <= x2; x++)
      if (Get(x, y2))
        goto LNext4;
  return fFalse;
LNext4:

  // Start with a solid block.
  if (!bCopy.FBitmapCopy(*this))
    return fFalse;
  Block(x1, y1, x2, y2, fOn);

  // Loop over the four quadrants in the bitmap.
  for (y = -1; y <= 1; y += 2)
    for (x = -1; x <= 1; x += 2) {
      m2 = x < 0 ? x1 : x2;
      n2 = y < 0 ? y1 : y2;
      for (m = m2, n = n2;; n -= y)
        if (bCopy.Get(m2, n))
          break;
      n += y;

      // In each quadrant, shave off lines of on pixels from the block, if
      // that line doesn't hit any on pixels in the original bitmap.
      while (n != n2 + y) {
        if (bCopy.FLineUntil(m2, n, m, n2, NULL, NULL, fOn, fFalse, fFalse))
          n += y;
        else {
          Line(m2, n, m, n2, fOff);
          m -= x;
        }
      }
    }
  return fTrue;
}


// Zoom a bitmap by 200% plus one extra pixel. Boundaries or edges between
// on and off pixels in the old bitmap, become lines of on pixels in the new.

flag CMon::FBitmapAccentBoundary()
{
  CMon bNew;
  int x, y, xnew, ynew;
  bit o0, o1, o2, o3;

  if (!bNew.FAllocate((m_x << 1) + 1, (m_y << 1) + 1, this))
    return fFalse;
  bNew.BitmapOff();
  for (y = -1; y < m_y; y++) {
    ynew = (y + 1) << 1;
    for (x = -1; x < m_x; x++) {
      xnew = (x + 1) << 1;
      o0 = Get(x, y);   o1 = Get(x+1, y);
      o2 = Get(x, y+1); o3 = Get(x+1, y+1);
      if (o1 != o3 || o2 != o3 || o0 != o3)
        bNew.Set1(xnew, ynew);
      if (o1 != o3)
        bNew.Set1(xnew+1, ynew);
      if (o2 != o3)
        bNew.Set1(xnew, ynew+1);
    }
  }
  CopyFrom(bNew);
  return fTrue;
}


// Change a bitmap to highlight areas of contrast, i.e. so that each pixel is
// on if there's a neighboring pixel that's a different value from it.

flag CMon::FBitmapAccentContrast(flag fCorner)
{
  CMon bCopy;
  int x, y, d, dMax = DIRS + fCorner*DIRS;
  bit o;

  if (!bCopy.FBitmapCopy(*this))
    return fFalse;
  BitmapOff();
  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x; x++) {
      o = bCopy.Get(x, y);
      for (d = 0; d < dMax; d++)
        if (bCopy.Get(x + xoff[d], y + yoff[d]) != o) {
          Set1(x, y);
          break;
        }
    }
  return fTrue;
}


// Offset the content in a bitmap by the given number of pixels. This doesn't
// change the bitmap's size or lose any data, where pixels that move off an
// edge of the bitmap reappear on the opposite.

flag CMap::FBitmapSlide(int dx, int dy)
{
  CMap *bNew;

  bNew = Create();
  if (!bNew->FBitmapCopy(*this))
    return fFalse;
  while (dx < 0)
    dx += m_x;
  while (dx >= m_x)
    dx -= m_x;
  while (dy < 0)
    dy += m_y;
  while (dy >= m_y)
    dy -= m_y;

  // Copy content in four quadrants from the old bitmap to the new.
  bNew->BlockMove(*this, dx, dy, m_x-1, m_y-1, 0, 0);
  if (dx > 0)
    bNew->BlockMove(*this, 0, dy, dx-1, m_y-1, m_x-dx, 0);
  if (dy > 0)
    bNew->BlockMove(*this, dx, 0, m_x-1, dy-1, 0, m_y-dy);
  if (dx > 0 && dy > 0)
    bNew->BlockMove(*this, 0, 0, dx-1, dy-1, m_x-dx, m_y-dy);
  CopyFrom(*bNew);
  bNew->Destroy();
  return fTrue;
}


// Duplicate or delete a range of rows or columns in a bitmap, shifting the
// rest of the content appropriately.

flag CMap::FBitmapEditRowColumn(int z1, int z2, flag fX, flag fAdd)
{
  CMap *bNew;
  int x, y, d;

  // Do nothing if either index is outside the bounds of the bitmap.
  if (z1 < 0 || (fX && z1 >= m_x) || (!fX && z1 >= m_y) ||
    z2 < 0 || (fX && z2 >= m_x) || (!fX && z2 >= m_y))
    return fFalse;
  SortN(&z1, &z2);
  bNew = Create();

  // Figure out the new size of the bitmap.
  d = z2 - z1 + 1;
  x = m_x; y = m_y;
  if (fX)
    x += fAdd ? d : -d;
  else
    y += fAdd ? d : -d;
  if (!bNew->FAllocate(x, y, this))
    return fFalse;

  // Copy content in two halves from the old bitmap to the new.
  if (fX) {
    bNew->BlockMove(*this, 0, 0, fAdd ? z2 : z1-1, m_y-1, 0, 0);
    bNew->BlockMove(*this, fAdd ? z1 : z2+1, 0, m_x-1, m_y-1,
      fAdd ? z2+1 : z1, 0);
  } else {
    bNew->BlockMove(*this, 0, 0, m_x-1, fAdd ? z2 : z1-1, 0, 0);
    bNew->BlockMove(*this, 0, fAdd ? z1 : z2+1, m_x-1, m_y-1,
      0, fAdd ? z2+1 : z1);
  }
  CopyFrom(*bNew);
  bNew->Destroy();
  return fTrue;
}


// Highlight pixels within a given radius, that are definitely blocked by on
// pixels, when looking in all directions from a central point.

void CMon::BitmapSeen(int x0, int y0, int dr)
{
  int rSq, x, y, z, d, i, j, dx, dy, xAbs, yAbs, xInc, yInc;
  flag fCircle = dr < 0;

  if (fCircle) {
    neg(dr);
    rSq = Sq(dr);
  }

  // Loop in widening squares from the central point
  for (z = 1; z <= dr; z++) {
    x = x0 - z; y = y0 - z;
    d = DIRS1;
    for (i = 0; i < DIRS; i++) {

      // Loop along each edge of the current square
      for (j = 0; j < (z << 1); j++, x += xoff[d], y += yoff[d]) {
        if (!FLegal(x, y) ||
          (fCircle && Sq(x - x0) + Sq(y - y0) > rSq) || Get(x, y))
          continue;
        dx = x - x0; dy = y - y0;
        xAbs = NAbs(dx); yAbs = NAbs(dy);
        xInc = NSgn(dx); yInc = NSgn(dy);

        // Check whether an off pixel is behind on pixel(s)
        if (Get(x - xInc, y - yInc) &&
          (xInc == 0 || yInc == 0 || xAbs == yAbs ||
          Get(x - xInc*(xAbs > yAbs), y - yInc*(yAbs > xAbs))))
          Set1(x, y);
      }
      DirDec(d);
    }
  }
}


// Create a simple random dot stereogram in a bitmap, replacing a two level
// image in that bitmap. A second bitmap may be used for the random dots.

flag CMon::FStereogram(CONST CMon &bTexture, int width, int height)
{
  CMon bCopy;
  int x, y, h, w, wDrop = NAbs(height);
  flag fDeep = height < 0, fTexture, f;

  Assert(height < width);
  if (!bCopy.FBitmapCopy(*this))
    return fFalse;
  BitmapOff();
  fTexture = !bTexture.FNull() && bTexture.m_x == m_x && bTexture.m_y == m_y;

  for (y = 0; y < m_y; y++) {
    for (x = 0; x < m_x; x++) {
      f = bCopy._Get(x, y);
      h = f ? height : 0;
      w = width - h;
      if (x - w < 0 || (f == fDeep && bCopy.Get(x - wDrop, y) == !fDeep)) {
        if (fTexture ? bTexture.Get(x, y) : Rnd(0, 1))
          Set1(x, y);
      } else {
        if (Get(x - w, y))
          Set1(x, y);
      }
    }
  }
  return fTrue;
}


// Treat the bitmap as a board of the "Life" cellular automaton, and create
// the next generation.

long CMon::LifeGenerate(flag fTorus)
{
  CMon bCopy;
  int x, y, i, n, xT, yT;
  long count = 0;

  if (!bCopy.FBitmapCopy(*this))
    return -1;
  for (y = 0; y < m_y; y++)
    for (x = 0; x < m_x; x++) {

      // Count the number of neighboring live cells.
      n = 0;
      if (!fTorus) {
        for (i = 0; i < DIRS2; i++)
          n += bCopy.GetFast(x + xoff[i], y + yoff[i]);
      } else {
        for (i = 0; i < DIRS2; i++) {
          xT = x + xoff[i]; yT = y + yoff[i];
          if (xT < 0)
            xT = m_x-1;
          else if (xT >= m_x)
            xT = 0;
          if (yT < 0)
            yT = m_y-1;
          else if (yT >= m_y)
            yT = 0;
          n += bCopy.Get(xT, yT);
        }
      }

      if (bCopy.Get(x, y)) {
        if (gs.grfLifeDie & (1 << n)) {
          // A cell dies.
          Set0(x, y);
          count++;
        }
      } else {
        if (gs.grfLifeBorn & (1 << n)) {
          // A new cell is born.
          Set1(x, y);
          count++;
        }
      }
    }
  return count;
}


// Parse a "Life" rulestring such as "23/3", where the first sequence is the
// numbers of neighbors allowed to survive, and the second is neighbors needed
// for a new cell to be born. Implements the SetLife operation.

flag FSetLife(CONST char *sz, int cch)
{
  int grfDie = 0x1FF, grfBorn = 0, grf;
  flag fBorn = fFalse;
  char ch;

  while (cch-- > 0) {
    ch = *sz++;
    if (ch == '/')
      fBorn = fTrue;
    else if (!FBetween(ch, '0', '8'))
      return fFalse;
    else {
      grf = 1 << (ch - '0');
      if (fBorn)
        grfBorn |= grf;
      else
        grfDie &= ~grf;
    }
  }
  gs.grfLifeDie  = grfDie;
  gs.grfLifeBorn = grfBorn;
  return fTrue;
}


/*
******************************************************************************
** Bitmap File Routines
******************************************************************************
*/

// Read header information about a Windows bitmap from a file. Return its
// size, bits per pixel, and colors in the palette if any.

flag FReadBitmapHeader(FILE *file, flag fNoHeader,
  int *x, int *y, int *cf, int *ck)
{
  char ch, ch2;
  long l;
  int cbExtra, i;

  // BitmapFileHeader
  if (!fNoHeader) {
    ch = getbyte(); ch2 = getbyte();
    if (ch != 'B' || ch2 != 'M') {
      PrintSz_W("This file does not look like a Windows bitmap.\n");
      return fFalse;
    }
    skiplong();
    skipword(); skipword();
    skiplong();
  }
  // BitmapInfo / BitmapInfoHeader
  cbExtra = getlong();
  *x = NAbs((int)getlong()); *y = NAbs((int)getlong());
  skipword(); *cf = getword();
  l = getlong();
  if (l != 0/*BI_RGB*/ && l != 3/*BI_BITFIELDS*/) {
    PrintSz_W("This Windows bitmap file can't be uncompressed.\n");
    return fFalse;
  }
  for (i = 0; i < 3; i++) {
    skiplong();
  }
  *ck = getlong(); skiplong();
  for (cbExtra -= 40; cbExtra > 0; cbExtra--)
    skipbyte();
  if (l == 3/*BI_BITFIELDS*/)
    for (i = 0; i < 3; i++) {
      skiplong();
    }
  return fTrue;
}


// Load a monochrome bitmap from a Windows bitmap format file. It is assumed
// FReadBitmapHeader() has already been called.

flag CMon::FReadBitmapCore(FILE *file, int x, int y)
{
  dword il;
  char ch;

  // RgbQuad
  skiplong(); skiplong();
  // Data
  if (!FBitmapSizeSet(x, y))
    return fFalse;
  for (y = m_y-1; y >= 0; y--) {
    for (x = 0; x < m_x; x += 32) {
      il = _Il(x, y);
      *_Pl(il) = getlong();
    }
  }
  return fTrue;
}


// Save a monochrome bitmap to a Windows bitmap format file.

void CMon::WriteBitmap(FILE *file, KV kv0, KV kv1) CONST
{
  int x, y;
  dword dw;
  byte *pb = (byte *)&dw;

  // BitmapFileHeader
  putbyte('B'); putbyte('M');
  dw = 14+40 + 8 + 4*m_y*(((m_x-1) >> 5) + 1);
  putlong(dw);
  putword(0); putword(0);
  putlong(14+40 + 8);
  // BitmapInfo / BitmapInfoHeader
  putlong(40);
  putlong(m_x); putlong(m_y);
  putword(1); putword(1);
  putlong(0 /*BI_RGB*/); putlong(0);
  putlong(0); putlong(0);
  putlong(0); putlong(0);
  // RgbQuad
  putbyte(RgbB(kv0)); putbyte(RgbG(kv0)); putbyte(RgbR(kv0)); putbyte(0);
  putbyte(RgbB(kv1)); putbyte(RgbG(kv1)); putbyte(RgbR(kv1)); putbyte(0);
  // Data
  for (y = m_y-1; y >= 0; y--)
    for (x = 0; x < m_x; x += 32) {
      dw = _L(x, y);
      if (x + 32 > m_x) {
        SwapN(pb[0], pb[3]); SwapN(pb[1], pb[2]);
        dw &= ~((1 << (x + 32 - m_x)) - 1);
        SwapN(pb[0], pb[3]); SwapN(pb[1], pb[2]);
      }
      putlong(dw);
    }
}


// Load a plain text file, into a new monochrome bitmap. Each character maps
// to one pixel.

flag CMon::FReadText(FILE *file)
{
  int x, xmax = 0, y = 0;
  char ch;

  // Figure out the number of rows and the longest row in the file.
  loop {
    ch = getbyte();
    if (ch == EOF || ch == chNull)
      break;
    x = 0;
    loop {
      if (ch < ' ') {
        skiplf();
        break;
      }
      x++;
      ch = getbyte();
    }
    if (x > xmax)
      xmax = x;
    y++;
  }
  if (!FBitmapSizeSet(xmax, y))
    return fFalse;
  BitmapOff();
  fseek(file, 0, SEEK_SET);

  // Actually read in the content of the text file.
  y = 0;
  loop {
    ch = getbyte();
    if (ch == EOF || ch == chNull)
      break;
    x = 0;
    loop {
      if (ch < ' ') {
        skiplf();
        break;
      }
      if (ch >= chOn)
        Set1(x, y);
      x++;
      ch = getbyte();
    }
    y++;
  }
  return fTrue;
}


// Save a monochrome bitmap to a plain text file. Use one character per pixel.

void CMon::WriteText(FILE *file, flag fClip, flag fThin, flag fTab) CONST
{
  int x, y, x2 = m_x-1, grf;

  for (y = 0; y < m_y; y++) {
    if (fClip)
      for (x2 = m_x-1; x2 >= 0 && !Get(x2, y); x2--)
        ;
    for (x = 0; x <= x2; x++) {
      if (Get(x, y)) {
        if (fThin) {
          grf = Get(x, y-1) | Get(x-1, y) << 1 |
            Get(x, y+1) << 2 | Get(x+1, y) << 3;
          if (grf == 10)
            putbyte(chX);
          else if (grf == 5)
            putbyte(chY);
          else
            putbyte(chXY);
        } else
          putbyte(chOn);
      } else {
        if (!fClip || !fTab)
          putbyte(chOff);
      }
      if (fTab && x < x2)
        putbyte(chTab);
    }
    putbyte('\n');
  }
}


// Load an X11 bitmap from a file, into a new monochrome bitmap.

flag CMon::FReadXbm(FILE *file)
{
  char line[cchSzDef], ch;
  flag fSuper;
  int x, y, i, j;
  dword value;

  do {
    fscanf(file, "%s", S(line));
  } while (line[0] != '#');
  fscanf(file,   "%s%d",          S(line), &x);
  fscanf(file, "%s%s%d", S(line), S(line), &y);
  if (!FBitmapSizeSet(x, y))
    return fFalse;
  fscanf(file, "%s", S(line));
  fscanf(file, "%s", S(line));
  fSuper = FEqSzI(line, "short");
  do {
    fscanf(file, "%s", S(line));
  } while (line[0] != '{');
  for (y = 0; y < m_y; y++) {
    x = 0;
    while (x < m_x) {
      while ((ch = getbyte()) != 'x')
        ;
      ch = getbyte(); value = NHex(ch) << 4;
      ch = getbyte(); value += NHex(ch);
      if (fSuper) {
        ch = getbyte(); value = ((value << 4) + NHex(ch)) << 4;
        ch = getbyte(); value += NHex(ch);
      }
      j = fSuper ? 16 : 8;
      if (x + j > m_x)
        j = m_x - x;
      for (i = 0; i < j; i++) {
        Set(x + i, y, (bit)((value & 1) ^ 1));
        value = value >> 1;
      }
      x += (fSuper ? 16 : 8);
    }
  }
  while ((ch = getbyte()) >= ' ')
    ;
  return fTrue;
}


// Save a monochrome bitmap to an X11 bitmap text file.

void CMon::WriteXbm(FILE *file, CONST char *szName, char mode) CONST
{
  int x, y, i, temp = 0;
  dword value;
  char sz[cchSzDef], *pchStart, *pchEnd;

  sprintf(S(sz), "%s", szName);
  for (pchEnd = sz; *pchEnd != chNull; pchEnd++)
    ;
  for (pchStart = pchEnd; pchStart > sz && *pchStart != '\\'; pchStart--)
    ;
  if (*pchStart == '\\')
    pchStart++;
  for (pchEnd = pchStart; *pchEnd != chNull && *pchEnd != '.'; pchEnd++)
    ;
  *pchEnd = chNull;
  fprintf(file, "#define %s_width %d\n" , pchStart, m_x);
  fprintf(file, "#define %s_height %d\n", pchStart, m_y);
  fprintf(file, "static %s %s_bits[] = {",
    mode != 'S' ? "char" : "short", pchStart);

  for (y = 0; y < m_y; y++) {
    x = 0;
    do {
      if (y + x > 0)
        fprintf(file, ",");
      if (temp == 0)
        fprintf(file, "\n%s", mode == 'N' ? "  " : (mode == 'C' ? " " : ""));
      value = 0;
      for (i = (mode != 'S' ? 7 : 15); i >= 0; i--)
        value = (value << 1) + (!Get(x + i, y) && x + i < m_x);
      if (mode == 'N')
        putbyte(' ');
      fprintf(file, "0x");
      if (mode == 'S')
        fprintf(file, "%c%c",
          ChHex(value >> 12), ChHex((value >> 8) & 15));
      fprintf(file, "%c%c",
        ChHex((value >> 4) & 15), ChHex(value & 15));
      temp++;
      if ((mode == 'N' && temp >= 12) || (mode == 'C' && temp >= 15) ||
        (mode == 'S' && temp >= 11))
        temp = 0;
      x += (mode != 'S' ? 8 : 16);
    } while (x < m_x);
  }
  fprintf(file, "};\n");
}


// Load a Daedalus monochrome bitmap from a file, into a new bitmap.

flag CMon::FReadDaedalusBitmapCore(FILE *file, int x, int y)
{
  int xT, nCur;
  char ch;

  if (!FBitmapSizeSet(x, y))
    return fFalse;
  BitmapOff();
  skipcrlf();
  for (y = 0; y < m_y; y++) {
    x = 0;
    loop {
      ch = getbyte();
      if (ch < ' ') {
        skiplf();
        break;
      }
      if (ch < 'a') {
        // This character encodes 6 pixels.
        nCur = (int)(ch - '!');
        for (xT = 5; xT >= 0; xT--) {
          if (FOdd(nCur))
            Set1(x + xT, y);
          nCur >>= 1;
        }
        x += 6;
      } else {
        // Check for the duplicate row marker '|'.
        if (ch == '|') {
          Assert(x == 0 && y > 0);
          for (nCur = 0, ch = getbyte(); FDigitCh(ch); ch = getbyte())
            nCur = nCur * 10 + (ch - '0');
          loop {
            BlockMove(*this, 0, y-1, m_x-1, y-1, 0, y);
            if (--nCur <= 0)
              break;
            y++;
          }
          skipcrlf();
          break;
        }
        // This character contains some multiple of 6 on or off pixels.
        if (ch >= 'n') {
          LineX(x, x + (ch - 'n' + 2) * 6 - 1, y, fOn);
          x += (ch - 'n' + 2) * 6;
        } else
          x += (ch - 'a' + 2) * 6;
      }
    }
  }
  return fTrue;
}


// Save a bitmap to a Daedalus monochrome bitmap custom format file.

void CMon::WriteDaedalusBitmap(FILE *file, flag fClip) CONST
{
  int x, y, xT, xmax, nCur, nSav, cn, cRow = 0;
  flag fBlank;
  byte bT;

  fprintf(file, "DB\n%d %d 1\n\n", m_x, m_y);
  for (y = 0; y <= m_y; y++) {

    // Check if current row same as previous. If so write '|' dup marker.
    if (y > 0) {
      if (y < m_y) {
        for (x = 0; x < m_x; x++)
          if (Get(x, y) != Get(x, y-1))
            goto LNormal;
        cRow++;
        continue;
      }
LNormal:
      // Output number of times row was duplicated
      if (cRow > 0) {
        if (!fBlank || cRow > 1) {
          putbyte('|');
          if (cRow > 1)
            fprintf(file, "%d", cRow);
        }
        putbyte('\n');
        cRow = 0;
      }
    }
    if (y >= m_y)
      continue;

    // Output a unique row.
    xmax = m_x - 1;
    if (fClip)
      while (xmax >= 0 && !Get(xmax, y))
        xmax--;
    fBlank = (xmax < 0);
    cn = 0;
    for (x = 0; x <= xmax + 6; x += 6) {
      // Compute a number encoding the next 6 pixels in this row.
      nCur = 0;
      for (xT = 0; xT < 6; xT++)
        nCur = (nCur << 1) + (x + xT < m_x && Get(x + xT, y));
      // Check for whether ready to write next character/run to file.
      if (x > 0 && (nCur != nSav ||
        (nCur != 0 && nCur != 63) || cn >= 14 || x > xmax)) {
        if (cn <= 1)
          putbyte('!' + nSav);
        else {
          Assert(nSav == 0 || nSav == 63);
          bT = nSav == 0 ? 'a' - 2 + cn : 'n' - 2 + cn;
          putbyte(bT);
        }
        cn = 0;
      }
      nSav = nCur;
      cn++;
    }
    putbyte('\n');
  }
}

/* graphics.cpp */
