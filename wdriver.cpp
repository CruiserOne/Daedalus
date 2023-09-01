/*
** Daedalus (Version 3.4) File: wdriver.cpp
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
** This file contains the Windows driver for Daedalus as a whole, and
** code to execute menu commands and Windows events.
**
** Created: 4/8/2013.
** Last code change: 8/29/2023.
*/

#include <windows.h>
#include <stdio.h>
#include <time.h>

#define APSTUDIO_INVOKED
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
// Global variables

WI wi = {NULL, NULL, NULL, NULL, NULL, NULL,
  {0, 0, 640, 480}, NULL, NULL, 0,
  NULL, NULL, NULL, NULL, NULL, NULL};

// Global variables for common dialogs

OPENFILENAME ofn = {sizeof(OPENFILENAME), (HWND)NULL, (HINSTANCE)NULL, NULL,
  NULL, 0, 1, NULL, cchSzMaxFile, NULL, cchSzMaxFile, NULL, NULL,
  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 0, 0, NULL, 0L, NULL, NULL};
PRINTDLG prd = {
  sizeof(PRINTDLG), (HWND)NULL, (HGLOBAL)NULL, (HGLOBAL)NULL, (HDC)NULL,
  PD_NOPAGENUMS | PD_NOSELECTION | PD_RETURNDC | PD_USEDEVMODECOPIES,
  0, 0, 0, 0, 1, (HINSTANCE)NULL, 0L, NULL, NULL, (LPCSTR)NULL, (LPCSTR)NULL,
  (HGLOBAL)NULL, (HGLOBAL)NULL};
CHOOSECOLOR cc = {sizeof(CHOOSECOLOR), (HWND)NULL, (HWND)NULL, kvGray, NULL,
  CC_FULLOPEN | CC_RGBINIT, 0L, NULL, NULL};


/*
******************************************************************************
** Function Hooks
******************************************************************************
*/

// Bring up a message box and return the button pressed.

int PrintSzCore(CONST char *sz, int nPriority)
{
  char szTitle[cchSzDef], szT[cchSzDef], *pchTitle, *pchT;
  UINT nIcon, nType = MB_OK;
  int nRet = IDOK;
  flag fSav;

  // Don't show normal priority messages if Ignore Messages is active.
  if (ws.fIgnorePrint && FBetween(nPriority, 0, ws.nIgnorePrint))
    goto LDone;
  if (ws.fSaverRun) {
    ws.fSaverMouse = fFalse;
    ShowCursor(fTrue);
  }

  // Figure out what text to show in the title bar of the message box.
  pchTitle = NULL;
  if (ws.nTitleMessage >= 0) {
    pchT = SzVar(ws.nTitleMessage);
    if (pchT != NULL) {
      CopySz(pchT, szT, cchSzDef);
      pchTitle = szT;
    }
  }
  if (pchTitle == NULL || *pchTitle == chNull)
    pchTitle = ws.szAppName;
  switch (nPriority) {
  case nPrintWarning:
    sprintf(S(szTitle), "%s Warning", pchTitle);
    nIcon = MB_ICONSTOP;
    break;
  case nPrintError:
    sprintf(S(szTitle), "%s Error", pchTitle);
    nIcon = MB_ICONEXCLAMATION;
    break;
  default:
    sprintf(S(szTitle), "%s", pchTitle);
    nIcon = MB_ICONINFORMATION;
    break;
  }

  // Figure out what buttons to include in the message box.
  if (nPriority < 0) {
    nIcon = MB_ICONQUESTION;
    switch (nPriority) {
    case nAskOkCancel:         nType = MB_OKCANCEL;         break;
    case nAskRetryCancel:      nType = MB_RETRYCANCEL;      break;
    case nAskYesNo:            nType = MB_YESNO;            break;
    case nAskYesNoCancel:      nType = MB_YESNOCANCEL;      break;
    case nAskAbortRetryIgnore: nType = MB_ABORTRETRYIGNORE; break;
    }
  }

  // Actually bring up the message box and wait for user to press a button.
  if (ws.fCopyMessage)
    FSzCopy(sz);
  fSav = ws.fInSpree; ws.fInSpree = fTrue;
  nRet = MessageBox(wi.hwndMain, sz,
    ws.szTitle != NULL ? ws.szTitle : szTitle, nType | nIcon | MB_TASKMODAL);
  ws.fInSpree = fSav;
  ws.cDialog++;

LDone:
  ws.szTitle = NULL;
  if (ws.fSaverRun)
    ShowCursor(fFalse);
  if (nRet == IDCANCEL || nRet == IDABORT)
    nRet = -1;
  else if (nRet == IDNO || nRet == IDIGNORE)
    nRet = 0;
  else
    nRet = 1;
  return nRet;
}


// Allocate a memory buffer of a given size, in a Windows specific manner.

void *PAllocate(long lcb)
{
  char sz[cchSzMax];
  void *pv;

#ifdef DEBUG
  pv = GlobalAlloc(GMEM_FIXED, lcb + sizeof(dword)*3);
#else
  pv = GlobalAlloc(GMEM_FIXED, lcb);
#endif

  // Handle success or failure of the allocation.
  if (pv == NULL) {
    sprintf(S(sz), "Failed to allocate memory (%ld bytes).\n", lcb);
    PrintSz_E(sz);
  } else {
    us.cAlloc++;
    us.cAllocTotal++;
    us.cAllocSize += lcb;
  }

#ifdef DEBUG
  // Put sentinels at ends of allocation to check for buffer overruns.
  *(dword *)pv = dwCanary;
  *(dword *)((byte *)pv + sizeof(dword)) = lcb;
  *(dword *)((byte *)pv + sizeof(dword)*2 + lcb) = dwCanary;
  return (byte *)pv + sizeof(dword)*2;
#else
  return pv;
#endif
}


// Free a memory buffer allocated with PAllocate.

void DeallocateP(void *pv)
{
  Assert(pv != NULL);
#ifdef DEBUG
  // Ensure buffer wasn't overrun during its existence.
  void *pvSys;
  dword lcb, dw;
  pvSys = (byte *)pv - sizeof(dword)*2;
  Assert(pvSys != NULL);
  dw = *(dword *)pvSys;
  Assert(dw == dwCanary);
  lcb = *(dword *)((byte *)pvSys + sizeof(dword));
  dw = *(dword *)((byte *)pv + lcb);
  Assert(dw == dwCanary);
  GlobalFree((HGLOBAL)pvSys);
#else
  GlobalFree((HGLOBAL)pv);
#endif
  us.cAlloc--;
}


// Redraw the screen, if Allow Partial Screen Updates is on. An action that
// modifies the bitmap has reached an intermediate point, where the screen
// may be updated here. This is half way between not updating the screen until
// the action has been completed, and updating after every pixel change.

void UpdateDisplay()
{
  int xlSav, ylSav, xhSav, yhSav;

  if (!ws.fAllowUpdate && !gs.fTraceDot)
    return;
  xlSav = xl; ylSav = yl; xhSav = xh; yhSav = yh;
  DirtyView();
  xl = xlSav; yl = ylSav; xh = xhSav; yh = yhSav;
}


// Draw a monochrome or color pixel on the window. Used to update the screen
// when one pixel of the bitmap changes, to avoid redrawing the whole bitmap.

