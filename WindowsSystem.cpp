#ifdef PLATFORM_WINDOWS

#include "platform/WindowsSystem.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <commdlg.h>
#include <map>
#include <chrono>

// Need to link with OpenGL libraries
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

namespace CardGameLib {
namespace Platform {

// Static map to store WindowsSystem instances for each window handle
static std::map<HWND, WindowsSystem*> g_windowMap;

WindowsSystem::WindowsSystem()
    : m_hwnd(nullptr)
    , m_hdc(nullptr)
    , m_hglrc(nullptr)
    , m_hInstance(GetModuleHandle(nullptr))
    , m_width(800)
    , m_height(600)
    , m_visible(false)
{
    // Get performance counter frequency for high-precision timing
    QueryPerformanceFrequency(&m_counterFrequency);
    QueryPerformanceCounter(&m_startTime);
    
    Initialize();
}

WindowsSystem::~WindowsSystem()
{
    DestroyWindow();
}

void WindowsSystem::Initialize()
{
    RegisterWindowClass();
}

bool WindowsSystem::RegisterWindowClass()
{
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_hInstance;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = L"CardGameLibWindowClass";
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    
    return RegisterClassEx(&wc) != 0;
}

bool WindowsSystem::CreateWindow(int width, int height, const std::string& title)
{
    m_width = width;
    m_height = height;
    
    // Convert title to wide string
    std::wstring wideTitle = StringToWString(title);
    
    // Calculate window size based on desired client area
    RECT windowRect = { 0, 0, width, height };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
    
    // Create the window
    m_hwnd = ::CreateWindowEx(
        0,
        L"CardGameLibWindowClass",
        wideTitle.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,
        nullptr,
        m_hInstance,
        nullptr
    );
    
    if (!m_hwnd) {
        return false;
    }
    
    // Store this instance in the map
    g_windowMap[m_hwnd] = this;
    
    // Get the device context
    m_hdc = ::GetDC(m_hwnd);
    if (!m_hdc) {
        ::DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
        return false;
    }
    
    return true;
}

void WindowsSystem::DestroyWindow()
{
    // Clean up OpenGL context
    DestroyGLContext();
    
    // Release the device context
    if (m_hwnd && m_hdc) {
        ::ReleaseDC(m_hwnd, m_hdc);
        m_hdc = nullptr;
    }
    
    // Destroy the window
    if (m_hwnd) {
        // Remove from map
        g_windowMap.erase(m_hwnd);
        
        ::DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
}

void WindowsSystem::SetWindowTitle(const std::string& title)
{
    if (m_hwnd) {
        ::SetWindowText(m_hwnd, StringToWString(title).c_str());
    }
}

void WindowsSystem::SetWindowSize(int width, int height)
{
    if (m_hwnd) {
        m_width = width;
        m_height = height;
        
        // Calculate window size based on desired client area
        RECT windowRect = { 0, 0, width, height };
        AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
        
        ::SetWindowPos(
            m_hwnd,
            nullptr,
            0, 0,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            SWP_NOMOVE | SWP_NOZORDER
        );
    }
}

void WindowsSystem::GetWindowSize(int& width, int& height) const
{
    width = m_width;
    height = m_height;
}

void WindowsSystem::SetWindowPosition(int x, int y)
{
    if (m_hwnd) {
        ::SetWindowPos(
            m_hwnd,
            nullptr,
            x, y,
            0, 0,
            SWP_NOSIZE | SWP_NOZORDER
        );
    }
}

void WindowsSystem::GetWindowPosition(int& x, int& y) const
{
    if (m_hwnd) {
        RECT rect;
        ::GetWindowRect(m_hwnd, &rect);
        x = rect.left;
        y = rect.top;
    } else {
        x = 0;
        y = 0;
    }
}

void WindowsSystem::ShowWindow()
{
    if (m_hwnd) {
        ::ShowWindow(m_hwnd, SW_SHOW);
        ::UpdateWindow(m_hwnd);
        m_visible = true;
    }
}

void WindowsSystem::HideWindow()
{
    if (m_hwnd) {
        ::ShowWindow(m_hwnd, SW_HIDE);
        m_visible = false;
    }
}

bool WindowsSystem::IsWindowVisible() const
{
    return m_visible;
}

void WindowsSystem::SetWindowEventCallback(WindowEventCallback callback)
{
    m_eventCallback = callback;
}

bool WindowsSystem::CreateGLContext()
{
    if (!m_hwnd || !m_hdc) {
        return false;
    }
    
    // Set pixel format
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;
    
    int pixelFormat = ::ChoosePixelFormat(m_hdc, &pfd);
    if (pixelFormat == 0) {
        return false;
    }
    
    if (!::SetPixelFormat(m_hdc, pixelFormat, &pfd)) {
        return false;
    }
    
    // Create OpenGL context
    m_hglrc = ::wglCreateContext(m_hdc);
    if (!m_hglrc) {
        return false;
    }
    
    // Make context current
    if (!::wglMakeCurrent(m_hdc, m_hglrc)) {
        ::wglDeleteContext(m_hglrc);
        m_hglrc = nullptr;
        return false;
    }
    
    // Initialize OpenGL
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    
    // Set up viewport
    glViewport(0, 0, m_width, m_height);
    
    return true;
}

void WindowsSystem::DestroyGLContext()
{
    if (m_hglrc) {
        ::wglMakeCurrent(nullptr, nullptr);
        ::wglDeleteContext(m_hglrc);
        m_hglrc = nullptr;
    }
}

void WindowsSystem::MakeGLContextCurrent()
{
    if (m_hdc && m_hglrc) {
        ::wglMakeCurrent(m_hdc, m_hglrc);
    }
}

void WindowsSystem::SwapBuffers()
{
    if (m_hdc) {
        ::SwapBuffers(m_hdc);
    }
}

void WindowsSystem::SetVSync(bool enabled)
{
    // Load the wglSwapIntervalEXT function if available
    typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC)(int interval);
    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = nullptr;
    
    const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
    if (strstr(extensions, "WGL_EXT_swap_control") != nullptr) {
        wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
        if (wglSwapIntervalEXT) {
            wglSwapIntervalEXT(enabled ? 1 : 0);
        }
    }
}

void WindowsSystem::PollEvents()
{
    MSG msg = {};
    while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }
}

void WindowsSystem::GetMousePosition(int& x, int& y) const
{
    POINT point;
    ::GetCursorPos(&point);
    
    if (m_hwnd) {
        ::ScreenToClient(m_hwnd, &point);
    }
    
    x = point.x;
    y = point.y;
}

void WindowsSystem::SetMousePosition(int x, int y)
{
    if (m_hwnd) {
        POINT point = { x, y };
        ::ClientToScreen(m_hwnd, &point);
        ::SetCursorPos(point.x, point.y);
    }
}

void WindowsSystem::ShowMouse()
{
    ::ShowCursor(TRUE);
}

void WindowsSystem::HideMouse()
{
    ::ShowCursor(FALSE);
}

double WindowsSystem::GetTime() const
{
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    
    return static_cast<double>(currentTime.QuadPart - m_startTime.QuadPart) / 
           static_cast<double>(m_counterFrequency.QuadPart);
}

void WindowsSystem::Sleep(int milliseconds)
{
    ::Sleep(milliseconds);
}

std::string WindowsSystem::OpenFileDialog(const std::string& title, 
                                        const std::string& defaultPath,
                                        const std::string& filter)
{
    OPENFILENAME ofn = {};
    WCHAR szFile[260] = {};
    
    // Convert default path to wide string
    if (!defaultPath.empty()) {
        wcscpy_s(szFile, StringToWString(defaultPath).c_str());
    }
    
    // Convert title and filter to wide strings
    std::wstring wideTitle = StringToWString(title);
    std::wstring wideFilter = StringToWString(filter);
    
    // Replace '|' with null character
    for (size_t i = 0; i < wideFilter.length(); ++i) {
        if (wideFilter[i] == L'|') {
            wideFilter[i] = L'\0';
        }
    }
    
    // Add a double null-terminator
    wideFilter.push_back(L'\0');
    
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(WCHAR);
    ofn.lpstrFilter = wideFilter.c_str();
    ofn.lpstrTitle = wideTitle.c_str();
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    
    if (::GetOpenFileName(&ofn)) {
        return WCharToString(szFile);
    }
    
    return "";
}

std::string WindowsSystem::SaveFileDialog(const std::string& title, 
                                        const std::string& defaultPath,
                                        const std::string& filter)
{
    OPENFILENAME ofn = {};
    WCHAR szFile[260] = {};
    
    // Convert default path to wide string
    if (!defaultPath.empty()) {
        wcscpy_s(szFile, StringToWString(defaultPath).c_str());
    }
    
    // Convert title and filter to wide strings
    std::wstring wideTitle = StringToWString(title);
    std::wstring wideFilter = StringToWString(filter);
    
    // Replace '|' with null character
    for (size_t i = 0; i < wideFilter.length(); ++i) {
        if (wideFilter[i] == L'|') {
            wideFilter[i] = L'\0';
        }
    }
    
    // Add a double null-terminator
    wideFilter.push_back(L'\0');
    
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(WCHAR);
    ofn.lpstrFilter = wideFilter.c_str();
    ofn.lpstrTitle = wideTitle.c_str();
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
    
    if (::GetSaveFileName(&ofn)) {
        return WCharToString(szFile);
    }
    
    return "";
}

void WindowsSystem::SetClipboardText(const std::string& text)
{
    if (!::OpenClipboard(m_hwnd)) {
        return;
    }
    
    // Empty the clipboard
    ::EmptyClipboard();
    
    // Allocate global memory
    HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
    if (!hGlobal) {
        ::CloseClipboard();
        return;
    }
    
    // Lock the memory and copy the text
    char* pGlobal = (char*)::GlobalLock(hGlobal);
    memcpy(pGlobal, text.c_str(), text.size());
    pGlobal[text.size()] = '\0';
    ::GlobalUnlock(hGlobal);
    
    // Set clipboard data
    ::SetClipboardData(CF_TEXT, hGlobal);
    ::CloseClipboard();
}

std::string WindowsSystem::GetClipboardText() const
{
    std::string result;
    
    if (!::OpenClipboard(m_hwnd)) {
        return result;
    }
    
    // Get clipboard data
    HANDLE hData = ::GetClipboardData(CF_TEXT);
    if (hData) {
        // Lock the memory and copy the text
        const char* pszText = (const char*)::GlobalLock(hData);
        if (pszText) {
            result = pszText;
            ::GlobalUnlock(hData);
        }
    }
    
    ::CloseClipboard();
    return result;
}

LRESULT CALLBACK WindowsSystem::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Get the WindowsSystem instance for this window
    WindowsSystem* platform = nullptr;
    if (g_windowMap.find(hwnd) != g_windowMap.end()) {
        platform = g_windowMap[hwnd];
    }
    
