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

#include "Common/ReplaySimulation.h"

#include "Common/GameEngine.h"
#include "Common/LocalFileSystem.h"
#include "Common/Recorder.h"
#include "Common/WorkerProcess.h"
#include "GameLogic/GameLogic.h"
#include "GameClient/GameClient.h"

// GeneralsX @build BenderAI 12/02/2026 Cross-platform executable path retrieval
#ifndef _WIN32
#ifdef __linux__
#include <unistd.h>  // readlink
#include <limits.h>  // PATH_MAX
#elif defined(__APPLE__)
#include <mach-o/dyld.h>  // _NSGetExecutablePath
#endif
#endif


Bool ReplaySimulation::s_isRunning = false;
UnsignedInt ReplaySimulation::s_replayIndex = 0;
UnsignedInt ReplaySimulation::s_replayCount = 0;

namespace
{
int countProcessesRunning(const std::vector<WorkerProcess>& processes)
{
	int numProcessesRunning = 0;
	size_t i = 0;
	for (; i < processes.size(); ++i)
	{
		if (processes[i].isRunning())
			++numProcessesRunning;
	}
	return numProcessesRunning;
}
} // namespace

int ReplaySimulation::simulateReplaysInThisProcess(const std::vector<AsciiString> &filenames)
{
	int numErrors = 0;

	if (!TheGlobalData->m_headless)
	{
		s_isRunning = true;
		s_replayIndex = 0;
		s_replayCount = static_cast<UnsignedInt>(filenames.size());

		// If we are not in headless mode, we need to run the replay in the engine.
		for (; s_replayIndex < s_replayCount; ++s_replayIndex)
		{
			TheRecorder->playbackFile(filenames[s_replayIndex]);
			TheGameEngine->execute();
			if (TheRecorder->sawCRCMismatch())
				numErrors++;
			if (!s_isRunning)
				break;
			TheGameEngine->setQuitting(FALSE);
		}
		s_isRunning = false;
		s_replayIndex = 0;
		s_replayCount = 0;
		return numErrors != 0 ? 1 : 0;
	}
	// Note that we use printf here because this is run from cmd.
	DWORD totalStartTimeMillis = GetTickCount();
	for (size_t i = 0; i < filenames.size(); i++)
	{
		AsciiString filename = filenames[i];
		printf("Simulating Replay \"%s\"\n", filename.str());
		fflush(stdout);
		DWORD startTimeMillis = GetTickCount();
		if (TheRecorder->simulateReplay(filename))
		{
			UnsignedInt totalTimeSec = TheRecorder->getPlaybackFrameCount() / LOGICFRAMES_PER_SECOND;
			while (TheRecorder->isPlaybackInProgress())
			{
				TheGameClient->updateHeadless();

				const int progressFrameInterval = 10*60*LOGICFRAMES_PER_SECOND;
				if (TheGameLogic->getFrame() != 0 && TheGameLogic->getFrame() % progressFrameInterval == 0)
				{
					// Print progress report
					UnsignedInt gameTimeSec = TheGameLogic->getFrame() / LOGICFRAMES_PER_SECOND;
					UnsignedInt realTimeSec = (GetTickCount()-startTimeMillis) / 1000;
					printf("Elapsed Time: %02d:%02d Game Time: %02d:%02d/%02d:%02d\n",
							realTimeSec/60, realTimeSec%60, gameTimeSec/60, gameTimeSec%60, totalTimeSec/60, totalTimeSec%60);
					fflush(stdout);
				}
				TheGameLogic->UPDATE();
				if (TheRecorder->sawCRCMismatch())
				{
					// GeneralsX @tweak BenderAI 13/04/2026 Add per-replay CRC mismatch diagnostics for headless runs
					printf("CRC mismatch detected while simulating replay \"%s\"\n", filename.str());
					fflush(stdout);
					numErrors++;
					break;
				}
			}
			UnsignedInt gameTimeSec = TheGameLogic->getFrame() / LOGICFRAMES_PER_SECOND;
			UnsignedInt realTimeSec = (GetTickCount()-startTimeMillis) / 1000;
			printf("Elapsed Time: %02d:%02d Game Time: %02d:%02d/%02d:%02d\n",
					realTimeSec/60, realTimeSec%60, gameTimeSec/60, gameTimeSec%60, totalTimeSec/60, totalTimeSec%60);
			fflush(stdout);
		}
		else
		{
			// GeneralsX @tweak BenderAI 13/04/2026 Add replay filename and resolved replay directory in open failures
			printf("Cannot open replay \"%s\"\n", filename.str());
			printf("Replay directory at failure: \"%s\"\n", TheRecorder->getReplayDir().str());
			fflush(stdout);
			numErrors++;
		}
	}
	if (filenames.size() > 1)
	{
		printf("Simulation of all replays completed. Errors occurred: %d\n", numErrors);

		UnsignedInt realTime = (GetTickCount()-totalStartTimeMillis) / 1000;
		printf("Total Time: %d:%02d:%02d\n", realTime/60/60, realTime/60%60, realTime%60);
		fflush(stdout);
	}

	return numErrors != 0 ? 1 : 0;
}

