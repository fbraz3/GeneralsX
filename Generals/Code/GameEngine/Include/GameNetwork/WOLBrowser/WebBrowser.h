/*
**	Command & Conquer Generals(tm)
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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

/******************************************************************************
*
* NAME
*     $Archive:  $
*
* DESCRIPTION
*     Web Browser
*
* PROGRAMMER
*     Bryan Cleveland
*     $Author:  $
*
* VERSION INFO
*     $Revision:  $
*     $Modtime:  $
*
******************************************************************************/

#pragma once

#ifndef __WEBBROWSER_H__
#define __WEBBROWSER_H__

#include "Common/SubsystemInterface.h"
#ifndef _WIN32
// Browser functionality disabled on non-Windows platforms
#else
#include <atlbase.h>
#include <windows.h>
#include <Common/GameMemory.h>
#include "EABrowserDispatch/BrowserDispatch.h"
#include "FEBDispatch.h"
#endif
#include <Lib/BaseType.h>

class GameWindow;

class WebBrowserURL : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( WebBrowserURL, "WebBrowserURL" )

public:

	WebBrowserURL();
	// virtual destructor prototype defined by memory pool object

	const FieldParse *getFieldParse( void ) const { return m_URLFieldParseTable; }

	AsciiString m_tag;
	AsciiString m_url;

	WebBrowserURL *m_next;

	static const FieldParse m_URLFieldParseTable[];		///< the parse table for INI definition

};



class WebBrowser :
#ifdef _WIN32
		public FEBDispatch<WebBrowser, IBrowserDispatch, &IID_IBrowserDispatch>,
#endif
		public SubsystemInterface
	{
	public:
		void init( void );
		void reset( void );
		void update( void );

		// Create an instance of the embedded browser for Dune Emperor.
		virtual Bool createBrowserWindow(const char *tag, GameWindow *win) = 0;
		virtual void closeBrowserWindow(GameWindow *win) = 0;

		WebBrowserURL *makeNewURL(AsciiString tag);
		WebBrowserURL *findURL(AsciiString tag);

	protected:
		// Protected to prevent direct construction via new, use CreateInstance() instead.
		WebBrowser();
		virtual ~WebBrowser();

		// Protected to prevent copy and assignment
		WebBrowser(const WebBrowser&);
		const WebBrowser& operator=(const WebBrowser&);

//		Bool RetrievePageURL(const char* page, char* url, int size);
//		Bool RetrieveHTMLPath(char* path, int size);

	protected:
#ifdef _WIN32
		ULONG mRefCount;
#endif
		WebBrowserURL *m_urlList;

#ifdef _WIN32
	//---------------------------------------------------------------------------
	// IUnknown methods
	//---------------------------------------------------------------------------
	protected:
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) IUNKNOWN_NOEXCEPT;
		ULONG STDMETHODCALLTYPE AddRef(void) IUNKNOWN_NOEXCEPT;
		ULONG STDMETHODCALLTYPE Release(void) IUNKNOWN_NOEXCEPT;

	//---------------------------------------------------------------------------
	// IBrowserDispatch methods
	//---------------------------------------------------------------------------
	public:
		STDMETHOD(TestMethod)(Int num1);
#endif
	};

extern CComObject<WebBrowser> *TheWebBrowser;
#endif // __WEBBROWSER_H__
