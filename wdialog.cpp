/*
** Daedalus (Version 3.5) File: wdialog.cpp
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
** This file contains processing for all the Windows dialogs, and related
** dialog utilities.
**
** Created: 2/11/2001.
** Last code change: 10/30/2024.
*/

#include <windows.h>
#include <stdio.h>

#include "resource.h"
#include "util.h"
#include "graphics.h"
#include "color.h"
#include "threed.h"
#include "maze.h"
#include "draw.h"
#include "daedalus.h"
#include "wdriver.h"


#ifdef WIN
enum _extension {
  iextBmp = 0,
  iextTxt = 1,
  iextXbm = 2,
  iextTga = 3,
  iextPcx = 4,
  iextWmf = 5,
  iextSvg = 6,
  iextD3  = 7,
  iextDS  = 8,
  iextDW  = 9,
  iextDP  = 10,
  iextDB  = 11,
  iextAll = 12,
  iextMax = 13,
};

CONST char *rgszOpen[iextMax] = {"Open Bitmap", "Open Text",
  "Open X11 Bitmap", "Open Targa Bitmap", "Open Paint Bitmap",
  "" /* Open Picture */, "" /* Open Vector */, "Open 3D Bitmap",
  "Open Script", "Open Wireframe", "Open Patches",
  "Open Daedalus Bitmap", "Open File"};
CONST char *rgszSave[iextMax] = {"Save Bitmap", "Save Text",
  "Save X11 Bitmap", "Save Targa Bitmap", "" /* Save Paint Bitmap */,
  "Save Picture", "Save Vector", "Save 3D Bitmap",
  "" /* Save Script */, "Save Wireframe", "Save Patches",
  "Save Daedalus Bitmap", "" /* Save File */};
CONST char *rgszFilter[iextMax] = {
  "Windows Bitmaps (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0",
  "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0",
  "X11 Bitmaps (*.xbm)\0*.xbm\0All Files (*.*)\0*.*\0",
  "Targa Bitmaps (*.tga)\0*.tga\0All Files (*.*)\0*.*\0",
  "Paint Bitmaps (*.pcx)\0*.pcx\0All Files (*.*)\0*.*\0",
  "Windows Metafiles (*.wmf)\0*.wmf\0All Files (*.*)\0*.*\0",
  "Scalable Vector Graphics (*.svg)\0*.svg\0All Files (*.*)\0*.*\0",
  "3D Bitmaps (*.d3)\0*.d3\0All Files (*.*)\0*.*\0",
  "Daedalus Scripts (*.ds)\0*.ds\0All Files (*.*)\0*.*\0",
  "Wireframe Files (*.dw)\0*.dw\0All Files (*.*)\0*.*\0",
  "Patch Files (*.dp)\0*.dp\0All Files (*.*)\0*.*\0",
  "Daedalus Bitmaps (*.db)\0*.db\0All Files (*.*)\0*.*\0",
  "All Daedalus Files\0*.bmp;*.txt;*.d?;*.pcx\0All Files (*.*)\0*.*\0"};
CONST char *rgszExt[iextMax] = {
  "bmp", "txt", "xbm", "tga", "pcx", "wmf", "svg",
  "d3", "ds", "dw", "dp", "db", ""};


/*
******************************************************************************
** Dialog Utilities
******************************************************************************
*/

// Bring up the specified dialog, as invoked by various menu commands.

void DoDialog(flag API pfn(HWND, uint, WORD, LONG), WORD dlg)
{
  DLGPROC dlgproc;
  flag fSav;

  if (ws.fSaverRun)
    ShowCursor(fTrue);
  dlgproc = (DLGPROC)MakeProcInstance(pfn, ws.hinst);
  fSav = ws.fInSpree; ws.fInSpree = fTrue;
  DialogBox(wi.hinst, MAKEINTRESOURCE(dlg), wi.hwnd, dlgproc);
  ws.fInSpree = fSav;
  ws.cDialog++;
  FreeProcInstance((FARPROC)dlgproc);
  if (ws.fSaverRun)
    ShowCursor(fFalse);
}


// Set the contents of a dialog edit control to the specified long integer.

void SetDlgItemLong(HWND hdlg, int id, long l)
{
  char sz[cchSzDef];

  sprintf(S(sz), "%ld", l);
  SetEdit(id, sz);
}


// Set the contents of a dialog edit control to a floating point number.

void SetDlgItemReal(HWND hdlg, int id, real r)
{
  char sz[cchSzDef], *pch;

  sprintf(S(sz), "%lf", r);
  for (pch = sz; *pch; pch++)
    ;
  while (*(--pch) == '0')
    ;
  pch[1 + (*pch == '.')] = chNull;
  SetEdit(id, sz);
}


// Return the contents of a dialog edit control as a long integer.

long GetDlgItemLong(HWND hdlg, int id)
{
  char sz[cchSzDef];

  GetEdit(id, sz);
  return LFromRgch(sz, CchSz(sz));
}


// Return the contents of a dialog edit control as a floating point number.

real GetDlgItemReal(HWND hdlg, int id)
{
  char sz[cchSzDef];

  GetEdit(id, sz);
  return atof(sz);
}


// Return which radio button is set in the specified radio button range.

int GetDlgItemRadio(HWND hdlg, int idLo, int idHi)
{
  int id;

  for (id = idLo; id <= idHi; id++)
    if (GetCheck(id))
      return id - idLo;
  return 0;
}


// Set the contents of a combo control in a dialog indicating a color field
// to the given value, and fill its dropdown with the default color list.

void SetEditColor(HWND hdlg, int id, KV kv)
{
  char sz[cchSzDef];
  int i;

  ConvertKvToSz(kv, sz, cchSzDef); SetEdit(id, sz);
  for (i = 0; i < cColorMain; i++)
    SetCombo(id, rgszColor[i]);
}


// Return whether a given integer dialog value is within a valid range,
// displaying an error if not.

flag FEnsureBetweenN(int n, int n1, int n2, CONST char *szError)
{
  char sz[cchSzMax];

  if (FBetween(n, n1, n2))
    return fTrue;
  sprintf(S(sz),
    "The %s field of %d is out of range.\nIt must be from %d to %d.\n",
    szError, n, n1, n2);
  PrintSz_W(sz);
  return fFalse;
}


// Return whether a given floating point dialog value is within a valid range,
// displaying an error if not.

flag FEnsureBetweenR(real r, int n1, int n2, CONST char *szError)
{
  char sz[cchSzMax];

  if (FBetween(r, (real)n1, (real)n2))
    return fTrue;
  sprintf(S(sz),
    "The %s field of %.1lf is out of range.\nIt must be from %d to %d.\n",
    szError, r, n1, n2);
  PrintSz_W(sz);
  return fFalse;
}


// Return whether a given integer dialog value is high enough to be valid,
// displaying an error if not.

flag FEnsureLeastN(int n, int n1, char *szError)
{
  char sz[cchSzMax];

  if (n >= n1)
    return fTrue;
  sprintf(S(sz),
    "The %s field of %d is out of range.\nIt must be at least %d.\n",
    szError, n, n1);
  PrintSz_W(sz);
  return fFalse;
}


// Implements the various menu commands and operations that open files.

flag FFileOpen(int wCmd, CONST char *szFile, FILE *fileIn)
{
  FILE *file = NULL;
  char sz[cchSzMax], *pch;
  int isz, iTry = 0;
  flag fWaited = fFalse, fSav, fRet;

  switch (wCmd) {
  case cmdOpen:
    isz = iextAll;
    break;
  case cmdOpenBitmap:
    isz = iextBmp;
    break;
  case cmdOpenText:
    isz = iextTxt;
    break;
  case cmdOpenXbm:
    isz = iextXbm;
    break;
  case cmdOpen3D:
    isz = iextD3;
    break;
  case cmdOpenColmapTarga:
    isz = iextTga;
    break;
  case cmdOpenColmapPaint:
    isz = iextPcx;
    break;
  case cmdOpenScript:
    isz = iextDS;
    break;
  case cmdOpenWire:
    isz = iextDW;
    break;
  case cmdOpenPatch:
    isz = iextDP;
    break;
  case cmdOpenDB:
    isz = iextDB;
    break;
  default:
    Assert(fFalse);
  }
  ofn.lpstrTitle  = rgszOpen[isz];
  ofn.lpstrFilter = rgszFilter[isz];
  ofn.lpstrDefExt = rgszExt[isz];
  if (isz < iextAll &&
    (ofn.lpstrFile[0] == chNull || ofn.lpstrFile[0] == '*')) {
    ofn.lpstrFile[0] = '*';
    ofn.lpstrFile[1] = '.';
    CopySz(ofn.lpstrDefExt, &ofn.lpstrFile[2], cchSzMaxFile-2);
  }

  // Get the file handle to read from.
  if (fileIn != NULL)

    // For embedded files in scripts, there's already an open file handle.
    file = fileIn;
  else {
    if (szFile == NULL) {
      if (ws.rgszStartup != NULL) {
        Assert(wCmd != cmdOpen);
        goto LLoad;
      }

      // If not given the name of a file to open, bring up the Windows
      // standard open dialog and prompt for one.
      fSav = ws.fInSpree; ws.fInSpree = fTrue;
      fRet = GetOpenFileName((LPOPENFILENAME)&ofn);
      ws.fInSpree = fSav;
      ws.cDialog++;
      if (!fRet)
        goto LExit;
      sprintf(S(sz), "%s", ofn.lpstrFile);
    } else
      sprintf(S(sz), "%s", szFile);

    // Remove surrounding double quotes if present.
    if (*sz == '"') {
      for (pch = sz; *pch; pch++) {
        *pch = *(pch + 1);
        if (*pch == '"') {
          *pch = chNull;
          break;
        }
      }
    }

    // Open the file for reading given its name.
    if (wi.hMutex == NULL && FSzVar(ws.nFileLock))
      wi.hMutex = CreateMutex(NULL, fFalse, ws.rgszVar[ws.nFileLock]);
    if (wi.hMutex != NULL) {
      WaitForSingleObject(wi.hMutex, 1000);
      fWaited = fTrue;
    }

    file = FileOpen(sz, "rb");
    if (file == NULL) {
      ws.szTitle = ofn.lpstrTitle;
      sprintf(S(sz), "The file %s could not be opened.",
        szFile == NULL ? ofn.lpstrFile : szFile);
      PrintSz_E(sz);
      fRet = fFalse;
      goto LExit;
    }
  }

LLoad:
  fRet = FReadFile(wCmd, file, file != NULL && fileIn == NULL);
LExit:
  if (fWaited)
    ReleaseMutex(wi.hMutex);
  return fRet;
}


// Implements the various menu commands that save files.

