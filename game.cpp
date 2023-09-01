/*
** Daedalus (Version 3.4) File: game.cpp
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
** This file contains implementations of operations used by games that
** come with Daedalus.
**
** Created: 11/6/2018.
** Last code change: 8/29/2023.
*/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
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
** Game Routines
******************************************************************************
*/

#define zVis 16   // Visibility distance
#define dobj 17   // Ammo to launcher offset
#define zClimb 64 // Vertical step units

enum _bitmapcustom {
  bmpWall  = 0,  // Block side texture bitmap
  bmpVar   = 1,  // Wall height bitmap
  bmpElev  = 2,  // Floor elevation bitmap
  bmpFloor = 3,  // Floor texture bitmap
  bmpBlock = 4,  // Block top texture bitmap
  bmpSide  = 5,  // Floor side texture bitmap
  bmpLit   = 6,  // Fog lit location bitmap
  bmpInv   = 7,  // Inventory bitmap
  bmpAlly  = 8,  // Faction bitmap
  bmpH2O   = 9,  // Water location bitmap
  bmpWasp  = 10, // Wasp nest bitmap
  bmpTrap  = 11, // Net trap bitmap
  bmpFire  = 12, // Firewall side texture
  bmpFire2 = 13, // Firewall top texture
};

enum _variablecustom {
  numTurn  = 16,   // @p: Turn count
  numLeft  = 19,   // @s: Tribues left
  numStart = 37,   // @SK: Start turn
  numEarly = 63,   // @Sk: Early death chance
  numRng   = 70,   // @Sr: Throwable weapon range distance
  numFinal = 78,   // @Sz: Firewall finale start turn
  numPeace = 80,   // @SPeace: If tributes keep no enemies
  numAlly  = 83,   // Alliance proposal probability
  numTrap  = 84,   // Stepping into trap probability
  numMuttS = 91,   // Whether mutts can swim
  numMuttC = 92,   // Whether mutts can climb
  numTube  = 118,  // Launch tube color
  numCorn  = 119,  // Cornucopia color
  numMaze  = 120,  // Maze color
  numDists = 152,  // Max Districts possible
  numDist  = 153,  // Districts in play
  numPer   = 154,  // Tributes per District
  numLaun  = 164,  // Launch tube radius
  numFMax  = 165,  // Maximum food allowed
  numHPMax = 166,  // Maximum health allowed
  numInv   = 167,  // Inventory size
  numDmg   = 200,  // Weapon damage
  numUsage = 250,  // Weapon usage
  numFave  = 300,  // Weapon favored district
  numMater = 350,  // Item material
  numCoor  = 900,  // Tribute X & Y coordinates
  numElev  = 1200, // Tribute Z coordinate
  numHP    = 1500, // Tribute hitpoints
  numFood  = 1800, // Tribute food
  numKill  = 2100, // Tribute kills
  numWield = 2400, // Tribute wielded weapon index
  numWear  = 2700, // Tribute worn item index
  numNet   = 3000, // Tribute netting
  numDest  = 3300, // Tribute destination coordinates
  numWasp  = 3600, // Tribute hallucination
  numLead  = 3900, // Tribute leadership
};

enum _stringcustom {
  strX = 3, // X increments
  strY = 4, // Y increments
};

enum _action {
  actWait  = 0,  // Do nothing and wait
  actUse   = 1,  // Use item
  actGet   = 2,  // Pick up whatever standing over
  actMove  = 3,  // Move or melee attack in direction
  actThrow = 4,  // Throw item in direction
  actDrop  = 5,  // Drop item
  actDown  = 6,  // Climb down
  actUp    = 7,  // Climb up
  actGive  = 8,  // Give item in direction
  actDig   = 9,  // Dig
  actFire  = 10, // Fire weapon in direction
  actFree  = 11, // Free tribute from net
  actAlly  = 12, // Propose alliance
  actLead  = 13, // Claim leadership
};

enum _object {
  objFist  = 0,  // Fist weapon (empty slot)
  objRock  = 1,  // Rock ammo
  objDart  = 2,  // Dart ammo
  objArrow = 3,  // Arrow ammo
  objBull  = 4,  // Gun ammo
  objGren  = 5,  // Grenade explosive
  objNet   = 6,  // Net weapon
  objClub  = 7,  // Club weapon
  objTorch = 8,  // Blowtorch weapon
  objBrick = 9,  // Brick weapon
  objPick  = 10, // Pick weapon
  objTeeth = 11, // Mutt teeth
  objKnife = 12, // Knife weapon
  objMace  = 13, // Mace weapon
  objSpear = 14, // Spear weapon
  objAxe   = 15, // Axe weapon
  objSword = 16, // Sword weapon
  objTrid  = 17, // Trident weapon
  objSling = 18, // Slingshot launcher
  objBlow  = 19, // Blowgun launcher
  objBow   = 20, // Bow launcher
  objGun   = 21, // Gun launcher
  objBerry = 22, // Berries
  objNight = 23, // Nightlock berries
  objBread = 24, // Loaf of bread
  objFish  = 25, // Fish
  objHeal  = 26, // First-aid kit
  objGogg  = 27, // Nightvision goggles
  objHelm  = 28, // Helmet
  objArmor = 29, // Body armor
  objPara  = 30, // Parachute
  objMine0 = 31, // Landmine (inactive)
  objMine1 = 32, // Landmine (active)
  objMax   = 33, // Number of objects
};

