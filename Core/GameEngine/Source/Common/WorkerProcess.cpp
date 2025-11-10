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

// Provide a POSIX fallback implementation for non-Windows targets.
// On Windows we keep the original behavior; on POSIX we implement
// equivalent functionality using fork/pipe/exec and waitpid.
#if !defined(_WIN32)
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

// Define minimal job object related constants so Windows-only code can compile.
#ifndef JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE
#define JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE 0x00002000
#endif

#endif // !_WIN32

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

#if defined(_WIN32)
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
#else
	// POSIX implementation using fork/exec and pipes.
	int pipefd[2];
	if (pipe(pipefd) != 0)
		return false;

	// Make read end non-blocking to mirror PeekNamedPipe/ReadFile behavior
	int flags = fcntl(pipefd[0], F_GETFL, 0);
	fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);

	pid_t pid = fork();
	if (pid < 0)
	{
		close(pipefd[0]);
		close(pipefd[1]);
		return false;
	}
	if (pid == 0)
	{
		// Child
		dup2(pipefd[1], STDOUT_FILENO);
		dup2(pipefd[1], STDERR_FILENO);
		close(pipefd[0]);
		close(pipefd[1]);
		// Execute via shell so UnicodeString command works as a single string
		execl("/bin/sh", "sh", "-c", (const char*)command.str(), (char*)NULL);
		// If exec fails
		_exit(127);
	}

	// Parent
	close(pipefd[1]);
	m_readHandle = (HANDLE)(intptr_t)pipefd[0];
	m_processHandle = (HANDLE)(intptr_t)pid;
	m_jobHandle = NULL; // not used on POSIX
	return true;
#endif
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
#if defined(_WIN32)
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
#else
	DEBUG_ASSERTCRASH(m_readHandle != NULL, ("Is not expected NULL"));
	int fd = (int)(intptr_t)m_readHandle;
	char buffer[1024];
	ssize_t n = read(fd, buffer, sizeof(buffer)-1);
	if (n > 0)
	{
		for (ssize_t i = 0; i < n; ++i)
			if (buffer[i] == '\r') buffer[i] = ' ';
		buffer[n] = 0;
		m_stdOutput.concat(buffer);
		// there may be more data; return false to indicate still running
		return false;
	}
	if (n == 0)
	{
		// EOF: child closed pipe — indicate output fully read
		return true;
	}
	// n < 0
	if (errno == EAGAIN || errno == EWOULDBLOCK)
		return false; // no data available now
	// other error -> treat as finished
	return true;
#endif
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

#if defined(_WIN32)
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
#else
	pid_t pid = (pid_t)(intptr_t)m_processHandle;
	int status = 0;
	pid_t r = waitpid(pid, &status, WNOHANG);
	if (r == 0)
	{
		// still running
		return;
	}
	if (r == pid)
	{
		if (WIFEXITED(status))
			m_exitcode = (DWORD)WEXITSTATUS(status);
		else
			m_exitcode = (DWORD)1;
	}

	// cleanup
	if (m_readHandle != NULL)
	{
		close((int)(intptr_t)m_readHandle);
		m_readHandle = NULL;
	}
	m_processHandle = NULL;
	m_jobHandle = NULL;
	m_isDone = true;
#endif
}

void WorkerProcess::kill()
{
	if (!isRunning())
		return;

	if (m_processHandle != NULL)
	{
#if defined(_WIN32)
	TerminateProcess(m_processHandle, 1);
	CloseHandle(m_processHandle);
	m_processHandle = NULL;
#else
	pid_t pid = (pid_t)(intptr_t)m_processHandle;
	::kill(pid, SIGKILL);
	m_processHandle = NULL;
#endif
	}

	if (m_readHandle != NULL)
	{
#if defined(_WIN32)
	CloseHandle(m_readHandle);
	m_readHandle = NULL;
#else
	close((int)(intptr_t)m_readHandle);
	m_readHandle = NULL;
#endif
	}

	if (m_jobHandle != NULL)
	{
#if defined(_WIN32)
	CloseHandle(m_jobHandle);
	m_jobHandle = NULL;
#else
	m_jobHandle = NULL;
#endif
	}

	m_stdOutput.clear();
	m_isDone = false;
}