int ReplaySimulation::simulateReplaysInWorkerProcesses(const std::vector<AsciiString> &filenames, int maxProcesses)
{
	// GeneralsX @bugfix BenderAI 13/04/2026 WorkerProcess is still Windows-only; use in-process replay simulation on non-Windows.
#ifndef _WIN32
	printf("Worker processes are unavailable on this platform. Falling back to in-process replay simulation.\n");
	fflush(stdout);
	return simulateReplaysInThisProcess(filenames);
#endif

	DWORD totalStartTimeMillis = GetTickCount();

	// GeneralsX @build BenderAI 12/02/2026 Cross-platform executable path retrieval
#ifdef _WIN32
	WideChar exePath[1024];
	GetModuleFileNameW(nullptr, exePath, ARRAY_SIZE(exePath));
#else
	char exePathNarrow[1024];
#ifdef __linux__
	ssize_t len = readlink("/proc/self/exe", exePathNarrow, sizeof(exePathNarrow) - 1);
	if (len != -1) {
		exePathNarrow[len] = '\0';
	} else {
		exePathNarrow[0] = '\0';
	}
#elif defined(__APPLE__)
	uint32_t size = sizeof(exePathNarrow);
	if (_NSGetExecutablePath(exePathNarrow, &size) != 0) {
		exePathNarrow[0] = '\0';
	}
#else
	exePathNarrow[0] = '\0';
#endif
	WideChar exePath[1024];
	// GeneralsX @bugfix BenderAI 13/04/2026 Ensure wide executable path is properly null-terminated for worker command formatting.
	size_t exePathWideLen = 0;
	while (exePathWideLen < ARRAY_SIZE(exePath) - 1 && exePathNarrow[exePathWideLen] != '\0')
	{
		exePath[exePathWideLen] = static_cast<WideChar>(exePathNarrow[exePathWideLen]);
		exePathWideLen++;
	}
	exePath[exePathWideLen] = L'\0';
#endif

	std::vector<WorkerProcess> processes;
	std::vector<AsciiString> processReplayNames;
	std::vector<UnicodeString> processCommands;
	int filenamePositionStarted = 0;
	int filenamePositionDone = 0;
	int numErrors = 0;

	while (true)
	{
		int i;
		for (i = 0; i < processes.size(); i++)
			processes[i].update();

		// Get result of finished processes and print output in order
		while (!processes.empty())
		{
			if (!processes[0].isDone())
				break;
			AsciiString replayName = processReplayNames[0];
			UnicodeString replayCommand = processCommands[0];
			AsciiString stdOutput = processes[0].getStdOutput();
			printf("%d/%d Replay \"%s\"\n", filenamePositionDone+1, (int)filenames.size(), replayName.str());
			if (stdOutput.getLength() > 0)
				printf("%s", stdOutput.str());
			DWORD exitcode = processes[0].getExitCode();
			if (exitcode != 0)
			{
				// GeneralsX @tweak BenderAI 13/04/2026 Add worker command and exit diagnostics for replay failures
				printf("Worker exit code: %lu\n", (unsigned long)exitcode);
				wprintf(L"Worker command: %ls\n", replayCommand.str());
				printf("Error while simulating replay \"%s\"\n", replayName.str());
			}
			fflush(stdout);
			numErrors += exitcode == 0 ? 0 : 1;
			processes.erase(processes.begin());
			processReplayNames.erase(processReplayNames.begin());
			processCommands.erase(processCommands.begin());
			filenamePositionDone++;
		}

		int numProcessesRunning = countProcessesRunning(processes);

		// Add new processes when we are below the limit and there are replays left
		while (numProcessesRunning < maxProcesses && filenamePositionStarted < filenames.size())
		{
			UnicodeString filenameWide;
			filenameWide.translate(filenames[filenamePositionStarted]);
			UnicodeString command;
			// GeneralsX @bugfix BenderAI 13/04/2026 Use wide format specifiers to avoid truncating command paths on non-Windows
			command.format(L"\"%ls\"%ls%ls -replay \"%ls\"",
				exePath,
				TheGlobalData->m_windowed ? L" -win" : L"",
				TheGlobalData->m_headless ? L" -headless" : L"",
				filenameWide.str());

			processes.push_back(WorkerProcess());
			if (!processes.back().startProcess(command))
			{
				// GeneralsX @tweak BenderAI 13/04/2026 Add explicit worker start diagnostics instead of generic replay error
				printf("Failed to start worker process for replay \"%s\"\n", filenames[filenamePositionStarted].str());
				wprintf(L"Worker command: %ls\n", command.str());
				fflush(stdout);
				numErrors++;
				processes.pop_back();
				filenamePositionStarted++;
				continue;
			}

			processReplayNames.push_back(filenames[filenamePositionStarted]);
			processCommands.push_back(command);

			filenamePositionStarted++;
			numProcessesRunning++;
		}

		if (processes.empty())
			break;

		// Don't waste CPU here, our workers need every bit of CPU time they can get
		Sleep(100);
	}

	DEBUG_ASSERTCRASH(filenamePositionStarted == filenames.size(), ("inconsistent file position 1"));
	DEBUG_ASSERTCRASH(filenamePositionDone == filenames.size(), ("inconsistent file position 2"));

	printf("Simulation of all replays completed. Errors occurred: %d\n", numErrors);

	UnsignedInt realTime = (GetTickCount()-totalStartTimeMillis) / 1000;
	printf("Total Wall Time: %d:%02d:%02d\n", realTime/60/60, realTime/60%60, realTime%60);
	fflush(stdout);

	return numErrors != 0 ? 1 : 0;
}