flag FFileSave(int wCmd, CONST char *szFile)
{
  char sz[cchSzMax], szT[cchSzDef], *pchEnd, *pchLeaf, *pchExt, *pch;
  CMaz b;
  CMazK c;
  int isz, xs, ys, x, y, i;
  flag fWall, fSav, fRet;

  switch (wCmd) {
  case cmdSaveBitmap:
  case cmdSaveDB:
  case cmdSaveWallCenter:
  case cmdSaveWallTile:
  case cmdSaveWallStretch:
  case cmdSaveWallFit:
  case cmdSaveWallFill:
    isz = iextBmp;
    break;
  case cmdSaveText:
  case cmdSaveDOS:
  case cmdSaveDOS2:
  case cmdSaveDOS3:
    isz = iextTxt;
    break;
  case cmdSaveXbmN:
  case cmdSaveXbmC:
  case cmdSaveXbmS:
    isz = iextXbm;
    break;
  case cmdSaveColmapTarga:
    isz = iextTga;
    break;
  case cmdSave3DN:
  case cmdSave3DC:
  case cmdSave3DS:
    isz = iextD3;
    break;
  case cmdSaveWire:
    isz = iextDW;
    break;
  case cmdSavePatch:
    isz = iextDP;
    break;
  case cmdSavePicture:
    isz = iextWmf;
    break;
  case cmdSaveVector:
    isz = iextSvg;
    break;
  default:
    Assert(fFalse);
  }
  ofn.lpstrTitle  = rgszSave[isz];
  ofn.lpstrFilter = rgszFilter[isz];
  ofn.lpstrDefExt = rgszExt[isz];
  if (ofn.lpstrFile[0] == chNull || ofn.lpstrFile[0] == '*') {
    ofn.lpstrFile[0] = '*';
    ofn.lpstrFile[1] = '.';
    CopySz(ofn.lpstrDefExt, &ofn.lpstrFile[2], cchSzMaxFile-2);
  }

  fWall = wCmd == cmdSaveWallCenter || wCmd == cmdSaveWallTile ||
    wCmd == cmdSaveWallStretch || wCmd == cmdSaveWallFit ||
    wCmd == cmdSaveWallFill;
  if (!fWall) {
    if (szFile == NULL) {

      // If not given the name of a file to save, bring up the Windows
      // standard save dialog and prompt for one.
      fSav = ws.fInSpree; ws.fInSpree = fTrue;
      fRet = GetSaveFileName((LPOPENFILENAME)&ofn);
      ws.fInSpree = fSav;
      ws.cDialog++;
      if (!fRet)
        return fFalse;
      sprintf(S(sz), "%s", ofn.lpstrFile);
    } else
      sprintf(S(sz), "%s", szFile);
  } else {

    // For the Save As Wallpaper commands, always save to the file
    // "Daedalus.bmp" in the Windows directory.
    GetWindowsDirectory(sz, cchSzDef);
    pch = sz + CchSz(sz);
    sprintf(SO(pch, sz), "\\%s.bmp", ws.szAppName);
    ofn.lpstrTitle = "Save Wallpaper";
  }

  if ((wCmd != cmdSaveBitmap && wCmd != cmdSaveText && wCmd != cmdSaveDB) ||
    (wCmd == cmdSaveText && dr.f3D) || ws.xCutoff < 1 || ws.yCutoff < 1 ||
    (bm.b.m_x <= ws.xCutoff && bm.b.m_y <= ws.yCutoff)) {

    // Standard case: Go save the file.
    if (!FWriteFile(bm.b, bm.k, wCmd, sz, ofn.lpstrTitle))
      return fFalse;

    // For Save As Wallpaper commands, tell Windows the wallpaper has changed.
    if (fWall) {
      WriteProfileString("Desktop", "TileWallpaper",
        wCmd == cmdSaveWallTile ? "1" : "0");
      WriteProfileString("Desktop", "WallpaperStyle",
        wCmd == cmdSaveWallStretch ? "2" : (wCmd == cmdSaveWallFit ? "6" :
        (wCmd == cmdSaveWallFill ? "10" : "0")));
      SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, sz,
        SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
    }
  } else {

    // If the 2D Bitmap Cutoff File Settings are set, and the bitmap is large
    // enough, save the bitmap as an array of smaller bitmap files instead.
    if (!bm.fColor) {
      if (!b.FAllocate(ws.xCutoff, ws.yCutoff, &bm.b))
        return fFalse;
      xs = bm.b.m_x; ys = bm.b.m_y;
    } else {
      if (!c.FAllocate(ws.xCutoff, ws.yCutoff, &bm.k))
        return fFalse;
      xs = bm.k.m_x; ys = bm.k.m_y;
    }
    xs = (xs - 1) / ws.xCutoff + 1;
    ys = (ys - 1) / ws.yCutoff + 1;
    if (xs*ys > 9999) {
      ws.szTitle = ofn.lpstrTitle;
      PrintSz_E("There would be too many bitmaps for bitmap splitting.");
      return fFalse;
    }

    // Analyze the specified filename. The name itself can't be more than four
    // characters long, since four numbers will be appended to it, and the
    // final name should not be longer than 8 characters.
    for (pchEnd = sz; *pchEnd; pchEnd++)
      ;
    for (pchLeaf = pchEnd; pchLeaf > sz && *pchLeaf != '\\'; pchLeaf--)
      ;
    if (*pchLeaf == '\\')
      pchLeaf++;
    for (pchExt = pchLeaf; *pchExt && *pchExt != '.'; pchExt++)
      ;
    if (pchExt - pchLeaf > 4) {
      ws.szTitle = ofn.lpstrTitle;
      PrintSz_E("The filename is too long for bitmap splitting.");
      return fFalse;
    }
    for (pch = pchEnd + 4; pch - 4 >= pchExt; pch--)
      *pch = *(pch - 4);

    // Save each bitmap section in sequence.
    for (y = 0; y < ys; y++)
      for (x = 0; x < xs; x++) {
        if (!bm.fColor) {
          b.BitmapSet(dr.fSet);
          b.BlockMove(bm.b, x*ws.xCutoff, y*ws.yCutoff,
            (x+1)*ws.xCutoff-1, (y+1)*ws.yCutoff-1, 0, 0);
        } else {
          c.BitmapSet(kvSet);
          c.BlockMove(bm.k, x*ws.xCutoff, y*ws.yCutoff,
            (x+1)*ws.xCutoff-1, (y+1)*ws.yCutoff-1, 0, 0, -1);
        }
        i = y*xs + x;
        sprintf(S(szT), "%04d", i);
        for (i = 0; i < 4; i++)
          pchExt[i] = szT[i];
        if (!FWriteFile(b, c, wCmd, sz, ofn.lpstrTitle))
          return fFalse;
      }
  }
  return fTrue;
}


// Copy a string to the clipboard. Implements the fMessageCopy setting.

flag FSzCopy(CONST char *sz)
{
  HGLOBAL hglobal;
  byte *hpb;
  int cb;

  // Allocate a memory buffer equal to the size of the string.
  cb = CchSz(sz) + 1;
  hglobal = GlobalAlloc(GMEM_MOVEABLE, cb);
  if (hglobal == (HGLOBAL)NULL)
    return fFalse;
  hpb = (byte *)GlobalLock(hglobal);

  // Copy the contents of the string into the memory buffer.
  CopySz(sz, (char *)hpb, cb);
  GlobalUnlock(hglobal);

  // Set the Windows clipboard to the memory buffer.
  if (!OpenClipboard(wi.hwnd)) {
    GlobalFree(hglobal);
    return fFalse;
  }
  EmptyClipboard();
  SetClipboardData(CF_TEXT, hglobal);
  CloseClipboard();
  return fTrue;
}


// Implements the Copy Bitmap, Copy Text, and Copy Picture menu commands.

flag FFileCopy(int wCmd)
{
  HFILE hfile;
  LONG lSize, lHeader;
  HGLOBAL hglobal, hmfp;
  METAFILEPICT mfp;
  HMETAFILE hmf;
  byte *hpb;

  // Save the bitmap to a temporary file.
  if (!FWriteFile(bm.b, bm.k, wCmd, ws.szFileTemp, "Copy"))
    return fFalse;

  // Open that temporary file.
  hfile = _lopen(ws.szFileTemp, OF_READ);
  if (hfile == HFILE_ERROR) {
    _unlink(ws.szFileTemp);
    return fFalse;
  }

  // Allocate a memory buffer equal to the size of the file.
  lSize = _llseek(hfile, 0, 2);
  lHeader = wCmd == cmdCopyText ? 0 :
    (wCmd == cmdCopyBitmap ? sizeof(BITMAPFILEHEADER) : 22);
  hglobal = GlobalAlloc(GMEM_MOVEABLE, lSize - lHeader +
    (wCmd == cmdCopyText));
  if (hglobal == (HGLOBAL)NULL) {
    _unlink(ws.szFileTemp);
    return fFalse;
  }
  hpb = (byte *)GlobalLock(hglobal);

  // Read the contents of the file into the memory buffer.
  _llseek(hfile, lHeader, 0);
  _hread(hfile, hpb, lSize - lHeader);
  if (wCmd == cmdCopyText)
    hpb[lSize] = 0;
  _lclose(hfile);
  _unlink(ws.szFileTemp);
  GlobalUnlock(hglobal);

  // Set the Windows clipboard to the memory buffer.
  if (!OpenClipboard(wi.hwnd)) {
    GlobalFree(hglobal);
    return fFalse;
  }
  EmptyClipboard();
  if (wCmd == cmdCopyText)
    SetClipboardData(CF_TEXT, hglobal);
  else if (wCmd == cmdCopyBitmap)
    SetClipboardData(CF_DIB, hglobal);
  else {
    // For metafiles a special structure with a pointer to the picture data
    // needs to be allocated and used.
    hpb = (byte *)GlobalLock(hglobal);
    mfp.mm = MM_ANISOTROPIC;
    mfp.xExt = *(((word *)hpb)+(9+17+3+5+4)) * 50;
    mfp.yExt = *(((word *)hpb)+(9+17+3+5+3)) * 50;
    hmf = SetMetaFileBitsEx(lSize - lHeader, hpb);
    GlobalUnlock(hglobal);
    mfp.hMF = hmf;
    hmfp = GlobalAlloc(GMEM_MOVEABLE, sizeof(METAFILEPICT));
    if (hmfp == (HGLOBAL)NULL)
      return fFalse;
    hpb = (byte *)GlobalLock(hmfp);
    *(METAFILEPICT FAR *)hpb = mfp;
    GlobalUnlock(hmfp);
    SetClipboardData(CF_METAFILEPICT, hmfp);
  }
  CloseClipboard();
  return fTrue;
}


// Implements the Paste menu command.

