#pragma once

enum eFontWeights
{
  FW_NORMAL = 400,
  FW_BOLD = 700
};

typedef HANDLE HFONT;
typedef HANDLE HDC;
typedef HANDLE HBITMAP;

static HFONT CreateFontA(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCSTR lpszFace)
{
  return nullptr;
}

static void ExtTextOutW(HDC hdc, int x, int y, UINT fuOptions, const struct RECT *lprc, wchar_t *lpString, UINT cbCount, const int *lpDx)
{
}
