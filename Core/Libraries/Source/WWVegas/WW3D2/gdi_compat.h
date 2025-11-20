/*
 * GDI (Graphics Device Interface) Compatibility Layer
 * Maps Windows GDI functions to cross-platform equivalents
 * For non-Windows platforms, provides stub implementations
 * 
 * NOTE: Most GDI types are already defined in win32_sdl_types_compat.h
 * This file provides only the function stubs and missing constants.
 */

#ifndef GDI_COMPAT_H
#define GDI_COMPAT_H

// Include types that are already defined
#include "win32_sdl_types_compat.h"

// =====================================================================
// GDI Constants - ExtTextOut Options
// =====================================================================
#ifndef ETO_OPAQUE
#define ETO_OPAQUE 0x0002  // Fill background with current background color
#endif

#ifndef ETO_CLIPPED
#define ETO_CLIPPED 0x0004  // Clip text to rectangle
#endif

// =====================================================================
// GDI Functions - Math & Utility  
// =====================================================================
// MulDiv: Multiply two values and divide by third (with rounding)
// This prevents overflow during intermediate calculation
inline int MulDiv(int nNumber, int nNumerator, int nDenominator) {
    if (nDenominator == 0) return 0;
    return (nNumber * nNumerator) / nDenominator;
}

    // On non-Windows platforms, provide stub functions for GDI calls
    // All types (SIZE, RECT, BITMAPINFO, TEXTMETRIC, etc.) are already
    // defined in win32_sdl_types_compat.h

    // Inline stub functions for non-Windows platforms
    inline int ExtTextOutW(HDC hdc, int x, int y, unsigned int options, const RECT* rect, 
                           const wchar_t* str, unsigned int count, const int* lpDx) { 
        return 1; 
    }
    
    inline int GetTextExtentPoint32W(HDC hdc, const wchar_t* str, int count, SIZE* size) { 
        if (size) { size->cx = 0; size->cy = 0; }
        return 1; 
    }
    
    inline HDC GetDC(HWND hwnd) { return nullptr; }
    
    inline int ReleaseDC(HWND hwnd, HDC hdc) { return 1; }
    
    inline HBITMAP CreateDIBSection(HDC hdc, const BITMAPINFO* pbmi, unsigned int iUsage, 
                                    void** ppvBits, void* hSection, unsigned long dwOffset) { 
        return nullptr; 
    }
    
    inline HDC CreateCompatibleDC(HDC hdc) { return nullptr; }
    
    inline void* SelectObject(HDC hdc, void* hgdiobj) { return nullptr; }
    
    inline unsigned long SetBkColor(HDC hdc, unsigned long color) { return 0; }
    
    inline unsigned long SetTextColor(HDC hdc, unsigned long color) { return 0; }
    
    inline int GetTextMetrics(HDC hdc, TEXTMETRIC* lptm) { return 1; }
    
    inline HFONT CreateFont(int nHeight, int nWidth, int nEscapement, int nOrientation, 
                            int fnWeight, int fdwItalic, int fdwUnderline, int fdwStrikeOut, 
                            int fdwCharSet, int fdwOutputPrecision, int fdwClipPrecision, 
                            int fdwQuality, int fdwPitchAndFamily, const char* lpszFace) { 
        return nullptr; 
    }
    
    inline int DeleteObject(void* hObject) { return 1; }
    
    inline int DeleteDC(HDC hdc) { return 1; }


#endif // GDI_COMPAT_H
