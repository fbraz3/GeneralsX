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

// Phase 54: POSIX command line compatibility implementation
// This file provides global storage for command line arguments on non-Windows platforms

#ifndef _WIN32

#include <string>

// Global storage for POSIX command line arguments
int g_posix_argc = 0;
char** g_posix_argv = nullptr;
std::string g_posix_cmdline;

#endif // !_WIN32
