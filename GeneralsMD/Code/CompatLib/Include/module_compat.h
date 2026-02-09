#pragma once

class CComModule
{
  public:
    void Init(void*, HINSTANCE hInstance)
    {
      m_hInstance = hInstance;
    }
    void Term() {}

  private:
    HINSTANCE m_hInstance;
};

bool GetModuleFileName(HINSTANCE hInstance, char* buffer, int size);

typedef uintptr_t (*FARPROC)();
typedef HANDLE HMODULE;

HMODULE LoadLibrary(const char* lpFileName);
FARPROC GetProcAddress(HMODULE hModule, const char* lpProcName);
void FreeLibrary(HMODULE hModule);