#include "types_compat.h"
#include "wnd_compat.h"
// GeneralsX @build BenderAI 10/02/2026 - SDL3 only for windowing builds (Phase 2)
#ifdef SAGE_USE_SDL3
#include <SDL3/SDL.h>
#endif

DWORD GetWindowLong(HWND hWnd, int nIndex)
{
  return 0;
}

void AdjustWindowRect(RECT *pRect, DWORD dwStyle, BOOL bMenu)
{
  // This is left blank, since the SDL3 window is always going to be the in-game resolution size
  // Windows seems to need to take into account the decoration size, which SDL3 does not need to
}

BOOL ShowWindow(HWND hWnd, int nCmdShow)
{
  return TRUE;
}

void SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
#ifdef SAGE_USE_SDL3
  SDL_Window *window = (SDL_Window *)hWnd;
  if (!window) return;

  if (!(uFlags & SWP_NOMOVE)) {
    SDL_SetWindowPosition(window, X, Y);
  }

  if (!(uFlags & SWP_NOSIZE)) {
    SDL_SetWindowSize(window, cx, cy);
  }
#endif
}

void GetWindowRect(HWND hWnd, RECT *pRect)
{

}

void GetClientRect(HWND hWnd, RECT *pRect)
{

}

HWND GetDesktopWindow() {
  return NULL;
}

HDC GetDC(HWND hWnd) {
  return NULL;
}

int ReleaseDC(HWND hWnd, HDC hDC) 
{
  return 0;
}

void SetDeviceGammaRamp(HDC hDC, LPVOID lpRamp)
{

}

int MessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
  return 0;
}

void SetCursor(void *hCursor)
{

}

void GetCursorPos(struct POINT *ptCursor) {

}

void ScreenToClient(HWND hWnd, struct POINT *ptCursor)
{

}

void ClientToScreen(HWND hWnd, struct POINT *ptCursor)
{

}
