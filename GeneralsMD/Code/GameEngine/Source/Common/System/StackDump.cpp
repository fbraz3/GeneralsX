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

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#if (defined(RTS_DEBUG) || defined(IG_DEBUG_STACKTRACE)) && defined(_WIN32)

#pragma pack(push, 8)

#include "Common/StackDump.h"
#include "Common/Debug.h"

#include "DbgHelpLoader.h"
// GeneralsX @refactor fbraz3 27/11/2026 Isolate Win32 CONTEXT/DbgHelp stack-walking into Platform layer.
#include "PlatformWin32StackDump.h"

//*****************************************************************************
//	Prototypes
//*****************************************************************************
BOOL InitSymbolInfo();
void GetFunctionDetails(void *pointer, char*name, char*filename, unsigned int* linenumber, unsigned int* address);
void WriteStackLine(void*address, void (*callback)(const char*));

static void WriteStackLineCallback(void *address, void *userData)
{
	WriteStackLine(address, reinterpret_cast<void (*)(const char *)>(userData));
}

static void EmitRegisterLine(const char *regLine, void *userData)
{
	AsciiString *outMsg = static_cast<AsciiString *>(userData);
	outMsg->format("%s", regLine);
	g_LastErrorDump.concat(*outMsg);
	DEBUG_LOG(("%s", regLine));
}


//*****************************************************************************
//*****************************************************************************
void StackDumpDefaultHandler(const char*line)
{
	DEBUG_LOG(("%s", line));
}


//*****************************************************************************
//*****************************************************************************
void StackDump(void (*callback)(const char*))
{
	if (callback == nullptr)
	{
		callback = StackDumpDefaultHandler;
	}

	if (!InitSymbolInfo())
		return;

	callback("Call Stack\n**********\n");
	Platform::WalkWin32CurrentStack(3, 30, WriteStackLineCallback, reinterpret_cast<void *>(callback));
}


//*****************************************************************************
//*****************************************************************************
void StackDumpFromContext(uintptr_t eip, uintptr_t esp, uintptr_t ebp, void (*callback)(const char*))
{
	if (callback == nullptr)
	{
		callback = StackDumpDefaultHandler;
	}

	if (!InitSymbolInfo())
		return;

	callback("Call Stack\n**********\n");
	Platform::WalkWin32StackFromContext(nullptr, eip, esp, ebp, 0, 30, WriteStackLineCallback, reinterpret_cast<void *>(callback));
}


//*****************************************************************************
//*****************************************************************************
BOOL InitSymbolInfo()
{
	if (DbgHelpLoader::isLoaded())
		return TRUE;

	if (DbgHelpLoader::isFailed())
		return FALSE;

	if (!DbgHelpLoader::load())
	{
		atexit(DbgHelpLoader::unload);
		return FALSE;
	}

	char pathname[_MAX_PATH+1];
	char drive[10];
	char directory[_MAX_PATH+1];
	HANDLE process;

	DbgHelp::SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME | SYMOPT_LOAD_LINES | SYMOPT_OMAP_FIND_NEAREST);

	process = GetCurrentProcess();

	//Get the apps name
	::GetModuleFileName(nullptr, pathname, _MAX_PATH);

	// turn it into a search path
	_splitpath(pathname, drive, directory, nullptr, nullptr);
	sprintf(pathname, "%s:\\%s", drive, directory);

	// append the current directory to build a search path for SymInit
	::lstrcat(pathname, ";.;");

	if(DbgHelp::SymInitialize(process, pathname, FALSE))
	{
		// regenerate the name of the app
		::GetModuleFileName(nullptr, pathname, _MAX_PATH);
		if(DbgHelp::SymLoadModule(process, nullptr, pathname, nullptr, 0, 0))
		{
				//Load any other relevant modules (ie dlls) here
				atexit(DbgHelpLoader::unload);
				return TRUE;
		}
	}

	DbgHelpLoader::unload();
	return FALSE;
}


//*****************************************************************************
//*****************************************************************************
void GetFunctionDetails(void *pointer, char*name, char*filename, unsigned int* linenumber, unsigned int* address)
{
	if (!InitSymbolInfo())
		return;

	Platform::GetWin32SymbolDetails(pointer, name, 512, filename, MAX_PATH, linenumber, address);
}


//*****************************************************************************
// Gets last x addresses from the stack
//*****************************************************************************
void FillStackAddresses(void**addresses, unsigned int count, unsigned int skip)
{
	if (!InitSymbolInfo())
		return;

	Platform::FillWin32StackAddresses(addresses, count, skip + 1);
}



//*****************************************************************************
// Do full stack dump using an address array
//*****************************************************************************
void StackDumpFromAddresses(void**addresses, unsigned int count, void (*callback)(const char *))
{
	if (callback == nullptr)
	{
		callback = StackDumpDefaultHandler;
	}

	if (!InitSymbolInfo())
		return;

	while ((count--) && (*addresses!=nullptr))
	{
		WriteStackLine(*addresses,callback);
		addresses++;
	}
}


