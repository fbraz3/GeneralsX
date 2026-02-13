#pragma once
// TheSuperHackers @build fighter19 10/02/2026 Bender - Need Win32 types
#include "types_compat.h"
#include "tchar_compat.h"
#define GWL_STYLE 1

struct RECT;
DWORD GetWindowLong(HWND hWnd, int nIndex);
void AdjustWindowRect(RECT *pRect, DWORD dwStyle, BOOL bMenu);

typedef enum eShowWindowFlags
{
  SW_HIDE = 0,
  SW_SHOWNORMAL = 1,
  SW_NORMAL = 1,
  SW_SHOWMINIMIZED = 2,
  SW_SHOWMAXIMIZED = 3,
  SW_MAXIMIZE = 3,
  SW_SHOWNOACTIVATE = 4,
  SW_SHOW = 5,
  SW_MINIMIZE = 6,
  SW_SHOWMINNOACTIVE = 7,
  SW_SHOWNA = 8,
  SW_RESTORE = 9,
  SW_SHOWDEFAULT = 10,
  SW_FORCEMINIMIZE = 11,
  SW_MAX = 11,
} eShowWindowFlags;
BOOL ShowWindow(HWND hWnd, int nCmdShow);

typedef enum eSetWindowPosFlags
{
  SWP_NOSIZE = 0x0001,
  SWP_NOMOVE = 0x0002,
  SWP_NOZORDER = 0x0004,
} eSetWindowPosFlags;

#define HWND_TOPMOST ((HWND)-1)
#define HWND_NOTOPMOST ((HWND)-2)

void SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
void GetWindowRect(HWND hWnd, RECT *pRect);

void GetClientRect(HWND hWnd, RECT *pRect);

HWND GetDesktopWindow();
HDC GetDC(HWND hWnd);
int ReleaseDC(HWND hWnd, HDC hDC);

void SetDeviceGammaRamp(HDC hDC, LPVOID lpRamp);

typedef enum eMessageBoxType
{
  MB_OK = 0x00000000L,
  MB_OKCANCEL = 0x00000001L,
  MB_ABORTRETRYIGNORE = 0x00000002L,
  MB_YESNOCANCEL = 0x00000003L,
  MB_YESNO = 0x00000004L,
  MB_RETRYCANCEL = 0x00000005L,
  MB_CANCELTRYCONTINUE = 0x00000006L,
  MB_ICONHAND = 0x00000010L,
  MB_ICONERROR = 0x00000010L,
  MB_TASKMODAL = 0x00002000L,
  MB_SYSTEMMODAL = 0x00001000L,
} eMessageBoxType;

typedef enum eMessageBoxResult
{
  IDABORT = 3,
  IDRETRY = 4,
  IDIGNORE = 5,
  IDYES = 6,
  IDNO = 7,
} eMessageBoxResult;

int MessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);

// TheSuperHackers @build BenderAI 12/02/2026 Add MessageBoxW/A stubs for Linux
// Windows: Use real Win32 API (via namespace resolution ::MessageBoxW/A)
// Linux: Stub returns safe default value
#ifndef _WIN32
inline int MessageBoxW(HWND hWnd, const wchar_t* lpText, const wchar_t* lpCaption, UINT uType) {
    return MessageBox(hWnd, nullptr, nullptr, uType);
}
inline int MessageBoxA(HWND hWnd, const char* lpText, const char* lpCaption, UINT uType) {
    return MessageBox(hWnd, nullptr, nullptr, uType);
}
#endif

void SetCursor(void *hCursor);
void GetCursorPos(struct POINT *ptCursor);
void ScreenToClient(HWND hWnd, struct POINT *ptCursor);
void ClientToScreen(HWND hWnd, struct POINT *ptCursor);
#define D3DCURSOR_IMMEDIATE_UPDATE 0x00000001