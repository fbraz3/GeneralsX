/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "Common/urllaunch.h"
#include <SDL2/SDL.h>  // Phase 40: SDL_OpenURL for cross-platform URL opening

#define FILE_PREFIX     L"file://"


///////////////////////////////////////////////////////////////////////////////
HRESULT MakeEscapedURL( LPWSTR pszInURL, LPWSTR *ppszOutURL )
{
    if( ( NULL == pszInURL ) || ( NULL == ppszOutURL ) )
    {
        return( E_INVALIDARG );
    }

    //
    // Do we need to pre-pend file://?
    //
    BOOL fNeedFilePrefix = ( 0 == wcsstr( pszInURL, L"://" ) );

    //
    // Count how many characters need to be escaped
    //
    LPWSTR pszTemp = pszInURL;
    DWORD cEscapees = 0;

    while( TRUE )
    {
        LPWSTR pchToEscape = wcspbrk( pszTemp, L" #$%&\\+,;=@[]^{}" );

        if( NULL == pchToEscape )
        {
            break;
        }

        cEscapees++;

        pszTemp = pchToEscape + 1;
    }

    //
    // Allocate sufficient outgoing buffer space
    //
    int cchNeeded = wcslen( pszInURL ) + ( 2 * cEscapees ) + 1;

    if( fNeedFilePrefix )
    {
        cchNeeded += wcslen( FILE_PREFIX );
    }

    *ppszOutURL = new WCHAR[ cchNeeded ];

    if( NULL == *ppszOutURL )
    {
        return( E_OUTOFMEMORY );
    }

    //
    // Fill in the outgoing escaped buffer
    //
    pszTemp = pszInURL;

    LPWSTR pchNext = *ppszOutURL;

    if( fNeedFilePrefix )
    {
        wcscpy( *ppszOutURL, FILE_PREFIX );
        pchNext += wcslen( FILE_PREFIX );
    }

    while( TRUE )
    {
        LPWSTR pchToEscape = wcspbrk( pszTemp, L" #$%&\\+,;=@[]^{}" );

        if( NULL == pchToEscape )
        {
            //
            // Copy the rest of the input string and get out
            //
            wcscpy( pchNext, pszTemp );
            break;
        }

        //
        // Copy all characters since the previous escapee
        //
        int cchToCopy = pchToEscape - pszTemp;

        if( cchToCopy > 0 )
        {
            wcsncpy( pchNext, pszTemp, cchToCopy );

            pchNext += cchToCopy;
        }

        //
        // Expand this character into an escape code and move on
        //
        pchNext += swprintf( pchNext, L"%%%02x", *pchToEscape );

        pszTemp = pchToEscape + 1;
    }

    return( S_OK );
}


///////////////////////////////////////////////////////////////////////////////
// Phase 40: Replaced GetShellOpenCommand with direct SDL_OpenURL (no registry lookup needed)
// SDL_OpenURL handles platform-specific browser opening on macOS, Linux, Windows
HRESULT GetShellOpenCommand( LPTSTR ptszShellOpenCommand, DWORD cbShellOpenCommand )
{
    // Phase 40: Registry-based browser detection removed - SDL_OpenURL is cross-platform
    // This function is now deprecated but maintained for backwards compatibility
    // In production code, call SDL_OpenURL() directly instead
    
    if( ptszShellOpenCommand == NULL || cbShellOpenCommand == 0 )
    {
        return E_INVALIDARG;
    }
    
    // Return a stub command - no longer used
    _tcscpy_s( ptszShellOpenCommand, cbShellOpenCommand / sizeof(TCHAR), _T("sdl_openurl") );
    return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
// Phase 40: Replaced Win32 CreateProcess with cross-platform SDL_OpenURL
// SDL_OpenURL is the SDL2 equivalent for opening URLs in the system browser
HRESULT LaunchURL( LPCWSTR pszURL )
{
    if( pszURL == NULL || wcslen(pszURL) == 0 )
    {
        return E_INVALIDARG;
    }

    // Phase 40: Convert wide character URL to UTF-8 for SDL_OpenURL
    // SDL2 expects const char* in UTF-8 format
    
    // Calculate required buffer size for UTF-8 conversion
    int urlLen = WideCharToMultiByte(CP_UTF8, 0, pszURL, -1, NULL, 0, NULL, NULL);
    if( urlLen <= 0 )
    {
        printf("LaunchURL: Failed to determine URL buffer size\n");
        return E_FAIL;
    }
    
    char* pszUrlUtf8 = new char[urlLen];
    if( pszUrlUtf8 == NULL )
    {
        return E_OUTOFMEMORY;
    }
    
    // Convert wide URL to UTF-8
    WideCharToMultiByte(CP_UTF8, 0, pszURL, -1, pszUrlUtf8, urlLen, NULL, NULL);
    
    printf("LaunchURL: Opening URL via SDL_OpenURL: %s\n", pszUrlUtf8);
    
    // Phase 40: Use SDL_OpenURL instead of Win32 registry + CreateProcess
    // Returns 0 on success, -1 on failure
    int result = SDL_OpenURL(pszUrlUtf8);
    
    delete[] pszUrlUtf8;
    
    if( result == 0 )
    {
        printf("LaunchURL: SDL_OpenURL succeeded\n");
        return S_OK;
    }
    else
    {
        printf("LaunchURL: SDL_OpenURL failed with error code %d\n", result);
        return E_FAIL;
    }
}