enum _desire {
  desPunch   = 0,
  desUp      = 1,
  desDown    = 9999,
  desStart   = 10000,
  desChase   = 10000,
  desBetray  = 30000 + 0,
  desLead    = 30000 + 80,
  desAlly    = 30000 + 90,
  desGive    = 30000 + 100,
  desDrop    = 30000 + 110,
  desDig     = 30000 + 120,
  desMine    = 30000 + 130,
  desWear    = 30000 + 140,
  desEat     = 30000 + 150,
  desHeal    = 30000 + 160,
  desWield   = 30000 + 170,
  desGet     = 30000 + 180,
  desFree    = 30000 + 190,
  desMelee   = 30000 + 500,
  desThrow   = 30000 + 500,
  desFlee    = 40000,
  desGiveHi  = 60000 + 940,
  desGetHi   = 60000 + 950,
  desWieldHi = 60000 + 960,
  desFleeHi  = 60000 + 970,
  desHealHi  = 60000 + 980,
  desEatHi   = 60000 + 990,
};

#define Xoff(d) (ws.rgszVar[strX][d] - '1')
#define Yoff(d) (ws.rgszVar[strY][d] - '1')
#define ZElev(n) ws.rglVar[numElev + (n)]
#define NWater(x, y) \
  (ws.rgbMask[bmpH2O].Get(x, y) ? RgbB(bm.k2.Get(x, y)) : 0)
#define FFirewall(x, y) (ws.rgcTexture[bmpBlock].Get(x, y) == bmpFire2)
#define NDist(i) (((i) >= nTribMax ? nDistMax : (i) / nPer) + 1)
#define NTrib(k) (255 - (k))
#define NObj(k) (RgbG(k) & 127)
#define NWield(i) ws.rglVar[numWield + (i)]
#define FObjInInv(o) ((grfObjInv & (1 << (o))) != 0)
#define FObjsInInv(o1, o2) ((grfObjInv & ((1 << (o1)) | (1 << (o2)))) != 0)
#define ObjInvN(i, n) ((i) < 0 ? 0 : ws.rgcTexture[bmpInv].Get(n, i))
#define ObjInv(i) ObjInvN(i, nSelf)
#define WepDmgN(o, n) \
  (ws.rglVar[numDmg + (o)] + (ws.rglVar[numFave + (o)] == NDist(n))*2)
#define WepDmg(o) WepDmgN(o, nSelf)
#define DmgN(n) (ws.rglVar[numNet + (n)] > 0 ? 1 : \
  WepDmgN(ObjInvN(NWield(n), n), n))
#define FactionN(i, n) ws.rgcTexture[bmpAlly].Get(n, i)
#define Faction(i) FactionN(i, nSelf)
#define FAlly(n) ((n) <= 100)
#define FEnemy(n) ((n) > 150)
#define FWep(o)       ((o) != objFist && (ws.rglVar[numUsage + (o)] > 0))
#define FWepMelee(o)  ((o) != objFist && (ws.rglVar[numUsage + (o)] & 9) == 1)
#define FWepThrow(o)  ((o) != objFist && (ws.rglVar[numUsage + (o)] & 2) > 0)
#define FWepAmmo(o)   ((o) != objFist && (ws.rglVar[numUsage + (o)] & 4) > 0)
#define FWepLaunch(o) ((o) != objFist && (ws.rglVar[numUsage + (o)] & 8) > 0)
#define FWepAuto(o)   ((ws.rglVar[numUsage + (o)] & 24) == 24)
#define FConsum(o) \
  ((o) == objBerry || (o) == objBread || (o) == objFish || (o) == objHeal)
#define FFisher(o) ((o) == objNet || (o) == objSpear || (o) == objTrid)
#define PowerN(n) (ws.rglVar[numHP + (n)] * DmgN(n))
#define NCompare(n) (nPower * 100 / PowerN(n))
#define CheckWrap(x, y) if (dr.nEdge == nEdgeTorus) bm.b.Legalize2(x, y)
#define Desire(a, i, d, p) if ((p) > nPriority) \
  { nAction = (a); nItem = (i); nDir = (d); nPriority = (p); }
#define CheckBad(f, n) if (f) { ws.rglVar[iLetterZ] = -(n); return fFalse; }

// Determine the best move for a computer controlled Hunger Games tribute.
// Contains AI specialized to the Hunger Games script, and implements the
// Hunger operation. Loop over all possible actions, assigning a value to
// each, then select whichever potential action has the highest value.

