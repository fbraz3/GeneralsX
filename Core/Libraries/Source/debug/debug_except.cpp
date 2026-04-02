/*
**Command & Conquer Generals Zero Hour(tm)
**Copyright 2025 Electronic Arts Inc.
**
**This program is free software: you can redistribute it and/or modify
**it under the terms of the GNU General Public License as published by
**the Free Software Foundation, either version 3 of the License, or
**(at your option) any later version.
**
**This program is distributed in the hope that it will be useful,
**but WITHOUT ANY WARRANTY; without even the implied warranty of
**MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**GNU General Public License for more details.
**
**You should have received a copy of the GNU General Public License
**along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/////////////////////////////////////////////////////////////////////////EA-V1
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/debug/debug_except.cpp $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/03 11:55:26 $
//
// (c) 2003 Electronic Arts
//
// Unhandled exception handler
//////////////////////////////////////////////////////////////////////////////

#include "debug.h"
#include "internal_except.h"
#include <windows.h>

DebugExceptionhandler::DebugExceptionhandler()
{
  // don't do anything here!
}

const char *DebugExceptionhandler::GetExceptionType(struct _EXCEPTION_POINTERS *exptr, char *explanation)
{
  wsprintf(explanation, "Exception code 0x%08x", exptr->ExceptionRecord->ExceptionCode);
  return "EXCEPTION";
}

void DebugExceptionhandler::LogExceptionLocation(Debug &dbg, struct _EXCEPTION_POINTERS *exptr)
{
  char buf[512];
#if defined(_WIN64)
  DebugStackwalk::Signature::GetSymbol((unsigned __int64)exptr->ContextRecord->Rip, buf, sizeof(buf));
#else
  DebugStackwalk::Signature::GetSymbol(exptr->ContextRecord->Eip, buf, sizeof(buf));
#endif
  dbg << "Exception occured at\n" << buf << ".";
}

void DebugExceptionhandler::LogRegisters(Debug &dbg, struct _EXCEPTION_POINTERS *exptr)
{
  struct _CONTEXT &ctx = *exptr->ContextRecord;
#if defined(_WIN64)
  dbg << Debug::FillChar('0') << Debug::Hex()
      << "RAX:" << Debug::Width(16) << (unsigned __int64)ctx.Rax
      << " RBX:" << Debug::Width(16) << (unsigned __int64)ctx.Rbx
      << " RCX:" << Debug::Width(16) << (unsigned __int64)ctx.Rcx << "\n"
      << "RDX:" << Debug::Width(16) << (unsigned __int64)ctx.Rdx
      << " RSI:" << Debug::Width(16) << (unsigned __int64)ctx.Rsi
      << " RDI:" << Debug::Width(16) << (unsigned __int64)ctx.Rdi << "\n"
      << "RIP:" << Debug::Width(16) << (unsigned __int64)ctx.Rip
      << " RSP:" << Debug::Width(16) << (unsigned __int64)ctx.Rsp
      << " RBP:" << Debug::Width(16) << (unsigned __int64)ctx.Rbp << "\n"
      << Debug::FillChar() << Debug::Dec();
#else
  dbg << Debug::FillChar('0') << Debug::Hex()
      << "EAX:" << Debug::Width(8) << ctx.Eax
      << " EBX:" << Debug::Width(8) << ctx.Ebx
      << " ECX:" << Debug::Width(8) << ctx.Ecx << "\n"
      << "EDX:" << Debug::Width(8) << ctx.Edx
      << " ESI:" << Debug::Width(8) << ctx.Esi
      << " EDI:" << Debug::Width(8) << ctx.Edi << "\n"
      << "EIP:" << Debug::Width(8) << ctx.Eip
      << " ESP:" << Debug::Width(8) << ctx.Esp
      << " EBP:" << Debug::Width(8) << ctx.Ebp << "\n"
      << Debug::FillChar() << Debug::Dec();
#endif
}

void DebugExceptionhandler::LogFPURegisters(Debug &dbg, struct _EXCEPTION_POINTERS *exptr)
{
#if defined(_WIN64)
  (void)exptr;
  dbg << "FP registers not available in legacy handler for Win64\n";
#else
  struct _CONTEXT &ctx = *exptr->ContextRecord;
  if (!(ctx.ContextFlags & CONTEXT_FLOATING_POINT))
  {
    dbg << "FP registers not available\n";
    return;
  }
  dbg << "FP register dump omitted in modernized path\n";
#endif
}

long __stdcall DebugExceptionhandler::ExceptionFilter(struct _EXCEPTION_POINTERS* pExPtrs)
{
  static bool inExceptionFilter;
  if (inExceptionFilter)
  {
    MessageBox(nullptr, "Exception in exception handler", "Fatal error", MB_OK);
    return EXCEPTION_CONTINUE_SEARCH;
  }
  inExceptionFilter = true;

  char regInfo[512];
  DebugStackwalk::Signature sig;

  Debug &dbg = Debug::Instance;
  ++dbg.disableAssertsEtc;
  if (dbg.curType != DebugIOInterface::StringType::MAX)
    dbg.FlushOutput();
  dbg.StartOutput(DebugIOInterface::StringType::Exception, "");

  dbg << "\n" << Debug::RepeatChar('=', 80) << "\n";
  dbg << GetExceptionType(pExPtrs, regInfo) << ":\n" << regInfo << "\n\n";
  LogExceptionLocation(dbg, pExPtrs); dbg << "\n\n";
  LogRegisters(dbg, pExPtrs); dbg << "\n";
  LogFPURegisters(dbg, pExPtrs); dbg << "\n";

  dbg.m_stackWalk.StackWalk(sig, pExPtrs->ContextRecord);
  dbg << sig << "\n";

  dbg.FlushOutput();
  Debug::StaticExit();
  return EXCEPTION_EXECUTE_HANDLER;
}
