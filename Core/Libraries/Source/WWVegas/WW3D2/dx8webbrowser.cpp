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

//******************************************************************************************
//
// Earth And Beyond
// Copyright (c) 2002 Electronic Arts , Inc.  -  Westwood Studios
//
// File Name		: dx8webbrowser.cpp
// Description		: Implementation of D3D Embedded Browser wrapper.
// Author			: Darren Schueller
// Date of Creation	: 6/4/2002
//
//******************************************************************************************
// $Header: $
//******************************************************************************************

#include "dx8webbrowser.h"
#include "ww3d.h"
#include "dx8wrapper.h"

#if ENABLE_EMBEDDED_BROWSER

#if defined(_MSC_VER) && _MSC_VER < 1300

// Import the Browser Type Library
// BGC, the path for the dll file is pretty odd, no?
//      I'll leave it like this till I can figure out a
//      better way.
#import "../../../../../Run/BrowserEngine.dll" no_namespace

#else

#include <windows.h>
#include <objbase.h>
#include <oleauto.h>

#include "EABrowserEngine/BrowserEngine.h"

class AutoBSTR
{
public:
	AutoBSTR() : m_bstr(NULL) {}
	explicit AutoBSTR(const char* ansi) : m_bstr(NULL) { reset(ansi); }
	~AutoBSTR() { reset(NULL); }

	BSTR get() const { return m_bstr; }
	operator BSTR() const { return m_bstr; }

	void reset(const char* ansi)
	{
		if (m_bstr)
		{
			SysFreeString(m_bstr);
			m_bstr = NULL;
		}
		if (!ansi)
		{
			return;
		}

		int wideLen = MultiByteToWideChar(CP_ACP, 0, ansi, -1, NULL, 0);
		if (wideLen <= 0)
		{
			return;
		}

		m_bstr = SysAllocStringLen(NULL, wideLen - 1);
		if (!m_bstr)
		{
			return;
		}
		MultiByteToWideChar(CP_ACP, 0, ansi, -1, m_bstr, wideLen);
	}

private:
	BSTR m_bstr;
};

#endif

#if defined(_MSC_VER) && _MSC_VER < 1300
static IFEBrowserEngine2Ptr pBrowser = 0;
#else
static IFEBrowserEngine2* pBrowser = 0;
#endif

HWND		DX8WebBrowser::hWnd = 0;

bool DX8WebBrowser::Initialize(	const char* badpageurl,
											const char* loadingpageurl,
											const char* mousefilename,
											const char* mousebusyfilename)
{
	if(pBrowser == 0)
	{
		// Initialize COM
		CoInitialize(0);

		// Create an instance of the browser control
#if defined(_MSC_VER) && _MSC_VER < 1300
		HRESULT hr = pBrowser.CreateInstance(__uuidof(FEBrowserEngine2));
#else
		HRESULT hr = CoCreateInstance(__uuidof(FEBrowserEngine2), NULL, CLSCTX_INPROC_SERVER, __uuidof(IFEBrowserEngine2), reinterpret_cast<void**>(&pBrowser));
#endif

		if(hr == REGDB_E_CLASSNOTREG)
		{
			HMODULE lib = ::LoadLibrary("BrowserEngine.DLL");
			if(lib)
			{
				FARPROC proc = ::GetProcAddress(lib,"DllRegisterServer");
				if(proc)
				{
					proc();
					// Create an instance of the browser control
					#if defined(_MSC_VER) && _MSC_VER < 1300
					hr = pBrowser.CreateInstance(__uuidof(FEBrowserEngine2));
					#else
					hr = CoCreateInstance(__uuidof(FEBrowserEngine2), NULL, CLSCTX_INPROC_SERVER, __uuidof(IFEBrowserEngine2), reinterpret_cast<void**>(&pBrowser));
					#endif
				}
				FreeLibrary(lib);
			}
		}

		// Initialize the browser.
		if(hr == S_OK)
		{
			hWnd = (HWND)WW3D::Get_Window();
			pBrowser->Initialize(reinterpret_cast<long*>(DX8Wrapper::_Get_D3D_Device8()));

			if(badpageurl)
				#if defined(_MSC_VER) && _MSC_VER < 1300
				pBrowser->put_BadPageURL(_bstr_t(badpageurl));
				#else
				pBrowser->put_BadPageURL(AutoBSTR(badpageurl));
				#endif

			if(loadingpageurl)
				#if defined(_MSC_VER) && _MSC_VER < 1300
				pBrowser->put_LoadingPageURL(_bstr_t(loadingpageurl));
				#else
				pBrowser->put_LoadingPageURL(AutoBSTR(loadingpageurl));
				#endif

			if(mousefilename)
				#if defined(_MSC_VER) && _MSC_VER < 1300
				pBrowser->put_MouseFileName(_bstr_t(mousefilename));
				#else
				pBrowser->put_MouseFileName(AutoBSTR(mousefilename));
				#endif

			if(mousebusyfilename)
				#if defined(_MSC_VER) && _MSC_VER < 1300
				pBrowser->put_MouseBusyFileName(_bstr_t(mousebusyfilename));
				#else
				pBrowser->put_MouseBusyFileName(AutoBSTR(mousebusyfilename));
				#endif
		}
		else
		{
			pBrowser = 0;
			return false;
		}
	}

	return true;
}

