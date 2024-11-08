/*
** Daedalus (Version 3.5) File: wutil.cpp
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
** This file contains Windows specific utilities and operations for Daedalus.
**
** Created: 11/16/2002.
** Last code change: 10/30/2024.
*/

#include <windows.h>
#include <objbase.h>
#include <comdef.h>
#include <comdefsp.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <shlwapi.h>
#include <stdio.h>
#include <math.h>

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
/*
******************************************************************************
** Windows Routines
******************************************************************************
*/

// Set the window display bitmap to the contents of the active bitmap. This
// usually gets called whenever the active bitmap changes in a major way. The
// window bitmap is designed to avoid flicker in screen updates.

void CopyBitmapToDeviceBitmap()
{
  HDC hdc;

  if (!bm.fColor) {
    if (bm.b.m_x != bm.xa || bm.b.m_y != bm.ya) {
      SelectObject(wi.hdcBmp, wi.hbmpPrev);
      DeleteObject(wi.hbmp);
      wi.hbmp = CreateCompatibleBitmap(wi.hdcBmp, bm.b.m_x, bm.b.m_y);
      if (wi.hbmp == NULL)
        PrintSz_E("Failed to create monochrome display bitmap.");
      wi.hbmpPrev = (HBITMAP)SelectObject(wi.hdcBmp, wi.hbmp);
      bm.xa = bm.b.m_x;
      bm.ya = bm.b.m_y;
    }
    wi.bi.biWidth = bm.xa;
    wi.bi.biHeight = -bm.ya;
    SetDIBitsToDevice(wi.hdcBmp, 0, 0, bm.xa, bm.ya, 0, 0, 0, bm.ya,
      bm.b.m_rgb, (BITMAPINFO *)&wi.bi, DIB_RGB_COLORS);
  } else {
    if (bm.k.m_x != bm.xaC || bm.k.m_y != bm.yaC) {
      SelectObject(wi.hdcBmp, wi.hbmpPrev);
      if (wi.hbmpC != NULL)
        DeleteObject(wi.hbmpC);
      hdc = GetDC(wi.hwnd);
      wi.hbmpC = CreateCompatibleBitmap(hdc, bm.k.m_x, bm.k.m_y);
      if (wi.hbmpC == NULL)
        PrintSz_E("Failed to create color display bitmap.");
      ReleaseDC(wi.hwnd, hdc);
      wi.hbmpPrev = (HBITMAP)SelectObject(wi.hdcBmp, wi.hbmpC);
      bm.xaC = bm.k.m_x;
      bm.yaC = bm.k.m_y;
    }
    wi.biC.biWidth = bm.xaC;
    wi.biC.biHeight = -bm.yaC;
    SetDIBitsToDevice(wi.hdcBmp, 0, 0, bm.xaC, bm.yaC, 0, 0, 0, bm.yaC,
      bm.k.m_rgb, (BITMAPINFO *)&wi.biC, DIB_RGB_COLORS);
  }
}


// The active bitmap being viewed has changed content in a major way. This
// usually means to go update the screen.

void DirtyView()
{
  if (!ws.fNoDirty)
    return;
  CopyBitmapToDeviceBitmap();
  Redraw(wi.hwnd);
}


// Change a pixel in both the main bitmap and on the screen. This updates just
// the pixel in question as opposed to redrawing the whole bitmap.

void BitmapDotCore(int x, int y, bit o)
{
  if (!bm.b.FLegal(x, y))
    return;
  bm.b.Set(x, y, o);
  if (!bm.fColor && ws.fNoDirty)
    SetPixel(wi.hdcBmp, x, y, o ? kvWhite : kvBlack);
}


// Draw a spot, updating bitmaps and screen appropriately. This is called from
// the Set At Dot command and when moving the dot when Drag Is Erase is set.

