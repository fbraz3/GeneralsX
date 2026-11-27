/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 Electronic Arts Inc.
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

// PlatformWin32StackDump.h - Win32 architecture-neutral stack walk, symbol resolution, and register dump helpers.
// GeneralsX @refactor fbraz3 27/11/2026 Isolate Win32 CONTEXT/DbgHelp stack-walking and preserve full x64 CONTEXT records.

#pragma once

#if defined(_WIN32)

#include "DbgHelpLoader.h"
#include "Utility/cpu_context_compat.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace Platform {

inline uintptr_t GetWin32ContextPC(const CONTEXT *context)
{
	return context != nullptr ? static_cast<uintptr_t>(GX_CTX_PC(context)) : 0;
}

inline void WalkWin32StackFromContext(
	const CONTEXT *initialContext,
	uintptr_t fallbackPc,
	uintptr_t fallbackSp,
	uintptr_t fallbackFp,
	unsigned int skipFrames,
	unsigned int maxFrames,
	void (*onFrame)(void *address, void *userData),
	void *userData)
{
	CONTEXT walkCtx;
	GXCtxReg pc = static_cast<GXCtxReg>(fallbackPc);
	GXCtxReg sp = static_cast<GXCtxReg>(fallbackSp);
	GXCtxReg fp = static_cast<GXCtxReg>(fallbackFp);

	if (initialContext != nullptr)
	{
		// Preserve all captured registers (including R13/non-volatiles used by x64 unwind metadata).
		walkCtx = *initialContext;
		pc = static_cast<GXCtxReg>(GX_CTX_PC(&walkCtx));
		sp = static_cast<GXCtxReg>(GX_CTX_SP(&walkCtx));
		fp = static_cast<GXCtxReg>(GX_CTX_FP(&walkCtx));
	}
	else
	{
		std::memset(&walkCtx, 0, sizeof(CONTEXT));
		walkCtx.ContextFlags = CONTEXT_FULL;
		GX_CTX_PC(&walkCtx) = static_cast<DWORD_PTR>(pc);
		GX_CTX_SP(&walkCtx) = static_cast<DWORD_PTR>(sp);
		GX_CTX_FP(&walkCtx) = static_cast<DWORD_PTR>(fp);
	}

	STACKFRAME stack_frame;
	std::memset(&stack_frame, 0, sizeof(STACKFRAME));
	stack_frame.AddrPC.Mode = AddrModeFlat;
	stack_frame.AddrPC.Offset = pc;
	stack_frame.AddrStack.Mode = AddrModeFlat;
	stack_frame.AddrStack.Offset = sp;
	stack_frame.AddrFrame.Mode = AddrModeFlat;
	stack_frame.AddrFrame.Offset = fp;

	HANDLE thread = GetCurrentThread();
	HANDLE process = GetCurrentProcess();
	BOOL ok = TRUE;

	while (ok && skipFrames > 0)
	{
		ok = DbgHelp::StackWalk(
			GX_IMAGE_FILE_MACHINE_NATIVE,
			process,
			thread,
			&stack_frame,
			GX_STACKWALK_CONTEXT(&walkCtx),
			nullptr,
			DbgHelp::SymFunctionTableAccess,
			DbgHelp::SymGetModuleBase,
			nullptr);
		--skipFrames;
	}

	while (ok && maxFrames > 0)
	{
		ok = DbgHelp::StackWalk(
			GX_IMAGE_FILE_MACHINE_NATIVE,
			process,
			thread,
			&stack_frame,
			GX_STACKWALK_CONTEXT(&walkCtx),
			nullptr,
			DbgHelp::SymFunctionTableAccess,
			DbgHelp::SymGetModuleBase,
			nullptr);
		if (ok && onFrame != nullptr)
		{
			onFrame(reinterpret_cast<void *>(static_cast<uintptr_t>(stack_frame.AddrPC.Offset)), userData);
		}
		--maxFrames;
	}
}

inline void WalkWin32CurrentStack(
	unsigned int skipFrames,
	unsigned int maxFrames,
	void (*onFrame)(void *address, void *userData),
	void *userData)
{
	CONTEXT ctx;
	GXCtxReg pc = 0;
	GXCtxReg sp = 0;
	GXCtxReg fp = 0;
	GXCaptureContextRegisters(&ctx, &pc, &sp, &fp);
	WalkWin32StackFromContext(&ctx, static_cast<uintptr_t>(pc), static_cast<uintptr_t>(sp), static_cast<uintptr_t>(fp), skipFrames, maxFrames, onFrame, userData);
}

namespace detail {
struct FillAddressesState {
	void **cursor;
	unsigned int remaining;
};

inline void CollectFrameAddress(void *address, void *userData)
{
	FillAddressesState *state = static_cast<FillAddressesState *>(userData);
	if (state->remaining > 0)
	{
		*(state->cursor++) = address;
		--(state->remaining);
	}
}
} // namespace detail

inline void FillWin32StackAddresses(void **addresses, unsigned int count, unsigned int skip)
{
	if (!addresses || count == 0)
		return;

	detail::FillAddressesState state = { addresses, count };
	WalkWin32CurrentStack(skip, count, detail::CollectFrameAddress, &state);
	while (state.remaining > 0)
	{
		*(state.cursor++) = nullptr;
		--(state.remaining);
	}
}

