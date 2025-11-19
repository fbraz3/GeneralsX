/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
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

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine
#include "Common/WorkerProcess.h"


WorkerProcess::WorkerProcess()
{
	m_processHandle = NULL;
	m_readHandle = NULL;
	m_jobHandle = NULL;
	m_exitcode = 0;
	m_isDone = false;
}

bool WorkerProcess::startProcess(UnicodeString command)
{
	m_stdOutput.clear();
	m_isDone = false;

	// Create pipe for reading console output
	SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
	saAttr.bInheritHandle = TRUE;
	HANDLE writeHandle = NULL;
	if (!CreatePipe(&m_readHandle, &writeHandle, &saAttr, 0))
		return false;
	SetHandleInformation(m_readHandle, HANDLE_FLAG_INHERIT, 0);

	STARTUPINFOW si = { sizeof(STARTUPINFOW) };
	si.dwFlags = STARTF_FORCEOFFFEEDBACK; // Prevent cursor wait animation
	si.dwFlags |= STARTF_USESTDHANDLES;
	si.hStdError = writeHandle;
	si.hStdOutput = writeHandle;

	PROCESS_INFORMATION pi = { 0 };

	if (!CreateProcessW(NULL, (LPWSTR)command.str(),
			NULL, NULL, /*bInheritHandles=*/TRUE, 0,
			NULL, 0, &si, &pi))
	{
		CloseHandle(writeHandle);
		CloseHandle(m_readHandle);
		m_readHandle = NULL;
		return false;
	}

	CloseHandle(pi.hThread);
	CloseHandle(writeHandle);
	m_processHandle = pi.hProcess;

	// We want to make sure that when our process is killed, our workers automatically terminate as well.
	// In Windows, the way to do this is to attach the worker to a job we own.
	m_jobHandle = CreateJobObjectW != NULL ? CreateJobObjectW(NULL, NULL) : NULL;
	if (m_jobHandle != NULL)
	{
		JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobInfo = { 0 };
		jobInfo.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
		SetInformationJobObject(m_jobHandle, (JOBOBJECTINFOCLASS)JobObjectExtendedLimitInformation, &jobInfo, sizeof(jobInfo));
		AssignProcessToJobObject(m_jobHandle, m_processHandle);
	}

	return true;
}

bool WorkerProcess::isRunning() const
{
	return m_processHandle != NULL;
}

bool WorkerProcess::isDone() const
{
	return m_isDone;
}

DWORD WorkerProcess::getExitCode() const
{
	return m_exitcode;
}

AsciiString WorkerProcess::getStdOutput() const
{
	return m_stdOutput;
}

bool WorkerProcess::fetchStdOutput()
{
	while (true)
	{
		// Call PeekNamedPipe to make sure ReadFile won't block
		DWORD bytesAvailable = 0;
		DEBUG_ASSERTCRASH(m_readHandle != NULL, ("Is not expected NULL"));
		BOOL success = PeekNamedPipe(m_readHandle, NULL, 0, NULL, &bytesAvailable, NULL);
		if (!success)
			return true;
		if (bytesAvailable == 0)
		{
			// Child process is still running and we have all output so far
			return false;
		}

		DWORD readBytes = 0;
		char buffer[1024];
		success = ReadFile(m_readHandle, buffer, ARRAY_SIZE(buffer)-1, &readBytes, NULL);
		if (!success)
			return true;
		DEBUG_ASSERTCRASH(readBytes != 0, ("expected readBytes to be non null"));

		// Remove \r, otherwise each new line is doubled when we output it again
		for (int i = 0; i < readBytes; i++)
			if (buffer[i] == '\r')
				buffer[i] = ' ';
		buffer[readBytes] = 0;
		m_stdOutput.concat(buffer);
	}
}

void WorkerProcess::update()
{
	if (!isRunning())
		return;

	if (!fetchStdOutput())
	{
		// There is still potential output pending
		return;
	}

	// Pipe broke, that means the process already exited. But we call this just to make sure
	WaitForSingleObject(m_processHandle, INFINITE);
	GetExitCodeProcess(m_processHandle, &m_exitcode);
	CloseHandle(m_processHandle);
	m_processHandle = NULL;

	CloseHandle(m_readHandle);
	m_readHandle = NULL;

	CloseHandle(m_jobHandle);
	m_jobHandle = NULL;

	m_isDone = true;
}

void WorkerProcess::kill()
{
	if (!isRunning())
		return;

	if (m_processHandle != NULL)
	{
		TerminateProcess(m_processHandle, 1);
		CloseHandle(m_processHandle);
		m_processHandle = NULL;
	}

	if (m_readHandle != NULL)
	{
		CloseHandle(m_readHandle);
		m_readHandle = NULL;
	}

	if (m_jobHandle != NULL)
	{
		CloseHandle(m_jobHandle);
		m_jobHandle = NULL;
	}

	m_stdOutput.clear();
	m_isDone = false;
}

#else  // Unix/macOS stubs

WorkerProcess::WorkerProcess()
{
	m_processHandle = NULL;
	m_readHandle = NULL;
	m_jobHandle = NULL;
	m_exitcode = 0;
	m_isDone = false;
}

bool WorkerProcess::startProcess(UnicodeString command)
{
	// TODO: Implement Unix process creation with pipes using fork/exec
	// For now, return false to indicate worker processes are not supported on this platform
	return false;
}

bool WorkerProcess::isRunning() const
{
	return false;
}

bool WorkerProcess::isDone() const
{
	return m_isDone;
}

DWORD WorkerProcess::getExitCode() const
{
	return 0;
}

AsciiString WorkerProcess::getStdOutput() const
{
	return m_stdOutput;
}

bool WorkerProcess::fetchStdOutput()
{
	return true;
}

void WorkerProcess::update()
{
	// No-op on Unix/macOS
}

void WorkerProcess::kill()
{
	// No-op on Unix/macOS
}

#endif  // _WIN32

