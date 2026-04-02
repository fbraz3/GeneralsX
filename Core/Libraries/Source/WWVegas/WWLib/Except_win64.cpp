/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
*/

#include "Except.h"

#if defined(_WIN32) && defined(_WIN64)

#include <windows.h>
#include <dbghelp.h>

#include <algorithm>
#include <vector>

#include "always.h"
#include "wwdebug.h"

unsigned long ExceptionReturnStack = 0;
unsigned long ExceptionReturnAddress = 0;
unsigned long ExceptionReturnFrame = 0;

namespace
{
	struct ThreadInfoLocal {
		unsigned long id;
		char name[128];
		bool main;
	};

	std::vector<ThreadInfoLocal> g_thread_list;
	CRITICAL_SECTION g_thread_lock;
	bool g_lock_inited = false;
	bool g_exit_on_exception = true;
	bool g_trying_to_exit = false;
	unsigned long g_main_thread_id = 0;
	void (*g_app_callback)() = nullptr;
	char *(*g_app_version_callback)() = nullptr;

	void Ensure_Lock()
	{
		if (!g_lock_inited) {
			InitializeCriticalSection(&g_thread_lock);
			g_lock_inited = true;
		}
	}
}

void Load_Image_Helper()
{
	// GeneralsX @build BenderAI 01/04/2026 DbgHelp is linked directly in Win64 path; no dynamic bootstrap required.
}

void Register_Thread_ID(unsigned long thread_id, char *thread_name, bool main_thread)
{
	Ensure_Lock();
	EnterCriticalSection(&g_thread_lock);

	ThreadInfoLocal info = {};
	info.id = thread_id;
	info.main = main_thread;
	if (thread_name != nullptr) {
		strlcpy(info.name, thread_name, ARRAY_SIZE(info.name));
	}
	g_thread_list.push_back(info);

	if (main_thread || g_main_thread_id == 0) {
		g_main_thread_id = thread_id;
	}

	LeaveCriticalSection(&g_thread_lock);
}

void Unregister_Thread_ID(unsigned long thread_id, char *thread_name)
{
	(void)thread_name;
	if (!g_lock_inited) {
		return;
	}

	EnterCriticalSection(&g_thread_lock);
	g_thread_list.erase(
		std::remove_if(g_thread_list.begin(), g_thread_list.end(),
			[thread_id](const ThreadInfoLocal &it) { return it.id == thread_id; }),
		g_thread_list.end());
	LeaveCriticalSection(&g_thread_lock);
}

void Register_Application_Exception_Callback(void (*app_callback)())
{
	g_app_callback = app_callback;
}

void Register_Application_Version_Callback(char *(*app_version_callback)())
{
	g_app_version_callback = app_version_callback;
}

void Set_Exit_On_Exception(bool set)
{
	g_exit_on_exception = set;
}

bool Is_Trying_To_Exit()
{
	return g_trying_to_exit;
}

unsigned long Get_Main_Thread_ID()
{
	if (g_main_thread_id == 0) {
		g_main_thread_id = GetCurrentThreadId();
	}
	return g_main_thread_id;
}

int Stack_Walk(unsigned long *return_addresses, int num_addresses, CONTEXT *context)
{
	(void)context;
	if (return_addresses == nullptr || num_addresses <= 0) {
		return 0;
	}

	void *frames[256] = {};
	const USHORT max_frames = static_cast<USHORT>(num_addresses > 256 ? 256 : num_addresses);
	const USHORT captured = CaptureStackBackTrace(1, max_frames, frames, nullptr);
	for (USHORT i = 0; i < captured; ++i) {
		return_addresses[i] = static_cast<unsigned long>(reinterpret_cast<uintptr_t>(frames[i]));
	}
	return static_cast<int>(captured);
}

bool Lookup_Symbol(void *code_ptr, char *symbol, int &displacement)
{
	if (symbol == nullptr || code_ptr == nullptr) {
		return false;
	}

	HANDLE process = GetCurrentProcess();
	DWORD64 address = reinterpret_cast<DWORD64>(code_ptr);
	char buffer[sizeof(SYMBOL_INFO) + 256] = {};
	SYMBOL_INFO *sym = reinterpret_cast<SYMBOL_INFO *>(buffer);
	sym->SizeOfStruct = sizeof(SYMBOL_INFO);
	sym->MaxNameLen = 255;
	DWORD64 disp64 = 0;

	if (SymFromAddr(process, address, &disp64, sym) == FALSE) {
		return false;
	}

	strlcpy(symbol, sym->Name, 256);
	displacement = static_cast<int>(disp64);
	return true;
}

int Exception_Handler(int exception_code, EXCEPTION_POINTERS *e_info)
{
	(void)e_info;
	g_trying_to_exit = true;
	char msg[256] = {};
	snprintf(msg, ARRAY_SIZE(msg), "Exception Handler (Win64): exception code %08X\n", exception_code);
	OutputDebugStringA(msg);

	if (g_app_version_callback != nullptr) {
		const char *ver = g_app_version_callback();
		if (ver != nullptr) {
			snprintf(msg, ARRAY_SIZE(msg), "Exception Handler (Win64): app version %s\n", ver);
			OutputDebugStringA(msg);
		}
	}

	if (g_app_callback != nullptr) {
		g_app_callback();
	}

	if (g_exit_on_exception) {
		return EXCEPTION_EXECUTE_HANDLER;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

#endif