    switch (uMsg) {
        case WM_CLOSE:
            if (platform && platform->m_eventCallback) {
                platform->m_eventCallback(WindowEventType::CLOSE, 0, 0);
            }
            return 0;
            
        case WM_SIZE:
            if (platform && platform->m_eventCallback) {
                platform->m_width = LOWORD(lParam);
                platform->m_height = HIWORD(lParam);
                platform->m_eventCallback(WindowEventType::RESIZE, platform->m_width, platform->m_height);
                
                // Update OpenGL viewport
                if (platform->m_hglrc) {
                    glViewport(0, 0, platform->m_width, platform->m_height);
                }
            }
            break;
            
        case WM_ACTIVATE:
            if (platform && platform->m_eventCallback) {
                if (LOWORD(wParam) != WA_INACTIVE) {
                    platform->m_eventCallback(WindowEventType::FOCUS, 0, 0);
                } else {
                    platform->m_eventCallback(WindowEventType::UNFOCUS, 0, 0);
                }
            }
            break;
    }
    
    return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

std::string WindowsSystem::WCharToString(const wchar_t* wstr)
{
    if (!wstr) return "";
    
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &strTo[0], size_needed, nullptr, nullptr);
    
    // Remove the null terminator from the string
    if (!strTo.empty() && strTo.back() == '\0') {
        strTo.pop_back();
    }
    
    return strTo;
}

std::wstring WindowsSystem::StringToWString(const std::string& str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstrTo[0], size_needed);
    
    // Remove the null terminator from the string
    if (!wstrTo.empty() && wstrTo.back() == L'\0') {
        wstrTo.pop_back();
    }
    
    return wstrTo;
}

// Factory function implementation
PlatformSystem* CreatePlatformSystem()
{
    return new WindowsSystem();
}

} // namespace Platform
} // namespace CardGameLib

#endif // PLATFORM_WINDOWS