flag FHungerGame(int nInput)
{
  CMaz b;
  int nAction = actWait, nItem = 0, nDir = 0, nPriority = -1,
    rgnMove[DIRS2], rgnDesObj[objMax], rgnConObj[objMax], rgnValObj[objMax],
    rgnDesDir[DIRS2], nSelf, zLit, nDistMax, nDist, nPer, nTribMax, nTribLeft,
    nTurn, nTurnMin, iDist, xSelf, ySelf, zSelf, zElev, xDest, yDest, zRng,
    grfObjInv = 0, iWield, objWield, iWear, objWear, objHere, nHealth,
    nHealthMax, nFood, nFoodMax, nInv, nPropose, nTrap, iItem, nEarly, nMove,
    nDes, nDmg, nDmgMax, nDmgWield, nHealthEnemy = 0, nDamageEnemy = 0,
    nPower, nPowerEnemy, iItemMax, cInvFree = 0, cConsum = 0, cAlly = 0,
    dGive = -1, nGive = 255, nLead = -1, obj, x, y, d, x2, y2, d2, i, j, l;
  flag fMutt, fNet, fWater, fCanClimb = fFalse, fCanFish, fCanSwim, fCanSee,
    fAllDeep = fTrue, fNearFire, fMelee = fFalse, fRanged = fFalse, fSafe;

  // Ensure valid game space
  Assert(objSling - objRock == dobj && objBlow - objDart == dobj &&
    objBow - objArrow == dobj && objGun - objBull == dobj);
  if (bm.k.FNull() || bm.k2.FNull() || bm.k3.FNull() ||
    ws.cbMask <= bmpH2O || ws.ccTexture <= bmpH2O ||
    ws.rgcTexture[bmpInv].FNull() || ws.rgcTexture[bmpAlly].FNull())
    return fFalse;
  if (!FEnsureLVar(numWasp) || !FEnsureSzVar(strY) ||
    ws.rgszVar[strX] == NULL || ws.rgszVar[strY] == NULL ||
    CchSz(ws.rgszVar[strX]) != 8 || CchSz(ws.rgszVar[strY]) != 8)
    return fFalse;
  i = (zVis << 1) + 1;
  if (!b.FAllocate(i, i, NULL))
    return fFalse;

  // Special case for firewall generation
  if (nInput == -1) {
    for (y = 0; y < bm.k.m_x; y++)
      for (x = 0; x < bm.k.m_x; x++) {
        if (!bm.b.Get(x, y))
          continue;
        bm.b3.Set1(x, y);
        bm.k.Set(x, y, Rgb(Rnd(255, 255-32), Rnd(127+32, 127-32), 0));
        ws.rgcTexture[bmpWall].Set(x, y, UD(bmpFire, bmpFire));
        ws.rgcTexture[bmpBlock].Set(x, y, bmpFire2);
        i = j = ws.rgcTexture[bmpElev].Get(x, y);
        if (bm.b2.Get(x, y))
          j = UdU(ws.rgcTexture[bmpVar].Get(x, y));
        ws.rgcTexture[bmpVar].Set(x, y,
          UD(Min(j + Rnd(zClimb, zClimb << 1), 4095), i));
        bm.b.Set0(x, y);
        bm.b2.Set0(x, y);
        ws.rgbMask[bmpLit].Set1(x, y);
      }
    return fTrue;
  }

  // Setup initial variables
  nSelf = UdU(nInput);
  nDistMax = ws.rglVar[numDists];
  nDist = ws.rglVar[numDist];
  nPer = ws.rglVar[numPer];
  nTribMax = nDist * nPer;
  nTribLeft = ws.rglVar[numLeft];
  nTurn = ws.rglVar[numTurn];
  nTurnMin = ws.rglVar[numStart];
  nHealthMax = ws.rglVar[numHPMax];
  nFoodMax = ws.rglVar[numFMax];
  nInv = ws.rglVar[numInv];
  zRng = ws.rglVar[numRng];
  nPropose = ws.rglVar[numAlly];
  nTrap = ws.rglVar[numTrap];
  iDist = NDist(nSelf);
  fMutt = iDist > nDist;
  l = ws.rglVar[numCoor + nSelf];
  xSelf = UdU(l); ySelf = UdD(l);
  zSelf = ZElev(nSelf);
  zElev = ws.rgcTexture[bmpElev].Get(xSelf, ySelf);
  nEarly = ws.rglVar[numLaun];
  nHealth = ws.rglVar[numHP + nSelf];
  nFood = ws.rglVar[numFood + nSelf];
  fNet = ws.rglVar[numNet + nSelf] > 0;
  fWater = NWater(xSelf, ySelf) > 0;
  iWield = NWield(nSelf);
  objWield = ObjInv(iWield);
  nDmgWield = WepDmg(objWield);
  fCanFish = iDist == 4 || FFisher(objWield);
  fCanSwim = iDist == 4 || objWield == objClub ||
    (fMutt && ws.rglVar[numMuttS]);
  fCanSee = fMutt || iDist == 13;
  iWear = ws.rglVar[numWear + nSelf];
  objWear = ObjInv(iWear);
  l = bm.k3.Get(xSelf, ySelf);
  objHere = NObj(l);
  for (iItem = 0; iItem < nInv; iItem++) {
    obj = ObjInv(iItem);
    grfObjInv |= (1 << obj);
    cInvFree += (obj == objFist);
    cConsum += FConsum(obj);
  }
  zLit = ws.rglVar[numWasp + nSelf] ? 1 : (nInput < 4096 || fCanSee ||
    objWear == objGogg ? zVis : NMin(zVis, UdD(nInput)));
  if (FWepLaunch(objWield) &&
    (!(FObjInInv(objWield-dobj) || FWepAuto(objWield)) || fNet))
    nDmgWield = 1;
  nPower = nHealth * nDmgWield;
  l = ws.rglVar[numDest + nSelf];
  xDest = UdU(l); yDest = UdD(l);
  for (i = 0; i < nTribMax; i++)
    cAlly += ws.rglVar[numHP + i] > 0 && FAlly(Faction(i));
  fNearFire = nTurn == ws.rglVar[numFinal] &&
    (!FBetween(xSelf, 2, bm.b.m_x-3) || !FBetween(ySelf, 2, bm.b.m_y-3));

  // Corruption checks
  j = 0;
  for (i = 0; i < nTribMax; i++) {
    if (ws.rglVar[numHP + i] <= 0)
      continue;
    j++;
    l = ws.rglVar[numCoor + i];
    x = UdU(l); y = UdD(l);
    l = bm.k3.Get(x, y);
    l = RgbR(l);
    CheckBad(l == 0, 2000 + i);
    l = NTrib(l);
    CheckBad(l != i, 3000 + i);
  }
  CheckBad(j != nTribLeft, 4000 + j);

  // Setup what's next to us
  for (d = 0; d < DIRS2; d++) {
    x = xSelf + xoff[d];
    y = ySelf + yoff[d];
    CheckWrap(&x, &y);
    if (d < DIRS && !fCanClimb && (!fMutt || ws.rglVar[numMuttC]) &&
      ((bm.b.Get(x, y) && !RgbR(bm.b3.Get(x, y)) &&
      UdU(ws.rgcTexture[bmpVar].Get(x, y)) >= zSelf + zClimb*3/2) ||
      ws.rgcTexture[bmpElev].Get(x, y) >= zSelf + zClimb*3/2))
      fCanClimb = fTrue;
    i = NWater(x, y);
    if (!(i > 0 && i <= 170))
      fAllDeep = fFalse;
  }
  for (d = 0; d < DIRS2; d++) {
    rgnMove[d] = -2;
    x = xSelf + Xoff(d);
    y = ySelf + Yoff(d);
    CheckWrap(&x, &y);
    if (!bm.k.FLegal(x, y))
      continue;
    l = bm.k3.Get(x, y);
    obj = NObj(l);
    l = RgbR(l);

    // Can't step into solid objects (unless can chop, dig, melt, or burn it)
    if (bm.b.Get(x, y) && l == 0) {
      j = bm.k.Get(x, y);
      if (!((iDist == 7 && objWield == objAxe && RgbG(j) > RgbR(j) &&
          (ws.rgcTexture[bmpBlock].Get(x, y) != bmpWasp ||
          zSelf >= ws.rgcTexture[bmpElev].Get(x, y) + zClimb*2)) ||
        (iDist == 2 && objWield == objPick &&
          (RgbG(j) == RgbB(j) || j == ws.rglVar[numMaze])) ||
        (iDist == 13 && objWield == objTorch &&
          (j == ws.rglVar[numCorn] || RgbG(j) > RgbR(j)))))
        continue;
    } else
      fNearFire |= FFirewall(x, y);
    // Shouldn't step onto activated landmine
    if (obj == objMine1 && zSelf <= zElev && !fMutt &&
      (nTrap == 0 || Rnd(0, 99) >= nTrap))
      continue;
    // Shouldn't step onto wasp nest or Net trap
    i = ws.rgcTexture[bmpFloor].Get(x, y);
    if ((i == bmpWasp || (!fMutt && i == bmpTrap)) && zSelf <= zElev &&
      (nTrap == 0 || Rnd(0, 99) >= nTrap))
      continue;
    // Never step into deep water if can't swim
    i = NWater(x, y);
    if (i > 0 && i <= 170 && !fCanSwim && l == 0)
      continue;
    // Never step into space so fall and take damage
    if (iDist != 11 && objWear != objPara &&
      zSelf >= ws.rgcTexture[bmpElev].Get(x, y) + zClimb*2) {
      for (d2 = 0; d2 < DIRS; d2++) {
        x2 = x + xoff[d2];
        y2 = y + yoff[d2];
        CheckWrap(&x2, &y2);
        if ((bm.b.Get(x2, y2) && !RgbR(bm.k3.Get(x2, y2)) &&
          UdU(ws.rgcTexture[bmpVar].Get(x2, y2)) >= zSelf + zClimb/2) ||
          ws.rgcTexture[bmpElev].Get(x2, y2) >= zSelf)
          break;
      }
      if (d2 >= DIRS)
        continue;
    }
    // Never step next to firewall because it may expand over you
    for (d2 = 0; d2 < DIRS; d2++) {
      x2 = x + xoff[d2];
      y2 = y + yoff[d2];
      CheckWrap(&x2, &y2);
      if (FFirewall(x2, y2))
        break;
    }
    if (d2 < DIRS)
      continue;

    if (l == 0) {
      // Shouldn't move next to launch platforms before the opening gong
      if (nTurn < 0) {
        if (fMutt)
          continue;
        for (d2 = 0; d2 < DIRS2; d2++) {
          x2 = x + xoff[d2];
          y2 = y + yoff[d2];
          CheckWrap(&x2, &y2);
          if (bm.k2.Get(x2, y2) == ws.rglVar[numTube])
            break;
        }
        if (d2 < DIRS2) {
          l = bm.k2.Get(x, y);
          if ((l | 0x000f1f) != kvBrown &&
            Rnd(1, ws.rglVar[numEarly] * DIRS2 * nTurnMin) > 1)
            continue;
        }
      }
      rgnMove[d] = -1;
    } else {
      l = NTrib(l);
      i = Faction(l);
      rgnMove[d] = i;
      if (!FAlly(i)) {
        fMelee = fTrue;
        nHealthEnemy += ws.rglVar[numHP + l];
        nDamageEnemy += DmgN(l);
      }
    }
  }
  nPowerEnemy = nHealthEnemy * nDamageEnemy;

  // Setup how interested in objects
  for (obj = 0; obj < objMax; obj++) {
    nDes = 0;
    nDmg = WepDmg(obj);
    if (FWep(obj)) {
      // Don't want non-throwable ammo unless already have launcher
      // Don't want melee only weapon worse than melee weapon wielded
      if (FWepLaunch(obj))
        nDes = FObjInInv(obj-dobj) || FWepAuto(obj) ? nDmg : 1;
      else if (FWepAmmo(obj))
        nDes = FObjInInv(obj+dobj) ? WepDmg(obj+dobj) : nDmg * FWepThrow(obj);
      else if (FWepThrow(obj) || WepDmg(obj) >= nDmgWield)
        nDes = nDmg;
    } else {
      // Never want Nightlock or active Landmine
      switch (obj) {
      case objFist:  nDes = 0; break;
      case objBerry: nDes = nFood < nFoodMax ? 6 : nDmg; break;
      case objNight: nDes = 0; break;
      case objBread: nDes = nFood < nFoodMax ? 7 : 4; break;
      case objFish:  nDes = nFood < nFoodMax ? 6 : nDmg; break;
      case objHeal:  nDes = nHealth < nHealthMax ? 9 : nDmg; break;
      case objGogg:  nDes = ds.fStar ? 8 : nDmg; break;
      case objHelm:  nDes = !ds.fStar ? 8 : nDmg; break;
      case objArmor: nDes = !ds.fStar ? 9 : nDmg; break;
      case objPara:  nDes = nDmg; break;
      case objMine0: nDes = (iDist == 3) ? 5 : nDmg; break;
      case objMine1: nDes = 0; break;
      default: nDes = nDmg; break;
      }
      // Special: Ignore food if very full, and First-Aid if limit low
      if ((nFood >= 2000 && (obj == objBerry || obj == objBread ||
        obj == objFish)) || (nHealthMax < 2 && obj == objHeal))
        nDes = 0;
    }
    if (fMutt)
      nDes = 0;
    rgnValObj[obj] = nDes;
    // No duplicate launcher or duplicate melee only weapon
    if ((FWepLaunch(obj) || (FWepMelee(obj) && !FWepThrow(obj))) &&
      FObjInInv(obj))
      nDes = 0;
    // No melee only weapon equal to melee only weapon already wielded
    if (FWepMelee(obj) && !FWepThrow(obj) && FWepMelee(objWield) &&
      !FWepThrow(objWield) && WepDmg(obj) <= nDmgWield)
      nDes = 0;
    // No duplicate or extraneous nightvision goggles, armor, or parachute
    if ((obj == objGogg && (fCanSee || FObjInInv(objGogg))) ||
      (obj == objHelm && FObjsInInv(objHelm, objArmor)) ||
      (obj == objArmor && FObjInInv(objArmor)) || (obj == objPara &&
      (FObjsInInv(objPara, objGogg) || FObjsInInv(objHelm, objArmor))))
      nDes = 0;
    rgnConObj[obj] = nDes;
    /* Save last inventory slot for consumables, and don't want anything at
      all if inventory full (unless need First-Aid). */
    if ((cInvFree <= 0 && !(obj == objHeal && nHealth < nHealthMax)) ||
      (cInvFree <= 1 && cConsum <= 0 && !FConsum(obj)))
      nDes = 0;
    rgnDesObj[obj] = nDes;
  }

  // Look around and setup how interested in directions
  b.BitmapOff();
  for (y = -zVis; y <= zVis; y++)
    for (x = -zVis; x <= zVis; x++) {
      x2 = xSelf + x; y2 = ySelf + y;
      CheckWrap(&x2, &y2);
      if (bm.b.Get(x2, y2) && !RgbR(bm.k3.Get(x2, y2)))
        b.Set(x+zVis, y+zVis, fOn);
      else {
        i = NWater(x2, y2);
        if (i > 0 && i <= 170 && !fCanSwim)
          b.Set(x+zVis, y+zVis, fOn);
      }
    }
  b.BitmapSeen(zVis, zVis, zVis);
  ClearPb(rgnDesDir, DIRS2 * sizeof(int));
  for (y = -zLit; y <= zLit; y++)
    for (x = -zLit; x <= zLit; x++) {
      if (b.Get(x+zVis, y+zVis))
        continue;
      x2 = xSelf + x; y2 = ySelf + y;
      CheckWrap(&x2, &y2);
      nDes = 0;
      l = bm.k3.Get(x2, y2);
      obj = NObj(l);
      if (obj > 0) {
        // Ignore objects others are standing on top of
        if (RgbR(bm.k3.Get(x2, y2)))
          obj = 0;
        // Ignore objects in deep water if can't swim
        i = NWater(x2, y2);
        if (i > 0 && i <= 170 && !fCanSwim)
          obj = 0;
        // Ignore Fish if can't catch them
        if (obj == objFish && i > 0 && !fCanFish)
          obj = 0;
      }
      if (obj > 0) {
        i = rgnDesObj[obj];
        if (i > 0)
          nDes = i + (zVis - NMax(NAbs(x), NAbs(y))) * 10;
      } else {
        l = RgbR(l);
        if (l != 0) {
          l = NTrib(l);
          if (fMutt)
            nDes = (l < nTribMax)*3; // Mutts seek out all non-Mutts
          else if (FAlly(Faction(l))) {
            x2 = ws.rglVar[numLead + nSelf]; y2 = ws.rglVar[numLead + l];
            nDes = 1 + (y2 > x2 || (y2 == x2 && l < nSelf))*2; // Seek allies
          } else {
            CheckBad(ws.rglVar[numHP + l] <= 0, 1000 + l);
            i = NCompare(l);
            if (i > 100)
              nDes = (i-1) / 100 + 3; // Seek out weaker enemies
          }
        }
      }
      if (nDes > 0)
        for (d = 0; d < DIRS2; d++) {
          x2 = Xoff(d);
          y2 = Yoff(d);
          if (NMax(NAbs(x - x2), NAbs(y - y2)) < NMax(NAbs(x), NAbs(y)))
            rgnDesDir[d] = NMax(rgnDesDir[d], nDes);
        }
    }

  // Seek out waypoint coordinates
  if (nTurn > nEarly) {
    // Don't if follower in alliance, i.e. if higher leadership tribute nearby
    nLead = nSelf;
    x2 = ws.rglVar[numLead + nLead];
    for (i = 0; i < nTribMax; i++) {
      if (i == nSelf)
        continue;
      if (ws.rglVar[numHP + i] > 0 && FAlly(Faction(i))) {
        l = ws.rglVar[numCoor + i];
        x = UdU(l); y = UdD(l);
        if (NMax(NDistance(xSelf, x, bm.b.m_x),
          NDistance(ySelf, y, bm.b.m_y)) <= zRng) {
          y2 = ws.rglVar[numLead + i];
          if (y2 > x2 || (y2 == x2 && i < nLead)) {
            nLead = i;
            x2 = y2;
          }
        }
      }
    }
    // Move directions toward waypoint coordinates are given extra desire.
    if (nLead == nSelf || fMutt) {
      i = 0;
      for (d = 0; d < DIRS2; d++) {
        if (rgnMove[d] <= -2)
          continue;
        x = xSelf + Xoff(d);
        y = ySelf + Yoff(d);
        if (NMax(NDistance(xDest, x, bm.b.m_x), NDistance(yDest, y,
          bm.b.m_y)) < NMax(NDistance(xDest, xSelf, bm.b.m_x),
          NDistance(yDest, ySelf, bm.b.m_y))) {
          rgnDesDir[d] += Rnd(1, 2);
          i++;
        }
      }
      // If can't make progress, indicate want new destination
      if (i < 1)
        ws.rglVar[numDest + nSelf] = UD(xSelf, ySelf);
    }
  }

  // Move or melee attack
  for (d = 0; d < DIRS2; d++) {
    nMove = rgnMove[d];
    if (nMove <= -2)
      continue;
    x = xSelf + Xoff(d);
    y = ySelf + Yoff(d);
    // Consider move to an adjacent space
    if (nMove == -1) {
      nDes = rgnDesDir[d]*10 + Rnd(1, 9);
      // Approach or flee cornucopia at start of game
      if (nTurn <= nEarly &&
        NMax(NAbs(xDest - x), NAbs(yDest - y)) <
        NMax(NAbs(xDest - xSelf), NAbs(yDest - ySelf)))
        nDes += desStart;
      // Flee firewall
      if (fNearFire) {
        for (d2 = 0; d2 < DIRS; d2++) {
          x2 = x + xoff[d2];
          y2 = y + yoff[d2];
          CheckWrap(&x2, &y2);
          if (FFirewall(x2, y2) ||
            !FBetween(x2, 1, bm.b.m_x-2) || !FBetween(y2, 1, bm.b.m_y-2))
            break;
        }
        if (d2 >= DIRS)
          nDes += desFleeHi;
      // Flee stronger non-allies
      } else if (fMelee && !fMutt && nPower * 100 / nPowerEnemy <= 50) {
        nHealthEnemy = nDamageEnemy = 0;
        for (d2 = 0; d2 < DIRS2; d2++) {
          x2 = x + Xoff(d2);
          y2 = y + Yoff(d2);
          CheckWrap(&x2, &y2);
          l = RgbR(bm.k3.Get(x2, y2));
          if (l != 0) {
            l = NTrib(l);
            if (!FAlly(Faction(l))) {
              nHealthEnemy += ws.rglVar[numHP + l];
              nDamageEnemy += DmgN(l);
            }
          }
        }
        if (nHealthEnemy * nDamageEnemy <= 0 ||
          nPower * 100 / (nHealthEnemy * nDamageEnemy) >= 200)
          nDes += desFlee;
      // Chase weaker non-allies if armed
      } else if (!fMelee && nDmgWield > 1 && objWield > 0) {
        i = 0;
        for (d2 = 0; d2 < DIRS2; d2++) {
          x2 = x + Xoff(d2);
          y2 = y + Yoff(d2);
          CheckWrap(&x2, &y2);
          l = RgbR(bm.k3.Get(x2, y2));
          if (l != 0) {
            l = NTrib(l);
            if (!FAlly(Faction(l)) && (fMutt || NCompare(l) >= 200))
              i = desChase;
          }
        }
        nDes += i;
      }
    // Consider attacking an adjacent tribute
    } else {
      CheckWrap(&x, &y);
      l = RgbR(bm.k3.Get(x, y));
      // Consider giving item to allied or neutral tribute
      if (l != 0) {
        j = NTrib(l);
        if (nMove < nGive && NAbs(zSelf - ZElev(j)) <= zClimb) {
          i = 0;
          for (iItem = 0; iItem < nInv; iItem++) {
            obj = ObjInvN(iItem, j);
            i += (obj == objFist);
          }
          if (i >= 2 &&
            (FAlly(nMove) || (nTribLeft - cAlly > 1 && FAlly(nMove - 50)))) {
            nGive = nMove;
            dGive = d;
          }
        }
      }
      // Interact with allies (shouldn't attack them)
      if (FAlly(nMove)) {
        if (l != 0) {
          j = NTrib(l);
          // Free ally trapped in Net
          if (ws.rglVar[numNet + j])
            Desire(actFree, 0, d, desFree);
          // Declare leadership with respect to ally
          if ((nLead == nSelf || nLead == j) && !fMutt && Rnd(0, 99) <= 0)
            Desire(actLead, j, d, desLead);
          // Backstab non-close ally if can one-shot them
          if (ws.rglVar[numPeace] == 0 && nMove > 50 &&
            ws.rglVar[numHP + j] == 1 && nDmgWield > 1 &&
            !FWepLaunch(objWield) && ws.rglVar[numFave + objWield] == iDist)
            Desire(actMove, 0, d, desBetray + nMove);
        }
        continue;
      }
      // Can't attack more than one square above or below
      if (l != 0) {
        j = NTrib(l);
        if (NAbs(zSelf - ZElev(j)) > zClimb) {
          if (NCompare(j) > 100) {
            if (zSelf > ZElev(j)) {
              Desire(actDown, 0, 0, desUp + desChase);
            } else if (fCanClimb)
              Desire(actUp, 0, 0, desDown + desChase);
          }
          continue;
        }
      }
      nDes = desMelee + nMove;
      // Don't attack with fist unless no other option
      if (nDmgWield <= 1 || FWepLaunch(objWield))
        nDes = desPunch;
    }
    Desire(actMove, 0, d, nDes);
  }

  // Throw attack
  iItemMax = nDmgMax = -1;
  for (iItem = nInv-1; iItem >= 0; iItem--) {
    obj = ObjInv(iItem);
    // Look for ammoless launcher to fire (1st priority)
    nDmg = FWepAuto(obj) && objWield == obj ? WepDmg(obj) : -1;
    if (nDmg > nDmgMax) {
      iItemMax = iItem;
      nDmgMax = nDmg;
    }
    // Look for ammo to launch with wielded launcher (2nd priority)
    nDmg = FWepAmmo(obj) && objWield == obj+dobj ? WepDmg(obj+dobj) : -1;
    if (nDmg > nDmgMax) {
      iItemMax = iItem;
      nDmgMax = nDmg;
    }
    // Look for unwielded throwable weapon (3rd priority)
    nDmg = FWepThrow(obj) && (!FWepMelee(obj) || iItem != iWield) ?
      WepDmg(obj) : -1;
    if (nDmg > nDmgMax) {
      iItemMax = iItem;
      nDmgMax = nDmg;
    }
  }
  for (d = 0; d < DIRS2; d++) {
    // If best item thrown in each direction, what would be hit?
    x = xSelf; y = ySelf;
    for (j = 0; j < zRng; j++) {
      x += Xoff(d);
      y += Yoff(d);
      CheckWrap(&x, &y);
      if (!bm.k.FLegal(x, y))
        break;
      l = RgbR(bm.k3.Get(x, y));
      if (bm.b.Get(x, y) && l == 0)
        break;
      if (l == 0)
        continue;
      l = NTrib(l);
      i = Faction(l);
      if (FAlly(i)) // Don't ever attack allies
        break;
      if (!FEnemy(i) && j >= zVis) // Don't attack non-enemies in darkness
        break;
      fRanged = fTrue;
      if (iItemMax >= 0 && !fNet) {
        obj = ObjInv(iItemMax);
        if (!FWepAuto(obj)) {
          Desire(actThrow, iItemMax, d, desThrow + i);
        } else
          Desire(actFire, iItemMax, d, desThrow + i);
      }
      if (!FEnemy(i) && !fMutt && NDist(l) <= nDist && !fMelee && !fNet &&
        nTribLeft - cAlly > 1 && (nPropose > 0 && Rnd(1, 100) <= nPropose)) {
        i = NCompare(l);
        if (i >= 50 && i <= 200)
          Desire(actAlly, l, d, desAlly);
      }
    }
  }
  fSafe = !fMelee && !fRanged;

  // Climb
  if (zSelf > zElev && fSafe &&
    (zSelf - zClimb > zElev || objHere != objMine1))
    Desire(actDown, 0, 0, desDown);
  if (fCanClimb && nTurn >= 0)
    Desire(actUp, 0, 0, desUp);

  // Dig
  if (zSelf <= zElev && !fWater && objHere == 0 && objWield == objPick &&
    fSafe && cInvFree > 1) {
    if (iDist != 12)
      for (d = 0; d < DIRS2; d++) {
        x = xSelf + xoff[d];
        y = ySelf + yoff[d];
        CheckWrap(&x, &y);
        if (bm.k2.Get(x, y) == ws.rglVar[numTube])
          break;
      }
    if (iDist == 12 || d < DIRS2) {
      l = bm.k2.Get(xSelf, ySelf);
      if (l != ws.rglVar[numTube] && (l | 0x000f1f) != kvBrown)
        Desire(actDig, 0, 0, desDig);
    }
  }
  if (zSelf <= zElev && fSafe) {
    // Destroy item
    if (objHere != 0 && cAlly <= 1 && zSelf - zElev <= zClimb/2 && !fNet &&
      (objWield == objPick || objWield == objTorch) &&
      ((objWield == objPick) == (ws.rglVar[numMater + objHere] == 1)))
      Desire(actDig, 0, 0, desDig);
    // Set Net trap
    if (objWield == objNet && objHere == 0 && FObjsInInv(objSword, objTrid))
      Desire(actDig, 0, 0, desDig);
  }

  // Pick up item
  if (cInvFree > 0 && zSelf - zElev <= zClimb/2 && !fNet &&
    rgnDesObj[objHere] > 0 && !(objHere == objFish && fWater && !fCanFish))
    Desire(actGet, 0, 0, (FWep(objHere) && rgnDesObj[objHere] - nDmgWield >=
      3) || (FWepMelee(objHere) && nDmgWield <= 1) ? desGetHi : desGet);

  // Wield weapon
  for (iItem = 0; iItem < nInv; iItem++) {
    obj = ObjInv(iItem);
    if (!FWep(obj) || iItem == iWield)
      continue;
    nDmg = WepDmg(obj);
    if (FWepLaunch(obj) && (!(FObjInInv(obj-dobj) || FWepAuto(obj)) || fNet))
      nDmg = 1;
    // Wield club if needed to move out of deep water
    if (fAllDeep) {
      if (!fCanSwim && obj == objClub)
        Desire(actUse, iItem, 0, desWieldHi + nDmg);
      if (fSafe)
        continue;
    }
    if (FWepLaunch(obj)) {
      /* Wield launcher if have ammo for it and better than what currently
        have, or if equal to wielded melee weapon and not in melee. */
      if ((FObjInInv(obj-dobj) || FWepAuto(obj)) && (nDmg > nDmgWield ||
        (nDmg >= nDmgWield && FWepMelee(objWield) && !fMelee)))
        Desire(actUse, iItem, 0, (fSafe ? desWield : desWieldHi) + nDmg);
      // Wield launcher if not wielding anything and not threatened.
      if (objWield < 1 && fSafe)
        Desire(actUse, iItem, 0, desWield + nDmg);
    }
    if (FWepMelee(obj)) {
      // Wield if strictly better than what currently have.
      if (nDmg > nDmgWield)
        Desire(actUse, iItem, 0, (fSafe ? desWield : desWieldHi) + nDmg);
      /* Wield if equal to what currently have, and can switch from throwable
        to melee only, e.g. Trident to Sword. */
      if (nDmg == nDmgWield &&
        !FWepThrow(obj) && FWepThrow(objWield) && fSafe)
        Desire(actUse, iItem, 0, desWield + nDmg);
    }
    // Wield lesser weapon if it can be used to catch fish here
    if (FFisher(obj) && objHere == objFish && zSelf - zElev <= zClimb/2 &&
      !fCanFish && nFood < nFoodMax && cInvFree > 0 && fSafe)
      Desire(actUse, iItem, 0, desWield + nDmg);
  }

  // Use, drop, or give item
  for (iItem = nInv-1; iItem >= 0; iItem--) {
    obj = ObjInv(iItem);
    if (obj == objFist)
      continue;

    // Heal self
    if (obj == objHeal &&
      (nHealth < nHealthMax || ws.rglVar[numWasp + nSelf] > 0)) {
      if (nFood <= 1 && nHealth <= 1)
        Desire(actUse, iItem, 0, desHealHi);
      if (fSafe)
        Desire(actUse, iItem, 0, desHeal);
    }

    // Eat food
    if (obj == objBerry || obj == objBread || obj == objFish) {
      if (nFood <= 1 && nHealth <= 1)
        Desire(actUse, iItem, 0, desEatHi + (obj == objBread));
      if (nFood < nFoodMax && nTurn > (nEarly * 3 >> 1) + nSelf && fSafe)
        Desire(actUse, iItem, 0, desEat + (obj == objBread));
    }
    if (obj == objNight && nTribLeft == 2)
      for (i = 0; i < nTribMax; i++)
        if (i != nSelf && ws.rglVar[numHP + i] > 0 && Faction(i) <= 0) {
          Desire(actUse, iItem, 0, desEatHi + 2);
          break;
        }

    // Wear item
    if ((objWear != objPara || zSelf - zElev <= zClimb) && fSafe) {
      if (obj == objGogg && (iWear < 0 ||
        (objWear != objGogg && ds.fStar)) && !fCanSee)
        Desire(actUse, iItem, 0, desWear + rgnValObj[obj])
      if (obj == objHelm && (iWear < 0 ||
        (objWear != objHelm && objWear != objArmor && !ds.fStar)))
        Desire(actUse, iItem, 0, desWear + rgnValObj[obj])
      if (obj == objArmor && (iWear < 0 ||
        (objWear != objArmor && !ds.fStar)))
        Desire(actUse, iItem, 0, desWear + rgnValObj[obj])
    }
    if (obj == objPara && iWear < 0 && fSafe)
      Desire(actUse, iItem, 0, desWear + rgnValObj[obj])

    // Place landmine
    if (fNet)
      continue;
    if (obj == objMine0 && zSelf <= zElev && objHere == 0 && fSafe)
      Desire(actUse, iItem, 0, desMine);

    // Drop item
    if (iItem == iWield || iItem == iWear || nTurn <= (nEarly << 1) ||
      (!fSafe && dGive < 0) || (fMutt && obj == objTeeth) ||
      (objHere == objMine1 && zSelf <= zElev))
      continue;
    // Always drop Nightlock or active Landmine as soon as possible
    if (obj == objNight || obj == objMine1)
      Desire(actDrop, iItem, 0, desDrop);
    // Drop duplicate or extraneous tool
    if (((obj == objGogg || obj == objHelm || obj == objArmor) &&
      objWear == obj) || (obj == objHelm && objWear == objArmor) ||
      (obj == objPara && iWear >= 0) || (obj == objGogg && fCanSee))
      Desire(actDrop, iItem, 0, desDrop);
    /* Drop melee only weapon worse than melee weapon being wielded, or
      equal to melee only weapon already wielded. */
    if (FWepMelee(obj) && !FWepThrow(obj) && FWepMelee(objWield) &&
      objWield != objNet && (WepDmg(obj) < nDmgWield ||
      (WepDmg(obj) == nDmgWield && !FWepThrow(objWield)))) {
      if (dGive < 0 || !FAlly(nGive - ws.rglVar[numDmg + obj]*10) ||
        (FAlly(nGive) && Rnd(0, 1))) {
        Desire(actDrop, iItem, 0, desDrop);
      } else
        Desire(actGive, iItem, dGive, FAlly(nGive) ? desGive : desGiveHi);
    }
    // Improve full inventory if on top of better weapon or better food
    if (cInvFree <= 1) {
      // Don't drop launcher to pick up unthrowable ammo for that launcher
      // Don't drop anything for a Fish if can't catch fish
      // Don't drop last consumable item in inventory
      if (obj > 0 && objHere > 0 && rgnValObj[obj] < rgnConObj[objHere] &&
        !(FWepAmmo(objHere) && !FWepThrow(objHere) && obj == objHere+dobj) &&
        !(objHere == objFish && fWater && !fCanFish) &&
        !(cConsum <= 1 && FConsum(obj))) {
        if (dGive < 0 || !FAlly(nGive - ws.rglVar[numDmg + obj]*10) ||
          (FAlly(nGive) && Rnd(0, 1))) {
          Desire(actDrop, iItem, 0, desDrop - rgnValObj[obj]);
        } else
          Desire(actGive, iItem, dGive,
            (FAlly(nGive) ? desGive : desGiveHi) - rgnValObj[obj]);
      }
    }

    // Give item
    if (dGive >= 0 && !FAlly(nGive) &&
      FAlly(nGive - ws.rglVar[numDmg + obj]*10))
      Desire(actGive, iItem, dGive, desGiveHi - rgnValObj[obj]);
  }

  // Pick best move of all possible
  ws.rglVar[iLetterX] = nItem;
  ws.rglVar[iLetterY] = nDir;
  ws.rglVar[iLetterZ] = nAction;
  return fTrue;
}

/* daedalus.cpp */
