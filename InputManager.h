#pragma once

#include <vector>
#include <functional>

namespace CardGameLib {
namespace Input {

enum class MouseButton {
    LEFT,
    RIGHT,
    MIDDLE
};

enum class MouseEventType {
    PRESS,
    RELEASE,
    MOVE,
    WHEEL
};

enum class KeyEventType {
    PRESS,
    RELEASE
};

struct MouseEvent {
    MouseEventType type;
    MouseButton button;
    int x;
    int y;
    int scrollDelta;
};

struct KeyEvent {
    KeyEventType type;
    int keyCode;
};

// Callback definitions
using MouseCallback = std::function<void(const MouseEvent&)>;
using KeyCallback = std::function<void(const KeyEvent&)>;
using MouseButtonCallback = std::function<void(int x, int y, bool isDown)>;

class InputManager {
public:
    InputManager();
    ~InputManager();
    
    // Initialize input system
    void Initialize();
    
    // Update input state (called each frame)
    void Update();
    
    // Mouse state
    bool IsMouseButtonDown(MouseButton button) const;
    void GetMousePosition(int& x, int& y) const;
    
    // Key state
    bool IsKeyDown(int keyCode) const;
    
    // Event registration
    void RegisterMouseCallback(MouseCallback callback);
    void RegisterKeyCallback(KeyCallback callback);
    void SetMouseButtonCallback(MouseButtonCallback callback);
    
    // Event handling (to be called by the platform layer)
    void OnMouseButtonEvent(MouseButton button, bool pressed, int x, int y);
    void OnMouseMoveEvent(int x, int y);
    void OnMouseWheelEvent(int delta);
    void OnKeyEvent(int keyCode, bool pressed);
    
private:
    // Mouse state
    bool m_mouseButtons[3];
    int m_mouseX;
    int m_mouseY;
    int m_prevMouseX;
    int m_prevMouseY;
    
    // Key state (using a simple array for key codes)
    bool m_keys[512];
    
    // Callbacks
    std::vector<MouseCallback> m_mouseCallbacks;
    std::vector<KeyCallback> m_keyCallbacks;
    MouseButtonCallback m_mouseButtonCallback;
};

} // namespace Input
} // namespace CardGameLib
