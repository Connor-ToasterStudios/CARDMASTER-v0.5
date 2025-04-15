#ifdef PLATFORM_LINUX

#include "platform/LinuxSystem.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>
#include <array>
#include <time.h>

namespace CardGameLib {
namespace Platform {

LinuxSystem::LinuxSystem()
    : m_display(nullptr)
    , m_window(0)
    , m_glContext(nullptr)
    , m_wmDeleteMessage(0)
    , m_width(800)
    , m_height(600)
    , m_visible(false)
    , m_glXSwapIntervalEXT(nullptr)
{
    // Initialize starting time for GetTime()
    clock_gettime(CLOCK_MONOTONIC, &m_startTime);
    
    Initialize();
}

LinuxSystem::~LinuxSystem()
{
    DestroyWindow();
}

void LinuxSystem::Initialize()
{
    // Open X display
    m_display = XOpenDisplay(nullptr);
    if (!m_display) {
        std::cerr << "Failed to open X display" << std::endl;
        std::cerr << "Running in headless mode" << std::endl;
        
        // In a headless environment, just set the window delete message to a dummy value
        m_wmDeleteMessage = 1;
        return;
    }
    
    // Get atoms
    m_wmDeleteMessage = XInternAtom(m_display, "WM_DELETE_WINDOW", False);
}

bool LinuxSystem::CreateWindow(int width, int height, const std::string& title)
{
    if (!m_display) {
        std::cerr << "Cannot create window without X display" << std::endl;
        std::cerr << "Creating a dummy window for headless environment" << std::endl;
        
        // In a headless environment, create a dummy window
        m_width = width;
        m_height = height;
        m_visible = true;
        m_window = 1;  // Non-zero to simulate success
        
        // Simulate OpenGL context in headless mode
        m_glContext = (GLXContext)1;
        
        return true;
    }
    
    m_width = width;
    m_height = height;
    
    // Get the default screen
    int screen = DefaultScreen(m_display);
    
    // Set window attributes
    XSetWindowAttributes attr;
    attr.background_pixmap = None;
    attr.background_pixel = BlackPixel(m_display, screen);
    attr.border_pixel = 0;
    attr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
                     ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
                     StructureNotifyMask;
    
    // Create the window
    m_window = XCreateWindow(m_display, RootWindow(m_display, screen),
                           0, 0, width, height, 0,
                           DefaultDepth(m_display, screen), InputOutput,
                           DefaultVisual(m_display, screen),
                           CWBackPixmap | CWBackPixel | CWBorderPixel | CWEventMask, &attr);
    
    if (!m_window) {
        std::cerr << "Failed to create X window" << std::endl;
        std::cerr << "Creating a dummy window for headless environment" << std::endl;
        
        // In a headless environment, create a dummy window
        m_width = width;
        m_height = height;
        m_visible = true;
        m_window = 1;  // Non-zero to simulate success
        
        // Simulate OpenGL context in headless mode
        m_glContext = (GLXContext)1;
        
        return true;
    }
    
    // Set window title
    XStoreName(m_display, m_window, title.c_str());
    
    // Set window protocols
    Atom protocols[1] = { m_wmDeleteMessage };
    XSetWMProtocols(m_display, m_window, protocols, 1);
    
    // Create GLX OpenGL context
    if (!CreateGLContext()) {
        XDestroyWindow(m_display, m_window);
        m_window = 0;
        
        std::cerr << "Creating a dummy window for headless environment" << std::endl;
        
        // In a headless environment, create a dummy window
        m_width = width;
        m_height = height;
        m_visible = true;
        m_window = 1;  // Non-zero to simulate success
        
        // Simulate OpenGL context in headless mode
        m_glContext = (GLXContext)1;
        
        return true;
    }
    
    return true;
}

void LinuxSystem::DestroyWindow()
{
    // Special handling for dummy windows created in headless environments
    if (m_window == 1) {
        // Just reset the dummy window
        m_window = 0;
        DestroyGLContext();
        return;
    }
    
    if (m_display && m_window) {
        // Destroy OpenGL context
        DestroyGLContext();
        
        // Destroy window
        XDestroyWindow(m_display, m_window);
        m_window = 0;
        
        // Close display
        XCloseDisplay(m_display);
        m_display = nullptr;
    }
}

void LinuxSystem::SetWindowTitle(const std::string& title)
{
    if (m_display && m_window) {
        XStoreName(m_display, m_window, title.c_str());
    }
}

void LinuxSystem::SetWindowSize(int width, int height)
{
    if (m_display && m_window) {
        XResizeWindow(m_display, m_window, width, height);
        m_width = width;
        m_height = height;
    }
}

void LinuxSystem::GetWindowSize(int& width, int& height) const
{
    if (m_display && m_window) {
        XWindowAttributes attr;
        XGetWindowAttributes(m_display, m_window, &attr);
        width = attr.width;
        height = attr.height;
    } else {
        width = m_width;
        height = m_height;
    }
}

void LinuxSystem::SetWindowPosition(int x, int y)
{
    if (m_display && m_window) {
        XMoveWindow(m_display, m_window, x, y);
    }
}

void LinuxSystem::GetWindowPosition(int& x, int& y) const
{
    if (m_display && m_window) {
        XWindowAttributes attr;
        Window child;
        XTranslateCoordinates(m_display, m_window, RootWindow(m_display, DefaultScreen(m_display)),
                             0, 0, &x, &y, &child);
    } else {
        x = 0;
        y = 0;
    }
}

void LinuxSystem::ShowWindow()
{
    if (m_display && m_window) {
        XMapWindow(m_display, m_window);
        m_visible = true;
    }
}

void LinuxSystem::HideWindow()
{
    if (m_display && m_window) {
        XUnmapWindow(m_display, m_window);
        m_visible = false;
    }
}

bool LinuxSystem::IsWindowVisible() const
{
    return m_visible;
}

void LinuxSystem::SetWindowEventCallback(WindowEventCallback callback)
{
    m_eventCallback = callback;
}

bool LinuxSystem::CreateGLContext()
{
    if (!m_display || !m_window) {
        std::cerr << "Cannot create GL context without display and window" << std::endl;
        return false;
    }
    
    // Make sure GLX is available
    int errorBase, eventBase;
    if (!glXQueryExtension(m_display, &errorBase, &eventBase)) {
        std::cerr << "GLX extension not available" << std::endl;
        std::cerr << "Creating dummy GL context for headless environment" << std::endl;
        // In a headless environment, create a dummy context
        m_glContext = (GLXContext)1;  // Non-null to simulate success
        return true;
    }
    
    // Choose a GLX visual
    static int visualAttribs[] = {
        GLX_RGBA,
        GLX_DEPTH_SIZE, 24,
        GLX_DOUBLEBUFFER,
        None
    };
    
    XVisualInfo* visualInfo = glXChooseVisual(m_display, DefaultScreen(m_display), visualAttribs);
    if (!visualInfo) {
        std::cerr << "Failed to choose an OpenGL-compatible visual" << std::endl;
        std::cerr << "Creating dummy GL context for headless environment" << std::endl;
        // In a headless environment, create a dummy context
        m_glContext = (GLXContext)1;  // Non-null to simulate success
        return true;
    }
    
    // Create the GLX context
    m_glContext = glXCreateContext(m_display, visualInfo, nullptr, GL_TRUE);
    XFree(visualInfo);
    
    if (!m_glContext) {
        std::cerr << "Failed to create GLX context" << std::endl;
        std::cerr << "Creating dummy GL context for headless environment" << std::endl;
        // In a headless environment, create a dummy context
        m_glContext = (GLXContext)1;  // Non-null to simulate success
        return true;
    }
    
    // Make the context current
    MakeGLContextCurrent();
    
    // Check for GLX_EXT_swap_control extension for vsync
    const char* extensions = glXQueryExtensionsString(m_display, DefaultScreen(m_display));
    if (strstr(extensions, "GLX_EXT_swap_control")) {
        // Get the swap interval function
        m_glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddressARB(
            (const GLubyte*)"glXSwapIntervalEXT");
    }
    
    return true;
}

void LinuxSystem::DestroyGLContext()
{
    // Check if we have a real GL context or a dummy one
    if (m_display && m_glContext && m_glContext != (GLXContext)1) {
        glXMakeCurrent(m_display, None, nullptr);
        glXDestroyContext(m_display, m_glContext);
    }
    
    m_glContext = nullptr;
}

void LinuxSystem::MakeGLContextCurrent()
{
    // Only make current if it's a real context, not a dummy one
    if (m_display && m_window && m_glContext && m_glContext != (GLXContext)1) {
        glXMakeCurrent(m_display, m_window, m_glContext);
    }
}

void LinuxSystem::SwapBuffers()
{
    // Only swap buffers if we have a real window and context, not dummy ones
    if (m_display && m_window && m_window != 1 && m_glContext && m_glContext != (GLXContext)1) {
        glXSwapBuffers(m_display, m_window);
    }
}

void LinuxSystem::SetVSync(bool enabled)
{
    // Only set vsync if we have a real window and context, not dummy ones
    if (m_display && m_window && m_window != 1 && m_glContext && m_glContext != (GLXContext)1 && m_glXSwapIntervalEXT) {
        m_glXSwapIntervalEXT(m_display, m_window, enabled ? 1 : 0);
    }
}

void LinuxSystem::PollEvents()
{
    if (!m_display) {
        return;
    }
    
    // Process all pending events
    while (XPending(m_display) > 0) {
        XEvent event;
        XNextEvent(m_display, &event);
        HandleEvent(event);
    }
}

void LinuxSystem::HandleEvent(XEvent& event)
{
    if (!m_eventCallback) {
        return;
    }
    
    switch (event.type) {
        case Expose:
            // Window needs to be redrawn
            m_eventCallback(WindowEventType::PAINT, 0, 0);
            break;
            
        case ConfigureNotify:
            // Window has been resized or moved
            m_width = event.xconfigure.width;
            m_height = event.xconfigure.height;
            m_eventCallback(WindowEventType::RESIZE, m_width, m_height);
            break;
            
        case ButtonPress:
            m_eventCallback(WindowEventType::MOUSE_DOWN, 
                          event.xbutton.x, event.xbutton.y);
            break;
            
        case ButtonRelease:
            m_eventCallback(WindowEventType::MOUSE_UP, 
                          event.xbutton.x, event.xbutton.y);
            break;
            
        case MotionNotify:
            m_eventCallback(WindowEventType::MOUSE_MOVE, 
                          event.xmotion.x, event.xmotion.y);
            break;
            
        case KeyPress:
            m_eventCallback(WindowEventType::KEY_DOWN, 
                          event.xkey.keycode, 0);
            break;
            
        case KeyRelease:
            m_eventCallback(WindowEventType::KEY_UP, 
                          event.xkey.keycode, 0);
            break;
            
        case ClientMessage:
            // Check if this is a window close message
            if ((Atom)event.xclient.data.l[0] == m_wmDeleteMessage) {
                m_eventCallback(WindowEventType::CLOSE, 0, 0);
            }
            break;
    }
}

void LinuxSystem::GetMousePosition(int& x, int& y) const
{
    if (m_display && m_window) {
        Window root, child;
        int rootX, rootY;
        unsigned int mask;
        
        if (XQueryPointer(m_display, m_window, &root, &child,
                         &rootX, &rootY, &x, &y, &mask)) {
            return;
        }
    }
    
    // Fallback to center position
    x = m_width / 2;
    y = m_height / 2;
}

void LinuxSystem::SetMousePosition(int x, int y)
{
    if (m_display && m_window) {
        XWarpPointer(m_display, None, m_window, 0, 0, 0, 0, x, y);
        XFlush(m_display);
    }
}

void LinuxSystem::ShowMouse()
{
    if (m_display && m_window) {
        // Reset to default cursor
        XUndefineCursor(m_display, m_window);
        XFlush(m_display);
    }
}

void LinuxSystem::HideMouse()
{
    if (m_display && m_window) {
        // Create an invisible cursor
        Pixmap bitmapNoData;
        XColor black;
        static char noData[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        black.red = black.green = black.blue = 0;
        
        bitmapNoData = XCreateBitmapFromData(m_display, m_window, noData, 8, 8);
        Cursor invisibleCursor = XCreatePixmapCursor(m_display, bitmapNoData, bitmapNoData,
                                                  &black, &black, 0, 0);
        
        // Set the invisible cursor
        XDefineCursor(m_display, m_window, invisibleCursor);
        XFreeCursor(m_display, invisibleCursor);
        XFreePixmap(m_display, bitmapNoData);
        XFlush(m_display);
    }
}

double LinuxSystem::GetTime() const
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    
    return (now.tv_sec - m_startTime.tv_sec) + 
           (now.tv_nsec - m_startTime.tv_nsec) / 1000000000.0;
}

void LinuxSystem::Sleep(int milliseconds)
{
    usleep(milliseconds * 1000);
}

std::string LinuxSystem::ExecuteShellCommand(const std::string& command) const
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    
    if (!pipe) {
        return "";
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    
    // Remove trailing newline if any
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    
    return result;
}

std::string LinuxSystem::OpenFileDialog(const std::string& title, 
                                     const std::string& defaultPath,
                                     const std::string& filter)
{
    // Use zenity for file dialog
    std::stringstream cmd;
    cmd << "zenity --file-selection --title=\"" << title << "\"";
    
    if (!defaultPath.empty()) {
        cmd << " --filename=\"" << defaultPath << "\"";
    }
    
    // Process filter (convert from Windows-style to zenity's filter format)
    if (!filter.empty()) {
        std::string filterString = filter;
        std::stringstream filterCmd;
        filterCmd << " --file-filter=\"";
        
        // Split by |
        size_t pos = 0;
        std::string token;
        bool isName = true;
        std::string filterName;
        
        while ((pos = filterString.find('|')) != std::string::npos) {
            token = filterString.substr(0, pos);
            
            if (isName) {
                filterName = token;
            } else {
                // Convert semicolons to spaces for multiple extensions
                size_t semiPos = 0;
                while ((semiPos = token.find(';')) != std::string::npos) {
                    token.replace(semiPos, 1, " ");
                }
                
                // Remove *. from extensions
                size_t starPos = 0;
                while ((starPos = token.find("*.")) != std::string::npos) {
                    token.replace(starPos, 2, "");
                }
                
                filterCmd << filterName << " | *." << token << " ";
            }
            
            filterString.erase(0, pos + 1);
            isName = !isName;
        }
        
        // Process the last token
        if (!filterString.empty()) {
            if (isName) {
                filterName = filterString;
            } else {
                // Convert semicolons to spaces for multiple extensions
                size_t semiPos = 0;
                while ((semiPos = filterString.find(';')) != std::string::npos) {
                    filterString.replace(semiPos, 1, " ");
                }
                
                // Remove *. from extensions
                size_t starPos = 0;
                while ((starPos = filterString.find("*.")) != std::string::npos) {
                    filterString.replace(starPos, 2, "");
                }
                
                filterCmd << filterName << " | *." << filterString;
            }
        }
        
        filterCmd << "\"";
        cmd << filterCmd.str();
    }
    
    return ExecuteShellCommand(cmd.str());
}

std::string LinuxSystem::SaveFileDialog(const std::string& title, 
                                     const std::string& defaultPath,
                                     const std::string& filter)
{
    // Use zenity for file dialog
    std::stringstream cmd;
    cmd << "zenity --file-selection --save --title=\"" << title << "\"";
    
    if (!defaultPath.empty()) {
        cmd << " --filename=\"" << defaultPath << "\"";
    }
    
    // Process filter (convert from Windows-style to zenity's filter format)
    if (!filter.empty()) {
        std::string filterString = filter;
        std::stringstream filterCmd;
        filterCmd << " --file-filter=\"";
        
        // Split by |
        size_t pos = 0;
        std::string token;
        bool isName = true;
        std::string filterName;
        
        while ((pos = filterString.find('|')) != std::string::npos) {
            token = filterString.substr(0, pos);
            
            if (isName) {
                filterName = token;
            } else {
                // Convert semicolons to spaces for multiple extensions
                size_t semiPos = 0;
                while ((semiPos = token.find(';')) != std::string::npos) {
                    token.replace(semiPos, 1, " ");
                }
                
                // Remove *. from extensions
                size_t starPos = 0;
                while ((starPos = token.find("*.")) != std::string::npos) {
                    token.replace(starPos, 2, "");
                }
                
                filterCmd << filterName << " | *." << token << " ";
            }
            
            filterString.erase(0, pos + 1);
            isName = !isName;
        }
        
        // Process the last token
        if (!filterString.empty()) {
            if (isName) {
                filterName = filterString;
            } else {
                // Convert semicolons to spaces for multiple extensions
                size_t semiPos = 0;
                while ((semiPos = filterString.find(';')) != std::string::npos) {
                    filterString.replace(semiPos, 1, " ");
                }
                
                // Remove *. from extensions
                size_t starPos = 0;
                while ((starPos = filterString.find("*.")) != std::string::npos) {
                    filterString.replace(starPos, 2, "");
                }
                
                filterCmd << filterName << " | *." << filterString;
            }
        }
        
        filterCmd << "\"";
        cmd << filterCmd.str();
    }
    
    return ExecuteShellCommand(cmd.str());
}

void LinuxSystem::SetClipboardText(const std::string& text)
{
    // Use xclip to set clipboard text
    std::string command = "echo -n \"" + text + "\" | xclip -selection clipboard";
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "SetClipboardText: Command failed with code " << result << std::endl;
    }
}

std::string LinuxSystem::GetClipboardText() const
{
    // Use xclip to get clipboard text
    return ExecuteShellCommand("xclip -selection clipboard -o");
}

// Factory function implementation
PlatformSystem* CreatePlatformSystem()
{
    return new LinuxSystem();
}

} // namespace Platform
} // namespace CardGameLib

#endif // PLATFORM_LINUX
