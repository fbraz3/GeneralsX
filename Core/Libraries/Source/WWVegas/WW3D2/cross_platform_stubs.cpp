// Cross-platform stub implementations for GeneralsX
// This file provides concrete implementations for classes that are needed
// during the linking phase but are not available on non-Windows platforms

#include <cstring>
#include <cstddef>
// Use the real engine headers to avoid ODR and ABI mismatches
#include "../../../../GameEngine/Include/Common/AsciiString.h"
#include "win32_compat.h"

// Simple stub class definitions
class ErrorDumpClass {
public:
    ErrorDumpClass() = default;
    ~ErrorDumpClass() = default;
};

class WebBrowser {
public:
    WebBrowser() = default;
    virtual ~WebBrowser() = default;
};

// Global variables
ErrorDumpClass g_LastErrorDump;
WebBrowser* TheWebBrowser = nullptr;

// Forward declarations
class Object;
class Coord3D;
class MouseIO;

// Command source type enum
enum CommandSourceType { 
    COMMANDSOURCE_PLAYER = 0,
    COMMANDSOURCE_AI = 1,
    COMMANDSOURCE_SCRIPT = 2 
};

// Graphics API type enum
enum GraphicsAPIType {
    GRAPHICS_API_DIRECTX8 = 0,
    GRAPHICS_API_OPENGL = 1
};

// Mouse base class with MouseCursor enum
class Mouse {
public:
    enum MouseCursor {
        CURSOR_ARROW = 0,
        CURSOR_HAND = 1,
        CURSOR_CROSSHAIR = 2
    };

    Mouse() {}
    virtual ~Mouse() {}
    
    virtual bool init() { return true; }
    virtual void reset() {}
    virtual void update() {}
    virtual void regainFocus() {}
    virtual void loseFocus() {}
    virtual void setVisibility(bool visible) {}
    virtual void capture() {}
    virtual void releaseCapture() {}
    virtual void setCursor(MouseCursor cursor) {}
};

// Win32Mouse class declaration
class Win32Mouse : public Mouse {
public:
    Win32Mouse();
    virtual ~Win32Mouse();
    
    virtual bool init() override;
    virtual void reset() override;
    virtual void update() override;
    bool getMouseEvent(MouseIO* io, bool peek);
    virtual void regainFocus() override;
    virtual void loseFocus() override;
    virtual void setVisibility(bool visible) override;
    virtual void capture() override;
    virtual void releaseCapture() override;
    virtual void setCursor(Mouse::MouseCursor cursor) override;
    bool initCursorResources();
    
    void addWin32Event(unsigned int message, unsigned long wParam, unsigned long lParam, unsigned long time);
    Mouse::MouseCursor getMouseCursor() const;
};

// Win32Mouse implementation
Win32Mouse::Win32Mouse() {}
Win32Mouse::~Win32Mouse() {}
bool Win32Mouse::init() { return true; }
void Win32Mouse::reset() {}
void Win32Mouse::update() {}
bool Win32Mouse::getMouseEvent(MouseIO* io, bool peek) { return false; }
void Win32Mouse::regainFocus() {}
void Win32Mouse::loseFocus() {}
void Win32Mouse::setVisibility(bool visible) {}
void Win32Mouse::capture() {}
void Win32Mouse::releaseCapture() {}
void Win32Mouse::setCursor(Mouse::MouseCursor cursor) {}
bool Win32Mouse::initCursorResources() { return true; }
void Win32Mouse::addWin32Event(unsigned int message, unsigned long wParam, unsigned long lParam, unsigned long time) {}
Mouse::MouseCursor Win32Mouse::getMouseCursor() const { return Mouse::CURSOR_ARROW; }

// Registry class declaration
class RegistryClass {
public:
    RegistryClass(const char* path, bool createIfNotExists);
    ~RegistryClass();
    
    int Get_Int(const char* key, int defaultValue);
    void Set_Int(const char* key, int value);
    void Get_String(const char* key, char* buffer, int bufferSize, const char* defaultValue);
    void Set_String(const char* key, const char* value);
};

// Registry class implementation
RegistryClass::RegistryClass(const char* path, bool createIfNotExists) {}
RegistryClass::~RegistryClass() {}
int RegistryClass::Get_Int(const char* key, int defaultValue) { return defaultValue; }
void RegistryClass::Set_Int(const char* key, int value) {}
void RegistryClass::Get_String(const char* key, char* buffer, int bufferSize, const char* defaultValue) {
    if (buffer && bufferSize > 0) {
        strncpy(buffer, defaultValue ? defaultValue : "", bufferSize - 1);
        buffer[bufferSize - 1] = '\0';
    }
}
void RegistryClass::Set_String(const char* key, const char* value) {}

// W3DRendererAdapter class declaration
class W3DRendererAdapter {
public:
    static bool s_useNewRenderer;
    static void Initialize(GraphicsAPIType api);
};

// W3DRendererAdapter implementation
bool W3DRendererAdapter::s_useNewRenderer = false;
void W3DRendererAdapter::Initialize(GraphicsAPIType api) {}

// DirectInputKeyboard class declaration
class DirectInputKeyboard {
public:
    DirectInputKeyboard();
    virtual ~DirectInputKeyboard();
};

// DirectInputKeyboard implementation
DirectInputKeyboard::DirectInputKeyboard() {}
DirectInputKeyboard::~DirectInputKeyboard() {}

// Win32LocalFileSystem class declaration
class Win32LocalFileSystem {
public:
    Win32LocalFileSystem();
    virtual ~Win32LocalFileSystem();
};

// Win32LocalFileSystem implementation
Win32LocalFileSystem::Win32LocalFileSystem() {}
Win32LocalFileSystem::~Win32LocalFileSystem() {}

// PartitionFilter class declarations
class PartitionFilterPossibleToEnter {
public:
    PartitionFilterPossibleToEnter(const Object* obj, CommandSourceType source);
    virtual ~PartitionFilterPossibleToEnter();
};

class PartitionFilterPossibleToHijack {
public:
    PartitionFilterPossibleToHijack(const Object* obj, CommandSourceType source);
    virtual ~PartitionFilterPossibleToHijack();
};

// PartitionFilter implementations
PartitionFilterPossibleToEnter::PartitionFilterPossibleToEnter(const Object* obj, CommandSourceType source) {}
PartitionFilterPossibleToEnter::~PartitionFilterPossibleToEnter() {}
PartitionFilterPossibleToHijack::PartitionFilterPossibleToHijack(const Object* obj, CommandSourceType source) {}
PartitionFilterPossibleToHijack::~PartitionFilterPossibleToHijack() {}

// PartitionManager class declaration
class PartitionManager {
public:
    static PartitionManager* getInstance() {
        static PartitionManager instance;
        return &instance;
    }
    
    int getPropShroudStatusForPlayer(int player, const Coord3D* coord) const;
};

// PartitionManager implementation
int PartitionManager::getPropShroudStatusForPlayer(int player, const Coord3D* coord) const {
    return 0;
}

// Registry function implementations
AsciiString GetRegistryLanguage(void) {
    // macOS/Linux default language when registry is unavailable.
    // Return a lowercase value to match common asset folder naming, e.g., Data/english/
    return AsciiString("english");
}

bool GetStringFromGeneralsRegistry(AsciiString section, AsciiString key, AsciiString& value) {
    value = AsciiString("");
    return false;
}