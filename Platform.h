#pragma once

#include <string>
#include <functional>

namespace CardGameLib {
namespace Platform {

// Window event type
enum class WindowEventType {
    CLOSE,
    RESIZE,
    FOCUS,
    UNFOCUS,
    PAINT,
    MOUSE_DOWN,
    MOUSE_UP,
    MOUSE_MOVE,
    KEY_DOWN,
    KEY_UP
};

// Window event callback
using WindowEventCallback = std::function<void(WindowEventType, int, int)>;

// Platform system interface
class PlatformSystem {
public:
    virtual ~PlatformSystem() = default;
    
    // Window management
    virtual bool CreateWindow(int width, int height, const std::string& title) = 0;
    virtual void DestroyWindow() = 0;
    virtual void SetWindowTitle(const std::string& title) = 0;
    virtual void SetWindowSize(int width, int height) = 0;
    virtual void GetWindowSize(int& width, int& height) const = 0;
    virtual void SetWindowPosition(int x, int y) = 0;
    virtual void GetWindowPosition(int& x, int& y) const = 0;
    virtual void ShowWindow() = 0;
    virtual void HideWindow() = 0;
    virtual bool IsWindowVisible() const = 0;
    virtual void SetWindowEventCallback(WindowEventCallback callback) = 0;
    
    // OpenGL context
    virtual bool CreateGLContext() = 0;
    virtual void DestroyGLContext() = 0;
    virtual void MakeGLContextCurrent() = 0;
    virtual void SwapBuffers() = 0;
    virtual void SetVSync(bool enabled) = 0;
    
    // Event handling
    virtual void PollEvents() = 0;
    
    // Mouse handling
    virtual void GetMousePosition(int& x, int& y) const = 0;
    virtual void SetMousePosition(int x, int y) = 0;
    virtual void ShowMouse() = 0;
    virtual void HideMouse() = 0;
    
    // Time
    virtual double GetTime() const = 0;
    virtual void Sleep(int milliseconds) = 0;
    
    // Platform detection
    virtual bool IsWindows() const = 0;
    virtual bool IsLinux() const = 0;
    
    // File dialog
    virtual std::string OpenFileDialog(const std::string& title, 
                                    const std::string& defaultPath,
                                    const std::string& filter) = 0;
    virtual std::string SaveFileDialog(const std::string& title, 
                                    const std::string& defaultPath,
                                    const std::string& filter) = 0;
    
    // Clipboard
    virtual void SetClipboardText(const std::string& text) = 0;
    virtual std::string GetClipboardText() const = 0;
};

// Factory function to create the appropriate platform system
PlatformSystem* CreatePlatformSystem();

} // namespace Platform
} // namespace CardGameLib
