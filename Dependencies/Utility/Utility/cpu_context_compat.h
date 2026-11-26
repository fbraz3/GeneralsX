/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 GeneralsX contributors
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

/*
 * Architecture-neutral access to the Windows CONTEXT record for the crash reporters, which assumed 32-bit x86
 * (CONTEXT::Eip/Esp/Ebp, inline __asm). RtlCaptureContext replaces the asm on every architecture.
 * KNOWN LIMITATION at x64: StackWalk64 needs the CONTEXT record, so pass GX_STACKWALK_CONTEXT(&ctx); unconverted
 * call sites yield no frames (a diagnostic gap, never a crash). Windows-only: all of it sits inside #ifdef _WIN32.
 */
#pragma once

#ifdef _WIN32

#include <windows.h>
#include <stddef.h>
#include <string.h>

/* Wide enough to hold a code address on the target: 4 bytes at x86, 8 at x64/arm64. Matches what
   STACKFRAME::AddrPC.Offset wants, since dbghelp.h remaps STACKFRAME to STACKFRAME64 under
   _WIN64 and widens Offset to DWORD64 with it. size_t rather than DWORD_PTR so the same spelling
   can appear in headers that non-Windows targets also parse. */
typedef size_t GXCtxReg;

#if defined(_M_X64) || defined(_M_AMD64)
	#define GX_CTX_PC(ctxptr) ((ctxptr)->Rip)
	#define GX_CTX_SP(ctxptr) ((ctxptr)->Rsp)
	#define GX_CTX_FP(ctxptr) ((ctxptr)->Rbp)
	#define GX_IMAGE_FILE_MACHINE_NATIVE IMAGE_FILE_MACHINE_AMD64
#elif defined(_M_ARM64)
	#define GX_CTX_PC(ctxptr) ((ctxptr)->Pc)
	#define GX_CTX_SP(ctxptr) ((ctxptr)->Sp)
	#define GX_CTX_FP(ctxptr) ((ctxptr)->Fp)
	#define GX_IMAGE_FILE_MACHINE_NATIVE IMAGE_FILE_MACHINE_ARM64
#else
	#define GX_CTX_PC(ctxptr) ((ctxptr)->Eip)
	#define GX_CTX_SP(ctxptr) ((ctxptr)->Esp)
	#define GX_CTX_FP(ctxptr) ((ctxptr)->Ebp)
	#define GX_IMAGE_FILE_MACHINE_NATIVE IMAGE_FILE_MACHINE_I386
#endif

/* StackWalk's ContextRecord argument. x86 walks the EBP chain and the original code deliberately
   passed null; x64 has no frame-pointer chain to walk and requires the record. */
#if defined(_M_IX86)
	#define GX_STACKWALK_CONTEXT(ctxptr) ((LPVOID)0)
#else
	#define GX_STACKWALK_CONTEXT(ctxptr) ((LPVOID)(ctxptr))
#endif

/*
 * Capture the calling thread's register state. ctx may be null if only the three values are
 * wanted. Deliberately NOT force-inlined: RtlCaptureContext records the state at its own call
 * site, so the program counter lands inside this helper. Every caller already walks with a
 * skip-frames count, and one extra frame is cheaper than reintroducing per-architecture assembly.
 */
static inline void GXCaptureContextRegisters(CONTEXT *ctx, GXCtxReg *pc, GXCtxReg *sp, GXCtxReg *fp)
{
	CONTEXT local;
	CONTEXT *use = (ctx != 0) ? ctx : &local;

	memset(use, 0, sizeof(CONTEXT));
	use->ContextFlags = CONTEXT_FULL;
	RtlCaptureContext(use);

	if (pc != 0) { *pc = (GXCtxReg)GX_CTX_PC(use); }
	if (sp != 0) { *sp = (GXCtxReg)GX_CTX_SP(use); }
	if (fp != 0) { *fp = (GXCtxReg)GX_CTX_FP(use); }
}

#endif /* _WIN32 */