void BitmapDot(int x, int y)
{
  int x2, y2;

  if (dr.fOnTemp) {

    // Draw a spot on the temporary bitmap.
    if (bm.b2.FNull())
      bm.b2.FBitmapCopy(bm.b);
    if (!bm.b2.FNull()) {
      if (!dr.fBig)
        bm.b2.Set(x, y, dr.fSet);
      else
        bm.b2.Block(x-1, y-1, x+1, y+1, dr.fSet);
    }
  } else if (!bm.fColor) {

    // Draw a spot on the main bitmap and on the screen.
    if (!dr.fBig)
      BitmapDotCore(x, y, dr.fSet);
    else {
      bm.b.Block(x-1, y-1, x+1, y+1, dr.fSet);
      PatBlt(wi.hdcBmp, x-1, y-1, 3, 3, dr.fSet ? WHITENESS : BLACKNESS);
    }
  } else {

    // Draw a spot on the color bitmap and on the screen.
    if (!bm.k.FNull() && bm.k.FLegal(x, y)) {
      if (!dr.fBig) {
        bm.k.Set(x, y, kvSet);
        SetPixel(wi.hdcBmp, x, y, kvSet);
      } else {
        bm.k.Block(x-1, y-1, x+1, y+1, kvSet);
        for (y2 = y-1; y2 <= y+1; y2++)
          for (x2 = x-1; x2 <= x+1; x2++)
            SetPixel(wi.hdcBmp, x2, y2, kvSet);
      }
    }
  }
}


// Launch an external application or open a Daedalus script, passing it the
// given file. This is called from the Open Documentation command, commands
// on the More Help submenu, and commands on the Run Script submenu.

flag FBootExternal(CONST char *szFile, flag fInternal)
{
  FILE *file;
  char szT[cchSzDef], *pch;

  // The file will be looked for in the directory of the Daedalus executable.
  GetModuleFileName(wi.hinst, szT, cchSzDef);
  for (pch = szT; *pch; pch++)
    ;
  while (pch > szT && *pch != '\\')
    pch--;
  if (*pch == '\\')
    pch++;
  sprintf(SO(pch, szT), "%s", szFile);
  file = FileOpen(szT, "r");
  if (file != NULL)
    goto LDone;

  // Check if the file exists before launching the external app.
  sprintf(S(szT), "%s", szFile);
  file = FileOpen(szT, "r");
  if (file != NULL)
    goto LDone;

  if (!fInternal) {
    sprintf(S(szT), "File '%s' not found.", szFile);
    PrintSz_E(szT);
  }
  return fFalse;

LDone:
  fclose(file);
  if (!fInternal)
    ShellExecute(NULL, NULL, szT, NULL, NULL, SW_SHOW);
  else
    FFileOpen(cmdOpenScript, szT, NULL);
  return fTrue;
}


// Set the delay of the Windows timer event, as controlled with the Repeat
// Delay In Msec setting.

void EnsureTimer(int nDelay)
{
  flag fWantTimer = FWantTimer();

  // Kill the existing timer, if a timer is no longer needed, or the desired
  // delay is different from the existing delay.
  if (ws.lTimer != 0 &&
    (nDelay < 0 || !fWantTimer || (fWantTimer && nDelay != ws.nTimerDelay))) {
    KillTimer(wi.hwndMain, 1);
    ws.lTimer = 0;
  }

  // Create a new timer, if one is needed and there's no active timer.
  if (ws.lTimer == 0 && nDelay >= 0 && fWantTimer)
    ws.lTimer = SetTimer(wi.hwndMain, 1, Max(nDelay, 1), NULL);
  ws.nTimerDelay = nDelay;
}


// Get the number of milliseconds that have passed. Implements the Get Timer
// command and the Timer and Timer2 functions.

ulong GetTimer()
{
  DWORD l;

  if (ws.fTimePause)
    l = ws.timePause;
  else
    l = GetTickCount();
  return l - ws.timeReset;
}


// Pause or unpause the timer. Implements the Pause Timer command.

void PauseTimer(flag f)
{
  DWORD l;

  if (f == ws.fTimePause)
    return;
  ws.fTimePause = f;
  CheckMenu(cmdTimePause, ws.fTimePause);
  if (ws.fTimePause)
    ws.timePause = GetTickCount();
  else {
    l = GetTickCount();
    ws.timeReset += (l - ws.timePause);
  }
}


// For each menu command that can have a check mark by it, determine its state
// and set or clear the check appropriately. This is called after actions that
// may change many settings at once, such as closing certain dialogs.

