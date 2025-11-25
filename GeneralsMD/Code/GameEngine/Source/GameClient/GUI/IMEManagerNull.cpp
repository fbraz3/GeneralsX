#ifndef _WIN32

#include "PreRTS.h"
#include "GameClient/IMEManager.h"
#include "GameClient/GameWindow.h"
#include "Common/UnicodeString.h"

/**
 * NullIMEManager provides a no-op IME implementation so non-Windows
 * platforms can satisfy linker/runtime dependencies without touching
 * platform-specific APIs. All methods either store trivial state or
 * immediately return default values.
 */
class NullIMEManager final : public IMEManagerInterface
{
public:
    NullIMEManager() : m_window(nullptr), m_enabled(FALSE) {}
    ~NullIMEManager() override = default;

    void init() override
    {
        m_enabled = FALSE;
        m_window = nullptr;
    }

    void reset() override
    {
        m_enabled = FALSE;
        m_window = nullptr;
    }

    void update() override {}

    void attach(GameWindow* window) override { m_window = window; }
    void detatch() override { m_window = nullptr; }

    void enable() override { m_enabled = TRUE; }
    void disable() override { m_enabled = FALSE; }

    Bool isEnabled() override { return m_enabled; }
    Bool isAttachedTo(GameWindow* window) override { return (m_window == window) ? TRUE : FALSE; }
    GameWindow* getWindow() override { return m_window; }

    Bool isComposing() override { return FALSE; }
    void getCompositionString(UnicodeString& string) override { string.clear(); }
    Int getCompositionCursorPosition() override { return 0; }
    Int getIndexBase() override { return 0; }

    Int getCandidateCount() override { return 0; }
    UnicodeString* getCandidate(Int) override { return nullptr; }
    Int getSelectedCandidateIndex() override { return 0; }
    Int getCandidatePageSize() override { return 0; }
    Int getCandidatePageStart() override { return 0; }

    Bool serviceIMEMessage(void*, UnsignedInt, Int, Int) override { return FALSE; }
    Int result() override { return 0; }

private:
    GameWindow* m_window;
    Bool m_enabled;
};

IMEManagerInterface* TheIMEManager = nullptr;

IMEManagerInterface* CreateIMEManagerInterface()
{
    static NullIMEManager g_nullImeManager;
    return &g_nullImeManager;
}

#endif // !_WIN32