void ScreenDot(int x, int y, bit o, KV kv)
{
  HDC hdc;
  HBRUSH hbr, hbrPrev;
  int xpDot, ypDot, i;
  MSG msg;
  flag fColor = (kv != ~0);

  if (dr.fInside || !ws.fNoDirty)
    return;
  if (!fColor)
    SetPixel(wi.hdcBmp, x, y, o ? kvWhite : kvBlack);
  else
    SetPixel(wi.hdcBmp, x, y, kv);
  hdc = GetDC(wi.hwnd);
  InitDC(hdc);
  if (bm.xa > 0 && bm.ya > 0 && bm.nHow < 3) {
    if (!fColor)
      hbr = CreateSolidBrush(o ? dr.kvOn : dr.kvOff);
    else
      hbr = CreateSolidBrush(kv);
    hbrPrev = (HBRUSH)SelectObject(hdc, hbr);
    xpDot = bm.xpOrigin + (x - bm.xaOrigin) * bm.xpPoint;
    ypDot = bm.ypOrigin + (y - bm.yaOrigin) * bm.ypPoint;
    PatBlt(hdc, xpDot, ypDot, bm.xpPoint, bm.ypPoint, PATCOPY);
    SelectObject(hdc, hbrPrev);
    DeleteObject(hbr);
  }
  ReleaseDC(wi.hwnd, hdc);

  // Pause for the Pixel Display Delay time.
  for (i = 0; i < ws.nDisplayDelay; i++) {
    PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
    // Pressing Space will interrupt this and all future pixel delays.
    if (GetAsyncKeyState(VK_SPACE) != 0) {
      ws.nDisplayDelay = 0;
      break;
    }
  }
}


// Bring up the Windows color picker dialog. Return the color selected.

KV KvDialog()
{
  cc.rgbResult = dr.kvDot;
  cc.lpCustColors = (ulong *)ws.rgkv;
  ChooseColor(&cc);
  ws.cDialog++;
  return cc.rgbResult;
}


/*
******************************************************************************
** Main Windows Processing
******************************************************************************
*/