void DX8WebBrowser::Shutdown()
{
	if(pBrowser)
	{
		// Shutdown the browser
		pBrowser->Shutdown();

		#if defined(_MSC_VER) && _MSC_VER < 1300
		pBrowser = 0;
		#else
		pBrowser->Release();
		pBrowser = 0;
		#endif

		hWnd = 0;

		// Shut down COM
		CoUninitialize();
	}
}


// ******************************************************************************************
// * Function Name: DX8WebBrowser::Update
// ******************************************************************************************
// * Description: 	Updates the browser image surfaces by copying the bits from the browser
// *						DCs to the D3D Image surfaces.
// *
// * Return Type:
// *
// * Argument:    	void
// *
// ******************************************************************************************
void	DX8WebBrowser::Update(void)
{
	if(pBrowser) pBrowser->D3DUpdate();
};


// ******************************************************************************************
// * Function Name: DX8WebBrowser::Render
// ******************************************************************************************
// * Description: 	Draws all browsers to the back buffer.
// *
// * Return Type:
// *
// * Argument:    	int backbufferindex
// *
// ******************************************************************************************
void	DX8WebBrowser::Render(int backbufferindex)
{
	if(pBrowser) pBrowser->D3DRender(backbufferindex);
};

// ******************************************************************************************
// * Function Name: DX8WebBrowser::CreateBrowser
// ******************************************************************************************
// * Description: 	Creates a browser window.
// *
// * Return Type:
// *
// * Argument:    	const char* browsername -	This is a "name" used to identify the
// *															browser instance.  Multiple browsers can
// *															be created, and are referenced using this name.
// * Argument:    	const char* url - The url to display.
// * Argument:    	int x - The position and size of the browser (in pixels)
// * Argument:    	int y
// * Argument:    	int w
// * Argument:    	int h
// * Argument:    	int updateticks - When non-zero, this forces the browser image to get updated
// *												at the specified rate (number of milliseconds) regardless
// *												of paint messages.  When this is zero (the default) the browser
// *												image is only updated whenever a paint message is received.
// *
// ******************************************************************************************
void	DX8WebBrowser::CreateBrowser(const char* browsername, const char* url, int x, int y, int w, int h, int updateticks, LONG options, LPDISPATCH gamedispatch)
{
	WWDEBUG_SAY(("DX8WebBrowser::CreateBrowser - Creating browser with the name %s, url = %s, (x, y, w, h) = (%d, %d, %d, %d), update ticks = %d", browsername, url, x, y, h, w, updateticks));
	if(pBrowser)
	{
		#if defined(_MSC_VER) && _MSC_VER < 1300
		_bstr_t brsname(browsername);
		pBrowser->CreateBrowser(brsname, _bstr_t(url), reinterpret_cast<long>(hWnd), x, y, w, h, options, gamedispatch);
		pBrowser->SetUpdateRate(brsname, updateticks);
		#else
		AutoBSTR brsname(browsername);
		AutoBSTR burl(url);
		pBrowser->CreateBrowser(brsname.get(), burl.get(), reinterpret_cast<long>(hWnd), x, y, w, h, options, gamedispatch);
		pBrowser->SetUpdateRate(brsname.get(), updateticks);
		#endif
	}
}


// ******************************************************************************************
// * Function Name: DX8WebBrowser::DestroyBrowser
// ******************************************************************************************
// * Description: 	Destroys the specified browser.  This closes the window and releases
// *						the browser instance.
// *
// * Return Type:
// *
// * Argument:    	const char* browsername - The name of the browser to destroy.
// *
// ******************************************************************************************
void	DX8WebBrowser::DestroyBrowser(const char* browsername)
{
	WWDEBUG_SAY(("DX8WebBrowser::DestroyBrowser - destroying browser %s", browsername));
	if(pBrowser)
		#if defined(_MSC_VER) && _MSC_VER < 1300
		pBrowser->DestroyBrowser(_bstr_t(browsername));
		#else
		pBrowser->DestroyBrowser(AutoBSTR(browsername));
		#endif
}


// ******************************************************************************************
// * Function Name: DX8WebBrowser::Is_Browser_Open
// ******************************************************************************************
// * Description: 	This function checks to see if a browser of the specified name exists and
// *						is currently open.
// *
// * Return Type:
// *
// * Argument:    	const char* browsername - The name of the browser to test.
// *
// ******************************************************************************************
bool	DX8WebBrowser::Is_Browser_Open(const char* browsername)
{
	if(pBrowser == 0) return false;
#if defined(_MSC_VER) && _MSC_VER < 1300
	return (pBrowser->IsOpen(_bstr_t(browsername)) != 0);
#else
	long isOpen;
	return (pBrowser->IsOpen(AutoBSTR(browsername), &isOpen) != 0);
#endif
}

// ******************************************************************************************
// * Function Name: DX8WebBrowser::Navigate
// ******************************************************************************************
// * Description: 	This function causes the browser to navigate to the specified page.
// *
// * Return Type:
// *
// * Argument:    	const char* browsername - The name of the browser to test.
// *						const char* url - The url to navigate to.
// *
// ******************************************************************************************
void	DX8WebBrowser::Navigate(const char* browsername, const char* url)
{
	if(pBrowser == 0) return;
	#if defined(_MSC_VER) && _MSC_VER < 1300
	pBrowser->Navigate(_bstr_t(browsername),_bstr_t(url));
	#else
	pBrowser->Navigate(AutoBSTR(browsername), AutoBSTR(url));
	#endif
}

#endif
