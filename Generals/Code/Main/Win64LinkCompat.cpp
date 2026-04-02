// GeneralsX @bugfix BenderAI 01/04/2026 Provide missing ATL/COM/GameSpy globals for Windows x64 modern toolchain.
#if defined(_WIN64)

#include <windows.h>
#include <oleauto.h>
#include <atlcore.h>

namespace ATL
{
    CAtlBaseModule::CAtlBaseModule() throw()
    {
        memset(static_cast<_ATL_BASE_MODULE *>(this), 0, sizeof(_ATL_BASE_MODULE));
        m_hInst = GetModuleHandleA(nullptr);
        m_hInstResource = m_hInst;
    }

    CAtlBaseModule::~CAtlBaseModule() throw()
    {
    }

    CAtlBaseModule _AtlBaseModule;
}

void __cdecl _com_issue_error(HRESULT hr)
{
    throw hr;
}

namespace _com_util
{
    wchar_t * __cdecl ConvertStringToBSTR(char const *pSrc)
    {
        if (pSrc == nullptr)
        {
            return nullptr;
        }

        int len = MultiByteToWideChar(CP_ACP, 0, pSrc, -1, nullptr, 0);
        if (len <= 0)
        {
            return nullptr;
        }

        BSTR out = SysAllocStringLen(nullptr, static_cast<UINT>(len - 1));
        if (out == nullptr)
        {
            return nullptr;
        }

        if (MultiByteToWideChar(CP_ACP, 0, pSrc, -1, out, len) == 0)
        {
            SysFreeString(out);
            return nullptr;
        }

        return out;
    }
}

extern "C"
{
    char gcd_secret_key[256] = "";
    char gcd_gamename[256] = "";
}

#endif
