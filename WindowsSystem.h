#pragma once

#ifdef PLATFORM_WINDOWS

#include "platform/Platform.h"
#include <Windows.h>
#include <string>

namespace CardGameLib {
namespace Platform {

class WindowsSystem : public PlatformSystem {
public:
    WindowsSystem();
    ~WindowsSystem();
    
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
    bool IsWindows() const override { return true; }
    bool IsLinux() const override { return false; }
    
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
    
    // Win32 specific methods
    HWND GetWindowHandle() const { return m_hwnd; }
    HGLRC GetGLContext() const { return m_hglrc; }
    HDC GetDeviceContext() const { return m_hdc; }
    
    // Window procedure
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
private:
    HWND m_hwnd;
    HDC m_hdc;
    HGLRC m_hglrc;
    HINSTANCE m_hInstance;
    
    int m_width;
    int m_height;
    bool m_visible;
    
    WindowEventCallback m_eventCallback;
    
    // Performance counter frequency (for high-precision timing)
    LARGE_INTEGER m_counterFrequency;
    LARGE_INTEGER m_startTime;
    
    // Initialize Windows system
    void Initialize();
    
    // Register window class
    bool RegisterWindowClass();
    
    // Convert wchar_t to string
    std::string WCharToString(const wchar_t* wstr);
    
    // Convert string to wchar_t
    std::wstring StringToWString(const std::string& str);
};

} // namespace Platform
} // namespace CardGameLib

#endif // PLATFORM_WINDOWS
