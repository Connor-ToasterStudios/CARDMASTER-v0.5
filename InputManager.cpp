#include "input/InputManager.h"

namespace CardGameLib {
namespace Input {

InputManager::InputManager()
    : m_mouseX(0)
    , m_mouseY(0)
    , m_prevMouseX(0)
    , m_prevMouseY(0)
    , m_mouseButtonCallback(nullptr)
{
    // Initialize mouse buttons state
    for (int i = 0; i < 3; i++) {
        m_mouseButtons[i] = false;
    }
    
    // Initialize keys state
    for (int i = 0; i < 512; i++) {
        m_keys[i] = false;
    }
}

InputManager::~InputManager()
{
    // No specific cleanup needed
}

void InputManager::Initialize()
{
    // No specific initialization needed
}

void InputManager::Update()
{
    // Store previous mouse position for calculating delta
    m_prevMouseX = m_mouseX;
    m_prevMouseY = m_mouseY;
}

bool InputManager::IsMouseButtonDown(MouseButton button) const
{
    return m_mouseButtons[static_cast<int>(button)];
}

void InputManager::GetMousePosition(int& x, int& y) const
{
    x = m_mouseX;
    y = m_mouseY;
}

bool InputManager::IsKeyDown(int keyCode) const
{
    if (keyCode >= 0 && keyCode < 512) {
        return m_keys[keyCode];
    }
    return false;
}

void InputManager::RegisterMouseCallback(MouseCallback callback)
{
    m_mouseCallbacks.push_back(callback);
}

void InputManager::RegisterKeyCallback(KeyCallback callback)
{
    m_keyCallbacks.push_back(callback);
}

void InputManager::SetMouseButtonCallback(MouseButtonCallback callback)
{
    m_mouseButtonCallback = callback;
}

void InputManager::OnMouseButtonEvent(MouseButton button, bool pressed, int x, int y)
{
    // Update internal state
    m_mouseButtons[static_cast<int>(button)] = pressed;
    m_mouseX = x;
    m_mouseY = y;
    
    // Create event
    MouseEvent event;
    event.type = pressed ? MouseEventType::PRESS : MouseEventType::RELEASE;
    event.button = button;
    event.x = x;
    event.y = y;
    event.scrollDelta = 0;
    
    // Notify callbacks
    for (const auto& callback : m_mouseCallbacks) {
        callback(event);
    }
    
    // Call simple mouse button callback if it exists
    if (m_mouseButtonCallback) {
        m_mouseButtonCallback(x, y, pressed);
    }
}

void InputManager::OnMouseMoveEvent(int x, int y)
{
    // Update internal state
    m_mouseX = x;
    m_mouseY = y;
    
    // Create event
    MouseEvent event;
    event.type = MouseEventType::MOVE;
    event.button = MouseButton::LEFT; // Not relevant for move events
    event.x = x;
    event.y = y;
    event.scrollDelta = 0;
    
    // Notify callbacks
    for (const auto& callback : m_mouseCallbacks) {
        callback(event);
    }
}

void InputManager::OnMouseWheelEvent(int delta)
{
    // Create event
    MouseEvent event;
    event.type = MouseEventType::WHEEL;
    event.button = MouseButton::LEFT; // Not relevant for wheel events
    event.x = m_mouseX;
    event.y = m_mouseY;
    event.scrollDelta = delta;
    
    // Notify callbacks
    for (const auto& callback : m_mouseCallbacks) {
        callback(event);
    }
}

void InputManager::OnKeyEvent(int keyCode, bool pressed)
{
    if (keyCode >= 0 && keyCode < 512) {
        // Update internal state
        m_keys[keyCode] = pressed;
        
        // Create event
        KeyEvent event;
        event.type = pressed ? KeyEventType::PRESS : KeyEventType::RELEASE;
        event.keyCode = keyCode;
        
        // Notify callbacks
        for (const auto& callback : m_keyCallbacks) {
            callback(event);
        }
    }
}

} // namespace Input
} // namespace CardGameLib