// The main program, the starting point of Daedalus, follows. This is like the
// "main" function in standard C. The program initializes here, then spins in
// a tight message processing loop until it terminates.

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance,
  LPSTR lpszCmdLine, int nCmdShow)
{
  WNDCLASS wndclass;
  MSG msg;
  HDC hdc;
  time_t lTime;
  int i, j;
  char ch;

  // This is where it all begins. Welcome! :)

  ws.szAppName = szDaedalus;
  ws.szFileTemp = szFileTempCore;

  // Check for being used as a screen saver.

  if (lpszCmdLine[0] == '/') {
    ch = lpszCmdLine[1];
    if (ch == 's')
      ws.fSaverRun = fTrue;
    else if (ch == 'c' || ch == 'a')
      ws.fSaverConfig = fTrue;
    else if (ch == 'p')
      return -1L;
    else if (ch == 'w')
      ws.fNoWindow = fTrue;
  }

  // Set up the window class shared by all instances of Daedalus.

  wi.hinst = hinstance;
  if (!hPrevInstance) {
    memset(&wndclass, 0, sizeof(WNDCLASS));
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNWINDOW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = wi.hinst;
    wndclass.hIcon = LoadIcon(wi.hinst, MAKEINTRESOURCE(icon));
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wndclass.lpszMenuName = MAKEINTRESOURCE(menu);
    wndclass.lpszClassName = ws.szAppName;
    if (!RegisterClass(&wndclass)) {
      PrintSz_E("The window class could not be registered.");
      return -1L;
    }
  }

  // Create the actual window to be used and drawn on by this instance.

  wi.hmenu = LoadMenu(wi.hinst, MAKEINTRESOURCE(menu));
  wi.hwndMain = CreateWindow(
    ws.szAppName,
    szDaedalus " " szVersion,
    WS_CAPTION |
    WS_SYSMENU |
    WS_MINIMIZEBOX |
    WS_MAXIMIZEBOX |
    WS_THICKFRAME |
    WS_VSCROLL |
    WS_HSCROLL |
    WS_CLIPCHILDREN |
    WS_OVERLAPPED,
    CW_USEDEFAULT, CW_USEDEFAULT,
    CW_USEDEFAULT, CW_USEDEFAULT,
    (HWND)NULL,
    wi.hmenu,
    wi.hinst,
    NULL);
  if (wi.hwndMain == NULL) {
    PrintSz_E("The window could not be created.");
    return -1L;
  }
  wi.haccel = LoadAccelerators(wi.hinst, MAKEINTRESOURCE(accelerator));
  if (!ws.fNoWindow)
    ShowWindow(wi.hwndMain, nCmdShow);

  // Set up some globals that can't be initialized at compile time.

  CoInitialize(NULL);
  ofn.hwndOwner      = wi.hwndMain;
  ofn.lpstrFile      = ws.szFileName;
  ofn.lpstrFileTitle = ws.szFileTitle;
  cc.hwndOwner       = wi.hwndMain;
  for (i = 0; i < cColorMain; i++) {
    j = i*16 + 15;
    ws.rgkv[i] = GrayN(j);
  }

  wi.bi.biSize = sizeof(BITMAPINFOHEADER);
  wi.bi.biPlanes = 1;
  wi.bi.biBitCount = 1;
  wi.bi.biCompression = BI_RGB;
  wi.bi.biSizeImage = 0;
  wi.bi.biXPelsPerMeter = wi.bi.biYPelsPerMeter = 1000;
  wi.bi.biClrUsed = 0;
  wi.bi.biClrImportant = 0;
  wi.rgbq[0].rgbBlue = wi.rgbq[0].rgbGreen = wi.rgbq[0].rgbRed = 0;
  wi.rgbq[1].rgbBlue = wi.rgbq[1].rgbGreen = wi.rgbq[1].rgbRed = 255;
  wi.rgbq[0].rgbReserved = wi.rgbq[1].rgbReserved = 0;
  wi.biC = wi.bi;
  wi.biC.biBitCount = cbPixelC << 3;
  wi.biI = wi.biC;

  hdc = GetDC(wi.hwnd);
  wi.hdcBmp = CreateCompatibleDC(hdc);
  ReleaseDC(wi.hwnd, hdc);
  wi.hbmp = CreateCompatibleBitmap(wi.hdcBmp, xStart, yStart);
  wi.hbmpPrev = (HBITMAP)SelectObject(wi.hdcBmp, wi.hbmp);

  GetAsyncKeyState(VK_SPACE); // Clear state for commands that check this.
  ws.timeReset = GetTickCount();

  // Create the default Maze seen when the program starts.

  if (!bm.b.FAllocate(xStart, yStart, NULL))
    return -1L;
  InitRndL(109);
  bm.b.CreateMazePerfect();
  us.fRndOld = fFalse;
  ms.nHuntType = 2;
  CopyBitmapToDeviceBitmap();
  time(&lTime);
  InitRndL((long)lTime ^ ws.timeReset);

#ifdef ASSERT
  // Ensure there's nothing wrong with the various action tables.

  i = ccmd;
  if (i != _APS_NEXT_COMMAND_VALUE - icmdBase)
    PrintSzNN_E("Command table sizes %d and %d differ.",
      ccmd, _APS_NEXT_COMMAND_VALUE - icmdBase);
  for (i = 0; i < ccmd; i++) {
    if ((int)rgcmd[i].wCmd != i + 1001) {
      PrintSzNN_E("Command table entry %d contains command %d.",
        i, rgcmd[i].wCmd);
      break;
    }
  }
  for (i = 0; i < copr; i++) {
    if (rgopr[i].iopr != i) {
      PrintSzNN_E("Operation table entry %d contains operation %d.",
        i, rgopr[i].iopr);
      break;
    }
  }
  for (i = 0; i < cvar; i++) {
    if (rgvar[i].ivar != i) {
      PrintSzNN_E("Variable table entry %d contains variable %d.",
        i, rgvar[i].ivar);
      break;
    }
  }
  for (i = 0; i < cfun; i++) {
    if (rgfun[i].ifun != i) {
      PrintSzNN_E("Function table entry %d contains function %d.",
        i, rgfun[i].ifun);
      break;
    }
  }
#endif

  // Handle actions on the Windows command line that started the program.

  if (rgszStartup[0] != NULL)
    RunCommandLines(rgszStartup);
  if (!ws.fSaverRun && !ws.fSaverConfig && !ws.fNoWindow) {
    if (lpszCmdLine[0] != chNull)
      RunCommandLine(lpszCmdLine, NULL);
    FBootExternal("daedalus.ds", fTrue);
  } else {
    if (ws.fSaverRun) {
      ShowCursor(fFalse);
      DoCommand(cmdWindowFull);
    }
    if (ws.fNoWindow) {
      FBootExternal("daedalus.ds", fTrue);
      ws.fQuitting = fTrue;
    } else if (!FBootExternal("daedalus.ds", fTrue)) {
      dr.nFrameXY = dr.nFrameD = dr.nFrameZ = 90;
      DoCommand(cmdInsideView);
      DoCommand(cmdGoMiddle);
      DoCommand(cmdMoveRandom);
      DoCommand(cmdSpree);
    }
  }

  // Process window messages until the program is told to terminate.

  ws.fStarting = fFalse;
  while (!ws.fQuitting && GetMessage(&msg, NULL, 0, 0)) {
    if (msg.message == WM_KEYDOWN)
      ws.nKey = (int)msg.wParam;
    if (!TranslateAccelerator(wi.hwndMain, wi.haccel, &msg))
      TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  wi.hwndMain = NULL;

  // The program is terminating. Free all memory that's been allocated.

  bm.b.Free();
  bm.b2.Free();
  bm.b3.Free();
  bm.k.Free();
  bm.k2.Free();
  bm.k3.Free();
  bm.kI.Free();
  bm.kR.Free();
  bm.kS.Free();
  if (bm.coor != NULL)
    DeallocateP(bm.coor);
  if (bm.patch != NULL)
    DeallocateP(bm.patch);
  if (ds.rgstar != NULL)
    DeallocateP(ds.rgstar);
  if (dr.rgcalc != NULL)
    DeallocateP(dr.rgcalc);
  if (dr.rgstar != NULL)
    DeallocateP(dr.rgstar);
  if (dr.rgmtn != NULL)
    DeallocateP(dr.rgmtn);
  if (dr.rgcld != NULL)
    DeallocateP(dr.rgcld);
  if (dr.rgtrans != NULL)
    DeallocateP(dr.rgtrans);
  if (ms.rgnInfEller != NULL)
    DeallocateP(ms.rgnInfEller);
  if (ws.rgbCmdMacro != NULL)
    DeallocateP(ws.rgbCmdMacro);
  if (ws.rgszMacro != NULL) {
    for (i = 0; i < ws.cszMacro; i++)
      if (ws.rgszMacro[i] != NULL)
        DeallocateP(ws.rgszMacro[i]);
    DeallocateP(ws.rgszMacro);
  }
  if (ws.rglVar != NULL)
    DeallocateP(ws.rglVar);
  if (ws.rgszVar != NULL) {
    for (i = 0; i < ws.cszVar; i++)
      if (ws.rgszVar[i] != NULL)
        DeallocateP(ws.rgszVar[i]);
    DeallocateP(ws.rgszVar);
  }
  DeallocateTextures();
  if (ws.rgsTrieAlloc != NULL)
    DeallocateP(ws.rgsTrieAlloc);
  if (ws.rgsTrieConst != NULL)
    DeallocateP(ws.rgsTrieConst);
  if (ws.rgnTrieConst != NULL)
    DeallocateP(ws.rgnTrieConst);
  if (ms.fileInf != NULL)
    fclose(ms.fileInf);

  // Check for memory leaks.
  if (gs.fErrorCheck && us.cAlloc != 0)
    PrintSzL_E("Number of allocations not freed before exiting: %ld",
      us.cAlloc);
  Assert(ws.nMacroDepth == 0);

  // Free Windows resources and unregister the window class.

  CoUninitialize();
  if (wi.hMutex != NULL)
    CloseHandle(wi.hMutex);
  SelectObject(wi.hdcBmp, wi.hbmpPrev);
  DeleteObject(wi.hbmp);
  if (wi.hbmpC != NULL)
    DeleteObject(wi.hbmpC);
  if (wi.hbmpI != NULL)
    DeleteObject(wi.hbmpI);
  DeleteDC(wi.hdcBmp);
  ClearPb(&wndclass, sizeof(WNDCLASS));
  UnregisterClass(ws.szAppName, wi.hinst);

  // Daedalus goes away for real here. Bye! :)

  return (int)msg.wParam;
}


// Execute a Windows command. This gets run when a menu option is manually
// selected, and when a command is automatically invoked via scripting.

flag DoCommandW(int wCmd)
{
  RECT rc;
  POINT pt;
  char sz[cchSzDef], *pch;
  int i, j, k, d;
  CMaz bT;
  CMap3 *pbT;
  long l;
  flag fDidRedraw = fFalse;

  switch (wCmd) {

  // File menu

  case cmdPrint:
    if (!DlgPrint())
      PrintSz_W("Printing did not complete successfully.");
    break;

  case cmdPrintSetup:
    l = prd.Flags;
    prd.Flags |= PD_PRINTSETUP;
    PrintDlg((LPPRINTDLG)&prd);
    prd.Flags = l;
    break;

  case cmdDlgFile:
    DoDialog(DlgFile, dlgFile);
    break;

  case cmdShellHelp:
    FBootExternal("daedalus.htm", fFalse);
    break;

  case cmdShellScript:
    FBootExternal("script.htm", fFalse);
    break;

  case cmdShellChange:
    FBootExternal("changes.htm", fFalse);
    break;

  case cmdShellLicense:
    FBootExternal("license.htm", fFalse);
    break;

  case cmdShellWeb:
    FBootExternal("daedalus.url", fFalse);
    break;

  case cmdShellWeb2:
    FBootExternal("daedlus2.url", fFalse);
    break;

  case cmdSetupAll:
  case cmdSetupUser:
    if (!FCreateProgramGroup(wCmd == cmdSetupAll))
      PrintSz_E("Failed to create program group.\nDaedalus "
        "can still be run from other icons or directly from its folder.");
    break;

  case cmdSetupDesktop:
    if (!FCreateDesktopIcon())
      PrintSz_E("Failed to create desktop icon.");
    break;

  case cmdSetupExtension:
    if (!FRegisterExtensions())
      PrintSz_E("Failed to register Daedalus file extensions.\nThis "
        "error is ignorable and all features of Daedalus will still work.");
    break;

  case cmdUnsetup:
    if (!FUnregisterExtensions())
      PrintSz_E("Failed to unregister Daedalus file extensions.");
    break;

  case cmdAbout:
    DoDialog(DlgAbout, dlgAbout);
    break;

  // Edit menu

  case cmdSpree:
    inv(ws.fSpree);
    EnsureTimer(ws.nTimerDelay);
    CheckMenu(cmdSpree, ws.fSpree);
    break;

  case cmdCommand:
    DoDialog(DlgCommand, dlgCommand);
    break;

  case cmdDlgEvent:
    DoDialog(DlgEvent, dlgEvent);
    break;

  case cmdCopyBitmap:
  case cmdCopyText:
  case cmdCopyPicture:
    FFileCopy(wCmd);
    break;

  case cmdPaste:
    FFilePaste();
    break;

  case cmdDlgDisplay:
    DoDialog(DlgDisplay, dlgDisplay);
    break;

  case cmdWindowFull:
    if (!ws.fWindowFull) {
      GetWindowRect(wi.hwnd, &wi.rcFull);
      wi.rcFull.right -= wi.rcFull.left; wi.rcFull.bottom -= wi.rcFull.top;
      pt.x = pt.y = 0;
      ClientToScreen(wi.hwnd, &pt);
      ws.fWindowTop = fTrue;
      GetWindowRect(GetDesktopWindow(), &rc);
      if (!SetWindowPosition(rc.left + (wi.rcFull.left - pt.x),
        rc.top + (wi.rcFull.top - pt.y),
        rc.right + (wi.rcFull.right - ws.xpClient),
        rc.bottom + (wi.rcFull.bottom - ws.ypClient))) {
        PrintSz_E("Failed to enter full screen mode.\n"
          "You may need to run Daedalus with greater permissions for full "
          "screen mode to succeed.");
        break;
      }
    } else {
      ws.fWindowTop = fFalse;
      if (wi.rcFull.left < 0)
        wi.rcFull.left = 0;
      if (wi.rcFull.top < 0)
        wi.rcFull.top = 0;
      SetWindowPosition(wi.rcFull.left, wi.rcFull.top,
        wi.rcFull.right, wi.rcFull.bottom);
    }
    inv(ws.fWindowFull);
    CheckMenu(cmdWindowFull, ws.fWindowFull);
    break;

  case cmdWindowBitmap:
    if (bm.nHow < 3) {
      GetWindowRect(wi.hwnd, &rc);
      pbT = PbFocus();
      MoveWindow(wi.hwnd, rc.left, rc.top,
        pbT->m_x * bm.xpPoint + (rc.right - rc.left - ws.xpClient),
        pbT->m_y * bm.ypPoint + (rc.bottom - rc.top - ws.ypClient), fTrue);
    }
    break;

  case cmdRedrawNow:
    CopyBitmapToDeviceBitmap();
    Redraw(wi.hwnd);
    fDidRedraw = fTrue;
    break;

  case cmdRepaintNow:
    Redraw(wi.hwnd);
    GetAsyncKeyState(VK_SPACE);  // Clear state as other commands check this.
    fDidRedraw = fTrue;
    break;

  case cmdScrollUp:
    PostMessage(wi.hwnd, WM_VSCROLL, SB_PAGEUP, 0);
    break;

  case cmdScrollDown:
    PostMessage(wi.hwnd, WM_VSCROLL, SB_PAGEDOWN, 0);
    break;

  case cmdScrollHome:
    PostMessage(wi.hwnd, WM_HSCROLL, SB_THUMBPOSITION, 0);
    PostMessage(wi.hwnd, WM_VSCROLL, SB_THUMBPOSITION, 0);
    break;

  case cmdScrollEnd:
    PostMessage(wi.hwnd, WM_HSCROLL, SB_THUMBPOSITION, nScrollDiv);
    PostMessage(wi.hwnd, WM_VSCROLL, SB_THUMBPOSITION, nScrollDiv);
    break;

  case cmdDlgColor:
    DoDialog(DlgColor, dlgColor);
    break;

  case cmdTimeGet:
    l = (GetTimer() + 500) / 1000;
    SetMacroReturn(l);
    i = (l % 60L); l /= 60L;
    j = (l % 60L); l /= 60L;
    k = (l % 24L); l /= 24L;
    d = l;
    sprintf(S(sz), "Time elapsed: ");
    pch = sz + CchSz(sz);
    if (d > 0) {
      sprintf(SO(pch, sz), "%d day%s, ", d, d == 1 ? "" : "s");
      while (*pch)
        pch++;
    }
    if (d > 0 || k > 0) {
      sprintf(SO(pch, sz), "%d hour%s, ", k, k == 1 ? "" : "s");
      while (*pch)
        pch++;
    }
    if (d > 0 || k > 0 || j > 0) {
      sprintf(SO(pch, sz), "%d minute%s, ", j, j == 1 ? "" : "s");
      while (*pch)
        pch++;
    }
    sprintf(SO(pch, sz), "%d second%s", i, i == 1 ? "" : "s");
    PrintSz_N(sz);
    break;

  case cmdTimeReset:
    ws.timeReset = GetTickCount();
    ws.timePause = ws.timeReset;
    break;

  case cmdTimePause:
    PauseTimer(!ws.fTimePause);
    break;

  case cmdDlgRandom:
    DoDialog(DlgRandom, dlgRandom);
    break;

  // Dot menu

  case cmdDlgDot:
    DoDialog(DlgDot, dlgDot);
    break;

  case cmdDlgInside:
    DoDialog(DlgInside, dlgInside);
    break;

  // Bitmap menu

  case cmdDlgSize:
    DoDialog(DlgSize, dlgSize);
    break;

  case cmdDlgZoom:
    DoDialog(DlgZoom, dlgZoom);
    break;

  case cmdCubemapFlip:
    DoDialog(DlgCubeFlip, dlgCubeFlip);
    break;

  // Color menu

  case cmdColmapBrightness:
    DoDialog(DlgBright, dlgBright);
    break;

  case cmdColmapReplace:
    DoDialog(DlgColorReplace, dlgColorReplace);
    break;

  // Maze menu

  case cmdDlgSizeMaze:
    DoDialog(DlgSizeMaze, dlgSizeMaze);
    break;

  case cmdBias:
    DoDialog(DlgBias, dlgBias);
    break;

  case cmdDlgMaze:
    DoDialog(DlgMaze, dlgMaze);
    break;

  // Create menu

  case cmdDlgLabyrinth:
    DoDialog(DlgLabyrinth, dlgLabyrinth);
    break;

  case cmdDlgCreate:
    DoDialog(DlgCreate, dlgCreate);
    break;

  // Solve menu
  // Draw menu

  case cmdDlgDraw:
    DoDialog(DlgDraw, dlgDraw);
    break;

  case cmdDlgDraw2:
    DoDialog(DlgDraw2, dlgDraw2);
    break;

  }
  return fDidRedraw;
}


#define FKey(ch) (rgbKey[ch] >= 128)

// This is the main message processor for the Daedalus window. Given a user
// input or other message, do the appropriate action and updates.

LRESULT WINAPI WndProc(HWND hwnd, uint wMsg, WPARAM wParam, LPARAM lParam)
{
  HDC hdc;
  PAINTSTRUCT ps;
  POINT pt;
  MSG msg;
  int iParam, icmd, x, y, xmax, ymax, i;
  byte rgbKey[256];
  flag fKey8, fKey4, fKey6, fKey2, fKeyU, fKeyD, fKeyShift, fArrow, fRedraw;

  wi.hwnd = hwnd;
  switch (wMsg) {

  // A command, most likely a menu option, was given.

  case WM_COMMAND:
    if (ws.fSaverRun)
      goto LClose;
    iParam = (int)wParam & 0xFFFF;
    icmd = iParam - icmdBase;
    if (icmd < 0 || icmd >= ccmd)
      return DefWindowProc(hwnd, wMsg, wParam, lParam);
    if (ws.iEventCommand > 0 && iParam != cmdAbout &&
      FCheckEvent(ws.iEventCommand, ws.nKey, icmd, 0))
      break;
    if (ws.rgbCmdMacro != NULL && iParam != cmdAbout) {
      if (ws.rgbCmdMacro[icmd] == 255) {
        if (!ws.fIgnorePrint) {
          PrintSz_W("This command has been disabled.\n");
          ws.fSkipSystem = (iParam == cmdMacro40/*Alt+F4*/);
        }
        break;
      }
      if (ws.rgbCmdMacro[icmd] != 0) {
        if (FCheckEvent(ws.rgbCmdMacro[icmd], ws.nKey, icmd, 0)) {
          ws.fSkipSystem = (iParam == cmdMacro40/*Alt+F4*/);
          break;
        }
      }
    }
    ws.nKey = 0;
    DoCommand(iParam);
    ws.fSkipSystem =
      (iParam == cmdMacro10/*F10*/ || iParam == cmdMacro22/*Shift+F10*/ ||
      iParam == cmdMacro34/*Ctrl+F10*/ || iParam == cmdMacro40/*Alt+F4*/);
    if (ws.fQuitting)
      goto LClose;
    break;

  // A special system command was given. Suppress its action if it was invoked
  // by a hotkey which ran a macro, to avoid both effects happening.

  case WM_SYSCOMMAND:
    iParam = (int)wParam & 0xFFF0;
    if (ws.fSaverRun) {
      if (iParam == SC_SCREENSAVE || iParam == SC_MONITORPOWER)
        return fTrue;
      goto LClose;
    }
    if (ws.fSkipSystem) {
      if (iParam == SC_KEYMENU || iParam == SC_CLOSE) {
        ws.fSkipSystem = fFalse;
        break;
      }
    }
    return DefWindowProc(hwnd, wMsg, wParam, lParam);

  // Part of the window was uncovered. Usually Windows quickly blanks it out
  // with white, before having the app redraw that section. Daedalus however
  // can quickly update the screen, so doesn't want anything to happen here.

  case WM_ERASEBKGND:
    return fTrue;

  // The window was just created. Setup the scrollbars.

  case WM_CREATE:
    SetScrollRange(hwnd, SB_HORZ, 0, nScrollDiv, fFalse);
    SetScrollRange(hwnd, SB_VERT, 0, nScrollDiv, fFalse);
    ws.xScroll = ws.yScroll = 0;
    break;

  // The window has been resized. Update our window size variables.

  case WM_SIZE:
    ws.xpClient = LOWORD(lParam);
    ws.ypClient = HIWORD(lParam);
    break;

  // All or part of the window needs to be redrawn. Go do so.

  case WM_PAINT:
    ClearPb(&ps, sizeof(PAINTSTRUCT));
    hdc = BeginPaint(hwnd, &ps);
    SetBkMode(hdc, TRANSPARENT);
    IntersectClipRect(hdc, ps.rcPaint.left, ps.rcPaint.top,
      ps.rcPaint.right, ps.rcPaint.bottom);
    Redraw(hwnd);
    EndPaint(hwnd, &ps);
    break;

  // The left button of the mouse has been clicked over the window.

  case WM_LBUTTONDOWN:
    if (ws.fSaverRun)
      goto LClose;
    if (FCheckEventClick(ws.iEventLeft, (long)lParam))
      break;
    goto LMouse;

  // The mouse is being dragged over the window.

  case WM_MOUSEMOVE:
    if (ws.fSaverRun) {
      if (!ws.fSaverMouse) {
        ws.fSaverMouse = fTrue;
        ws.xMouse = WLo(lParam); ws.yMouse = WHi(lParam);
        break;
      }
      if (NAbs(ws.xMouse - WLo(lParam)) + NAbs(ws.yMouse - WHi(lParam)) > 4)
        goto LClose;
      break;
    }
    if (FCheckEventClick(ws.iEventMouse, (long)lParam))
      break;
LMouse:
    if (bm.nHow >= 3 || (wMsg == WM_MOUSEMOVE &&
      (dr.fInside || dr.fNoMouseDrag || (wParam & MK_LBUTTON) == 0)))
      break;

    // Handle clicks in the perspective inside view.
    if (dr.fInside) {
      if (dr.nInside == nInsideVeryFree)
        ws.fInSpree = fTrue;
      MouseClickInside(WLo(lParam), WHi(lParam));
      if (dr.nInside == nInsideVeryFree) {
        Redraw(hwnd);
        ws.fInSpree = fFalse;
      }
      break;
    }

    // Teleport dot to the place on the bitmap where the mouse was clicked.
    x = WLo(lParam) - bm.xpOrigin; y = WHi(lParam) - bm.ypOrigin;
    x = x / bm.xpPoint + bm.xaOrigin - (x < 0);
    y = y / bm.ypPoint + bm.yaOrigin - (y < 0);
    if (!bm.fColor) {
      xmax = bm.b.m_x; ymax = bm.b.m_y;
    } else {
      xmax = bm.k.m_x; ymax = bm.k.m_y;
    }
    if (dr.nEdge == nEdgeStop && (x < 0 || x >= xmax || y < 0 || y >= ymax))
      break;
    else if (dr.nEdge == nEdgeTorus) {
      if (x < 0)
        x = 0;
      else if (x >= xmax)
        x = xmax-1;
      if (y < 0)
        y = 0;
      else if (y >= ymax)
        y = ymax-1;
    }
    if (dr.fTwo || dr.fBoth) {
      x = (x & ~1) | (dr.x & 1);
      y = (y & ~1) | (dr.y & 1);
      if (dr.nEdge == nEdgeTorus) {
        if (x >= xmax && xmax > 1)
          x -= 2;
        if (y >= ymax && ymax > 1)
          y -= 2;
      }
    }
    if (dr.f3D) {
      dr.x = bm.b.X3(x); dr.y = bm.b.Y3(y); dr.z = bm.b.Z3(x, y);
    } else {
      dr.x = x; dr.y = y; dr.z = 0;
    }
    CenterWithinCell();
    if (dr.fDrag) {
      if (wParam & MK_CONTROL) {

        // Ctrl+click means draw a rectangle.
        // Ctrl+Shift+click means draw a filled in rectangle.
        if (wParam & MK_SHIFT) {
          if (!bm.fColor)
            bm.b.Block(ws.xMouse, ws.yMouse, x, y, dr.fSet);
          else
            bm.k.Block(ws.xMouse, ws.yMouse, x, y, kvSet);
        } else {
          if (!bm.fColor)
            bm.b.Edge(ws.xMouse, ws.yMouse, x, y, dr.fSet);
          else
            bm.k.Edge(ws.xMouse, ws.yMouse, x, y, kvSet);
        }
        DirtyView();
        break;
      } else if (wMsg == WM_MOUSEMOVE || wParam & MK_SHIFT) {

        // Shift+click means draw a line from the last to current position.
        if (x != ws.xMouse || y != ws.yMouse) {
          if (!bm.fColor)
            bm.b.Line(ws.xMouse, ws.yMouse, x, y, dr.fSet);
          else
            bm.k.Line(ws.xMouse, ws.yMouse, x, y, kvSet);
          if (wMsg == WM_MOUSEMOVE && (wParam & MK_SHIFT) == 0) {
            ws.xMouse = x; ws.yMouse = y;
          }
          DirtyView();
        }
        break;
      }
    }
    if (x != ws.xMouse || y != ws.yMouse) {
      ws.xMouse = x; ws.yMouse = y;
      Redraw(hwnd);
    }
    break;

  // The right button of the mouse has been clicked over the window.

  case WM_RBUTTONDOWN:
    if (ws.fSaverRun)
      goto LClose;
    if (FCheckEventClick(ws.iEventRight, (long)lParam))
      break;
    inv(dr.fFollowMouse);
    EnsureTimer(ws.nTimerDelay);
    break;

  // The middle button or wheel of the mouse has been clicked over the window.

  case WM_MBUTTONDOWN:
    if (ws.fSaverRun)
      goto LClose;
    FCheckEventClick(ws.iEventMiddle, (long)lParam);
    break;

  // The "previous" or "next" buttons of the mouse have been clicked.

  case WM_XBUTTONDOWN:
    if (ws.fSaverRun)
      goto LClose;
    FCheckEventClick(wParam & 0x20 ? ws.iEventPrev : ws.iEventNext,
      (long)lParam);
    break;

  // A timer message is received at the defined repeat interval.

  case WM_TIMER:
    if (ws.fInSpree)
      break;
    ws.fInSpree = fTrue;
LTimer:
    fRedraw = fFalse;

    // Handle the Chase Mouse Point setting if active.
    if (dr.fFollowMouse && GetActiveWindow() == hwnd) {
      GetCursorPos(&pt);
      ScreenToClient(hwnd, &pt);
      if (ChildWindowFromPoint(hwnd, pt) != NULL) {
        if (bm.fDrewDot)
          DotMouseChase(pt.x, pt.y);
        else if (dr.fInside) {
          MouseClickInside(pt.x, pt.y);
          fRedraw = (dr.nInside == nInsideVeryFree);
        }
      }
    }

    // Move based on what keys are down for the Very Free Movement mode.
    fArrow = ds.fArrow && ds.fArrowFree && !dr.fInside;
    if ((fArrow || (dr.fInside && dr.nInside == nInsideVeryFree)) &&
      PeekMessage(&msg, hwnd, 0, 0, PM_NOREMOVE | PM_NOYIELD) == 0) {
      GetKeyboardState(rgbKey);

      fKey8 = FKey(VK_UP)    || FKey(VK_NUMPAD8) || FKey('8') ||
        FKey(VK_NUMPAD7) || FKey('7') || FKey(VK_NUMPAD9) || FKey('9');
      fKey4 = FKey(VK_LEFT)  || FKey(VK_NUMPAD4) || FKey('4') ||
        FKey(VK_NUMPAD7) || FKey('7') || FKey(VK_NUMPAD3) || FKey('3');
      fKey6 = FKey(VK_RIGHT) || FKey(VK_NUMPAD6) || FKey('6') ||
        FKey(VK_NUMPAD9) || FKey('9') || FKey(VK_NUMPAD1) || FKey('1');
      fKey2 = FKey(VK_DOWN)  || FKey(VK_NUMPAD2) || FKey('2') ||
        FKey(VK_CLEAR) || FKey(VK_NUMPAD5) || FKey('5') ||
        FKey(VK_NUMPAD1) || FKey('1') || FKey(VK_NUMPAD3) || FKey('3');
      fKeyU = FKey('U') && !(dr.f3DNotFree && dr.f3D);
      fKeyD = FKey('D') && !(dr.f3DNotFree && dr.f3D);
      fKeyShift = FKey(VK_SHIFT);

      if (fKey8 && fKey2)
        fKey8 = fKey2 = fFalse;
      if (fKey4 && fKey6)
        fKey4 = fKey6 = fFalse;
      if (fKeyU && fKeyD)
        fKeyU = fKeyD = fFalse;

      if (fKey8 || fKey2 || fKey4 || fKey6 || fKeyU || fKeyD) {
        if (fKey8)
          DoCommand(fKeyShift && fArrow && !fKey4 && !fKey6 ?
            cmdJump8 : cmdMove8);
        else if (fKey2)
          DoCommand(fKeyShift && fArrow && !fKey4 && !fKey6 ?
            cmdJump2 : cmdMove2);
        if (fKey4)
          DoCommand(fKeyShift ? cmdJump4 : cmdMove4);
        else if (fKey6)
          DoCommand(fKeyShift ? cmdJump6 : cmdMove6);
        if (fKeyU)
          DoCommand(cmdMoveU);
        else if (fKeyD)
          DoCommand(cmdMoveD);
        fRedraw = fTrue;
      }

      if (fRedraw && fArrow) {
        if (!ds.fDidPatch)
          FRenderPerspectiveWire(BFocus(), bm.coor, bm.ccoor);
        else
          FRenderPerspectivePatch(BFocus(), bm.patch, bm.cpatch);
        DirtyView();
        if (ws.fSpree && dr.fRedrawAfter)
          fRedraw = fFalse;
      }
    }

    // Autorepeat a command if autorepeat is active.
    if (ws.fSpree && (!fRedraw || !dr.fRedrawAfter)) {
      i = ws.fIgnorePrint;
      ws.fIgnorePrint = fTrue;
      if (dr.fSpreeRandom)
        dr.kvOn = kvRandomRainbow;
      if (DoCommand(ws.wCmdSpree))
        fRedraw = fFalse;
      ws.fIgnorePrint = i;
    }

    // Redraw the screen if anything above wants it to be.
    if (fRedraw)
      Redraw(hwnd);

    // For a zero repeat delay, immediately redo the timer actions, and don't
    // wait for Windows to send the event again. This may starve other apps.
    if (ws.nTimerDelay <= 0) {
      if (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE | PM_NOYIELD) == 0)
        goto LTimer;
      if (!TranslateAccelerator(msg.hwnd, wi.haccel, &msg))
        TranslateMessage(&msg);
      if (msg.message != WM_TIMER)
        WndProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
      if (FWantTimer() && !ws.fQuitting)
        goto LTimer;
    }
    ws.fInSpree = fFalse;
    break;

  // Define the maximum size the window may be resized to.

  case WM_GETMINMAXINFO:
    ((MINMAXINFO *)lParam)->ptMaxTrackSize.x = xBitmap;
    ((MINMAXINFO *)lParam)->ptMaxTrackSize.y = yBitmap;
    break;

  // The horizontal scrollbar was clicked on or moved in some way.

  case WM_HSCROLL:
    if (ws.fSaverRun)
      goto LClose;
    switch (LOWORD(wParam)) {
    case SB_LEFT:
      ws.xScroll = 0;
      break;
    case SB_RIGHT:
      ws.xScroll = nScrollDiv;
      break;
    case SB_LINEUP:
      ws.xScroll = max(0, ws.xScroll - 1);
      break;
    case SB_LINEDOWN:
      ws.xScroll = min(nScrollDiv, ws.xScroll + 1);
      break;
    case SB_PAGEUP:
      ws.xScroll = max(0, ws.xScroll - nScrollPage);
      break;
    case SB_PAGEDOWN:
      ws.xScroll = min(nScrollDiv, ws.xScroll + nScrollPage);
      break;
    case SB_THUMBPOSITION:
      ws.xScroll = HIWORD(wParam);
      break;
    default:
      return fFalse;
    }
    SetScrollPos(hwnd, SB_HORZ, ws.xScroll, fTrue);
    Redraw(hwnd);
    break;

  // The vertical scrollbar was clicked on or moved in some way.

  case WM_VSCROLL:
    if (ws.fSaverRun)
      goto LClose;
    switch (LOWORD(wParam)) {
    case SB_TOP:
      ws.yScroll = 0;
      break;
    case SB_BOTTOM:
      ws.yScroll = nScrollDiv;
      break;
    case SB_LINEUP:
      ws.yScroll = max(0, ws.yScroll - 1);
      break;
    case SB_LINEDOWN:
      ws.yScroll = min(nScrollDiv, ws.yScroll + 1);
      break;
    case SB_PAGEUP:
      ws.yScroll = max(0, ws.yScroll - nScrollPage);
      break;
    case SB_PAGEDOWN:
      ws.yScroll = min(nScrollDiv, ws.yScroll + nScrollPage);
      break;
    case SB_THUMBPOSITION:
      ws.yScroll = HIWORD(wParam);
      break;
    default:
      return fFalse;
    }
    SetScrollPos(hwnd, SB_VERT, ws.yScroll, fTrue);
    Redraw(hwnd);
    break;

  // The window is being closed. Clean up and prepare to exit the program.

  case WM_CLOSE:
LClose:
    if (ws.iEventQuit > 0) {
      ws.fQuitting = fFalse;
      RunMacro(ws.iEventQuit);
      if (LVar(iLetterZ) != 0) {
        ws.fSaverMouse = fFalse;
        return fTrue;
      }
    }
    if (ws.fNoExit)
      PrintSz_W("Program exit has been disabled.");
    else {
      ws.fQuitting = fTrue;
      EnsureTimer(-1);
    }
    break;

  // Messages not processed here are handled by Windows in a default way.

  default:
    return DefWindowProc(hwnd, wMsg, wParam, lParam);
  }
  return fFalse;
}