void RedoMenu(HWND hwnd)
{
  if (!ws.fMenuDirty)
    return;
  ws.fMenuDirty = fFalse;

  CheckMenu(cmdSpree, ws.fSpree);
  CheckMenu(cmdMessage, ws.fIgnorePrint);
  CheckMenu(cmdTimePause, ws.fTimePause);
  CheckMenu(cmdDotShow, dr.fDot);
  CheckMenu(cmdDotWall, dr.fWall);
  CheckMenu(cmdDotTwo, dr.fTwo);
  CheckMenu(cmdDotDrag, dr.fDrag);
  CheckMenu(cmdDotOn, !dr.fSet);
  CheckMenu(cmdDotBig, dr.fBig);
  CheckMenu(cmdDotBoth, dr.fBoth);
  CheckMenu(cmdInsideView, dr.fInside);
  CheckMenu(cmdInsideMap, dr.fMap);
  CheckMenu(cmd3D, dr.f3D);
  CheckMenu(cmdColmap, bm.fColor);
  CheckMenu(cmdPolishMaze, dr.fPolishMaze);
  DrawMenuBar(hwnd);
}


// Get the screen coordinates of the upper left corner of the Daedalus window
// and the pixel size of the window.

void GetWindowPosition(int *xo, int *yo, int *xs, int *ys)
{
  RECT rc;
  POINT pt;

  if (ws.fResizeClient) {

    // Units relative to upper left corner of the drawable or client area.
    *xs = ws.xpClient; *ys = ws.ypClient;
    pt.x = pt.y = 0;
    ClientToScreen(wi.hwnd, &pt);
    *xo = pt.x; *yo = pt.y;
  } else {

    // Units relative to upper left corner of the outer window frame.
    GetWindowRect(wi.hwnd, &rc);
    *xs = rc.right - rc.left; *ys = rc.bottom - rc.top;
    *xo = rc.left; *yo = rc.top;
  }
}


// Convert drawable or client area coordinates to outer window frame
// coordinates, for the upper left corner and size of the Daedalus window.

void AdjustWindowPosition(int *xo, int *yo, int *xs, int *ys)
{
  RECT rc;
  POINT pt;

  pt.x = pt.y = 0;
  ClientToScreen(wi.hwnd, &pt);
  GetWindowRect(wi.hwnd, &rc);
  *xs += (rc.right - rc.left - ws.xpClient);
  *ys += (rc.bottom - rc.top - ws.ypClient);
  *xo += rc.left - pt.x;
  *yo += rc.top - pt.y;
}


// Draw a string of text at the appropriate row in the inside display mode.

void DrawTextLine(HDC hdc, CONST char *sz, int cch, int y)
{
  SIZE size;
  KV kvSav1, kvSav2;
  int nSav, yp;

  GetTextExtentPoint(hdc, sz, cch, &size);
  kvSav1 = GetTextColor(hdc); kvSav2 = GetBkColor(hdc);
  SetTextColor(hdc, dr.kvOff); SetBkColor(hdc, dr.kvOn);
  if (dr.kvOff == dr.kvOn)
    nSav = SetBkMode(hdc, TRANSPARENT);
  yp = size.cy*y + (y < 0)*ws.ypClient;
  if (ds.nStereo == 0 || ds.fStereo3D) {
    TextOut(hdc, (ws.xpClient >> 1) - (size.cx >> 1), yp, sz, cch);
  } else {
    TextOut(hdc, (ws.xpClient >> 2) - (size.cx >> 1), yp, sz, cch);
    TextOut(hdc, (ws.xpClient*3 >> 2) - (size.cx >> 1), yp, sz, cch);
  }
  if (dr.kvOff == dr.kvOn)
    SetBkMode(hdc, nSav);
  SetTextColor(hdc, kvSav1); SetBkColor(hdc, kvSav2);
}


// Draw a line of text on the active bitmap, using a given Windows font.