flag FFilePaste()
{
  HFILE hfile;
  FILE *file;
  LONG lSize;
  HGLOBAL hglobal;
  byte *hpb;
  flag fText;

  // Check for a bitmap or text on the Windows clipboard, the two formats
  // Daedalus knows how to paste.
  if (!OpenClipboard(wi.hwnd))
    return fFalse;
  if (IsClipboardFormatAvailable(CF_DIB))
    fText = fFalse;
  else if (IsClipboardFormatAvailable(CF_TEXT))
    fText = fTrue;
  else {
    PrintSz_W("There is no bitmap on the clipboard.\n");
    return fFalse;
  }

  // Get a memory buffer containing the Windows clipboard.
  hglobal = GetClipboardData(fText ? CF_TEXT : CF_DIB);
  if (hglobal == (HGLOBAL)NULL)
    return fFalse;

  // Create a temporary file.
  hfile = _lcreat(ws.szFileTemp, 0);
  if (hfile == HFILE_ERROR)
    return fFalse;

  // Save the contents of the memory buffer to that temporary file.
  hpb = (byte *)GlobalLock(hglobal);
  lSize = (LONG)GlobalSize(hglobal);
  _hwrite(hfile, (char *)hpb, lSize);
  _lclose(hfile);
  GlobalUnlock(hglobal);

  // Load the file's contents into the program.
  file = FileOpen(ws.szFileTemp, "rb");
  if (file == NULL)
    return fFalse;
  if (!fText)
    FReadBitmap(file, fTrue);
  else {
    if (bm.b.FReadText(file))
      FShowColmap(fFalse);
  }
  fclose(file);
  _unlink(ws.szFileTemp);
  CloseClipboard();
  return fTrue;
}


// Bring up the Windows standard print dialog, receive any printing settings
// from the user, and proceed to print the current monochrome or color bitmap
// as displayed in the window. Called from the Print command.

flag DlgPrint()
{
  DLGPROC lpAbortDlg;
  ABORTPROC lpAbortProc;
  HDC hdc;
  LPDEVMODE lpDevMode = NULL;
  LPDEVNAMES lpDevNames;
  LPSTR lpszDriverName;
  LPSTR lpszDeviceName;
  LPSTR lpszPortName;
  DOCINFO doci;
  int xPrint, yPrint, xpOrigin, ypOrigin, xpView, ypView;
  flag f;

  // Bring up the Windows print dialog.
  if (!PrintDlg((LPPRINTDLG)&prd))
    return fTrue;

  // Get the printer DC.
  if (prd.hDC)
    hdc = prd.hDC;
  else {
    // If the dialog didn't just return the DC, try to make one manually.
    if (!prd.hDevNames)
      return fFalse;
    lpDevNames = (LPDEVNAMES)GlobalLock(prd.hDevNames);
    lpszDriverName = (LPSTR)lpDevNames + lpDevNames->wDriverOffset;
    lpszDeviceName = (LPSTR)lpDevNames + lpDevNames->wDeviceOffset;
    lpszPortName = (LPSTR)lpDevNames + lpDevNames->wOutputOffset;
    GlobalUnlock(prd.hDevNames);
    if (prd.hDevMode)
      lpDevMode = (LPDEVMODE)GlobalLock(prd.hDevMode);
    hdc = CreateDC(lpszDriverName, lpszDeviceName, lpszPortName, lpDevMode);
    if (prd.hDevMode && lpDevMode)
      GlobalUnlock(prd.hDevMode);
  }
  if (prd.hDevNames) {
    GlobalFree(prd.hDevNames);
    prd.hDevNames = (HGLOBAL)NULL;
  }
  if (prd.hDevMode) {
    GlobalFree(prd.hDevMode);
    prd.hDevMode = (HGLOBAL)NULL;
  }

  // Setup the abort dialog and start the print job.
  lpAbortDlg = (DLGPROC)MakeProcInstance((FARPROC)DlgAbort, ws.hinst);
  lpAbortProc = (ABORTPROC)MakeProcInstance((FARPROC)DlgAbortProc, ws.hinst);
  SetAbortProc(hdc, lpAbortProc);
  doci.cbSize = sizeof(DOCINFO);
  doci.lpszDocName = ws.szAppName;
  doci.lpszOutput = NULL;
  if (StartDoc(hdc, &doci) < 0) {
    f = fFalse;
    goto LDone;
  }
  ws.fAbort = fFalse;
  wi.hwndAbort = CreateDialog(wi.hinst, MAKEINTRESOURCE(dlgAbort), wi.hwnd,
    lpAbortDlg);
  if (!wi.hwndAbort)
    return fFalse;
  ShowWindow(wi.hwndAbort, SW_NORMAL);
  EnableWindow(wi.hwnd, fFalse);
  StartPage(hdc);

  // Scale the bitmap to the page.
  InitDC(hdc);
  xPrint = GetDeviceCaps(hdc, HORZRES);
  yPrint = GetDeviceCaps(hdc, VERTRES);
  xpOrigin = ypOrigin = 0;
  xpView = xPrint; ypView = yPrint;
  if (NMultDiv(xpView, 100, ypView) > NMultDiv(bm.xaView, 100, bm.yaView)) {
    xpView = NMultDiv(ypView, bm.xaView, bm.yaView);
    xpOrigin = NMultDiv(xPrint - xpView, ws.xScroll, nScrollDiv);
  } else {
    ypView = NMultDiv(xpView, bm.yaView, bm.xaView);
    ypOrigin = NMultDiv(yPrint - ypView, ws.yScroll, nScrollDiv);
  }

  // Actually "draw" the bitmap to the printer.
  if (!bm.fColor)
    f = (StretchDIBits(hdc, xpOrigin, ypOrigin, xpView, ypView,
      bm.xaOrigin, bm.yaOrigin, bm.xaView, bm.yaView,
      bm.b.m_rgb, (BITMAPINFO *)&wi.bi, DIB_RGB_COLORS, SRCCOPY) != 0);
  else
    f = (StretchDIBits(hdc, xpOrigin, ypOrigin, xpView, ypView,
      bm.xaOrigin, bm.yaOrigin, bm.xaView, bm.yaView,
      bm.k.m_rgb, (BITMAPINFO *)&wi.biC, DIB_RGB_COLORS, SRCCOPY) != 0);

  // Finalize and cleanup everything.
  if (!ws.fAbort) {
    f &= (EndPage(hdc) >= 0);
    f &= (EndDoc(hdc) >= 0);
  }
  EnableWindow(wi.hwnd, fTrue);
  DestroyWindow(wi.hwndAbort);

LDone:
  FreeProcInstance(lpAbortDlg);
  FreeProcInstance(lpAbortProc);
  DeleteDC(hdc);
  return f;
}


// Message loop function for the printing abort dialog. Loops until printing
// is completed or the user hits cancel, returning which result.

