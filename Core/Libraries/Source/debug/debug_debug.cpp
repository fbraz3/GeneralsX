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

/////////////////////////////////////////////////////////////////////////EA-V1
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/debug/debug_debug.cpp $
// $Author: mhoffe $
// $Revision: #2 $
// $DateTime: 2003/07/09 10:57:23 $
//
// (c) 2003 Electronic Arts
//
// Debug class implementation
//////////////////////////////////////////////////////////////////////////////
#include "debug.h"
#include "internal.h"
#include "internal_except.h"
#include "internal_io.h"
#include <stdlib.h>
#include <windows.h>
#include <WWCommon.h>
#include <new>      // needed for placement new prototype

#ifdef HAS_LOGS
#endif
#ifdef HAS_LOGS
#endif
#ifdef HAS_LOGS
#endif
    operator<<(" release");
  if (*m_buildDate)
    (*this) << " build " << m_buildDate;
}

void Debug::ExecCommand(const char *cmdstart, const char *cmdend)
{
  // split off into command and arguments

  // alloc & copy string
  char *strbuf=(char *)DebugAllocMemory(cmdend-cmdstart+1);
  memcpy(strbuf,cmdstart,cmdend-cmdstart);
  strbuf[cmdend-cmdstart]=0;

  // for simplicity I'm using a fixed size argv array here...
  // if there are more arguments given than we have we're
  // just dropping the excess arguments
  char *parts[100];
  int numParts=0;
  char *lastNonWhitespace=NULL;
  char *cur=strbuf;

  // regular reply or structured reply?
  DebugIOInterface::StringType reply;
  DebugCmdInterface::CommandMode mode;
  if (*cur=='!')
  {
    cur++;
    reply=DebugIOInterface::StringType::StructuredCmdReply;
    mode=DebugCmdInterface::CommandMode::Structured;
  }
  else
  {
    reply=DebugIOInterface::StringType::CmdReply;
    mode=DebugCmdInterface::CommandMode::Normal;
  }

  for (;;)
  {
    if (!lastNonWhitespace&&(*cur=='\''||*cur=='"'))
    {
      char quote=*cur++;

      if (numParts<sizeof(parts)/sizeof(*parts))
        parts[numParts++]=cur;

      while (*cur&&*cur!=quote)
        ++cur;
      if (*cur)
        *cur++=0;
    }
    else if (*cur==' '||*cur=='\t'||!*cur||*cur==';')
    {
      if (*cur==';')
        *cur=0;
      if (lastNonWhitespace)
      {
        if (numParts<sizeof(parts)/sizeof(*parts))
          parts[numParts++]=lastNonWhitespace;
        lastNonWhitespace=NULL;
        if (*cur)
          *cur++=0;
      }
      else if (*cur)
        ++cur;
      else
        break;
    }
    else
    {
      if (!lastNonWhitespace)
        lastNonWhitespace=cur;
      ++cur;
    }
  }

  if (numParts)
  {
    // part[0] is the command, part[1..numParts] are arguments

    // split off command group (if any)
    char *p=strchr(parts[0],'.');
    if (p&&p-parts[0]<sizeof(curCommandGroup))
    {
      memcpy(curCommandGroup,parts[0],p-parts[0]);
      curCommandGroup[p-parts[0]]=0;
      ++p;
    }
    else
      p=parts[0];

    StartOutput(reply,"%s.%s",curCommandGroup,p);

    if (mode!=DebugCmdInterface::CommandMode::Structured)
      AddOutput("> ",2);

    // repeat current command first
    AddOutput(cmdstart,cmdend-cmdstart);
    AddOutput("\n",1);

    // command group known?
    for (CmdInterfaceListEntry *cur=firstCmdGroup;cur;cur=cur->next)
      if (strcmp(curCommandGroup,cur->group) == 0)
        break;
    if (!cur)
    {
      // nope, show error message
      (*this) << "Unknown command group " << curCommandGroup;
      *p=0;
    }

    if (*p)
    {
      // must have command...

      // search for a matching command handler
      for (CmdInterfaceListEntry *cur=firstCmdGroup;cur;cur=cur->next)
      {
        if (strcmp(curCommandGroup,cur->group) != 0)
          continue;

        bool doneCommand=cur->cmdif->Execute(*this,p,mode,numParts-1,parts+1);
        if (doneCommand&&(strcmp(p,"help") != 0||numParts>1))
          break;
      }

      // display error message if command not found, break away
      if (!cur&&mode==DebugCmdInterface::CommandMode::Normal)
      {
        if (strcmp(p,"help") != 0)
          operator<<("Unknown command");
        else if (numParts>1)
          operator<<("Unknown command, help not available");
      }
    }

    // flush output only if there is already an active I/O class
    FlushOutput(false);
  }

  // cleanup
  DebugFreeMemory(strbuf);
}

// little helper to get app window
static BOOL CALLBACK EnumThreadWndProc(HWND hwnd, LPARAM lParam)
{
  *(HWND *)lParam=hwnd;
  return FALSE;
}

bool Debug::IsWindowed(void)
{
  // use cached result if possible
  if (m_isWindowed)
    return m_isWindowed>0;

  // find main app window
  HWND appHWnd=NULL;
  EnumThreadWindows(GetCurrentThreadId(),EnumThreadWndProc,(LPARAM)&appHWnd);
  if (!appHWnd)
  {
    // couldn't find main window, assume we're windowed anyway
    m_isWindowed=1;
    return true;
  }

  // we assume full screen if WS_CAPTION is not set
  m_isWindowed=(GetWindowLong(appHWnd,GWL_STYLE)&WS_CAPTION)?1:-1;
  return m_isWindowed>0;
}

//////////////////////////////////////////////////////////////////////////////

// And finally for a little list of C/C++ runtime replacement functions.

// Abort process due to fatal heap error
void __cdecl _heap_abort(void)
{
  DCRASH_RELEASE("Fatal heap error.");
}

#endif // _WIN32