void DrawTextLine2(HDC hdc, CONST char *szText, int cchText, int xp, int yp,
  CONST char *szFont, int nFontSize, int nFontWeight)
{
  HDC hdcT;
  HBITMAP hbmp, hbmpPrev;
  HFONT hfont, hfontPrev;
  SIZE size;
  KV kvSav1, kvSav2;
  CMap *pb = PbFocus();
  int x, y;

  // Setup Windows font and a context to draw the text.
  hdcT = CreateCompatibleDC(hdc);
  if (hdcT == NULL)
    return;
  hfont = CreateFont(nFontSize, 0, 0, 0, nFontWeight, fFalse, fFalse, fFalse,
    ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, pb->m_cfPix > 1 ?
    ANTIALIASED_QUALITY | PROOF_QUALITY : NONANTIALIASED_QUALITY |
    DRAFT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, szFont);
  if (hfont == NULL)
    return;
  hfontPrev = (HFONT)SelectObject(hdcT, hfont);
  GetTextExtentPoint(hdcT, szText, cchText, &size);
  hbmp = CreateCompatibleBitmap(hdc, size.cx, size.cy);
  if (hbmp == NULL)
    return;
  hbmpPrev = (HBITMAP)SelectObject(hdcT, hbmp);
  kvSav1 = GetTextColor(hdcT); kvSav2 = GetBkColor(hdcT);
  SetTextColor(hdcT, dr.kvOff); SetBkColor(hdcT, dr.kvOn);

  // Draw the text, then copy it to the Daedalus bitmap.
  TextOut(hdcT, 0, 0, szText, cchText);
  for (y = 0; y < size.cy; y++)
    for (x = 0; x < size.cx; x++)
      pb->Set(xp + x, yp + y, GetPixel(hdcT, x, y));

  SetTextColor(hdcT, kvSav1); SetBkColor(hdcT, kvSav2);
  SelectObject(hdcT, hfontPrev);
  SelectObject(hdcT, hbmpPrev);
  DeleteObject(hbmp);
  DeleteObject(hfont);
  DeleteDC(hdcT);
}


/*
******************************************************************************
** Setup
******************************************************************************
*/

// Create a Windows shortcut file, a link pointing to another file. Called
// from FCreateProgramGroup() and FCreateDesktopIcon() to setup the program.

flag FCreateShortcut(CONST char *szDir, CONST char *szName,
  CONST char *szFile, CONST char *szDesc, int nIcon)
{
  IShellLinkA *pisl = NULL;
  IPersistFile *pipf = NULL;
  char sz[cchSzMax], *pch;
  WCHAR wsz[cchSzMax];
  HRESULT hr;
  flag fRet = fFalse;

  // Define the link data.
  hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
    IID_IShellLink, (LPVOID *)&pisl);
  if (FAILED(hr))
    goto LExit;
  GetModuleFileName(wi.hinst, sz, cchSzDef);
  if (nIcon >= 0) {
    hr = pisl->SetIconLocation(sz, nIcon);
    if (FAILED(hr))
      goto LExit;
  }
  for (pch = sz; *pch; pch++)
    ;
  while (pch > sz && *pch != '\\')
    pch--;
  *pch = chNull;
  hr = pisl->SetWorkingDirectory(sz);
  if (FAILED(hr))
    goto LExit;
  sprintf(SO(pch, sz), "\\%s", szFile);
  hr = pisl->SetPath(sz);
  if (FAILED(hr))
    goto LExit;
  if (szDesc == NULL)
    szDesc = szName;
  hr = pisl->SetDescription(szDesc);
  if (FAILED(hr))
    goto LExit;

  // Save it to a link file.
  hr = pisl->QueryInterface(IID_IPersistFile, (LPVOID *)&pipf);
  if (FAILED(hr))
    goto LExit;
  sprintf(S(sz), "%s", szDir);
  for (pch = sz; *pch; pch++)
    ;
  sprintf(SO(pch, sz), "\\%s.lnk", szName);
  MultiByteToWideChar(CP_ACP, 0, sz, -1, wsz, cchSzMax);
  hr = pipf->Save(wsz, fTrue);
  if (FAILED(hr))
    goto LExit;

  fRet = fTrue;
LExit:
  if(pipf)
    pipf->Release();
  if(pisl)
    pisl->Release();
  return fRet;
}


// Delete a Windows shortcut file, a link pointing to another file. Called
// from FCreateProgramGroup() to setup the program.

void DeleteShortcut(CONST char *szDir, CONST char *szFile)
{
  char sz[cchSzMax];

  sprintf(S(sz), "%s\\%s.lnk", szDir, szFile);
  _unlink(sz);
}


// Add an icon pointing to the Daedalus executable on the Windows desktop.