/*
******************************************************************************
** Screen Display
******************************************************************************
*/

// Redraw what's being displayed in the window based on program settings and
// the contents of the active bitmap.

void Redraw(HWND hwnd)
{
  HDC hdc, hdcT;
  HBRUSH hbr, hbrPrev;
  HPEN hpen, hpenPrev;
  HBITMAP hbmpPrev;

  char sz[cchSzDef];
  DWORD lTimer, lTimer2;
  int cch;
  flag fFastUpdate, fT;

  int xaBm, yaBm, xaDot, yaDot, xpView, ypView, x, y, xT, yT;
  flag fArrow;

  // Setup some local and global variables.
  if (ws.fInRedraw)
    return;
  ws.fInRedraw = fTrue;
  hdc = GetDC(hwnd);
  InitDC(hdc);
  bm.fDrewDot = fFalse;
  if (ms.fSection) {
    xl = dr.x2; yl = dr.y2;
    xh = dr.x; yh = dr.y;
  } else
    bm.b.SetXyh();
  gs.bFocus = PbFocus();

  // Draw the perspective inside view.
  if (dr.fInside) {
    lTimer = dr.fDelay ? GetTickCount() : 0;
    if (!dr.fInSmooth &&
      !FBetween(dr.nInside, nInsideFree, nInsideVeryFree)) {
      dr.nOffsetX = dr.nOffsetY = dr.nOffsetZ = dr.nOffsetD = 0;
      if (dr.fNarrow) {
        dr.x |= 1; dr.y |= 1; dr.z &= ~1;
      }
    }
    if (bm.kI.FNull() || bm.kI.m_x != ws.xpClient || bm.kI.m_y != ws.ypClient)
      if (!bm.kI.FBitmapSizeSet(ws.xpClient, ws.ypClient))
        goto LDone;

    if (dr.nInside != nInsideSimple) {
      if (ds.nStereo == 0)
        RedrawInsidePerspective(bm.kI);
      else
        RedrawInsidePerspectiveStereo(bm.kI, bm.kS);
    } else {
      if (!dr.f3D)
        RedrawInside(bm.kI);
      else
        RedrawInside3(bm.kI);
    }
    if (dr.fMap)
      DrawOverlay(bm.kI, bm.b);
    if (ws.iEventInside2 > 0) {
      fT = ws.fNoDirty;
      RunMacro(ws.iEventInside2);
      if (ws.fNoDirty != fT) {
        ws.fNoDirty = fT;
        goto LDone;
      }
    }

    // Update the screen and clean up.
    if (bm.xaI != ws.xpClient || bm.yaI != ws.ypClient) {
      if (wi.hbmpI != NULL)
        DeleteObject(wi.hbmpI);
      wi.hbmpI = CreateCompatibleBitmap(hdc, bm.kI.m_x, bm.kI.m_y);
      if (wi.hbmpI == NULL)
        PrintSz_E("Failed to create inside display bitmap.");
      bm.xaI = bm.kI.m_x;
      bm.yaI = bm.kI.m_y;
      wi.biI.biWidth = bm.xaI;
      wi.biI.biHeight = -bm.yaI;
    }
    fFastUpdate = gs.fTraceDot ||
      (ws.iEventInside == 0 && !dr.fCompass && !dr.fLocation);
    hdcT = fFastUpdate ? hdc : CreateCompatibleDC(hdc);
    InitDC(hdcT);
    hbmpPrev = (HBITMAP)SelectObject(hdcT, wi.hbmpI);
    SetDIBitsToDevice(hdcT, 0, 0, bm.xaI, bm.yaI, 0, 0, 0, bm.yaI,
      bm.kI.m_rgb, (BITMAPINFO *)&wi.biI, DIB_RGB_COLORS);

    // Draw text on top of the window at appropriate points.
    if (ws.iEventInside > 0) {
      wi.hdcInside = hdcT;
      RunMacro(ws.iEventInside);
      wi.hdcInside = NULL;
    }
    if (dr.fCompass && !dr.fNoCompass) {
      FormatCompass(sz, FBetween(dr.nInside, nInsideFree, nInsideVeryFree),
        &cch);
      DrawTextLine(hdcT, sz, cch, -3);
    }
    if (dr.fLocation && !dr.fNoLocation) {
      FormatLocation(sz, dr.f3D);
      DrawTextLine(hdcT, sz, CchSz(sz), -2);
    }
    if (dr.fDelay) {
      lTimer2 = GetTickCount();
      FormatTimer(sz, lTimer, lTimer2);
      DrawTextLine(hdcT, sz, CchSz(sz), -4);
    }

    if (!fFastUpdate)
      BitBlt(hdc, 0, 0, bm.xaI, bm.yaI, hdcT, 0, 0, SRCCOPY);
    SelectObject(hdcT, hbmpPrev);
    if (!fFastUpdate)
      DeleteDC(hdcT);
    goto LDone;
  }

  // Draw the overhead view of the main or color bitmap.
  SetStretchBltMode(hdc, dr.nStretchMode == 1 ? WHITEONBLACK :
    (dr.nStretchMode == 2 ? BLACKONWHITE : COLORONCOLOR));
  if (!bm.fColor) {
    xaBm = bm.xa; yaBm = bm.ya;
  } else {
    xaBm = bm.xaC; yaBm = bm.yaC;
  }
  if (xaBm > 0 && yaBm > 0 && (!bm.fColor ? wi.hbmp : wi.hbmpC) != NULL) {
    hbr = CreateSolidBrush(dr.kvEdge);
    hbrPrev = (HBRUSH)SelectObject(hdc, hbr);
    SetTextColor(hdc, dr.kvOff);
    SetBkColor(hdc, dr.kvOn);
    if (!dr.f3D) {
      xaDot = dr.x; yaDot = dr.y;
    } else {
      xaDot = bm.b.X2(dr.x, dr.z); yaDot = bm.b.Y2(dr.y, dr.z);
    }

    // Determine how much of the bitmap to draw, and where to draw it.
    if (bm.nWhat > 0) {
      bm.xaView = Min(bm.xCell, xaBm); bm.yaView = Min(bm.yCell, yaBm);
      if (bm.nWhat > 1) {
        bm.xaView = Max(1, ws.xpClient / bm.xaView);
        bm.yaView = Max(1, ws.ypClient / bm.yaView);
        bm.xaView = Min(bm.xaView, xaBm); bm.yaView = Min(bm.yaView, yaBm);
      }
      bm.xaOrigin = xaDot - (bm.xaView >> 1);
      bm.yaOrigin = yaDot - (bm.yaView >> 1);
      if (bm.xaOrigin < 0)
        bm.xaOrigin = 0;
      else if (bm.xaOrigin + bm.xaView > xaBm)
        bm.xaOrigin = xaBm - bm.xaView;
      if (bm.yaOrigin < 0)
        bm.yaOrigin = 0;
      else if (bm.yaOrigin + bm.yaView > yaBm)
        bm.yaOrigin = yaBm - bm.yaView;
    } else {
      bm.xaView = xaBm; bm.yaView = yaBm;
      bm.xaOrigin = bm.yaOrigin = 0;
    }

    // Draw the bitmap or section of bitmap.
    if (bm.nHow < 3) {
      if (bm.nHow == 0 && bm.nWhat != 1) {
        bm.xpPoint = bm.xCell; bm.ypPoint = bm.yCell;
      } else {
        bm.xpPoint = Max(1, ws.xpClient / bm.xaView);
        bm.ypPoint = Max(1, ws.ypClient / bm.yaView);
      }
      if (bm.nHow == 1) {
        if (bm.xpPoint > bm.ypPoint)
          bm.xpPoint = bm.ypPoint;
        else
          bm.ypPoint = bm.xpPoint;
      }
      xpView = bm.xaView * bm.xpPoint; ypView = bm.yaView * bm.ypPoint;
      bm.xpOrigin = NMultDiv(ws.xpClient - xpView, ws.xScroll, nScrollDiv);
      bm.ypOrigin = NMultDiv(ws.ypClient - ypView, ws.yScroll, nScrollDiv);
      PatBlt(hdc, 0, 0, bm.xpOrigin + xpView, bm.ypOrigin, PATCOPY);
      PatBlt(hdc, 0, bm.ypOrigin, bm.xpOrigin, ypView, PATCOPY);
      PatBlt(hdc, bm.xpOrigin + xpView, 0, ws.xpClient -
        (bm.xpOrigin + xpView), bm.ypOrigin + ypView, PATCOPY);
      PatBlt(hdc, 0, bm.ypOrigin + ypView, ws.xpClient, ws.ypClient -
        (bm.ypOrigin + ypView), PATCOPY);
      StretchBlt(hdc, bm.xpOrigin, bm.ypOrigin, xpView, ypView,
        wi.hdcBmp, bm.xaOrigin, bm.yaOrigin, bm.xaView, bm.yaView, SRCCOPY);
    } else {
      StretchBlt(hdc, 0, 0, ws.xpClient, ws.ypClient,
        wi.hdcBmp, bm.xaOrigin, bm.yaOrigin, bm.xaView, bm.yaView, SRCCOPY);
    }
    SelectObject(hdc, hbrPrev);
    DeleteObject(hbr);

    // Draw the 2nd dot.
    if (dr.fDot2 && bm.nHow < 3) {
      xT = bm.xpPoint*((dr.nDotSize << 1) + 1);
      yT = bm.ypPoint*((dr.nDotSize << 1) + 1);
      hbr = CreateSolidBrush(dr.kvDot2);
      hbrPrev = (HBRUSH)SelectObject(hdc, hbr);
      if (!dr.f3D) {
        x = dr.x2; y = dr.y2;
      } else {
        x = bm.b.X2(dr.x2, dr.z2); y = bm.b.Y2(dr.y2, dr.z2);
      }
      bm.xpDot = bm.xpOrigin + (x - bm.xaOrigin) * bm.xpPoint;
      bm.ypDot = bm.ypOrigin + (y - bm.yaOrigin) * bm.ypPoint;
      x = bm.xpPoint; y = bm.ypPoint;
      if (dr.fCircle && xT > 2 && yT > 2) {
        hpenPrev = (HPEN)SelectObject(hdc, GetStockObject(NULL_PEN));
        Ellipse(hdc, bm.xpDot - x*dr.nDotSize, bm.ypDot - y*dr.nDotSize,
          bm.xpDot + x*(1 + dr.nDotSize) + 1,
          bm.ypDot + y*(1 + dr.nDotSize) + 1);
        SelectObject(hdc, hpenPrev);
      } else
        PatBlt(hdc, bm.xpDot - x*dr.nDotSize, bm.ypDot - y*dr.nDotSize,
          x*((dr.nDotSize << 1) + 1), y*((dr.nDotSize << 1) + 1), PATCOPY);
      SelectObject(hdc, hbrPrev);
      DeleteObject(hbr);
    }

    // Draw the dot.
    if (dr.fDot && bm.nHow < 3) {
      xT = bm.xpPoint*((dr.nDotSize << 1) + 1);
      yT = bm.ypPoint*((dr.nDotSize << 1) + 1);
      fArrow = dr.fArrow && xT >= 9 && yT >= 9;
      hbr = CreateSolidBrush(dr.kvDot);
      hbrPrev = (HBRUSH)SelectObject(hdc, hbr);
      bm.fDrewDot = fTrue;
      bm.xpDot = bm.xpOrigin + (xaDot - bm.xaOrigin) * bm.xpPoint;
      bm.ypDot = bm.ypOrigin + (yaDot - bm.yaOrigin) * bm.ypPoint;
      if (!fArrow || dr.kvDot != dr.kvInEdge) {
        x = bm.xpPoint; y = bm.ypPoint;
        if (dr.fCircle && xT > 2 && yT > 2) {
          hpenPrev = (HPEN)SelectObject(hdc, GetStockObject(NULL_PEN));
          Ellipse(hdc, bm.xpDot - x*dr.nDotSize, bm.ypDot - y*dr.nDotSize,
            bm.xpDot + x*(1 + dr.nDotSize) + 1,
            bm.ypDot + y*(1 + dr.nDotSize) + 1);
          SelectObject(hdc, hpenPrev);
        } else
          PatBlt(hdc, bm.xpDot - x*dr.nDotSize, bm.ypDot - y*dr.nDotSize,
            x*((dr.nDotSize << 1) + 1), y*((dr.nDotSize << 1) + 1), PATCOPY);
      }
      SelectObject(hdc, hbrPrev);
      DeleteObject(hbr);

      // Draw an arrow on the dot.
      if (fArrow) {
        hpen = CreatePen(PS_SOLID, 0, dr.kvInEdge);
        hpenPrev = (HPEN)SelectObject(hdc, hpen);
        x = bm.xpDot + bm.xpPoint/2 + xoff[dr.dir]*xT*3/8;
        y = bm.ypDot + bm.ypPoint/2 + yoff[dr.dir]*yT*3/8;
        MoveTo(hdc, x - xoff[dr.dir]*xT*6/8, y - yoff[dr.dir]*yT*6/8);
        LineTo(hdc, x, y);
        LineTo(hdc, x + (dr.dir == 1 ? 1 : -1)*xT*11/32,
          y + (dr.dir == 0 ? 1 : -1)*yT*11/32);
        MoveTo(hdc, x, y);
        LineTo(hdc, x + (dr.dir == 3 ? -1 : 1)*xT*11/32,
          y + (dr.dir == 2 ? -1 : 1)*yT*11/32);
        SelectObject(hdc, hpenPrev);
        DeleteObject(hpen);
      }
    }
  } else {

    // For a zero size bitmap, just fill everything with the border color.
    hbr = CreateSolidBrush(dr.kvEdge);
    hbrPrev = (HBRUSH)SelectObject(hdc, hbr);
    PatBlt(hdc, 0, 0, ws.xpClient, ws.ypClient, PATCOPY);
    SelectObject(hdc, hbrPrev);
    DeleteObject(hbr);
  }

LDone:
  ReleaseDC(hwnd, hdc);
  RedoMenu(wi.hwnd);
  ws.fInRedraw = fFalse;
}
#endif // WIN

/* wdriver.cpp */
