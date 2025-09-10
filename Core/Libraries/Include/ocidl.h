#pragma once
#ifndef _OCIDL_H_
#define _OCIDL_H_

// OLE Control interface compatibility for macOS port
#ifdef __APPLE__

#include "objbase.h"

// OLE Control types and interfaces compatibility layer
// This header provides minimal stubs for OLE control functionality

// Basic OLE types
typedef struct {
    DWORD dwSize;
    LPOLESTR lpszDocString;
    LPOLESTR lpszHelpFile;
    DWORD dwHelpContext;
} PROPPAGEINFO;

// Font description
typedef struct tagFONTDESC {
    UINT cbSizeofstruct;
    LPOLESTR lpstrName;
    LONG cySize;
    short sWeight;
    short sCharset;
    BOOL fItalic;
    BOOL fUnderline;
    BOOL fStrikethrough;
} FONTDESC, *LPFONTDESC;

// Picture description  
typedef struct tagPICTDESC {
    UINT cbSizeofstruct;
    UINT picType;
    union {
        struct {
            HBITMAP hbitmap;
            HPALETTE hpal;
        } bmp;
        struct {
            HMETAFILE hmeta;
            int xExt;
            int yExt;
        } wmf;
        struct {
            HICON hicon;
        } icon;
        struct {
            HENHMETAFILE hemf;
        } emf;
    };
} PICTDESC, *LPPICTDESC;

// Helper functions (stubbed)
inline HRESULT OleCreateFontIndirect(LPFONTDESC lpFontDesc, REFIID riid, LPVOID* lplpvObj) {
    return E_NOTIMPL;
}

inline HRESULT OleCreatePictureIndirect(LPPICTDESC lpPictDesc, REFIID riid, BOOL fOwn, LPVOID* lplpvObj) {
    return E_NOTIMPL;
}

#endif // __APPLE__
#endif // _OCIDL_H_
