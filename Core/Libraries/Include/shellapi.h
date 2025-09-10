#pragma once
#ifndef _SHELLAPI_H_
#define _SHELLAPI_H_

// Shell API compatibility for macOS port
#ifdef __APPLE__

#include "windows.h"

// Shell execution constants
#define SW_HIDE 0
#define SW_SHOWNORMAL 1
#define SW_NORMAL 1
#define SW_SHOWMINIMIZED 2
#define SW_SHOWMAXIMIZED 3
#define SW_MAXIMIZE 3
#define SW_SHOWNOACTIVATE 4
#define SW_SHOW 5
#define SW_MINIMIZE 6
#define SW_SHOWMINNOACTIVE 7
#define SW_SHOWNA 8
#define SW_RESTORE 9
#define SW_SHOWDEFAULT 10
#define SW_FORCEMINIMIZE 11
#define SW_MAX 11

// Shell execute function (stubbed)
inline HINSTANCE ShellExecute(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, 
                             LPCSTR lpParameters, LPCSTR lpDirectory, int nShowCmd) {
    // Stub implementation - in a real port, would use system() or execve()
    return (HINSTANCE)32; // Success code in Windows
}

inline HINSTANCE ShellExecuteA(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, 
                              LPCSTR lpParameters, LPCSTR lpDirectory, int nShowCmd) {
    return ShellExecute(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);
}

inline HINSTANCE ShellExecuteW(HWND hwnd, LPCWSTR lpOperation, LPCWSTR lpFile, 
                              LPCWSTR lpParameters, LPCWSTR lpDirectory, int nShowCmd) {
    // Stub implementation - in a real port, would convert wide strings and call system()
    return (HINSTANCE)32; // Success code in Windows
}

// Shell execute extended info structure
typedef struct _SHELLEXECUTEINFO {
    DWORD cbSize;
    ULONG fMask;
    HWND hwnd;
    LPCSTR lpVerb;
    LPCSTR lpFile;
    LPCSTR lpParameters;
    LPCSTR lpDirectory;
    int nShow;
    HINSTANCE hInstApp;
    LPVOID lpIDList;
    LPCSTR lpClass;
    HKEY hkeyClass;
    DWORD dwHotKey;
    union {
        HANDLE hIcon;
        HANDLE hMonitor;
    };
    HANDLE hProcess;
} SHELLEXECUTEINFO, *LPSHELLEXECUTEINFO;

// Shell execute extended function (stubbed)
inline BOOL ShellExecuteEx(LPSHELLEXECUTEINFO lpExecInfo) {
    // Stub implementation - in a real port, would use system() or execve()
    if (lpExecInfo) {
        lpExecInfo->hInstApp = (HINSTANCE)32; // Success code
    }
    return TRUE;
}

// Extract icon function (stubbed)
inline HICON ExtractIcon(HINSTANCE hInst, LPCSTR lpszExeFileName, UINT nIconIndex) {
    // Stub implementation - icons not supported in this compatibility layer
    return NULL;
}

inline HICON ExtractIconA(HINSTANCE hInst, LPCSTR lpszExeFileName, UINT nIconIndex) {
    return ExtractIcon(hInst, lpszExeFileName, nIconIndex);
}

inline HICON ExtractIconW(HINSTANCE hInst, LPCWSTR lpszExeFileName, UINT nIconIndex) {
    // Stub implementation - icons not supported in this compatibility layer
    return NULL;
}

// Shell notify icon structures and functions (stubbed)
#define NIM_ADD 0x00000000
#define NIM_MODIFY 0x00000001
#define NIM_DELETE 0x00000002

typedef struct _NOTIFYICONDATA {
    DWORD cbSize;
    HWND hWnd;
    UINT uID;
    UINT uFlags;
    UINT uCallbackMessage;
    HICON hIcon;
    CHAR szTip[128];
    DWORD dwState;
    DWORD dwStateMask;
    CHAR szInfo[256];
    UINT uTimeout;
    CHAR szInfoTitle[64];
    DWORD dwInfoFlags;
} NOTIFYICONDATA, *PNOTIFYICONDATA;

inline BOOL Shell_NotifyIcon(DWORD dwMessage, PNOTIFYICONDATA lpData) {
    // Stub implementation - system tray not supported in this compatibility layer
    return TRUE;
}

// Command line parsing (stubbed)
inline LPWSTR* CommandLineToArgvW(LPCWSTR lpCmdLine, int* pNumArgs) {
    // Stub implementation - command line parsing not implemented
    if (pNumArgs) *pNumArgs = 0;
    return NULL;
}

// Registry key type
typedef void* HKEY;

#endif // __APPLE__
#endif // _SHELLAPI_H_
