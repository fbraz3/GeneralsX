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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: DownloadManager.cpp //////////////////////////////////////////////////////
// Generals download manager code
// Author: Matthew D. Campbell, July 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine
#include "Common/FileSystem.h"
#include "Common/UnicodeString.h"
#include "GameNetwork/DownloadManager.h"
#include "gamespy/ghttp/ghttp.h"

#include <algorithm>
#include <limits>
#include <string>

#ifndef E_FAIL
#define E_FAIL                        ((HRESULT)0x80004005L)
#endif

DownloadManager *TheDownloadManager = NULL;

namespace
{
	static Int clampByteCountToInt( long long value )
	{
		if (value <= 0)
			return 0;
		const long long maxInt = static_cast<long long>(std::numeric_limits<Int>::max());
		if (value > maxInt)
			return std::numeric_limits<Int>::max();
		return static_cast<Int>(value);
	}

	static std::string normalizePathSeparators( const char *path )
	{
		std::string out = (path != NULL) ? path : "";
		for (size_t i = 0; i < out.size(); ++i)
		{
			if (out[i] == '\\')
				out[i] = '/';
		}
		return out;
	}

	static std::string joinUrl( const char *server, const char *file )
	{
		std::string f = (file != NULL) ? file : "";
		if (f.find("://") != std::string::npos)
			return f;

		std::string s = (server != NULL) ? server : "";
		if (!s.empty() && s.find("://") == std::string::npos)
			s = std::string("http://") + s;

		if (s.empty())
			return f;

		const bool serverEndsWithSlash = (!s.empty() && s[s.size() - 1] == '/');
		const bool fileStartsWithSlash = (!f.empty() && f[0] == '/');
		if (serverEndsWithSlash && fileStartsWithSlash)
			return s + f.substr(1);
		if (!serverEndsWithSlash && !fileStartsWithSlash)
			return s + "/" + f;
		return s + f;
	}

	static std::string base64Encode( const std::string &in )
	{
		static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		std::string out;
		out.reserve(((in.size() + 2) / 3) * 4);
		int val = 0;
		int valb = -6;
		for (unsigned char c : in)
		{
			val = (val << 8) + c;
			valb += 8;
			while (valb >= 0)
			{
				out.push_back(table[(val >> valb) & 0x3F]);
				valb -= 6;
			}
		}
		if (valb > -6)
			out.push_back(table[((val << 8) >> (valb + 8)) & 0x3F]);
		while ((out.size() % 4) != 0)
			out.push_back('=');
		return out;
	}

	static void ghttpProgressTrampoline(
		GHTTPRequest /*request*/,
		GHTTPState state,
		const char* /*buffer*/,
		GHTTPByteCount /*bufferLen*/,
		GHTTPByteCount bytesReceived,
		GHTTPByteCount totalSize,
		void* param)
	{
		DownloadManager *mgr = static_cast<DownloadManager *>(param);
		if (!mgr)
			return;
		mgr->onHttpProgress(static_cast<Int>(state), static_cast<long long>(bytesReceived), static_cast<long long>(totalSize));
	}

	static GHTTPBool ghttpCompletedTrampoline(
		GHTTPRequest /*request*/,
		GHTTPResult result,
		char* /*buffer*/,
		GHTTPByteCount bufferLen,
		void* param)
	{
		DownloadManager *mgr = static_cast<DownloadManager *>(param);
		if (!mgr)
			return GHTTPTrue;
		mgr->onHttpCompleted(static_cast<Int>(result), static_cast<long long>(bufferLen));
		return GHTTPTrue;
	}
}

DownloadManager::DownloadManager() :
	m_wasError(FALSE),
	m_sawEnd(FALSE),
	m_lastHttpState(-1),
	m_startTime(0),
	m_request(-1)
{
	ghttpStartup();
}

DownloadManager::~DownloadManager()
{
	reset();
	ghttpCleanup();
}

void DownloadManager::init( void )
{
	reset();
}

HRESULT DownloadManager::update( void )
{
	if (m_request >= 0)
	{
		ghttpRequestThink(static_cast<GHTTPRequest>(m_request));
		return S_OK;
	}

	if (isFileQueuedForDownload())
	{
		return downloadNextQueuedFile();
	}

	return S_OK;
}

void DownloadManager::reset( void )
{
	if (m_request >= 0)
	{
		ghttpCancelRequest(static_cast<GHTTPRequest>(m_request));
		m_request = -1;
	}

	m_queuedDownloads.clear();
	m_wasError = FALSE;
	m_sawEnd = FALSE;
	m_lastHttpState = -1;
	m_startTime = 0;

	setStatusText("Idle");
	setErrorText("");
}

HRESULT DownloadManager::OnError( Int error )
{
	m_wasError = TRUE;
	m_sawEnd = FALSE;

	AsciiString tmp;
	tmp.format("Download failed (error %d)", error);
	setErrorText(tmp.str());
	setStatusText("Error");
	return S_OK;
}

HRESULT DownloadManager::OnEnd()
{
	m_sawEnd = TRUE;
	setStatusText("Done");
	return S_OK;
}

HRESULT DownloadManager::OnQueryResume()
{
	return S_OK;
}

HRESULT DownloadManager::OnProgressUpdate( Int /*bytesread*/, Int /*totalsize*/, Int /*timetaken*/, Int /*timeleft*/ )
{
	return S_OK;
}