flag FCreateDesktopIcon()
{
  char szDir[cchSzMax], szExe[cchSzMax], szName[cchSzMax], *pch;
  LPITEMIDLIST pidl;
  FILE *file;

  // Create standard icon pointing to main Daedalus program.
  SHGetSpecialFolderLocation(wi.hwnd, CSIDL_DESKTOPDIRECTORY, &pidl);
  SHGetPathFromIDList(pidl, szDir);
  sprintf(S(szName), "%s %s", szDaedalus, szVersion);
  if (!FCreateShortcut(szDir, szName,
    "daedalus.exe", "Daedalus executable", 0))
    return fFalse;

  // Delete old previous version shortcuts from desktop.
  DeleteShortcut(szDir, "Daedalus 2.3");
  DeleteShortcut(szDir, "Daedalus 2.4");
  DeleteShortcut(szDir, "Daedalus 2.5");
  DeleteShortcut(szDir, "Daedalus 3.0");
  DeleteShortcut(szDir, "Daedalus 3.1");
  DeleteShortcut(szDir, "Daedalus 3.2");
  DeleteShortcut(szDir, "Daedalus 3.3");

  // Check whether hunger subdirectory exists with texture in it.
  GetModuleFileName(wi.hinst, szExe, cchSzMax);
  for (pch = szExe; *pch; pch++)
    ;
  while (pch > szExe && *pch != '\\')
    pch--;
  sprintf(SO(pch, szExe), "\\hunger\\Rock.bmp");
  file = FileOpen(szExe, "r");
  if (file == NULL)
    return fTrue;
  fclose(file);

  // Create icon pointing to hunger.ds script.
  sprintf(S(szName), "%s %s", rgszShortcut[8], szVersion);
  return FCreateShortcut(szDir, szName,
    rgszScript[8], "Hunger Games executable", 2);
}


// Create a Windows program group folder containing icons for Daedalus.

flag FCreateProgramGroup(flag fAll)
{
  char szDir[cchSzMax], szName[cchSzMax], *pch;
  LPITEMIDLIST pidl;
  int iScript;

  // Create program group folder.
  SHGetSpecialFolderLocation(wi.hwnd,
    fAll ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS, &pidl);
  SHGetPathFromIDList(pidl, szDir);
  for (pch = szDir; *pch; pch++)
    ;
  sprintf(SO(pch, szDir), "\\Daedalus");
  if (!CreateDirectory(szDir, NULL)) {
    if (GetLastError() != ERROR_ALREADY_EXISTS)
      return fFalse;
  }

  // Delete old previous version shortcuts from folder.
  DeleteShortcut(szDir, "Daedalus 2.3");
  DeleteShortcut(szDir, "Daedalus 2.4");
  DeleteShortcut(szDir, "Daedalus 2.5");
  DeleteShortcut(szDir, "Daedalus 3.0");
  DeleteShortcut(szDir, "Daedalus 3.1");
  DeleteShortcut(szDir, "Daedalus 3.2");
  DeleteShortcut(szDir, "Daedalus 3.3");
  DeleteShortcut(szDir, "Daedalus 3.4");
  DeleteShortcut(szDir, "Daedalus homepage");   // Now Daedalus website
  DeleteShortcut(szDir, "Carleton Farm Maze");  // Now Carleton Farm Maze #1

  // Add main shortcuts in folder.
  sprintf(S(szName), "%s %s", szDaedalus, szVersion);
  if (!FCreateShortcut(szDir, szName,
    "daedalus.exe", "Daedalus executable", -1))
    return fFalse;
  if (!FCreateShortcut(szDir, "Daedalus documentation",
    "daedalus.htm", NULL, -1))
    return fFalse;
  if (!FCreateShortcut(szDir, "Daedalus scripting", "script.htm", NULL, -1))
    return fFalse;
  if (!FCreateShortcut(szDir, "Daedalus changes", "changes.htm", NULL, -1))
    return fFalse;
  if (!FCreateShortcut(szDir, "Daedalus website", "daedalus.url", NULL, 3))
    return fFalse;

  // Create scripts subfolder.
  while (*pch)
    pch++;
  sprintf(SO(pch, szDir), "\\Scripts");
  if (!CreateDirectory(szDir, NULL)) {
    if (GetLastError() != ERROR_ALREADY_EXISTS)
      return fFalse;
  }

  // Add script shortcuts in subfolder
  for (iScript = 0; iScript <= cmdScriptLast - cmdScript01; iScript++) {
    if (!FCreateShortcut(szDir, rgszShortcut[iScript], rgszScript[iScript],
      NULL, iScript != 5/*Dragonslayer*/ && iScript != 8/*Hunger*/ &&
      iScript != 25/*Squares*/ && iScript != 26/*Mandelbrot*/ &&
      iScript != 27/*Pentris*/ && iScript != 28/*Grippy*/ ? 1 :
      (iScript != 28/*Grippy*/ ? 2 : 5)))
      return fFalse;
  }
  return fTrue;
}


