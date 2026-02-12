#pragma once

#include <stdint.h>

// TheSuperHackers @build fbraz 11/02/2026 Bender - Font/text constants
enum eFontWeights
{
  FW_NORMAL = 400,
  FW_BOLD = 700
};

// TheSuperHackers @build fbraz 11/02/2026 Bender - Character set constants
#define DEFAULT_CHARSET         1
#define ANSI_CHARSET            0
#define SYMBOL_CHARSET          2

// TheSuperHackers @build fbraz 11/02/2026 Bender - Font output precision
#define OUT_DEFAULT_PRECIS      0
#define OUT_STRING_PRECIS       1
#define OUT_CHARACTER_PRECIS    2

// TheSuperHackers @build fbraz 11/02/2026 Bender - Font clipping precision
#define CLIP_DEFAULT_PRECIS     0
#define CLIP_CHARACTER_PRECIS   1

// TheSuperHackers @build fbraz 11/02/2026 Bender - Font quality
#define DEFAULT_QUALITY         0
#define DRAFT_QUALITY           1
#define PROOF_QUALITY           2
#define ANTIALIASED_QUALITY     4

// TheSuperHackers @build fbraz 11/02/2026 Bender - Font pitch and family
#define DEFAULT_PITCH           0
#define FIXED_PITCH             1
#define VARIABLE_PITCH          2

// TheSuperHackers @build fbraz 11/02/2026 Bender - ExtTextOut flags
#define ETO_OPAQUE              0x0002
#define ETO_CLIPPED             0x0004

typedef HANDLE HFONT;
typedef HANDLE HDC;
typedef HANDLE HBITMAP;

// TheSuperHackers @build fbraz 10/02/2026 Bender
// RECT structure: DXVK windows_base.h already defines this (windows_base.h line ~145)
// Just forward-declare for type safety, actual definition comes from DXVK
struct RECT;

// TheSuperHackers @build fbraz 11/02/2026 Bender
// SIZE structure: DXVK windows_base.h already defines this (windows_base.h line ~157)  
// Just forward-declare for type safety, actual definition (including LPSIZE, PSIZE) comes from DXVK
struct SIZE;

// TheSuperHackers @build fbraz 11/02/2026 Bender - TEXTMETRIC structure (font metrics)
typedef struct tagTEXTMETRIC {
    long tmHeight;
    long tmAscent;
    long tmDescent;
    long tmInternalLeading;
    long tmExternalLeading;
    long tmAveCharWidth;
    long tmMaxCharWidth;
    long tmWeight;
    long tmOverhang;
    long tmDigitizedAspectX;
    long tmDigitizedAspectY;
    char tmFirstChar;
    char tmLastChar;
    char tmDefaultChar;
    char tmBreakChar;
    uint8_t tmItalic;
    uint8_t tmUnderlined;
    uint8_t tmStruckOut;
    uint8_t tmPitchAndFamily;
    uint8_t tmCharSet;
} TEXTMETRIC;

// TheSuperHackers @build fbraz 11/02/2026 Bender - GDI pointer types
// LPSIZE: SIZE pointer (SIZE itself provided by DXVK windows_base.h)
// LPTEXTMETRIC: TEXTMETRIC pointer  
typedef SIZE *LPSIZE;
typedef TEXTMETRIC *LPTEXTMETRIC;

// TheSuperHackers @build fighter19 10/02/2026 Bender - BMP screenshot structures (need before CreateDIBSection)
#ifndef _WIN32
#pragma pack(push, 1)

// RGB quad structure
typedef struct tagRGBQUAD {
  uint8_t rgbBlue;
  uint8_t rgbGreen;
  uint8_t rgbRed;
  uint8_t rgbReserved;
} RGBQUAD;

// Bitmap info header (DIB format)
typedef struct tagBITMAPINFOHEADER {
  uint32_t biSize;
  int32_t  biWidth;
  int32_t  biHeight;
  uint16_t biPlanes;
  uint16_t biBitCount;
  uint32_t biCompression;
  uint32_t biSizeImage;
  int32_t  biXPelsPerMeter;
  int32_t  biYPelsPerMeter;
  uint32_t biClrUsed;
  uint32_t biClrImportant;
} BITMAPINFOHEADER;