AsciiString g_LastErrorDump;
//*****************************************************************************
//*****************************************************************************
void WriteStackLine(void*address, void (*callback)(const char*))
{
	static char line[MAX_PATH + 512 + 64];
	static char function_name[512];
	static char filename[MAX_PATH];
	unsigned int linenumber;
	unsigned int addr;

	GetFunctionDetails(address, function_name, filename, &linenumber, &addr);
	snprintf(line, sizeof(line), "  %s(%d) : %s 0x%p", filename, linenumber, function_name, address);
		if (g_LastErrorDump.isNotEmpty()) {
			g_LastErrorDump.concat(line);
			g_LastErrorDump.concat("\n");
		}
	callback(line);
}


//*****************************************************************************
//*****************************************************************************
void DumpExceptionInfo( unsigned int u, EXCEPTION_POINTERS* e_info )
{
	DEBUG_LOG_RAW(("\n"));
	DEBUG_LOG(( "********** EXCEPTION DUMP ****************" ));
	/*
	** List of possible exceptions
	*/
	 g_LastErrorDump.clear();

	static const unsigned int _codes[] = {
		EXCEPTION_ACCESS_VIOLATION,
		EXCEPTION_ARRAY_BOUNDS_EXCEEDED,
		EXCEPTION_BREAKPOINT,
		EXCEPTION_DATATYPE_MISALIGNMENT,
		EXCEPTION_FLT_DENORMAL_OPERAND,
		EXCEPTION_FLT_DIVIDE_BY_ZERO,
		EXCEPTION_FLT_INEXACT_RESULT,
		EXCEPTION_FLT_INVALID_OPERATION,
		EXCEPTION_FLT_OVERFLOW,
		EXCEPTION_FLT_STACK_CHECK,
		EXCEPTION_FLT_UNDERFLOW,
		EXCEPTION_ILLEGAL_INSTRUCTION,
		EXCEPTION_IN_PAGE_ERROR,
		EXCEPTION_INT_DIVIDE_BY_ZERO,
		EXCEPTION_INT_OVERFLOW,
		EXCEPTION_INVALID_DISPOSITION,
		EXCEPTION_NONCONTINUABLE_EXCEPTION,
		EXCEPTION_PRIV_INSTRUCTION,
		EXCEPTION_SINGLE_STEP,
		EXCEPTION_STACK_OVERFLOW,
		0xffffffff
	};

	/*
	** Information about each exception type.
	*/
	static char const * _code_txt[] = {
		"Error code: EXCEPTION_ACCESS_VIOLATION\nDescription: The thread tried to read from or write to a virtual address for which it does not have the appropriate access.",
		"Error code: EXCEPTION_ARRAY_BOUNDS_EXCEEDED\nDescription: The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking.",
		"Error code: EXCEPTION_BREAKPOINT\nDescription: A breakpoint was encountered.",
		"Error code: EXCEPTION_DATATYPE_MISALIGNMENT\nDescription: The thread tried to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and so on.",
		"Error code: EXCEPTION_FLT_DENORMAL_OPERAND\nDescription: One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value.",
		"Error code: EXCEPTION_FLT_DIVIDE_BY_ZERO\nDescription: The thread tried to divide a floating-point value by a floating-point divisor of zero.",
		"Error code: EXCEPTION_FLT_INEXACT_RESULT\nDescription: The result of a floating-point operation cannot be represented exactly as a decimal fraction.",
		"Error code: EXCEPTION_FLT_INVALID_OPERATION\nDescription: Some strange unknown floating point operation was attempted.",
		"Error code: EXCEPTION_FLT_OVERFLOW\nDescription: The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type.",
		"Error code: EXCEPTION_FLT_STACK_CHECK\nDescription: The stack overflowed or underflowed as the result of a floating-point operation.",
		"Error code: EXCEPTION_FLT_UNDERFLOW\nDescription:	The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type.",
		"Error code: EXCEPTION_ILLEGAL_INSTRUCTION\nDescription:	The thread tried to execute an invalid instruction.",
		"Error code: EXCEPTION_IN_PAGE_ERROR\nDescription:	The thread tried to access a page that was not present, and the system was unable to load the page. For example, this exception might occur if a network connection is lost while running a program over the network.",
		"Error code: EXCEPTION_INT_DIVIDE_BY_ZERO\nDescription: The thread tried to divide an integer value by an integer divisor of zero.",
		"Error code: EXCEPTION_INT_OVERFLOW\nDescription: The result of an integer operation caused a carry out of the most significant bit of the result.",
		"Error code: EXCEPTION_INVALID_DISPOSITION\nDescription: An exception handler returned an invalid disposition to the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception.",
		"Error code: EXCEPTION_NONCONTINUABLE_EXCEPTION\nDescription: The thread tried to continue execution after a noncontinuable exception occurred.",
		"Error code: EXCEPTION_PRIV_INSTRUCTION\nDescription: The thread tried to execute an instruction whose operation is not allowed in the current machine mode.",
		"Error code: EXCEPTION_SINGLE_STEP\nDescription: A trace trap or other single-instruction mechanism signaled that one instruction has been executed.",
		"Error code: EXCEPTION_STACK_OVERFLOW\nDescription: The thread used up its stack.",
		"Error code: ?????\nDescription: Unknown exception."
	};

	DEBUG_LOG( ("Dump exception info") );
	CONTEXT *context = e_info->ContextRecord;
	/*
	** The following are set for access violation only
	*/
	int access_read_write=-1;
	ULONG_PTR access_address = 0;
	AsciiString msg;

// DOUBLE_DEBUG does a DEBUG_LOG, and concats to g_LastErrorDump.  jba.
#define DOUBLE_DEBUG(x) { msg.format x; g_LastErrorDump.concat(msg); DEBUG_LOG( x ); }

	if ( e_info->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION )
	{
		DOUBLE_DEBUG (("Exception is access violation"));
		access_read_write = (int)e_info->ExceptionRecord->ExceptionInformation[0];  // 0=read, 1=write
		access_address = e_info->ExceptionRecord->ExceptionInformation[1];
	}
	else
	{
		DOUBLE_DEBUG (("Exception code is %x", e_info->ExceptionRecord->ExceptionCode));
	}
	Int *winMainAddr = (Int *)WinMain;
	DOUBLE_DEBUG(("WinMain at %p", static_cast<void*>(winMainAddr)));
	/*
	** Match the exception type with the error string and print it out
	*/
	int i=0;
	for ( ; _codes[i] != 0xffffffff ; i++ )
	{
		if ( _codes[i] == e_info->ExceptionRecord->ExceptionCode )
		{
			DEBUG_LOG ( ("Found exception description") );
			break;
		}
	}
	DOUBLE_DEBUG( ("%s", _code_txt[i]));
	/** For access violations, print out the violation address and if it was read or write.
	*/
	if ( e_info->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION )
	{
		if ( access_read_write )
		{
			DOUBLE_DEBUG( ("Access address:%p was written to.", reinterpret_cast<void*>(access_address)));
		}
		else
		{
			DOUBLE_DEBUG( ("Access address:%p was read from.", reinterpret_cast<void*>(access_address)));
		}
	}

	DOUBLE_DEBUG (("\nStack Dump:"));
	if (InitSymbolInfo())
	{
		StackDumpDefaultHandler("Call Stack\n**********\n");
		Platform::WalkWin32StackFromContext(context, 0, 0, 0, 0, 30, WriteStackLineCallback, reinterpret_cast<void *>(StackDumpDefaultHandler));
	}

	DOUBLE_DEBUG (("\nDetails:"));

	DOUBLE_DEBUG (("Register dump..."));

	/*
	** Dump the registers.
	*/
	Platform::FormatWin32ContextRegisters(context, EmitRegisterLine, &msg);

	/*
	** Dump the bytes at instruction pointer. This will make it easier to match the crash address with later versions of the game.
	*/
	char scrap[512];
	DOUBLE_DEBUG ( ("IP bytes dump..."));
	const uintptr_t ipAddr = Platform::GetWin32ContextPC(context);
	sprintf (scrap, "\nBytes at CS:IP (%p)  : ", reinterpret_cast<void*>(ipAddr));

	unsigned char *eip_ptr = reinterpret_cast<unsigned char *>(ipAddr);
	char bytestr[32];

	for (int c = 0 ; c < 32 ; c++)
	{
		if (IsBadReadPtr(eip_ptr, 1))
		{
			lstrcat (scrap, "?? ");
		}
		else
		{
			sprintf (bytestr, "%02X ", *eip_ptr);
			strlcat(scrap, bytestr, ARRAY_SIZE(scrap));
		}
		eip_ptr++;
	}

	DOUBLE_DEBUG ( ( (scrap)));
	DEBUG_LOG(( "********** END EXCEPTION DUMP ****************" ));
	DEBUG_LOG_RAW(("\n"));
}


#pragma pack(pop)

#else

// Non-Windows or non-debug stubs
AsciiString g_LastErrorDump;
void StackDumpFromAddresses(void** addresses, unsigned int count, void (*callback)(const char*)) {}
void FillStackAddresses(void** addresses, unsigned int count, unsigned int skip) {}
void DumpExceptionInfo(unsigned int u, EXCEPTION_POINTERS* e_info) {}
void GetFunctionDetails(void *pointer, char*name, char*filename, unsigned int* linenumber, unsigned int* address) {}

#endif