// Add info about Daedalus specific file extensions to the Windows registry.
// Could edit HKCR\ instead of HKCU\Software\Classes\ to affect all users, but
// that would require Administrator priviledges on modern systems.

CONST char *rgszKey[] = {"ds", "d3", "dp", "dw", NULL};
CONST char *rgszValue[] =
  {"Script", "3D Bitmap", "Patch File", "Wireframe File"};

flag FRegisterExtensions()
{
  HKEY hkey;
  char szExe[cchSzMax], szIco[cchSzMax], szKey[cchSzDef], szTop[cchSzDef],
    *pch;
  int iKey;

  GetModuleFileName(wi.hinst, szExe, cchSzDef);
  sprintf(S(szIco), "%s,1", szExe);
  for (pch = szExe; *pch; pch++)
    ;
  sprintf(SO(pch, szExe), "%s", " %1");
  sprintf(S(szTop), "Software\\Classes\\");

  for (iKey = 0; rgszKey[iKey]; iKey++) {

    // Define .d? extension and point it to a Daedalus File Type.
    sprintf(S(szKey), "%s.%s", szTop, rgszKey[iKey]);
    if (RegCreateKey(HKEY_CURRENT_USER, szKey, &hkey) != ERROR_SUCCESS)
      return fFalse;
    sprintf(S(szKey), "Daedalus.%s", rgszKey[iKey]);
    if (RegSetValue(hkey, NULL, REG_SZ, szKey, CchSz(szKey)) != ERROR_SUCCESS)
      return fFalse;
    RegCloseKey(hkey);

    // Set File Type name for .d? extensions.
    sprintf(S(szKey), "%sDaedalus.%s", szTop, rgszKey[iKey]);
    if (RegCreateKey(HKEY_CURRENT_USER, szKey, &hkey) != ERROR_SUCCESS)
      return fFalse;
    sprintf(S(szKey), "Daedalus %s", rgszValue[iKey]);
    if (RegSetValue(hkey, NULL, REG_SZ, szKey, CchSz(szKey)) != ERROR_SUCCESS)
      return fFalse;
    RegCloseKey(hkey);

    // Make .d? extension files be opened by Daedalus.
    sprintf(S(szKey), "%sDaedalus.%s\\shell\\open\\command", szTop,
      rgszKey[iKey]);
    if (RegCreateKey(HKEY_CURRENT_USER, szKey, &hkey) != ERROR_SUCCESS)
      return fFalse;
    if (RegSetValue(hkey, NULL, REG_SZ, szExe, CchSz(szExe)) != ERROR_SUCCESS)
      return fFalse;
    RegCloseKey(hkey);

    // Make .d? extension files be edited by Notepad.
    sprintf(S(szKey), "%sDaedalus.%s\\shell\\edit\\command", szTop,
      rgszKey[iKey]);
    if (RegCreateKey(HKEY_CURRENT_USER, szKey, &hkey) != ERROR_SUCCESS)
      return fFalse;
    if (RegSetValue(hkey, NULL, REG_SZ, "Notepad %1", 10) != ERROR_SUCCESS)
      return fFalse;
    RegCloseKey(hkey);

    // Set icon for .d? extension files.
    sprintf(S(szKey), "%sDaedalus.%s\\DefaultIcon", szTop, rgszKey[iKey]);
    if (RegCreateKey(HKEY_CURRENT_USER, szKey, &hkey) != ERROR_SUCCESS)
      return fFalse;
    if (RegSetValue(hkey, NULL, REG_SZ, szIco, CchSz(szIco)) != ERROR_SUCCESS)
      return fFalse;
    RegCloseKey(hkey);
  }
  return fTrue;
}


// Remove Daedalus specific file extensions from the Windows registry.

