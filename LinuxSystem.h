#pragma once

#ifdef PLATFORM_LINUX

#include "platform/Platform.h"
// Include X11 headers directly now that we have them installed
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <GL/glx.h>
#include <string>
#include <map>
#include <time.h>

namespace CardGameLib {
namespace Platform {

class LinuxSystem : public PlatformSystem {
public:
    LinuxSystem();
    ~LinuxSystem();
    
    // Window management
    bool CreateWindow(int width, int height, const std::string& title) override;
    void DestroyWindow() override;
    void SetWindowTitle(const std::string& title) override;
    void SetWindowSize(int width, int height) override;
    void GetWindowSize(int& width, int& height) const override;
    void SetWindowPosition(int x, int y) override;
    void GetWindowPosition(int& x, int& y) const override;
    void ShowWindow() override;
    void HideWindow() override;
    bool IsWindowVisible() const override;
    void SetWindowEventCallback(WindowEventCallback callback) override;
    
    // OpenGL context
    bool CreateGLContext() override;
    void DestroyGLContext() override;
    void MakeGLContextCurrent() override;
    void SwapBuffers() override;
    void SetVSync(bool enabled) override;
    
    // Event handling
    void PollEvents() override;
    
    // Mouse handling
    void GetMousePosition(int& x, int& y) const override;
    void SetMousePosition(int x, int y) override;
    void ShowMouse() override;
    void HideMouse() override;
    
    // Time
    double GetTime() const override;
    void Sleep(int milliseconds) override;
    
    // Platform detection
    bool IsWindows() const override { return false; }
    bool IsLinux() const override { return true; }
    
    // File dialog
    std::string OpenFileDialog(const std::string& title, 
                           const std::string& defaultPath,
                           const std::string& filter) override;
    std::string SaveFileDialog(const std::string& title, 
                           const std::string& defaultPath,
                           const std::string& filter) override;
    
    // Clipboard
    void SetClipboardText(const std::string& text) override;
    std::string GetClipboardText() const override;
    
    // X11 specific methods
    Display* GetDisplay() const { return m_display; }
    Window GetWindow() const { return m_window; }
    GLXContext GetGLContext() const { return m_glContext; }
    
private:
    Display* m_display;
    Window m_window;
    GLXContext m_glContext;
    Atom m_wmDeleteMessage;
    
    int m_width;
    int m_height;
    bool m_visible;
    
    WindowEventCallback m_eventCallback;
    
    // For vsync support
    typedef void (*PFNGLXSWAPINTERVALEXTPROC)(Display*, GLXDrawable, int);
    PFNGLXSWAPINTERVALEXTPROC m_glXSwapIntervalEXT;
    
    // For high-precision timing
    struct timespec m_startTime;
    
    // Initialize X11 system
    void Initialize();
    
    // Handle X11 events
    void HandleEvent(XEvent& event);
    
    // Helper to execute shell commands (for file dialogs)
    std::string ExecuteShellCommand(const std::string& command) const;
};

} // namespace Platform
} // namespace CardGameLib

#endif // PLATFORM_LINUX