std::vector<AsciiString> ReplaySimulation::resolveFilenameWildcards(const std::vector<AsciiString> &filenames)
{
	// If some filename contains wildcards, search for actual filenames.
	// Note that we cannot do this in parseReplay because we require TheLocalFileSystem initialized.
	std::vector<AsciiString> filenamesResolved;
	for (std::vector<AsciiString>::const_iterator filename = filenames.begin(); filename != filenames.end(); ++filename)
	{
		if (filename->find('*') || filename->find('?'))
		{
			AsciiString dir1 = TheRecorder->getReplayDir();
			AsciiString dir2 = *filename;
			AsciiString wildcard = *filename;
			{
				int len = dir2.getLength();
				while (len)
				{
					char c = dir2.getCharAt(len-1);
					if (c == '/' || c == '\\')
					{
						wildcard.set(wildcard.str()+dir2.getLength());
						break;
					}
					dir2.removeLastChar();
					len--;
				}
			}

			FilenameList files;
			TheLocalFileSystem->getFileListInDirectory(dir2.str(), dir1.str(), wildcard, files, FALSE);
			for (FilenameList::iterator it = files.begin(); it != files.end(); ++it)
			{
				AsciiString file;
				file.set(it->str() + dir1.getLength());
				filenamesResolved.push_back(file);
			}
		}
		else
			filenamesResolved.push_back(*filename);
	}
	return filenamesResolved;
}

int ReplaySimulation::simulateReplays(const std::vector<AsciiString> &filenames, int maxProcesses)
{
	std::vector<AsciiString> filenamesResolved = resolveFilenameWildcards(filenames);
	if (maxProcesses == SIMULATE_REPLAYS_SEQUENTIAL)
		return simulateReplaysInThisProcess(filenamesResolved);
	else
		return simulateReplaysInWorkerProcesses(filenamesResolved, maxProcesses);
}