HRESULT DownloadManager::OnStatusUpdate( Int status )
{
	switch (static_cast<GHTTPState>(status))
	{
		case GHTTPSocketInit:
			setStatusText("Initializing");
			break;
		case GHTTPHostLookup:
		case GHTTPLookupPending:
			setStatusText("Resolving host");
			break;
		case GHTTPConnecting:
			setStatusText("Connecting");
			break;
		case GHTTPSecuringSession:
			setStatusText("Securing session");
			break;
		case GHTTPSendingRequest:
		case GHTTPPosting:
			setStatusText("Sending request");
			break;
		case GHTTPWaiting:
			setStatusText("Waiting for response");
			break;
		case GHTTPReceivingStatus:
		case GHTTPReceivingHeaders:
			setStatusText("Receiving headers");
			break;
		case GHTTPReceivingFile:
			setStatusText("Downloading");
			break;
		default:
			setStatusText("Working");
			break;
	}
	return S_OK;
}

HRESULT DownloadManager::downloadFile( AsciiString server, AsciiString username, AsciiString password, AsciiString file, AsciiString localfile, AsciiString /*regkey*/, Bool /*tryResume*/ )
{
	if (m_request >= 0)
	{
		ghttpCancelRequest(static_cast<GHTTPRequest>(m_request));
		m_request = -1;
	}

	m_wasError = FALSE;
	m_sawEnd = FALSE;
	m_lastHttpState = -1;
	m_startTime = 0;
	setErrorText("");

	m_lastLocalFile = localfile;

	const std::string localPath = normalizePathSeparators(localfile.str());
	if (!localPath.empty())
	{
		const size_t lastSlash = localPath.find_last_of('/');
		if (lastSlash != std::string::npos)
		{
			const std::string dir = localPath.substr(0, lastSlash);
			if (!dir.empty() && TheFileSystem)
				TheFileSystem->createDirectory(AsciiString(dir.c_str()));
		}
	}

	const std::string url = joinUrl(server.str(), file.str());
	if (url.empty())
	{
		OnError(static_cast<Int>(GHTTPInvalidURL));
		return E_FAIL;
	}

	std::string headers;
	if (!username.isEmpty())
	{
		const std::string userpass = std::string(username.str()) + ":" + std::string(password.str());
		headers = "Authorization: Basic " + base64Encode(userpass) + "\r\n";
	}

	m_startTime = static_cast<UnsignedInt>(time(NULL));
	setStatusText("Starting");

	m_request = static_cast<Int>(
		ghttpSaveEx(
			url.c_str(),
			localPath.c_str(),
			headers.empty() ? NULL : headers.c_str(),
			NULL,
			GHTTPFalse,
			GHTTPFalse,
			ghttpProgressTrampoline,
			ghttpCompletedTrampoline,
			this));

	if (IS_GHTTP_ERROR(m_request))
	{
		OnError(m_request);
		m_request = -1;
		return E_FAIL;
	}

	return S_OK;
}

void DownloadManager::queueFileForDownload( AsciiString server, AsciiString username, AsciiString password, AsciiString file, AsciiString localfile, AsciiString regkey, Bool tryResume )
{
	QueuedDownload q;
	q.server = server;
	q.userName = username;
	q.password = password;
	q.file = file;
	q.localFile = localfile;
	q.regKey = regkey;
	q.tryResume = tryResume;
	m_queuedDownloads.push_back(q);
}

HRESULT DownloadManager::downloadNextQueuedFile( void )
{
	if (m_queuedDownloads.empty())
		return S_OK;

	QueuedDownload q = m_queuedDownloads.front();
	m_queuedDownloads.pop_front();
	return downloadFile(q.server, q.userName, q.password, q.file, q.localFile, q.regKey, q.tryResume);
}

void DownloadManager::setStatusText( const char *text )
{
	UnicodeString tmp;
	tmp.translate(text ? text : "");
	m_statusString = tmp;
}

void DownloadManager::setErrorText( const char *text )
{
	UnicodeString tmp;
	tmp.translate(text ? text : "");
	m_errorString = tmp;
}

void DownloadManager::onHttpProgress( Int state, long long bytesReceived, long long totalSize )
{
	if (state != m_lastHttpState)
	{
		m_lastHttpState = state;
		OnStatusUpdate(state);
	}

	const UnsignedInt now = static_cast<UnsignedInt>(time(NULL));
	const Int timeTaken = (m_startTime > 0 && now >= m_startTime) ? static_cast<Int>(now - m_startTime) : 0;

	Int timeLeft = 0;
	if (totalSize > 0 && bytesReceived > 0 && timeTaken > 0)
	{
		const double bytesPerSecond = static_cast<double>(bytesReceived) / static_cast<double>(timeTaken);
		if (bytesPerSecond > 0.0)
		{
			const double remaining = static_cast<double>(totalSize - bytesReceived);
			if (remaining > 0.0)
				timeLeft = static_cast<Int>(remaining / bytesPerSecond);
		}
	}

	OnProgressUpdate(
		clampByteCountToInt(bytesReceived),
		clampByteCountToInt(totalSize),
		timeTaken,
		timeLeft);
}

void DownloadManager::onHttpCompleted( Int result, long long totalSize )
{
	const Int request = m_request;
	m_request = -1;

	if (result == static_cast<Int>(GHTTPSuccess))
	{
		OnProgressUpdate(clampByteCountToInt(totalSize), clampByteCountToInt(totalSize), static_cast<Int>(time(NULL) - m_startTime), 0);
		OnEnd();
	}
	else
	{
		AsciiString err;
		err.format("HTTP download failed (result %d)", result);
		setErrorText(err.str());
		OnError(result);
	}

	(void)request;
}