// Bitmap info structure (header + color palette)
typedef struct tagBITMAPINFO {
  BITMAPINFOHEADER bmiHeader;
  RGBQUAD          bmiColors[1];
} BITMAPINFO;

// Bitmap file header (BMP format)
typedef struct tagBITMAPFILEHEADER {
  uint16_t bfType;
  uint32_t bfSize;
  uint16_t bfReserved1;
  uint16_t bfReserved2;
  uint32_t bfOffBits;
} BITMAPFILEHEADER;

#pragma pack(pop)

// Bitmap compression modes
#define BI_RGB 0

#endif // !_WIN32

// TheSuperHackers @build fbraz 11/02/2026 Bender - GDI function stubs for text rendering (render2dsentence.cpp)
// Phase 3 out-of-scope: Return failure/nullptr to disable text rendering on Linux

// Math helper
static inline int MulDiv(int nNumber, int nNumerator, int nDenominator) {
    if (nDenominator == 0) return 0;
    return (int)(((long long)nNumber * nNumerator) / nDenominator);
}

// Font creation (full version with all parameters)
static inline HFONT CreateFont(int cHeight, int cWidth, int cEscapement, int cOrientation,
                                int cWeight, DWORD bItalic, DWORD bUnderline, DWORD bStrikeOut,
                                DWORD iCharSet, DWORD iOutPrecision, DWORD iClipPrecision,
                                DWORD iQuality, DWORD iPitchAndFamily, LPCSTR pszFaceName) {
    return nullptr;
}

static HFONT CreateFontA(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCSTR lpszFace)
{
  return nullptr;
}

static void ExtTextOutW(HDC hdc, int x, int y, UINT fuOptions, const struct RECT *lprc, wchar_t *lpString, UINT cbCount, const int *lpDx)
{
}

// Text measurement
static inline BOOL GetTextExtentPoint32W(HDC hdc, LPCWSTR lpString, int c, LPSIZE lpSize) {
    (void)hdc; (void)lpString; (void)c; (void)lpSize;
    return FALSE;
}

static inline BOOL GetTextExtentPoint32(HDC hdc, LPCSTR lpString, int c, LPSIZE lpSize) {
    (void)hdc; (void)lpString; (void)c; (void)lpSize;
    return FALSE;
}

static inline BOOL GetTextMetrics(HDC hdc, LPTEXTMETRIC lptm) {
    (void)hdc; (void)lptm;
    return FALSE;
}

// DIB section creation
static inline HBITMAP CreateDIBSection(HDC hdc, const BITMAPINFO *pbmi, UINT usage, void **ppvBits, HANDLE hSection, DWORD offset) {
    return nullptr;
}

// Device context
static inline HDC CreateCompatibleDC(HDC hdc) {
    return nullptr;
}

static inline BOOL DeleteDC(HDC hdc) {
    return TRUE;
}

// GDI object operations
static inline HGDIOBJ SelectObject(HDC hdc, HGDIOBJ h) {
    return nullptr;
}

static inline BOOL DeleteObject(HGDIOBJ ho) {
    return TRUE;
}

// Text/background color
static inline COLORREF SetBkColor(HDC hdc, COLORREF color) {
    return 0;
}

static inline COLORREF SetTextColor(HDC hdc, COLORREF color) {
    return 0;
}

// Font resource management
// TheSuperHackers @build BenderAI 12/02/2026 - Dynamic font loading stubs
// Used by GlobalLanguage for localized font support (e.g., Chinese, Arabic fonts)
// On Windows: Loads TrueType fonts from Data\<language>\Fonts\
// On Linux: No-op stubs (system fonts used instead, SDL_ttf handles rendering)
static inline int AddFontResource(const char* lpszFilename) {
    (void)lpszFilename;
    return 1;  // Return 1 (success) - font "loaded" (no-op)
}

static inline BOOL RemoveFontResource(const char* lpszFilename) {
    (void)lpszFilename;
    return TRUE;  // TRUE (success) - font "unloaded" (no-op)
}
