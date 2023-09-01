/*
** Daedalus (Version 3.4) File: wdriver.h
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
** This file contains Windows specific definitions for Daedalus as a whole.
**
** Created: 4/8/2013.
** Last code change: 8/29/2023.
*/


/*
******************************************************************************
** Macros
******************************************************************************
*/

#ifdef _WIN64
#define szArchCore "64 bit"
#else
#define szArchCore "32 bit"
#endif

#define nScrollDiv 16

#define API PASCAL
#ifndef WM_XBUTTONDOWN
#define WM_XBUTTONDOWN 0x20B
#endif
#ifndef SC_MONITORPOWER
#define SC_MONITORPOWER 0xF170
#endif
#define SetWindowOrg(hdc, x, y) SetWindowOrgEx(hdc, x, y, NULL)
#define SetViewportOrg(hdc, x, y) SetViewportOrgEx(hdc, x, y, NULL)
#define MoveTo(hdc, x, y) MoveToEx(hdc, x, y, NULL)
#define InitDC(hdc) SetMapMode(hdc, MM_TEXT); \
  SetWindowOrg(hdc, 0, 0); SetViewportOrg(hdc, 0, 0)
#define SetWindowPosition(xo, yo, xs, ys) SetWindowPos(wi.hwnd, \
  ws.fWindowTop ? GetTopWindow(NULL) : HWND_NOTOPMOST, xo, yo, xs, ys, 0)

// Dialog Macros

#define CheckMenu(cmd, f) \
  CheckMenuItem(wi.hmenu, cmd, f ? MF_CHECKED : MF_UNCHECKED);
#define SetCheck(id, f) CheckDlgButton(hdlg, id, f)
#define SetRadio(id, idLo, idHi) CheckRadioButton(hdlg, idLo, idHi, id)
#define SetEdit(id, sz) SetDlgItemText(hdlg, id, (LPCSTR)(sz))
#define SetEditN(id, n) SetDlgItemInt(hdlg, id, n, fTrue)
#define SetEditL(id, l) SetDlgItemLong(hdlg, id, l)
#define SetEditR(id, r) SetDlgItemReal(hdlg, id, r)
#define SetCombo(id, sz) \
  SendDlgItemMessage(hdlg, id, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)(sz))
#define ClearCombo(id) SendDlgItemMessage(hdlg, id, CB_RESETCONTENT, 0, 0);
#define GetCheck(id) IsDlgButtonChecked(hdlg, id)
#define GetEdit(id, sz) GetEditCore(id, sz, cchSzDef)
#define GetEditCore(id, sz, cch) GetDlgItemText(hdlg, id, sz, cch)
#define GetEditN(id) GetDlgItemInt(hdlg, id, NULL, fTrue)
#define GetEditL(id) GetDlgItemLong(hdlg, id)
#define GetEditR(id) GetDlgItemReal(hdlg, id)
#define GetRadio(idLo, idHi) GetDlgItemRadio(hdlg, idLo, idHi)
#define EnsureBetweenN(n, n1, n2, szError) \
  if (!FEnsureBetweenN(n, n1, n2, szError)) return fTrue;
#define EnsureBetweenR(r, n1, n2, szError) \
  if (!FEnsureBetweenR(r, n1, n2, szError)) return fTrue;
#define EnsureLeastN(n, n1, szError) \
  if (!FEnsureLeastN(n, n1, szError)) return fTrue;


/*
******************************************************************************
** Structures
******************************************************************************
*/

typedef struct _windowinternal {
  // Window settings

  HINSTANCE hinst;
  HWND hwndMain;
  HWND hwnd;
  HWND hwndAbort;
  HMENU hmenu;
  HACCEL haccel;

  // Internal settings

  RECT rcFull;
  HDC hdcInside;
  HANDLE hMutex;
  ulong timeSound;

  // Bitmap fields

  HDC hdcBmp;
  HBITMAP hbmpPrev;
  HBITMAP hbmp;
  HBITMAP hbmpC;
  HBITMAP hbmpI;
  BITMAPINFOHEADER bi;
  RGBQUAD rgbq[2];
  BITMAPINFOHEADER biC;
  BITMAPINFOHEADER biI;
} WI;


/*
******************************************************************************
** Externs
******************************************************************************
*/

// Global variables

extern WI wi;
extern OPENFILENAME ofn;
extern PRINTDLG prd;

// From wdriver.cpp

LRESULT PASCAL WndProc(HWND, uint, WPARAM, LPARAM);
void Redraw(HWND);

// From wutil.cpp

void CopyBitmapToDeviceBitmap(void);
void DirtyView(void);
void BitmapDot(int, int);
flag FBootExternal(CONST char *, flag);
void RedoMenu(HWND);
void GetWindowPosition(int *, int *, int *, int *);
void AdjustWindowPosition(int *, int *, int *, int *);
void DrawTextLine(HDC, CONST char *, int, int);
flag FCreateDesktopIcon(void);
flag FCreateProgramGroup(flag);
flag FRegisterExtensions(void);
flag FUnregisterExtensions(void);

// From wdialog.cpp

void DoDialog(flag API (HWND, uint, WORD, LONG), WORD);
flag FSzCopy(CONST char *);
flag FFileCopy(int);
flag FFilePaste();
flag DlgPrint(void);
flag API DlgAbortProc(HDC, int);
flag API DlgAbort(HWND, uint, WORD, LONG);
flag API DlgFile(HWND, uint, WORD, LONG);
flag API DlgAbout(HWND, uint, WORD, LONG);
flag API DlgCommand(HWND, uint, WORD, LONG);
flag API DlgEvent(HWND, uint, WORD, LONG);
flag API DlgDisplay(HWND, uint, WORD, LONG);
flag API DlgColor(HWND, uint, WORD, LONG);
flag API DlgRandom(HWND, uint, WORD, LONG);
flag API DlgDot(HWND, uint, WORD, LONG);
flag API DlgInside(HWND, uint, WORD, LONG);
flag API DlgSize(HWND, uint, WORD, LONG);
flag API DlgZoom(HWND, uint, WORD, LONG);
flag API DlgCubeFlip(HWND, uint, WORD, LONG);
flag API DlgBright(HWND, uint, WORD, LONG);
flag API DlgColorReplace(HWND, uint, WORD, LONG);
flag API DlgSizeMaze(HWND, uint, WORD, LONG);
flag API DlgBias(HWND, uint, WORD, LONG);
flag API DlgMaze(HWND, uint, WORD, LONG);
flag API DlgCreate(HWND, uint, WORD, LONG);
flag API DlgLabyrinth(HWND, uint, WORD, LONG);
flag API DlgDraw(HWND, uint, WORD, LONG);
flag API DlgDraw2(HWND, uint, WORD, LONG);
flag API DlgString(HWND, uint, WORD, LONG);
flag API DlgString2(HWND, uint, WORD, LONG);
flag API DlgString3(HWND, uint, WORD, LONG);

/* wdriver.h */
