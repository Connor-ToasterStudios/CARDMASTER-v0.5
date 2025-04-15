#pragma once

#include <vector>
#include <memory>
#include <string>
#include <functional>
#include "graphics/Renderer.h"
#include "input/InputManager.h"

namespace CardGameLib {
namespace UI {

// Forward declarations
class UIElement;

enum class UIElementType {
    PANEL,
    BUTTON,
    LABEL,
    TEXTBOX,
    LISTBOX,
    CHECKBOX,
    RADIOBUTTON,
    COMBOBOX,
    SCROLLBAR,
    CUSTOM
};

// UI event types
enum class UIEventType {
    CLICK,
    HOVER,
    FOCUS,
    BLUR,
    KEY_PRESS,
    TEXT_CHANGE,
    VALUE_CHANGE
};

// UI event data
struct UIEvent {
    UIEventType type;
    UIElement* element;
    int x, y;        // Mouse position for mouse events
    int keyCode;     // Key code for keyboard events
    std::string text; // Text for text events
    
    UIEvent() : type(UIEventType::CLICK), element(nullptr), x(0), y(0), keyCode(0) {}
};

// UI event callback
using UIEventCallback = std::function<void(const UIEvent&)>;

// Base class for UI elements
class UIElement {
public:
    UIElement();
    virtual ~UIElement();
    
    // Position and size
    virtual void SetPosition(float x, float y);
    virtual void SetSize(float width, float height);
    
    float GetX() const { return m_x; }
    float GetY() const { return m_y; }
    float GetWidth() const { return m_width; }
    float GetHeight() const { return m_height; }
    
    // Element state
    virtual void SetVisible(bool visible);
    virtual bool IsVisible() const;
    
    virtual void SetEnabled(bool enabled);
    virtual bool IsEnabled() const;
    
    virtual void SetFocused(bool focused);
    virtual bool IsFocused() const;
    
    // Element hierarchy
    virtual void SetParent(UIElement* parent);
    virtual UIElement* GetParent() const;
    
    virtual void AddChild(std::shared_ptr<UIElement> child);
    virtual void RemoveChild(UIElement* child);
    virtual void ClearChildren();
    
    virtual std::vector<std::shared_ptr<UIElement>> GetChildren() const;
    
    // Element type
    virtual UIElementType GetType() const = 0;
    
    // Event handling
    virtual void RegisterEventCallback(UIEventType type, UIEventCallback callback);
    virtual void TriggerEvent(const UIEvent& event);
    
    // Input processing
    virtual bool HandleMouseEvent(const Input::MouseEvent& event);
    virtual bool HandleKeyEvent(const Input::KeyEvent& event);
    
    // Rendering
    virtual void Render(Graphics::Renderer* renderer) = 0;
    
    // Identification
    virtual void SetID(const std::string& id);
    virtual const std::string& GetID() const;
    
    // Styling
    virtual void SetBackgroundColor(float r, float g, float b, float a);
    virtual void GetBackgroundColor(float& r, float& g, float& b, float& a) const;
    
    virtual void SetBorderColor(float r, float g, float b, float a);
    virtual void GetBorderColor(float& r, float& g, float& b, float& a) const;
    
    virtual void SetBorderWidth(float width);
    virtual float GetBorderWidth() const;
    
protected:
    float m_x, m_y;
    float m_width, m_height;
    bool m_visible;
    bool m_enabled;
    bool m_focused;
    
    UIElement* m_parent;
    std::vector<std::shared_ptr<UIElement>> m_children;
    
    std::string m_id;
    
    float m_bgColor[4];
    float m_borderColor[4];
    float m_borderWidth;
    
    // Event callbacks
    std::vector<std::pair<UIEventType, UIEventCallback>> m_eventCallbacks;
    
    // Helper methods
    bool IsPointInside(float x, float y) const;
    virtual void OnMouseDown(int x, int y, Input::MouseButton button);
    virtual void OnMouseUp(int x, int y, Input::MouseButton button);
    virtual void OnMouseMove(int x, int y);
    virtual void OnKeyDown(int keyCode);
    virtual void OnKeyUp(int keyCode);
};

// UI Manager class
class UIManager {
public:
    UIManager();
    ~UIManager();
    
    // Initialization
    void Initialize(Graphics::Renderer* renderer, Input::InputManager* inputManager);
    
    // UI elements management
    void AddElement(std::shared_ptr<UIElement> element);
    void RemoveElement(UIElement* element);
    void ClearElements();
    
    std::shared_ptr<UIElement> FindElementByID(const std::string& id);
    
    // Input processing
    void HandleInput();
    
    // Rendering
    void Render();
    
    // Focus management
    void SetFocusedElement(UIElement* element);
    UIElement* GetFocusedElement() const;
    
private:
    Graphics::Renderer* m_renderer;
    Input::InputManager* m_inputManager;
    
    std::vector<std::shared_ptr<UIElement>> m_rootElements;
    UIElement* m_focusedElement;
    UIElement* m_hoveredElement;
    
    // Input event handlers
    void OnMouseEvent(const Input::MouseEvent& event);
    void OnKeyEvent(const Input::KeyEvent& event);
    
    // Helper methods
    UIElement* FindElementAt(float x, float y);
    void FindElementAtRecursive(UIElement* element, float x, float y, UIElement*& result);
};

} // namespace UI
} // namespace CardGameLib