inline void GetWin32SymbolDetails(
	void *pointer,
	char *name,
	size_t nameSize,
	char *filename,
	size_t filenameSize,
	unsigned int *linenumber,
	unsigned int *address)
{
	if (name && nameSize > 0)
		std::snprintf(name, nameSize, "<Unknown>");
	if (filename && filenameSize > 0)
		std::snprintf(filename, filenameSize, "<Unknown>");
	if (linenumber)
		*linenumber = 0xFFFFFFFF;
	if (address)
		*address = 0xFFFFFFFF;

	DWORD_PTR symDisplacement = 0;
	HANDLE process = ::GetCurrentProcess();

	char symbol_buffer[512 + sizeof(IMAGEHLP_SYMBOL)];
	std::memset(symbol_buffer, 0, sizeof(symbol_buffer));

	PIMAGEHLP_SYMBOL psymbol = reinterpret_cast<PIMAGEHLP_SYMBOL>(symbol_buffer);
	psymbol->SizeOfStruct = sizeof(symbol_buffer);
	psymbol->MaxNameLength = 512;

	if (DbgHelp::SymGetSymFromAddr(process, reinterpret_cast<DWORD_PTR>(pointer), &symDisplacement, psymbol))
	{
		if (name && nameSize > 0)
		{
			std::snprintf(name, nameSize, "%s();", psymbol->Name);
		}

		IMAGEHLP_LINE line;
		std::memset(&line, 0, sizeof(line));
		line.SizeOfStruct = sizeof(line);
		DWORD lineDisplacement = 0;

		if (DbgHelp::SymGetLineFromAddr(process, reinterpret_cast<DWORD_PTR>(pointer), &lineDisplacement, &line))
		{
			if (filename && filenameSize > 0 && line.FileName)
			{
				std::snprintf(filename, filenameSize, "%s", line.FileName);
			}
			if (linenumber)
			{
				*linenumber = static_cast<unsigned int>(line.LineNumber);
			}
			if (address)
			{
				*address = static_cast<unsigned int>(line.Address);
			}
		}
	}
}

inline void FormatWin32ContextRegisters(const CONTEXT *context, void (*emitLine)(const char *line, void *userData), void *userData)
{
	if (!context || !emitLine)
		return;

	char buf[256];
#if defined(_M_X64) || defined(_M_AMD64)
	std::snprintf(buf, sizeof(buf), "Rip:%016llX\tRsp:%016llX\tRbp:%016llX", static_cast<unsigned long long>(context->Rip), static_cast<unsigned long long>(context->Rsp), static_cast<unsigned long long>(context->Rbp));
	emitLine(buf, userData);
	std::snprintf(buf, sizeof(buf), "Rax:%016llX\tRbx:%016llX\tRcx:%016llX", static_cast<unsigned long long>(context->Rax), static_cast<unsigned long long>(context->Rbx), static_cast<unsigned long long>(context->Rcx));
	emitLine(buf, userData);
	std::snprintf(buf, sizeof(buf), "Rdx:%016llX\tRsi:%016llX\tRdi:%016llX", static_cast<unsigned long long>(context->Rdx), static_cast<unsigned long long>(context->Rsi), static_cast<unsigned long long>(context->Rdi));
	emitLine(buf, userData);
	std::snprintf(buf, sizeof(buf), "R8 :%016llX\tR9 :%016llX\tR10:%016llX", static_cast<unsigned long long>(context->R8), static_cast<unsigned long long>(context->R9), static_cast<unsigned long long>(context->R10));
	emitLine(buf, userData);
	std::snprintf(buf, sizeof(buf), "R11:%016llX\tR12:%016llX\tR13:%016llX", static_cast<unsigned long long>(context->R11), static_cast<unsigned long long>(context->R12), static_cast<unsigned long long>(context->R13));
	emitLine(buf, userData);
	std::snprintf(buf, sizeof(buf), "R14:%016llX\tR15:%016llX", static_cast<unsigned long long>(context->R14), static_cast<unsigned long long>(context->R15));
	emitLine(buf, userData);
#else
	std::snprintf(buf, sizeof(buf), "Eip:%08X\tEsp:%08X\tEbp:%08X", context->Eip, context->Esp, context->Ebp);
	emitLine(buf, userData);
	std::snprintf(buf, sizeof(buf), "Eax:%08X\tEbx:%08X\tEcx:%08X", context->Eax, context->Ebx, context->Ecx);
	emitLine(buf, userData);
	std::snprintf(buf, sizeof(buf), "Edx:%08X\tEsi:%08X\tEdi:%08X", context->Edx, context->Esi, context->Edi);
	emitLine(buf, userData);
#endif
	std::snprintf(buf, sizeof(buf), "EFlags:%08X ", context->EFlags);
	emitLine(buf, userData);
	std::snprintf(buf, sizeof(buf), "CS:%04x  SS:%04x  DS:%04x  ES:%04x  FS:%04x  GS:%04x", context->SegCs, context->SegSs, context->SegDs, context->SegEs, context->SegFs, context->SegGs);
	emitLine(buf, userData);
}

} // namespace Platform

#endif // _WIN32