flag FUnregisterExtensions()
{
  char szKey[cchSzDef], szTop[cchSzDef];
  int iKey;

  sprintf(S(szTop), "Software\\Classes\\");

  for (iKey = 0; rgszKey[iKey]; iKey++) {

    // Delete .d? extension pointing to a Daedalus File Type.
    sprintf(S(szKey), "%s.%s", szTop, rgszKey[iKey]);
    if (SHDeleteKey(HKEY_CURRENT_USER, szKey) != ERROR_SUCCESS)
      return fFalse;

    // Delete icon for .d? extension files.
    sprintf(S(szKey), "%sDaedalus.%s", szTop, rgszKey[iKey]);
    if (SHDeleteKey(HKEY_CURRENT_USER, szKey) != ERROR_SUCCESS)
      return fFalse;
  }
  return fTrue;
}


/*
******************************************************************************
** System Hooks
******************************************************************************
*/

// Set a Windows variable to a numeric value.

void DoSetVariableW(int ivos, int n)
{
  int xo, yo, xs, ys;
  flag f = (n != 0);

  switch (ivos) {
  case vosEnsureTimer:
    EnsureTimer(n);
    break;
  case vosPauseTimer:
    PauseTimer(f);
    break;
  case vosScrollX:
    ws.xScroll = n; SetScrollPos(wi.hwnd, SB_HORZ, ws.xScroll, fTrue);
    break;
  case vosScrollY:
    ws.yScroll = n; SetScrollPos(wi.hwnd, SB_VERT, ws.yScroll, fTrue);
    break;
  case vosSizeX:
  case vosSizeY:
  case vosPositionX:
  case vosPositionY:
  case vosDrawable:
  case vosWindowTop:
    GetWindowPosition(&xo, &yo, &xs, &ys);
    switch (ivos) {
    case vosSizeX:     xs               = n; break;
    case vosSizeY:     ys               = n; break;
    case vosPositionX: xo               = n; break;
    case vosPositionY: yo               = n; break;
    case vosDrawable:  ws.fResizeClient = f; break;
    case vosWindowTop: ws.fWindowTop    = f; break;
    }
    if (ws.fResizeClient)
      AdjustWindowPosition(&xo, &yo, &xs, &ys);
    SetWindowPosition(xo, yo, xs, ys);
    break;
  case vosWindowMenu:
    ws.fWindowMenu = f;
    SetMenu(wi.hwnd, ws.fWindowMenu ? NULL : wi.hmenu);
    break;
  case vosWindowScroll:
    ws.fWindowScroll = f;
    ShowScrollBar(wi.hwnd, SB_BOTH, !ws.fWindowScroll);
    break;
  case vosShowColmap:
    CheckMenu(cmdColmap, f);
    if (f) {
      if (wi.hbmpC != NULL) {
        SelectObject(wi.hdcBmp, wi.hbmpPrev);
        wi.hbmpPrev = (HBITMAP)SelectObject(wi.hdcBmp, wi.hbmpC);
      }
    } else {
      SelectObject(wi.hdcBmp, wi.hbmpPrev);
      wi.hbmpPrev = (HBITMAP)SelectObject(wi.hdcBmp, wi.hbmp);
    }
    break;
  default:
    PrintSzN_E("Setting Windows variable %d is undefined.", ivos);
  }
}


// Get the value of a Windows variable, which will be stored in a numeric
// return parameter.

int NGetVariableW(int ivos)
{
  int n, nT;
  MSG msg;

  switch (ivos) {
  case vosGetTimer:  return GetTimer();                     break;
  case vosGetTick:   return GetTickCount();                 break;
  case vosSizeX:     GetWindowPosition(&nT, &nT, &n,  &nT); break;
  case vosSizeY:     GetWindowPosition(&nT, &nT, &nT, &n);  break;
  case vosPositionX: GetWindowPosition(&n,  &nT, &nT, &nT); break;
  case vosPositionY: GetWindowPosition(&nT, &n,  &nT, &nT); break;
  case vosEventSpace: n = (GetAsyncKeyState(VK_SPACE) != 0); break;
  case vosEventKey:   n = (PeekMessage(&msg, wi.hwnd,
    WM_KEYDOWN, WM_KEYDOWN, PM_NOREMOVE | PM_NOYIELD) != 0); break;
  case vosEventMouse: n = (PeekMessage(&msg, wi.hwnd,
    WM_LBUTTONDOWN, WM_RBUTTONDOWN, PM_NOREMOVE | PM_NOYIELD) != 0); break;
  case vosKvDialog: return KvDialog(); break;
  default:
    PrintSzN_E("Getting Windows variable %d is undefined.", ivos);
  }
  return n;
}