flag API DlgAbortProc(HDC hdc, int nCode)
{
  MSG msg;

  if (wi.hwndAbort == (HWND)NULL)
    return fTrue;
  while (!ws.fAbort && PeekMessage(&msg, (HWND)NULL, 0, 0, PM_REMOVE))
    if (!IsDialogMessage(wi.hwndAbort, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  return !ws.fAbort;
}


/*
******************************************************************************
** Windows Dialogs
******************************************************************************
*/

// Processing routine for the printing abort modeless dialog, as brought up
// temporarily when printing via the File Print menu command.

flag API DlgAbort(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  switch (msg) {
  case WM_INITDIALOG:
    SetFocus(GetDlgItem(hdlg, IDCANCEL));
    return fFalse;
  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      ws.fAbort = fTrue;
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing routine for the File Settings dialog.

flag API DlgFile(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  int n2x, n2y, n3;

  switch (msg) {
  case WM_INITDIALOG:
    SetEditL(deFs_ig, ms.nInfGen);
    SetEditL(deFs_ic, ms.nInfCutoff);
    SetEditL(deFs_in, ms.nInfFileCutoff);
    SetCheck(dxFs_e, ms.fInfEller);
    SetCheck(dxFs_is, ms.fInfAutoStart);
    SetRadio(dr1 + ms.nInfFile, dr1, dr4);
    SetEditN(deFs_2x, ws.xCutoff);
    SetEditN(deFs_2y, ws.yCutoff);
    SetEditN(deFs_3, ms.xCutoff);
    SetCheck(dxFs_t, ws.fTextClip);
    SetCheck(dxFs_l, ws.fLineChar);
    SetCheck(dxFs_b, ws.fTextTab);
    SetFocus(GetDlgItem(hdlg, deFs_ig));
    return fFalse;
  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      if (wParam == IDOK) {
        n2x = GetEditN(deFs_2x);
        n2y = GetEditN(deFs_2y);
        n3 = GetEditN(deFs_3);
        EnsureBetweenN(n2x, 0, xBitmap, "2D Bitmap X Cutoff");
        EnsureBetweenN(n2y, 0, yBitmap, "2D Bitmap Y Cutoff");
        EnsureBetweenN(n3, 1, xBitmap, "3D Bitmap X Cutoff");
        ms.nInfGen = GetEditL(deFs_ig);
        ms.nInfCutoff = GetEditL(deFs_ic);
        ms.nInfFileCutoff = GetEditL(deFs_in);
        ms.fInfEller = GetCheck(dxFs_e);
        ms.fInfAutoStart = GetCheck(dxFs_is);
        ms.nInfFile = GetRadio(dr1, dr4);
        ws.xCutoff = n2x;
        ws.yCutoff = n2y;
        ms.xCutoff = n3;
        ws.fTextClip = GetCheck(dxFs_t);
        ws.fLineChar = GetCheck(dxFs_l);
        ws.fTextTab = GetCheck(dxFs_b);
      }
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing routine for the About Daedalus dialog.

flag API DlgAbout(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];

  switch (msg) {
  case WM_INITDIALOG:
#ifdef DEBUG
    SetWindowText(hdlg, "About Daedalus (DEBUG)");
#endif
    sprintf(S(sz), "Daedalus version %s for %s Windows",
      szVersion, szArchCore);
    SetDlgItemText(hdlg, ds1, sz);
    return fTrue;
  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing routine for the Enter Command Line dialog.

flag API DlgCommand(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  char sz[cchSzOpr], szT[cchSzDef], *pch;
  int i;

  switch (msg) {
  case WM_INITDIALOG:
    SetEditN(deCl_n, 1);
    SetFocus(GetDlgItem(hdlg, deCl_cl));
    return fFalse;
  case WM_COMMAND:
    if (wParam == dbCl_g || wParam == dbCl_p) {
      GetEdit(deCl_n, szT);
      PchGetParameter(szT, NULL, NULL, (long *)&i, 1);
      EnsureLeastN(i, 0, "Macro Number");
      if (wParam == dbCl_g) {
        SetEdit(deCl_cl, i < ws.cszMacro && ws.rgszMacro[i] != NULL ?
          ws.rgszMacro[i] : "");
        if (FBetween(i, 1, cMacro)) {
          GetMenuString(wi.hmenu, cmdMacro01 + i - 1, sz, cchSzMax,
            MF_BYCOMMAND);
          for (pch = sz; *pch; pch++)
            ;
          while (pch >= sz && *pch != chTab)
            pch--;
          if (pch >= sz)
            *pch = chNull;
          SetEdit(deCl_mt, sz);
        }
      } else {
        GetEditCore(deCl_cl, sz, cchSzOpr);
        GetEdit(deCl_mt, szT);
        DoDefineMacro(i, sz, CchSz(sz), szT);
      }
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      if (wParam == IDOK) {
        GetEditCore(deCl_cl, sz, cchSzOpr);
        EndDialog(hdlg, fTrue);
        RunCommandLine(sz, NULL);
        ws.fReturning = fFalse;
      } else
        EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing routine for the Macro Events dialog.

flag API DlgEvent(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  int nme, nmw, nm, nm2, nf, ni, ni2, nc, ncl, ncr, ncm, ncp, ncn, nmm, nex;

  switch (msg) {
  case WM_INITDIALOG:
    SetEditN(deEv_me, ws.iEventEdge);
    SetEditN(deEv_mw, ws.iEventWall);
    SetEditN(deEv_m,  ws.iEventMove);
    SetEditN(deEv_m2, ws.iEventMove2);
    SetEditN(deEv_f,  ws.iEventFollow);
    SetEditN(deEv_i,  ws.iEventInside);
    SetEditN(deEv_i2, ws.iEventInside2);
    SetEditN(deEv_c,  ws.iEventCommand);
    SetEditN(deEv_cl, ws.iEventLeft);
    SetEditN(deEv_cr, ws.iEventRight);
    SetEditN(deEv_cm, ws.iEventMiddle);
    SetEditN(deEv_cp, ws.iEventPrev);
    SetEditN(deEv_cn, ws.iEventNext);
    SetEditN(deEv_mm, ws.iEventMouse);
    SetEditN(deEv_ex, ws.iEventQuit);
    SetFocus(GetDlgItem(hdlg, deEv_me));
    return fFalse;
  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      if (wParam == IDOK) {
        nme = GetEditN(deEv_me);
        nmw = GetEditN(deEv_mw);
        nm  = GetEditN(deEv_m);
        nm2 = GetEditN(deEv_m2);
        nf  = GetEditN(deEv_f);
        ni  = GetEditN(deEv_i);
        ni2 = GetEditN(deEv_i2);
        nc  = GetEditN(deEv_c);
        ncl = GetEditN(deEv_cl);
        ncr = GetEditN(deEv_cr);
        ncm = GetEditN(deEv_cm);
        ncp = GetEditN(deEv_cp);
        ncn = GetEditN(deEv_cn);
        nmm = GetEditN(deEv_mm);
        nex = GetEditN(deEv_ex);
        EnsureLeastN(nme, 0, "Edge Event");
        EnsureLeastN(nmw, 0, "Wall Event");
        EnsureLeastN(nm,  0, "Move Event");
        EnsureLeastN(nm2, 0, "After Move Event");
        EnsureLeastN(nf,  0, "Follow Event");
        EnsureLeastN(ni,  0, "After Redraw Inside Event");
        EnsureLeastN(ni2, 0, "Redraw Inside Event");
        EnsureLeastN(nc,  0, "Run Command Event");
        EnsureLeastN(ncl, 0, "Left Click Event");
        EnsureLeastN(ncr, 0, "Right Click Event");
        EnsureLeastN(ncm, 0, "Middle Click Event");
        EnsureLeastN(ncp, 0, "Previous Click Event");
        EnsureLeastN(ncn, 0, "Next Click Event");
        EnsureLeastN(nmm, 0, "Mouse Move Event");
        EnsureLeastN(nex, 0, "Program Exit Event");
        ws.iEventEdge    = nme;
        ws.iEventWall    = nmw;
        ws.iEventMove    = nm;
        ws.iEventMove2   = nm2;
        ws.iEventFollow  = nf;
        ws.iEventInside  = ni;
        ws.iEventInside2 = ni2;
        ws.iEventCommand = nc;
        ws.iEventLeft    = ncl;
        ws.iEventRight   = ncr;
        ws.iEventMiddle  = ncm;
        ws.iEventPrev    = ncp;
        ws.iEventNext    = ncn;
        ws.iEventMouse   = nmm;
        ws.iEventQuit    = nex;
      }
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing routine for the Display Settings dialog.

flag API DlgDisplay(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  int xsOld, ysOld, xsNew, ysNew, xoOld, yoOld, xoNew, yoNew,
    nx, ny, nsx, nsy, ntd, nti;
  flag f1, f2, f3, fMove = fFalse;

  switch (msg) {
  case WM_INITDIALOG:
    SetRadio(dr1 + bm.nWhat, dr1, dr3);
    SetRadio(dr4 + bm.nHow, dr4, dr7);
    SetEditN(deRe_x, bm.xCell);
    SetEditN(deRe_y, bm.yCell);
    GetWindowPosition(&xoOld, &yoOld, &xsOld, &ysOld);
    SetEditN(deRe_wx, xsOld);
    SetEditN(deRe_wy, ysOld);
    SetEditN(deRe_wxo, xoOld);
    SetEditN(deRe_wyo, yoOld);
    SetEditN(deRe_sx, ws.xScroll);
    SetEditN(deRe_sy, ws.yScroll);
    SetCheck(dxRe_wc, ws.fResizeClient);
    SetCheck(dxRe_wt, ws.fWindowTop);
    SetCheck(dxRe_wm, ws.fWindowMenu);
    SetCheck(dxRe_ws, ws.fWindowScroll);
    SetCheck(dxRe_c, bm.fColor);
    SetCheck(dxRe_d, ws.fNoDirty);
    SetCheck(dxRe_u, ws.fAllowUpdate);
    SetCheck(dxRe_ec, gs.fErrorCheck);
    SetCheck(dxRe_td, gs.fTraceDot);
    SetEditN(deRe_td, ws.nDisplayDelay);
    SetCheck(dxRe_hg, ws.fHourglass);
    SetCheck(dxRe_ar, ws.fSpree);
    SetEditN(deRe_ti, ws.nTimerDelay);
    SetCheck(dxRe_pt, ws.fTimePause);
    SetCheck(dxRe_sm, ws.fIgnorePrint);
    SetCheck(dxRe_sp, ws.fSound);
    SetFocus(GetDlgItem(hdlg, deRe_x));
    return fFalse;
  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      if (wParam == IDOK) {
        nx = GetEditN(deRe_x);
        ny = GetEditN(deRe_y);
        nsx = GetEditN(deRe_sx);
        nsy = GetEditN(deRe_sy);
        ntd = GetEditN(deRe_td);
        nti = GetEditN(deRe_ti);
        xsNew = GetEditN(deRe_wx); ysNew = GetEditN(deRe_wy);
        xoNew = GetEditN(deRe_wxo); yoNew = GetEditN(deRe_wyo);
        EnsureLeastN(nx, 1, "Horizontal Pixel Value");
        EnsureLeastN(ny, 1, "Vertical Pixel Value");
        EnsureBetweenN(nsx, 0, nScrollDiv, "Horizontal Scroll");
        EnsureBetweenN(nsy, 0, nScrollDiv, "Vertical Scroll");
        EnsureLeastN(ntd, 0, "Display Delay");
        EnsureLeastN(nti, 0, "Timer Delay");
        EnsureLeastN(xsNew, 0, "Horizontal Window Size");
        EnsureLeastN(ysNew, 0, "Vertical Window Size");
        bm.nWhat = GetRadio(dr1, dr3);
        bm.nHow = GetRadio(dr4, dr7);
        bm.xCell = nx;
        bm.yCell = ny;
        ws.fResizeClient = GetCheck(dxRe_wc);
        f1 = ws.fWindowTop; ws.fWindowTop = GetCheck(dxRe_wt);
        f2 = ws.fWindowMenu; ws.fWindowMenu = GetCheck(dxRe_wm);
        f3 = ws.fWindowScroll; ws.fWindowScroll = GetCheck(dxRe_ws);
        GetWindowPosition(&xoOld, &yoOld, &xsOld, &ysOld);
        if (xsNew != xsOld || ysNew != ysOld ||
          xoNew != xoOld || yoNew != yoOld || ws.fWindowTop != f1) {
          if (ws.fResizeClient)
            AdjustWindowPosition(&xoNew, &yoNew, &xsNew, &ysNew);
          fMove = fTrue;
        }
        if (ws.fWindowMenu != f2)
          SetMenu(wi.hwnd, ws.fWindowMenu ? NULL : wi.hmenu);
        if (ws.fWindowScroll != f3)
          ShowScrollBar(wi.hwnd, SB_BOTH, !ws.fWindowScroll);
        ws.xScroll = nsx; SetScrollPos(wi.hwnd, SB_HORZ, ws.xScroll, fTrue);
        ws.yScroll = nsy; SetScrollPos(wi.hwnd, SB_VERT, ws.yScroll, fTrue);
        ws.fNoDirty = GetCheck(dxRe_d);
        ws.fAllowUpdate = GetCheck(dxRe_u);
        gs.fTraceDot = GetCheck(dxRe_td);
        ws.nDisplayDelay = ntd;
        gs.fErrorCheck = GetCheck(dxRe_ec);
        ws.fHourglass = GetCheck(dxRe_hg);
        ws.fSpree = GetCheck(dxRe_ar);
        ws.fIgnorePrint = GetCheck(dxRe_sm);
        ws.fSound = GetCheck(dxRe_sp);
        PauseTimer(GetCheck(dxRe_pt));
        EnsureTimer(nti);
        FShowColmap(GetCheck(dxRe_c));
        ws.fMenuDirty = fTrue;
        Redraw(wi.hwnd);
      }
      EndDialog(hdlg, fTrue);
      if (fMove)
        SetWindowPosition(xoNew, yoNew, xsNew, ysNew);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing routine for the Set Colors dialog.

flag API DlgColor(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  char sz[cchSzDef];

  switch (msg) {
  case WM_INITDIALOG:
    SetEditColor(hdlg, dc_On,     dr.kvOn);
    SetEditColor(hdlg, dc_Off,    dr.kvOff);
    SetEditColor(hdlg, dc_Edge,   dr.kvEdge);
    SetEditColor(hdlg, dc_Dot,    dr.kvDot);
    SetEditColor(hdlg, dc_Map,    dr.kvMap);
    SetEditColor(hdlg, dc_InWall, dr.kvInWall);
    SetEditColor(hdlg, dc_InEdge, dr.kvInEdge);
    SetEditColor(hdlg, dc_InSky,  dr.kvInSky);
    SetEditColor(hdlg, dc_InDirt, dr.kvInDirt);
    SetEditColor(hdlg, dc_In3D,   dr.kvIn3D);
    SetEditColor(hdlg, dc_InCeil, dr.kvInCeil);
    SetEditColor(hdlg, dc_InMtn,  dr.kvInMtn);
    SetEditColor(hdlg, dc_InCld,  dr.kvInCld);
    SetEditColor(hdlg, dc_InFog,  dr.kvInFog);
    SetCheck(dxCo_ms, dr.fSpreeRandom);
    return fTrue;
  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      if (wParam == IDOK) {
        GetEdit(dc_On,     sz); dr.kvOn     = ParseColor(sz, fFalse);
        GetEdit(dc_Off,    sz); dr.kvOff    = ParseColor(sz, fFalse);
        GetEdit(dc_Edge,   sz); dr.kvEdge   = ParseColor(sz, fFalse);
        GetEdit(dc_Dot,    sz); dr.kvDot    = ParseColor(sz, fFalse);
        GetEdit(dc_Map,    sz); dr.kvMap    = ParseColor(sz, fFalse);
        GetEdit(dc_InWall, sz); dr.kvInWall = ParseColor(sz, fFalse);
        GetEdit(dc_InEdge, sz); dr.kvInEdge = ParseColor(sz, fFalse);
        GetEdit(dc_InSky,  sz); dr.kvInSky  = ParseColor(sz, fFalse);
        GetEdit(dc_InDirt, sz); dr.kvInDirt = ParseColor(sz, fFalse);
        GetEdit(dc_In3D,   sz); dr.kvIn3D   = ParseColor(sz, fFalse);
        GetEdit(dc_InCeil, sz); dr.kvInCeil = ParseColor(sz, fFalse);
        GetEdit(dc_InMtn,  sz); dr.kvInMtn  = ParseColor(sz, fFalse);
        GetEdit(dc_InCld,  sz); dr.kvInCld  = ParseColor(sz, fFalse);
        GetEdit(dc_InFog,  sz); dr.kvInFog  = ParseColor(sz, fFalse);
        dr.fSpreeRandom = GetCheck(dxCo_ms);
        Redraw(wi.hwnd);
      }
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing routine for the Random Settings dialog.

flag API DlgRandom(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  char sz[cchSzDef];
  int ns, nr;

  switch (msg) {
  case WM_INITDIALOG:
    SetEditN(deRa_s, ms.nRndSeed);
    SetEditN(deRa_b, ms.nRndBias);
    SetEditN(deRa_r, ms.nRndRun);
    SetFocus(GetDlgItem(hdlg, deRa_s));
    return fFalse;
  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      if (wParam == IDOK) {
        ns = GetEditN(deRa_s);
        nr = GetEditN(deRa_r);
        EnsureLeastN(nr, 0, "Random Run");
        ms.nRndBias = GetEditN(deRa_b);
        ms.nRndRun = nr;
        GetEdit(deRa_s, sz);
        if (sz[0] != chNull) {
          ms.nRndSeed = ns;
          InitRndL(ms.nRndSeed);
        }
      }
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing routine for the Dot Settings dialog.

flag API DlgDot(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  char sz[cchSzDef];
  int nd, nds, nra, x, y, i;
  KV kv;

  switch (msg) {
  case WM_INITDIALOG:
    SetEditN(deDs_x, dr.x);
    SetEditN(deDs_y, dr.y);
    SetEditN(deDs_z, dr.z);
    SetEditN(deDs_d, dr.dir);
    SetEditN(deDs_x2, dr.x2);
    SetEditN(deDs_y2, dr.y2);
    SetEditN(deDs_z2, dr.z2);
    SetCheck(dxDs_3, dr.f3D);
    SetCheck(dxDs_sd, dr.fDot);
    SetCheck(dxDs_dc, dr.fCircle);
    SetCheck(dxDs_da, dr.fArrow);
    SetEditN(deDs_ds, dr.nDotSize);
    SetCheck(dxDs_wi, dr.fWall);
    SetCheck(dxDs_nc, dr.fNoCorner);
    SetCheck(dxDs_gb, dr.fGlance);
    SetCheck(dxDs_cm, dr.fFollowMouse);
    SetCheck(dxDs_mc, dr.fNoDiagonal);
    SetCheck(dxDs_fp, dr.fFollow);
    SetEditN(deDs_ra, ms.nRadar);
    SetEditL(deDs_mc, dr.cMove);
    SetCheck(dxDs_mt, dr.fTwo);
    SetCheck(dxDs_dm, dr.fDrag);
    SetCheck(dxDs_de, !dr.fSet);
    SetCheck(dxDs_db, dr.fBig);
    SetCheck(dxDs_dt, dr.fBoth);
    SetCheck(dxDs_et, dr.fOnTemp);
    SetRadio(dr1 + dr.nEdge, dr1, dr3);
    if (!dr.f3D) {
      x = dr.x; y = dr.y;
    } else {
      x = bm.b.X2(dr.x, dr.z); y = bm.b.Y2(dr.y, dr.z);
    }
    if (!(bm.fColor ? bm.k.FLegal(x, y) : bm.b.FLegal(x, y)))
      sprintf(S(sz), "Outside the bitmap");
    else if (!bm.fColor)
      sprintf(S(sz), "%s", bm.b.Get(x, y) ? "On" : "Off");
    else {
      kv = bm.k.Get(x, y);
      if (!cs.fGraphNumber) {
        for (i = 0; i < cColor; i++)
          if (kv == rgkv[i])
            break;
        if (i >= cColor)
          sprintf(S(sz), "rgb %d %d %d", RgbR(kv), RgbG(kv), RgbB(kv));
        else
          sprintf(S(sz), "rgb %d %d %d (%s)", RgbR(kv), RgbG(kv), RgbB(kv),
            rgszColor[i]);
      } else
        sprintf(S(sz), "%d", kv);
    }
    SetDlgItemText(hdlg, ds1, sz);
    SetFocus(GetDlgItem(hdlg, deDs_x));
    return fFalse;
  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      if (wParam == IDOK) {
        nd = GetEditN(deDs_d);
        nds = GetEditN(deDs_ds);
        nra = GetEditN(deDs_ra);
        EnsureBetweenN(nd, 0, DIRS1, "Direction");
        EnsureLeastN(nds, 0, "Dot Size");
        EnsureLeastN(nra, 0, "Radar Length");
        dr.x = GetEditN(deDs_x);
        dr.y = GetEditN(deDs_y);
        dr.z = GetEditN(deDs_z);
        dr.dir = nd;
        dr.x2 = GetEditN(deDs_x2);
        dr.y2 = GetEditN(deDs_y2);
        dr.z2 = GetEditN(deDs_z2);
        dr.f3D = GetCheck(dxDs_3);
        dr.fDot = GetCheck(dxDs_sd);
        dr.fCircle = GetCheck(dxDs_dc);
        dr.fArrow = GetCheck(dxDs_da);
        dr.nDotSize = nds;
        dr.fWall = GetCheck(dxDs_wi);
        dr.fNoCorner = GetCheck(dxDs_nc);
        dr.fGlance = GetCheck(dxDs_gb);
        dr.fFollowMouse = GetCheck(dxDs_cm);
        dr.fNoDiagonal = GetCheck(dxDs_mc);
        dr.fFollow = GetCheck(dxDs_fp);
        ms.nRadar = nra;
        dr.cMove = GetEditL(deDs_mc);
        dr.fTwo = GetCheck(dxDs_mt);
        dr.fDrag = GetCheck(dxDs_dm);
        dr.fSet = !GetCheck(dxDs_de);
        dr.fBig = GetCheck(dxDs_db);
        dr.fBoth = GetCheck(dxDs_dt);
        dr.fOnTemp = GetCheck(dxDs_et);
        dr.nEdge = GetRadio(dr1, dr3);
        EnsureTimer(ws.nTimerDelay);
        ws.fMenuDirty = fTrue;
        Redraw(wi.hwnd);
      }
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing routine for the Inside Settings dialog.

flag API DlgInside(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  int nInside, nmt, ncl, nsi, ns, nsn, nsz, nss, nfd, ncp, nsw,
    nfm, nfr, nfu, nod;
  real rmr, rlf, rvs;

  switch (msg) {
  case WM_INITDIALOG:
    nInside = dr.fInside * dr.nInside; SetRadio(dr1 + nInside, dr1, dr6);
    SetCheck(dxIn_ci, dr.fCompass);
    SetCheck(dxIn_ca, dr.fCompassAngle);
    SetCheck(dxIn_li, dr.fLocation);
    SetCheck(dxIn_mo, dr.fMap);
    SetCheck(dxIn_se, dr.fSealEntrance);
    SetCheck(dxIn_mt, dr.fMountain);
    SetEditN(deIn_mt, dr.nPeakHeight);
    SetEditN(deIn_cl, dr.cCloud);
    SetCheck(dxIn_sm, dr.fSunMoon);
    SetCheck(dxIn_rb, dr.fRainbow);
    SetCheck(dxIn_si, ds.fStar);
    SetEditN(deIn_si, ds.cStar);
    SetEditR(deIn_mr, (real)dr.nMeteor / 100.0);
    SetCheck(dxIn_nw, dr.fNarrow);
    SetEditN(deIn_s, dr.zCell);
    SetEditN(deIn_sn, dr.zCellNarrow);
    SetEditN(deIn_sz, dr.zWall);
    SetEditN(deIn_z, dr.zElev);
    SetEditN(deIn_ss, dr.zStep);
    SetCheck(dxIn_tm, dr.fTexture);
    SetEditR(deIn_lf, dr.rLightFactor);
    SetEditN(deIn_fd, dr.nFog);
    SetEditN(deIn_cp, dr.nClip);
    SetEditR(deIn_vs, dr.dInside);
    SetEditN(deIn_sw, ds.nStereo);
    SetRadio(dr7 + dr.nTrans, dr7, dra);
    SetEditN(deIn_fm, dr.nFrameXY);
    SetEditN(deIn_fr, dr.nFrameD);
    SetEditN(deIn_fu, dr.nFrameZ);
    SetEditN(deIn_ox, dr.nOffsetX);
    SetEditN(deIn_oy, dr.nOffsetY);
    SetEditN(deIn_oz, dr.nOffsetZ);
    SetEditN(deIn_od, dr.nOffsetD);
    SetEditN(deIn_vm, dr.nSpeedXY);
    SetEditN(deIn_vr, dr.nSpeedD);
    SetEditN(deIn_vu, dr.nSpeedZ);
    SetCheck(dxIn_3, dr.f3DNotFree);
    SetFocus(GetDlgItem(hdlg, deIn_mt));
    return fFalse;
  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      if (wParam == IDOK) {
        nmt = GetEditN(deIn_mt);
        ncl = GetEditN(deIn_cl);
        nsi = GetEditN(deIn_si);
        rmr = GetEditR(deIn_mr);
        ns = GetEditN(deIn_s);
        nsn = GetEditN(deIn_sn);
        nsz = GetEditN(deIn_sz);
        nss = GetEditN(deIn_ss);
        rlf = GetEditR(deIn_lf);
        nfd = GetEditN(deIn_fd);
        ncp = GetEditN(deIn_cp);
        rvs = GetEditR(deIn_vs);
        nsw = GetEditN(deIn_sw);
        nfm = GetEditN(deIn_fm);
        nfr = GetEditN(deIn_fr);
        nfu = GetEditN(deIn_fu);
        nod = GetEditN(deIn_od);

        EnsureBetweenN(nmt, 0, 100, "Peak Height");
        EnsureBetweenN(ncl, 0, icldMax, "Cloud Count");
        EnsureBetweenN(nsi, 0, istarMax, "Star Count");
        EnsureBetweenR(rmr, 0, 50, "Meteor Rate");
        EnsureLeastN(ns, 1, "Cell Size");
        EnsureLeastN(nsn, 0, "Narrow Cell Size");
        EnsureLeastN(nsz, 1, "Wall Height");
        EnsureLeastN(nss, -1, "Step Height");
        EnsureBetweenR(rlf, -1, 1, "Light Factor");
        EnsureLeastN(nfd, 0, "Fog Distance");
        EnsureLeastN(ncp, 0, "Clip Plane");
        EnsureBetweenR(rvs, 1, 89, "Viewing Span");
        EnsureLeastN(nfm, 1, "Motion Frames");
        EnsureLeastN(nfr, 1, "Rotation Frames");
        EnsureLeastN(nfu, 1, "Up Down Frames");
        EnsureBetweenN(nod, -45, 45, "Direction Offset");

        nInside = GetRadio(dr1, dr6);
        dr.fInside = nInside > nInsideNone;
        if (dr.fInside)
          dr.nInside = nInside;
        dr.fCompass = GetCheck(dxIn_ci);
        dr.fCompassAngle = GetCheck(dxIn_ca);
        dr.fLocation = GetCheck(dxIn_li);
        dr.fMap = GetCheck(dxIn_mo);
        dr.fSealEntrance = GetCheck(dxIn_se);
        dr.fMountain = GetCheck(dxIn_mt);
        dr.nPeakHeight = nmt;
        dr.cCloud = ncl;
        dr.fSunMoon = GetCheck(dxIn_sm);
        dr.fRainbow = GetCheck(dxIn_rb);
        ds.fStar = GetCheck(dxIn_si);
        ds.cStar = nsi;
        dr.nMeteor = (int)(rmr * 100.0);
        dr.fNarrow = GetCheck(dxIn_nw);
        dr.zCell = ns;
        dr.zCellNarrow = nsn;
        dr.zWall = nsz;
        dr.zElev = GetEditN(deIn_z);
        dr.zStep = nss;
        dr.fTexture = GetCheck(dxIn_tm);
        dr.rLightFactor = rlf;
        dr.nFog = nfd;
        dr.nClip = ncp;
        dr.dInside = rvs;
        ds.nStereo = nsw;
        dr.nTrans = GetRadio(dr7, dra);
        dr.nFrameXY = nfm;
        dr.nFrameD = nfr;
        dr.nFrameZ = nfu;
        dr.nOffsetX = GetEditN(deIn_ox);
        dr.nOffsetY = GetEditN(deIn_oy);
        dr.nOffsetZ = GetEditN(deIn_oz);
        dr.nOffsetD = nod;
        dr.nSpeedXY = GetEditN(deIn_vm);
        dr.nSpeedD = GetEditN(deIn_vr);
        dr.nSpeedZ = GetEditN(deIn_vu);
        dr.f3DNotFree = GetCheck(dxIn_3);
        EnsureTimer(ws.nTimerDelay);
        ws.fMenuDirty = fTrue;
        Redraw(wi.hwnd);
      }
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing routine for the Size dialog.

flag API DlgSize(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  int nx, ny, nz, nw;
  CMap3 *b = PbFocus();

  switch (msg) {
  case WM_INITDIALOG:
    SetRadio(dr1, dr1, dr2);
    SetEditN(deSi_x, b->m_x);
    SetEditN(deSi_y, b->m_y);
    SetEditN(deSi_x3, b->m_x3);
    SetEditN(deSi_y3, b->m_y3);
    SetEditN(deSi_z3, b->m_z3);
    SetEditN(deSi_w3, b->m_w3);
    SetFocus(GetDlgItem(hdlg, deSi_x));
    return fFalse;
  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      if (wParam == IDOK) {
        nx = GetEditN(deSi_x3);
        ny = GetEditN(deSi_y3);
        nz = GetEditN(deSi_z3);
        nw = GetEditN(deSi_w3);
        EnsureLeastN(nx, 1, "3D Bitmap X");
        EnsureLeastN(ny, 1, "3D Bitmap Y");
        EnsureLeastN(nz, 1, "3D Bitmap Z");
        EnsureLeastN(nw, 1, "3D Bitmap W");
        b->m_x3 = nx;
        b->m_y3 = ny;
        b->m_z3 = nz;
        b->m_w3 = nw;
        DoSize(GetEditN(deSi_x), GetEditN(deSi_y),
          GetCheck(dr2), GetCheck(dxSi_c));
        DirtyView();
      }
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing routine for the Zoom dialog.

flag API DlgZoom(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  int x, y;
  real rx, ry;
  flag fPreserve, fTessellate;

  switch (msg) {
  case WM_INITDIALOG:
    SetRadio(dr1, dr1, dr2);
    SetEditN(deZo_x, 2);
    SetEditN(deZo_y, 2);
    SetRadio(dr3, dr3, dr5);
    SetFocus(GetDlgItem(hdlg, deZo_x));
    return fFalse;
  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      if (wParam == IDOK) {
        fPreserve = GetCheck(dr4);
        fTessellate = GetCheck(dr5);
        if (GetCheck(dr1)) {
          rx = GetEditR(deZo_x);
          ry = GetEditR(deZo_y);
          if (rx < 0.0)
            rx = -1.0 / rx;
          if (ry < 0.0)
            ry = -1.0 / ry;
          if (!fTessellate) {
            if (!bm.fColor)
              bm.b.FBitmapZoomBy(rx, ry, fPreserve);
            else
              bm.k.FBitmapZoomBy(rx, ry, fPreserve);
          } else {
            if (!bm.fColor)
              bm.b.FBitmapTessellateBy(rx, ry);
            else
              bm.k.FBitmapTessellateBy(rx, ry);
          }
        } else {
          x = GetEditN(deZo_x);
          y = GetEditN(deZo_y);
          if (x < 0)
            x = NAbs(bm.b.m_x + x);
          if (y < 0)
            y = NAbs(bm.b.m_y + y);
          if (!fTessellate) {
            if (!bm.fColor)
              bm.b.FBitmapZoomTo(x, y, fPreserve);
            else
              bm.k.FBitmapZoomTo(x, y, fPreserve);
          } else {
            if (!bm.fColor)
              bm.b.FBitmapTessellateTo(x, y);
            else
              bm.k.FBitmapTessellateTo(x, y);
          }
        }
        DirtyView();
      }
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing routine for the 3D Bitmap dialog.

flag API DlgCubeFlip(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  int i, j;

  switch (msg) {
  case WM_INITDIALOG:
    SetRadio(dr1, dr1, dr3);
    SetRadio(dr4, dr4, dr8);
    SetFocus(GetDlgItem(hdlg, dr1));
    return fFalse;
  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      if (wParam == IDOK) {
        i = GetRadio(dr1, dr3);
        j = GetRadio(dr4, dr8);
        PbFocus()->FCubeFlip2(i, j);
        DirtyView();
      }
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing routine for the Brightness dialog.

flag API DlgBright(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  real r;
  int nRadio, n;

  switch (msg) {
  case WM_INITDIALOG:
    SetRadio(dr1, dr1, dr4);
    SetEditN(deBr_v, 0);
    SetFocus(GetDlgItem(hdlg, dr1));
    return fFalse;
  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      if (wParam == IDOK) {
        nRadio = GetRadio(dr1, dr6);
        r = GetEditR(deBr_v);
        n = GetEditN(deBr_v);
        switch (nRadio) {
        case 0:
          EnsureBetweenR(r, -1, 1, "Brightness Factor");
          bm.k.ColmapBrightness(0, r, 0);
          break;
        case 1:
          EnsureBetweenN(n, -255, 255, "Brightness Offset");
          bm.k.ColmapBrightness(1, 0.0, n);
          break;
        case 2:
          EnsureBetweenR(r, -255, 255, "Brightness Multiplier");
          bm.k.ColmapBrightness(2, r, 0);
          break;
        case 3:
          EnsureBetweenN(n, -255*3, 255*3, "Brightness Limit");
          bm.k.ColmapBrightness(3, 0.0, n);
          break;
        case 4:
          bm.k.FColmapTransform(0, n);
          break;
        case 5:
          bm.k.FColmapTransform(1, n);
          break;
        }
        DirtyView();
      }
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing routine for the Replace Color dialog.

flag API DlgColorReplace(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  char sz[cchSzDef];
  KV kv1, kv2;
  flag fExchange;

  switch (msg) {
  case WM_INITDIALOG:
    SetEditColor(hdlg, dcRc_1, dr.kvOn);
    SetEditColor(hdlg, dcRc_2, dr.kvDot != dr.kvOn ? dr.kvDot : dr.kvOff);
    SetRadio(dr2 - cs.fRainbowBlend, dr1, dr2);
    SetEditN(deRc_s, cs.nRainbowStart);
    SetEditN(deRc_d, cs.nRainbowDistance);
    SetEditColor(hdlg, dcRc_3, cs.kvBlend1);
    SetEditColor(hdlg, dcRc_4, cs.kvBlend2);
    SetFocus(GetDlgItem(hdlg, dcRc_1));
    return fFalse;
  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      if (wParam == IDOK) {
        fExchange = GetCheck(dxRc_x);
        GetEdit(dcRc_1, sz); kv1 = ParseColor(sz, fFalse);
        GetEdit(dcRc_2, sz); kv2 = ParseColor(sz, !fExchange);
        cs.fRainbowBlend = GetCheck(dr1);
        cs.nRainbowStart = GetEditN(deRc_s);
        cs.nRainbowDistance = GetEditN(deRc_d);
        GetEdit(dcRc_3, sz); cs.kvBlend1 = ParseColor(sz, fFalse);
        GetEdit(dcRc_4, sz); cs.kvBlend2 = ParseColor(sz, fFalse);
        if (!fExchange)
          bm.k.ColmapReplace(kv1, kv2, dr.x, dr.y, dr.x2, dr.y2);
        else
          bm.k.ColmapExchange(kv1, kv2);
        DirtyView();
      }
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing routine for the Maze Size dialog.

flag API DlgSizeMaze(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  int x, y, nx, ny, nz, nw, f;
  CMap3 *b = PbFocus();

  switch (msg) {
  case WM_INITDIALOG:
    SetEditN(deSm_x, Max(b->m_x-1, 0) >> 1);
    SetEditN(deSm_y, Max(b->m_y-1, 0) >> 1);
    SetCheck(dxSm_e, !(FOdd(b->m_x) || FOdd(b->m_y)));
    SetEditN(deSm_x3, Max(b->m_x3-1, 0) >> 1);
    SetEditN(deSm_y3, Max(b->m_y3-1, 0) >> 1);
    SetEditN(deSm_z3, (b->m_z3 + 1) >> 1);
    SetEditN(deSm_w3, (b->m_w3 + 1) >> 1);
    SetFocus(GetDlgItem(hdlg, deSm_x));
    return fFalse;
  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      if (wParam == IDOK) {
        x = GetEditN(deSm_x);
        y = GetEditN(deSm_y);
        nx = GetEditN(deSm_x3);
        ny = GetEditN(deSm_y3);
        nz = GetEditN(deSm_z3);
        nw = GetEditN(deSm_w3);
        if (x < 0)
          x += (b->m_x - 1) >> 1;
        if (y < 0)
          y += (b->m_y - 1) >> 1;
        EnsureBetweenN(x, 0, (xBitmap - 1) >> 1, "Horizontal Size");
        EnsureBetweenN(y, 0, (yBitmap - 1) >> 1, "Vertical Size");
        EnsureLeastN(nx, 0, "3D Maze X");
        EnsureLeastN(ny, 0, "3D Maze Y");
        EnsureLeastN(nz, 0, "3D Maze Z");
        EnsureLeastN(nw, 0, "3D Maze W");
        f = 1 - GetCheck(dxSm_e);
        x = ((x + 1) << 1) - f;
        y = ((y + 1) << 1) - f;
        b->m_x3 = ((nx + 1) << 1) - f;
        b->m_y3 = ((ny + 1) << 1) - f;
        b->m_z3 = Max(nz << 1, 1);
        b->m_w3 = Max(nw << 1, 1);
        DoSize(x, y, fFalse, GetCheck(dxSm_c));
        DirtyView();
      }
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing routine for the Zoom Bias dialog.

flag API DlgBias(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  int x0, x1, y0, y1;

  switch (msg) {
  case WM_INITDIALOG:
    SetEditN(deBi_x0, 1);
    SetEditN(deBi_x1, 3);
    SetEditN(deBi_y0, 1);
    SetEditN(deBi_y1, 3);
    SetFocus(GetDlgItem(hdlg, deBi_x0));
    return fFalse;
  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      if (wParam == IDOK) {
        x0 = GetEditN(deBi_x0);
        x1 = GetEditN(deBi_x1);
        y0 = GetEditN(deBi_y0);
        y1 = GetEditN(deBi_y1);
        EnsureLeastN(x0, 0, "Even Horizontal Bias");
        EnsureLeastN(x1, 0, "Odd Horizontal Bias");
        EnsureLeastN(y0, 0, "Even Vertical Bias");
        EnsureLeastN(y1, 0, "Odd Vertical Bias");
        PbFocus()->FBitmapBias(x0, x1, y0, y1);
        DirtyView();
      }
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing routine for the Maze Settings dialog.

flag API DlgMaze(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  char sz[cchSzDef];
  int ntp, nww, nwp, nwr;

  switch (msg) {
  case WM_INITDIALOG:
    SetCheck(dxMa_pm, dr.fPolishMaze);
    SetCheck(dxMa_ad, ms.fSection);
    SetCheck(dxMa_te, ms.fTeleportEntrance);
    SetCheck(dxMa_cp, ms.fPoleNoDeadEnd);
    SetCheck(dxMa_ep, ms.fSolveEveryPixel);
    SetCheck(dxMa_de, ms.fSolveDotExit);
    SetCheck(dxMa_rp, ms.fRandomPath);
    SetCheck(dxMa_cs, ms.fCountShortest);
    sprintf(S(sz), "%03d", ms.nTweakPassage); SetEdit(deMa_tp, sz);
    SetEditL(deMa_mc, ms.cMaze);
    SetEditN(deMa_ww, ms.nWeaveOn);
    SetEditN(deMa_wp, ms.nWeaveOff);
    SetEditN(deMa_wr, ms.nWeaveRail);
    SetRadio(dr1 + ms.nEntrancePos, dr1, dr4);
    SetFocus(GetDlgItem(hdlg, deMa_tp));
    return fFalse;
  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      if (wParam == IDOK) {
        ntp = GetEditN(deMa_tp);
        nww = GetEditN(deMa_ww);
        nwp = GetEditN(deMa_wp);
        nwr = GetEditN(deMa_wr);
        EnsureBetweenN(ntp, 0, 999, "Tweak Passages Chances");
        EnsureLeastN(nww, 1, "Clarify Weave Wall Bias");
        EnsureLeastN(nwp, 1, "Clarify Weave Passage Bias");
        EnsureLeastN(nwr, 0, "Clarify Weave Railing Bias");
        dr.fPolishMaze = GetCheck(dxMa_pm);
        ms.fSection = GetCheck(dxMa_ad);
        ms.fTeleportEntrance = GetCheck(dxMa_te);
        ms.fPoleNoDeadEnd = GetCheck(dxMa_cp);
        ms.fSolveEveryPixel = GetCheck(dxMa_ep);
        ms.fSolveDotExit = GetCheck(dxMa_de);
        ms.fRandomPath = GetCheck(dxMa_rp);
        ms.fCountShortest = GetCheck(dxMa_cs);
        ms.nTweakPassage = ntp;
        ms.cMaze = GetEditL(deMa_mc);
        ms.nWeaveOn = nww;
        ms.nWeaveOff = nwp;
        ms.nWeaveRail = nwr;
        ms.nEntrancePos = GetRadio(dr1, dr4);
        DirtyView();
      }
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing routine for the Create Settings dialog.

flag API DlgCreate(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int no, no2, nos, nof, nt, nfi, nfa, nss, nsw, nfx, nfy, nfn,
    ncl, ncp, ncs, nvs, nts;
  flag f;

  switch (msg) {
  case WM_INITDIALOG:
    SetCheck(dxCr_r, ms.fRiver);
    SetCheck(dxCr_re, ms.fRiverEdge);
    SetCheck(dxCr_rf, ms.fRiverFlow);
    SetEditN(deCr_o, ms.omega);
    SetEditN(deCr_o2, ms.omega2);
    SetEditN(deCr_os, ms.omegas);
    SetEditN(deCr_of, ms.omegaf);
    SetRadio(dr1 + ms.nOmegaDraw, dr1, dr3);
    SetEdit(deCr_j, ms.szPlanair);
    SegmentGetSz(sz, cchSzMax); SetEdit(deCr_l, sz);
    SetCheck(dxCr_tw, ms.fTreeWall);
    SetCheck(dxCr_tr, ms.fTreeRandom);
    SetEditN(deCr_t, ms.nTreeRiver);
    SetEditN(deCr_fi, ms.nForsInit);
    SetEditN(deCr_fa, ms.nForsAdd);
    SetEditN(deCr_ss, ms.cSpiral);
    SetEditN(deCr_sw, ms.cSpiralWall);
    SetEditN(deCr_ra, ms.cRandomAdd);
    SetEditN(deCr_fx, ms.xFractal);
    SetEditN(deCr_fy, ms.yFractal);
    SetEditN(deCr_fn, ms.zFractal);
    SetCheck(dxCr_fi, ms.fFractalI);
    SetCheck(dxCr_c, ms.fCrackOff);
    SetEditN(deCr_cl, ms.nCrackLength);
    SetEditN(deCr_cp, ms.nCrackPass);
    SetEditN(deCr_cs, ms.nCrackSector);
    SetRadio(dr4 + ms.nBraid, dr4, dr6);
    SetEditN(deCr_vs, ms.nSparse);
    SetCheck(dxCr_kp, ms.fKruskalPic);
    SetCheck(dxCr_wc, ms.fWeaveCorner);
    SetCheck(dxCr_t, ms.fTiltDiamond);
    SetEditN(deCr_ts, ms.nTiltSize);
    SetFocus(GetDlgItem(hdlg, deCr_o));
    return fFalse;
  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      if (wParam == IDOK) {
        no = GetEditN(deCr_o);
        no2 = GetEditN(deCr_o2);
        nos = GetEditN(deCr_os);
        nof = GetEditN(deCr_of);
        GetEditCore(deCr_j, sz, cchSzMax);
        nt = GetEditN(deCr_t);
        nfi = GetEditN(deCr_fi);
        nfa = GetEditN(deCr_fa);
        nss = GetEditN(deCr_ss);
        nsw = GetEditN(deCr_sw);
        nfx = GetEditN(deCr_fx);
        nfy = GetEditN(deCr_fy);
        nfn = GetEditN(deCr_fn);
        ncl = GetEditN(deCr_cl);
        ncp = GetEditN(deCr_cp);
        ncs = GetEditN(deCr_cs);
        nvs = GetEditN(deCr_vs);
        nts = GetEditN(deCr_ts);
        f = GetCheck(dxCr_tr);

        if (!bm.b.FValidPlanair(sz))
          return fTrue;
        EnsureLeastN(no, 1, "Omega Dimensions");
        EnsureBetweenN(no2, 0, no-1, "Omega Inner Dimensions");
        EnsureBetweenN(nos, -1, 15999, "Omega Start Location");
        EnsureBetweenN(nof, -1, 15999, "Omega Finish Location");
        if (f)
          EnsureLeastN(nt, 0, "Tree Random Chance");
        EnsureBetweenN(nss, 1, SpiralMax, "Max Spirals");
        EnsureBetweenN(nsw, 1, SpiralWallMax, "Max Spiral Walls");
        EnsureLeastN(nfx, 2, "Fractal X");
        EnsureLeastN(nfy, 2, "Fractal Y");
        EnsureLeastN(nfn, 1, "Fractal Nesting Level");
        EnsureLeastN(ncl, 2, "Crack Line Length");
        EnsureLeastN(ncp, 1, "Crack Pass Limit");
        EnsureBetweenN(ncs, -360, 360, "Crack Line Sectors");
        EnsureLeastN(nvs, 0, "Cavern Sparseness");

        ms.fRiver = GetCheck(dxCr_r);
        ms.fRiverEdge = GetCheck(dxCr_re);
        ms.fRiverFlow = GetCheck(dxCr_rf);
        ms.omega = no;
        ms.omega2 = no2;
        ms.omegas = nos;
        ms.omegaf = nof;
        ms.nOmegaDraw = GetRadio(dr1, dr3);
        GetEditCore(deCr_j, ms.szPlanair, cchSzMax);
        GetEditCore(deCr_l, sz, cchSzMax); SegmentParseSz(sz);
        ms.fTreeWall = GetCheck(dxCr_tw);
        ms.fTreeRandom = f;
        ms.nTreeRiver = nt;
        ms.nForsInit = nfi;
        ms.nForsAdd = nfa;
        ms.cSpiral = nss;
        ms.cSpiralWall = nsw;
        ms.cRandomAdd = GetEditN(deCr_ra);
        ms.xFractal = nfx;
        ms.yFractal = nfy;
        ms.zFractal = nfn;
        ms.fFractalI = GetCheck(dxCr_fi);
        ms.fCrackOff = GetCheck(dxCr_c);
        ms.nCrackLength = ncl;
        ms.nCrackPass = ncp;
        ms.nCrackSector = ncs;
        ms.nBraid = GetRadio(dr4, dr6);
        ms.nSparse = nvs;
        ms.fKruskalPic = GetCheck(dxCr_kp);
        ms.fWeaveCorner = GetCheck(dxCr_wc);
        ms.fTiltDiamond = GetCheck(dxCr_t);
        ms.nTiltSize = nts;
      }
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing routine for the Labyrinth Settings dialog.

flag API DlgLabyrinth(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  char sz[cchSzDef], szT[cchSzDef];
  int ncs, nfa, naa, npc, npr, ncc;
  flag f;

  switch (msg) {
  case WM_INITDIALOG:
    SetRadio(dr1 + ms.nLabyrinth, dr1, dr6);
    SetRadio(dr7 + ms.lcs, dr7, dra);
    SetEditN(deLa_cs,
      ms.fClassicalCirc ? ms.nClassical*4 + 3 : ms.nClassical);
    SetCheck(dxLa_sc, ms.fClassicalCirc);
    SetEdit(deLa_cp, ms.szCustom);
    SetCheck(dxLa_ca, ms.fCustomAuto);
    SetEditN(deLa_fa, ms.nCustomFold);
    SetEditN(deLa_aa, ms.nCustomAsym);
    SetEditN(deLa_pc, ms.nCustomPartX);
    SetEditN(deLa_pr, ms.nCustomPartY);
    SetRadio(drb + ms.lcc, drb, drf);
    SetEditN(deLa_cc, ms.zCustomMiddle);
    SetFocus(GetDlgItem(hdlg, deLa_cp));
    return fFalse;
  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      if (wParam == IDOK) {
        GetEditCore(deLa_cp, szT, cchSzDef);
        if (!FCopyRgchToSzCustom(szT, CchSz(szT), sz)) {
          PrintSz_W("The Custom Paths field is invalid.");
          return fTrue;
        }
        ncs = GetEditN(deLa_cs);
        f = GetCheck(dxLa_sc);
        nfa = GetEditN(deLa_fa);
        naa = GetEditN(deLa_aa);
        npc = GetEditN(deLa_pc);
        npr = GetEditN(deLa_pr);
        ncc = GetEditN(deLa_cc);

        if (!FValidSzCustom(sz, NULL, NULL))
          return fTrue;
        EnsureLeastN(ncs, 0, "Classical Size");
        EnsureBetweenN(nfa, 0, 2, "Fanfolds Allowed");
        EnsureBetweenN(naa, 0, 2, "Asymmetry Allowed");
        EnsureBetweenN(npc, 0, 2, "Circuit Partitioning");
        EnsureBetweenN(npr, 0, 2, "Radius Partitioning");
        EnsureLeastN(ncc, 0, "Custom Center Size");

        ms.nLabyrinth = GetRadio(dr1, dr6);
        ms.lcs = GetRadio(dr7, dra);
        ms.nClassical = f ? ncs / 4 : ncs;
        ms.fClassicalCirc = f;
        CopySz(sz, ms.szCustom, cchSzDef);
        ms.fCustomAuto = GetCheck(dxLa_ca);
        ms.nCustomFold = nfa;
        ms.nCustomAsym = naa;
        ms.nCustomPartX = npc;
        ms.nCustomPartY = npr;
        ms.lcc = GetRadio(drb, drf);
        ms.zCustomMiddle = ncc;
      }
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing routine for the Draw Settings dialog.

flag API DlgDraw(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  int nth, nph;

  switch (msg) {
  case WM_INITDIALOG:
    SetEditN(deDr_x, ds.horsiz);
    SetEditN(deDr_y, ds.versiz);
    SetEditN(deDr_z, ds.depsiz);
    SetEditN(deDr_bx, ds.horbias);
    SetEditN(deDr_by, ds.verbias);
    SetEditN(deDr_mx, ds.hormin);
    SetEditN(deDr_my, ds.vermin);
    SetEditN(deDr_mz, ds.depmin);
    SetEditN(deDr_vx, ds.horv);
    SetEditN(deDr_vy, ds.verv);
    SetEditN(deDr_th, ds.theta);
    SetEditN(deDr_ph, ds.phi);
    SetEditN(deDr_sm, ds.speedm);
    SetEditN(deDr_sr, ds.speedr);
    SetCheck(dxDr_ri, ds.fRight);
    SetCheck(dxDr_mc, ds.fMerge);
    SetCheck(dxDr_ak, ds.fArrow);
    SetFocus(GetDlgItem(hdlg, deDr_x));
    return fFalse;
  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      if (wParam == IDOK) {
        nth = GetEditN(deDr_th);
        nph = GetEditN(deDr_ph);
        EnsureBetweenN(nth, 0, nDegMax-1, "Viewing Angle Theta");
        EnsureBetweenN(nph, -nDegHalf, nDegHalf-1, "Vertical Pitch Phi");
        ds.horsiz = GetEditN(deDr_x);
        ds.versiz = GetEditN(deDr_y);
        ds.depsiz = GetEditN(deDr_z);
        ds.horbias = GetEditN(deDr_bx);
        ds.verbias = GetEditN(deDr_by);
        ds.hormin = GetEditN(deDr_mx);
        ds.vermin = GetEditN(deDr_my);
        ds.depmin = GetEditN(deDr_mz);
        ds.horv = GetEditN(deDr_vx);
        ds.verv = GetEditN(deDr_vy);
        ds.theta = nth;
        ds.phi = nph;
        ds.speedm = GetEditN(deDr_sm);
        ds.speedr = GetEditN(deDr_sr);
        ds.fRight = GetCheck(dxDr_ri);
        ds.fMerge = GetCheck(dxDr_mc);
        ds.fArrow = GetCheck(dxDr_ak);
        EnsureTimer(ws.nTimerDelay);
      }
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing routine for the Obscure Draw Settings dialog.

flag API DlgDraw2(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  char sz[cchSzDef];
  int nbs, nhs;

  switch (msg) {
  case WM_INITDIALOG:
    SetCheck(dxDo_r, ds.fReflect);
    SetCheck(dxDo_ss, ds.fSkyShade);
    SetCheck(dxDo_gs, ds.fGroundShade);
    SetCheck(dxDo_e, ds.fEdges);
    SetCheck(dxDo_s, ds.fShading);
    SetCheck(dxDo_t, ds.fTouch);
    SetEditN(deDo_bs, ds.nBorder);
    SetEditN(deDo_hs, ds.nHoriz);
    SetEditN(deDo_s, (int)ds.rScale);
    SetEditN(deDo_xs, (int)ds.rxScale);
    SetEditN(deDo_ys, (int)ds.ryScale);
    SetEditN(deDo_zs, (int)ds.rzScale);
    SetEditColor(hdlg, dcDo_tc, ds.kvTrim);
    SetEditColor(hdlg, dcDo_sch, ds.kvSkyHi);
    SetEditColor(hdlg, dcDo_scl, ds.kvSkyLo);
    SetEditColor(hdlg, dcDo_gch, ds.kvGroundHi);
    SetEditColor(hdlg, dcDo_gcl, ds.kvGroundLo);
    SetEditColor(hdlg, dcDo_oc, ds.kvObject);
    SetEditR(deDo_vx, ds.vLight.m_x);
    SetEditR(deDo_vy, ds.vLight.m_y);
    SetEditR(deDo_vz, ds.vLight.m_z);
    SetFocus(GetDlgItem(hdlg, deDo_bs));
    return fFalse;
  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      if (wParam == IDOK) {
        nbs = GetEditN(deDo_bs);
        nhs = GetEditN(deDo_hs);
        EnsureLeastN(nbs, 0, "Border Width");
        EnsureLeastN(nhs, 0, "Horizon Width");
        ds.fReflect = GetCheck(dxDo_r);
        ds.fSkyShade = GetCheck(dxDo_ss);
        ds.fGroundShade = GetCheck(dxDo_gs);
        ds.fEdges = GetCheck(dxDo_e);
        ds.fShading = GetCheck(dxDo_s);
        ds.fTouch = GetCheck(dxDo_t);
        ds.nBorder = nbs;
        ds.nHoriz = nhs;
        ds.rScale = GetEditR(deDo_s);
        ds.rxScale = GetEditR(deDo_xs);
        ds.ryScale = GetEditR(deDo_ys);
        ds.rzScale = GetEditR(deDo_zs);
        GetEdit(dcDo_tc, sz); ds.kvTrim = ParseColor(sz, fFalse);
        GetEdit(dcDo_sch, sz); ds.kvSkyHi = ParseColor(sz, fFalse);
        GetEdit(dcDo_scl, sz); ds.kvSkyLo = ParseColor(sz, fFalse);
        GetEdit(dcDo_gch, sz); ds.kvGroundHi = ParseColor(sz, fFalse);
        GetEdit(dcDo_gcl, sz); ds.kvGroundLo = ParseColor(sz, fFalse);
        GetEdit(dcDo_oc, sz); ds.kvObject = ParseColor(sz, fFalse);
        ds.vLight.m_x = GetEditR(deDo_vx);
        ds.vLight.m_y = GetEditR(deDo_vy);
        ds.vLight.m_z = GetEditR(deDo_vz);
      }
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing routine for the GetString operation dialog.

flag API DlgString(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  char sz[cchSzMax], *pch;

  switch (msg) {
  case WM_INITDIALOG:
    pch = NULL;
    if (ws.nTitleMessage >= 0)
      pch = SzVar(ws.nTitleMessage);
    if (pch == NULL || *pch == chNull)
      pch = ws.szAppName;
    SetWindowText(hdlg, pch);
    SetDlgItemText(hdlg, ds1, ws.szDialog);
    SetEdit(de1, SzVar(ws.iszDialog));
    SetFocus(GetDlgItem(hdlg, de1));
    return fFalse;
  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      if (wParam == IDOK) {
        GetEditCore(de1, sz, cchSzMax);
        SetSzVar(sz, -1, ws.iszDialog);
      } else
        ws.iszDialog = -1;
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing routine for the GetString2 operation dialog.

flag API DlgString2(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  char sz[cchSzMax], *pch;

  switch (msg) {
  case WM_INITDIALOG:
    pch = NULL;
    if (ws.nTitleMessage >= 0)
      pch = SzVar(ws.nTitleMessage);
    if (pch == NULL || *pch == chNull)
      pch = ws.szAppName;
    SetWindowText(hdlg, pch);
    SetDlgItemText(hdlg, ds1, ws.szDialog);
    SetDlgItemText(hdlg, ds2, ws.szDialog2);
    SetEdit(de1, SzVar(ws.iszDialog));
    SetEdit(de2, SzVar(ws.iszDialog + 1));
    SetFocus(GetDlgItem(hdlg, de1));
    return fFalse;
  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      if (wParam == IDOK) {
        GetEditCore(de1, sz, cchSzMax);
        SetSzVar(sz, -1, ws.iszDialog);
        GetEditCore(de2, sz, cchSzMax);
        SetSzVar(sz, -1, ws.iszDialog + 1);
      } else
        ws.iszDialog = -1;
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing routine for the GetString2 operation dialog.

flag API DlgString3(HWND hdlg, uint msg, WORD wParam, LONG lParam)
{
  char sz[cchSzMax], *pch;

  switch (msg) {
  case WM_INITDIALOG:
    pch = NULL;
    if (ws.nTitleMessage >= 0)
      pch = SzVar(ws.nTitleMessage);
    if (pch == NULL || *pch == chNull)
      pch = ws.szAppName;
    SetWindowText(hdlg, pch);
    SetDlgItemText(hdlg, ds1, ws.szDialog);
    SetDlgItemText(hdlg, ds2, ws.szDialog2);
    SetDlgItemText(hdlg, ds3, ws.szDialog3);
    SetEdit(de1, SzVar(ws.iszDialog));
    SetEdit(de2, SzVar(ws.iszDialog + 1));
    SetEdit(de3, SzVar(ws.iszDialog + 2));
    SetFocus(GetDlgItem(hdlg, de1));
    return fFalse;
  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      if (wParam == IDOK) {
        GetEditCore(de1, sz, cchSzMax);
        SetSzVar(sz, -1, ws.iszDialog);
        GetEditCore(de2, sz, cchSzMax);
        SetSzVar(sz, -1, ws.iszDialog + 1);
        GetEditCore(de3, sz, cchSzMax);
        SetSzVar(sz, -1, ws.iszDialog + 2);
      } else
        ws.iszDialog = -1;
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}
#endif // WIN

/* wdialog.cpp */