// Execute a Windows operation given parameters.

void DoOperationW(int ioos, CONST char *rgch, int cch, int n1, int n2)
{
  char sz[cchSzOpr], szT[cchSzOpr];
  int i;
  MSG msg;

  switch (ioos) {
  case oosMessageIns:
    if (wi.hdcInside != NULL) {
      FormatRgchToSz(rgch, cch, sz, cchSzOpr);
      DrawTextLine(wi.hdcInside, sz, CchSz(sz), n2);
    }
    break;
  case oosDrawText:
    DrawTextLine2(GetDC(wi.hwnd), rgch, cch, n1, n2,
      ws.szFontName, ws.nFontSize, ws.nFontWeight);
    break;
  case oosTitle:
    FormatRgchToSz(rgch, cch, sz, cchSzOpr);
    if (n2) {
      sprintf(S(szT), "%s %s%s%s",
        ws.szAppName, szVersion, *sz ? " " : "", sz);
    } else
      sprintf(S(szT), "%s", sz);
    SetWindowText(wi.hwnd, szT);
    break;
  case oosBeep:
    if (ws.fSound)
      MessageBeep((UINT)-1);
    break;
  case oosBeep2:
    if (ws.fSound)
      Beep(n1, n2);
    break;
  case oosSound:
    if (!ws.fSound)
      break;
    FormatRgchToSz(rgch, cch, sz, cchSzOpr);
    if (ws.nSoundDelay < 0) {
      PlaySound(sz, NULL, SND_FILENAME | SND_NODEFAULT | SND_SYNC);
      break;
    }
    for (i = 0; i < ws.nSoundDelay; i += 100) {
      PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
      if (!(i == 0 && GetTickCount() - wi.timeSound < 100) && PlaySound(sz,
        NULL, SND_FILENAME | SND_NODEFAULT | SND_ASYNC | SND_NOSTOP))
        break;
      if (GetTickCount() - wi.timeSound > (ulong)ws.nSoundDelay ||
        NGetVariableW(vosEventKey)) {
        i = ws.nSoundDelay;
        break;
      }
      Sleep(100);
    }
    if (i >= ws.nSoundDelay)
      PlaySound(sz, NULL, SND_FILENAME | SND_NODEFAULT | SND_ASYNC);
    wi.timeSound = GetTickCount();
    break;
  case oosDelay:
    if (n1 >= 0)
      Sleep(n1);
    break;
  case oosSystem:
    SendMessage(wi.hwndMain, WM_SYSCOMMAND, n1 < 0 ? SC_MINIMIZE :
      (n1 == 0 ? SC_RESTORE : SC_MAXIMIZE), 0);
    break;
  case oosMenu:
    ModifyMenu(wi.hmenu, cmdMacro01 + n1 - 1, MF_BYCOMMAND | MF_STRING,
      cmdMacro01 + n1 - 1, rgch);
    break;
  case oosCheck:
    CheckMenu(n1, n2);
    break;

  default:
    PrintSzN_E("Windows operation %d is undefined.", ioos);
  }
}


// Execute a simple system specific action.

void SystemHook(int ihos)
{
  MSG msg;

  switch (ihos) {
  case hosRedraw:
    Redraw(wi.hwnd);
    break;
  case hosDirtyView:
    DirtyView();
    break;
  case hosYield:
    PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
    break;
  case hos3DOff:
    CheckMenu(cmd3D, fFalse);
    break;
  case hos3DOn:
    CheckMenu(cmd3D, fTrue);
    break;
  case hosDlgString:
    DoDialog(DlgString, dlgString);
    break;
  case hosDlgString2:
    DoDialog(DlgString2, dlgString2);
    break;
  case hosDlgString3:
    DoDialog(DlgString3, dlgString3);
    break;

  default:
    PrintSzN_E("Windows hook %d is undefined.", ihos);
  }
}


// Change to the system wait cursor, or restore the previous cursor. If fSet
// then set pcursor to previous cursor. If !fSet, then restore from pcursor.

void HourglassCursor(size_t *pcursor, flag fSet)
{
  if (fSet)
    *pcursor = (size_t)SetCursor(LoadCursor(NULL, IDC_WAIT));
  else
    SetCursor((HCURSOR)*pcursor);
}
#endif // WIN

/* wutil.cpp */
