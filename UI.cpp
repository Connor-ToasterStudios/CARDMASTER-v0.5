#include "ui/UI.h"

namespace CardGameLib {
namespace UI {

//
// UIElement implementation
//

UIElement::UIElement()
    : m_x(0.0f)
    , m_y(0.0f)
    , m_width(100.0f)
    , m_height(50.0f)
    , m_visible(true)
    , m_enabled(true)
    , m_focused(false)
    , m_parent(nullptr)
    , m_borderWidth(0.0f)
{
    // Default background color (transparent)
    m_bgColor[0] = 1.0f;
    m_bgColor[1] = 1.0f;
    m_bgColor[2] = 1.0f;
    m_bgColor[3] = 0.0f;
    
    // Default border color (black)
    m_borderColor[0] = 0.0f;
    m_borderColor[1] = 0.0f;
    m_borderColor[2] = 0.0f;
    m_borderColor[3] = 1.0f;
}

UIElement::~UIElement()
{
    // Clear children
    ClearChildren();
}

void UIElement::SetPosition(float x, float y)
{
    m_x = x;
    m_y = y;
}

void UIElement::SetSize(float width, float height)
{
    m_width = width;
    m_height = height;
}

void UIElement::SetVisible(bool visible)
{
    m_visible = visible;
}

bool UIElement::IsVisible() const
{
    return m_visible;
}

void UIElement::SetEnabled(bool enabled)
{
    m_enabled = enabled;
}

bool UIElement::IsEnabled() const
{
    return m_enabled;
}

void UIElement::SetFocused(bool focused)
{
    m_focused = focused;
}

bool UIElement::IsFocused() const
{
    return m_focused;
}

void UIElement::SetParent(UIElement* parent)
{
    m_parent = parent;
}

UIElement* UIElement::GetParent() const
{
    return m_parent;
}

void UIElement::AddChild(std::shared_ptr<UIElement> child)
{
    child->SetParent(this);
    m_children.push_back(child);
}

void UIElement::RemoveChild(UIElement* child)
{
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        if (it->get() == child) {
            (*it)->SetParent(nullptr);
            m_children.erase(it);
            break;
        }
    }
}

void UIElement::ClearChildren()
{
    for (auto& child : m_children) {
        child->SetParent(nullptr);
    }
    m_children.clear();
}

std::vector<std::shared_ptr<UIElement>> UIElement::GetChildren() const
{
    return m_children;
}

void UIElement::RegisterEventCallback(UIEventType type, UIEventCallback callback)
{
    m_eventCallbacks.push_back(std::make_pair(type, callback));
}

void UIElement::TriggerEvent(const UIEvent& event)
{
    for (const auto& callback : m_eventCallbacks) {
        if (callback.first == event.type) {
            callback.second(event);
        }
    }
}

bool UIElement::HandleMouseEvent(const Input::MouseEvent& event)
{
    // Check if the element is visible and enabled
    if (!m_visible || !m_enabled) {
        return false;
    }
    
    // Convert to absolute coordinates
    float absoluteX = m_x;
    float absoluteY = m_y;
    UIElement* parent = m_parent;
    while (parent) {
        absoluteX += parent->GetX();
        absoluteY += parent->GetY();
        parent = parent->GetParent();
    }
    
    // Check if the mouse event is inside this element
    bool isInside = event.x >= absoluteX && event.x < absoluteX + m_width &&
                    event.y >= absoluteY && event.y < absoluteY + m_height;
    
    // Handle mouse events based on type
    if (isInside) {
        if (event.type == Input::MouseEventType::PRESS) {
            OnMouseDown(event.x, event.y, event.button);
            return true;
        }
        else if (event.type == Input::MouseEventType::RELEASE) {
            OnMouseUp(event.x, event.y, event.button);
            return true;
        }
        else if (event.type == Input::MouseEventType::MOVE) {
            OnMouseMove(event.x, event.y);
            return true;
        }
    }
    
    // Handle children
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if ((*it)->HandleMouseEvent(event)) {
            return true;
        }
    }
    
    return false;
}

bool UIElement::HandleKeyEvent(const Input::KeyEvent& event)
{
    // Check if the element is visible, enabled, and focused
    if (!m_visible || !m_enabled || !m_focused) {
        return false;
    }
    
    // Handle key events based on type
    if (event.type == Input::KeyEventType::PRESS) {
        OnKeyDown(event.keyCode);
        return true;
    }
    else if (event.type == Input::KeyEventType::RELEASE) {
        OnKeyUp(event.keyCode);
        return true;
    }
    
    return false;
}

void UIElement::SetID(const std::string& id)
{
    m_id = id;
}

const std::string& UIElement::GetID() const
{
    return m_id;
}

void UIElement::SetBackgroundColor(float r, float g, float b, float a)
{
    m_bgColor[0] = r;
    m_bgColor[1] = g;
    m_bgColor[2] = b;
    m_bgColor[3] = a;
}

void UIElement::GetBackgroundColor(float& r, float& g, float& b, float& a) const
{
    r = m_bgColor[0];
    g = m_bgColor[1];
    b = m_bgColor[2];
    a = m_bgColor[3];
}

void UIElement::SetBorderColor(float r, float g, float b, float a)
{
    m_borderColor[0] = r;
    m_borderColor[1] = g;
    m_borderColor[2] = b;
    m_borderColor[3] = a;
}

void UIElement::GetBorderColor(float& r, float& g, float& b, float& a) const
{
    r = m_borderColor[0];
    g = m_borderColor[1];
    b = m_borderColor[2];
    a = m_borderColor[3];
}

void UIElement::SetBorderWidth(float width)
{
    m_borderWidth = width;
}

float UIElement::GetBorderWidth() const
{
    return m_borderWidth;
}

bool UIElement::IsPointInside(float x, float y) const
{
    // Convert to absolute coordinates
    float absoluteX = m_x;
    float absoluteY = m_y;
    UIElement* parent = m_parent;
    while (parent) {
        absoluteX += parent->GetX();
        absoluteY += parent->GetY();
        parent = parent->GetParent();
    }
    
    return x >= absoluteX && x < absoluteX + m_width &&
           y >= absoluteY && y < absoluteY + m_height;
}

void UIElement::OnMouseDown(int x, int y, Input::MouseButton button)
{
    UIEvent event;
    event.type = UIEventType::CLICK;
    event.element = this;
    event.x = x;
    event.y = y;
    
    TriggerEvent(event);
}

void UIElement::OnMouseUp(int x, int y, Input::MouseButton button)
{
    // Default implementation does nothing
}

void UIElement::OnMouseMove(int x, int y)
{
    UIEvent event;
    event.type = UIEventType::HOVER;
    event.element = this;
    event.x = x;
    event.y = y;
    
    TriggerEvent(event);
}

void UIElement::OnKeyDown(int keyCode)
{
    UIEvent event;
    event.type = UIEventType::KEY_PRESS;
    event.element = this;
    event.keyCode = keyCode;
    
    TriggerEvent(event);
}

void UIElement::OnKeyUp(int keyCode)
{
    // Default implementation does nothing
}

//
// UIManager implementation
//

UIManager::UIManager()
    : m_renderer(nullptr)
    , m_inputManager(nullptr)
    , m_focusedElement(nullptr)
    , m_hoveredElement(nullptr)
{
}

UIManager::~UIManager()
{
    ClearElements();
}

void UIManager::Initialize(Graphics::Renderer* renderer, Input::InputManager* inputManager)
{
    m_renderer = renderer;
    m_inputManager = inputManager;
    
    // Register input callbacks
    if (m_inputManager) {
        m_inputManager->RegisterMouseCallback([this](const Input::MouseEvent& event) {
            OnMouseEvent(event);
        });
        
        m_inputManager->RegisterKeyCallback([this](const Input::KeyEvent& event) {
            OnKeyEvent(event);
        });
    }
}

void UIManager::AddElement(std::shared_ptr<UIElement> element)
{
    m_rootElements.push_back(element);
}

void UIManager::RemoveElement(UIElement* element)
{
    for (auto it = m_rootElements.begin(); it != m_rootElements.end(); ++it) {
        if (it->get() == element) {
            m_rootElements.erase(it);
            break;
        }
    }
    
    // Clear focus if this element or a child had focus
    if (m_focusedElement == element || 
        (m_focusedElement && element->GetChildren().size() > 0)) {
        m_focusedElement = nullptr;
    }
}

void UIManager::ClearElements()
{
    m_rootElements.clear();
    m_focusedElement = nullptr;
    m_hoveredElement = nullptr;
}

std::shared_ptr<UIElement> UIManager::FindElementByID(const std::string& id)
{
    for (auto& element : m_rootElements) {
        if (element->GetID() == id) {
            return element;
        }
        
        // Search children recursively
        std::vector<std::shared_ptr<UIElement>> children = element->GetChildren();
        for (auto& child : children) {
            if (child->GetID() == id) {
                return child;
            }
        }
    }
    
    return nullptr;
}

void UIManager::HandleInput()
{
    // The input manager callbacks will handle input events
}

void UIManager::Render()
{
    if (!m_renderer) {
        return;
    }
    
    // Render each root element
    for (auto& element : m_rootElements) {
        if (element->IsVisible()) {
            element->Render(m_renderer);
        }
    }
}

void UIManager::SetFocusedElement(UIElement* element)
{
    // Clear focus from the previously focused element
    if (m_focusedElement && m_focusedElement != element) {
        m_focusedElement->SetFocused(false);
        
        UIEvent event;
        event.type = UIEventType::BLUR;
        event.element = m_focusedElement;
        m_focusedElement->TriggerEvent(event);
    }
    
    // Set focus to the new element
    m_focusedElement = element;
    
    if (m_focusedElement) {
        m_focusedElement->SetFocused(true);
        
        UIEvent event;
        event.type = UIEventType::FOCUS;
        event.element = m_focusedElement;
        m_focusedElement->TriggerEvent(event);
    }
}

UIElement* UIManager::GetFocusedElement() const
{
    return m_focusedElement;
}

void UIManager::OnMouseEvent(const Input::MouseEvent& event)
{
    // Process the mouse event through the UI hierarchy
    for (auto it = m_rootElements.rbegin(); it != m_rootElements.rend(); ++it) {
        if ((*it)->HandleMouseEvent(event)) {
            break;
        }
    }
    
    // Handle focus on mouse click
    if (event.type == Input::MouseEventType::PRESS && event.button == Input::MouseButton::LEFT) {
        UIElement* elementAtPoint = FindElementAt(static_cast<float>(event.x), static_cast<float>(event.y));
        SetFocusedElement(elementAtPoint);
    }
    
    // Update hovered element on mouse move
    if (event.type == Input::MouseEventType::MOVE) {
        UIElement* elementAtPoint = FindElementAt(static_cast<float>(event.x), static_cast<float>(event.y));
        if (elementAtPoint != m_hoveredElement) {
            m_hoveredElement = elementAtPoint;
        }
    }
}

void UIManager::OnKeyEvent(const Input::KeyEvent& event)
{
    // Forward key events to the focused element
    if (m_focusedElement) {
        m_focusedElement->HandleKeyEvent(event);
    }
}

UIElement* UIManager::FindElementAt(float x, float y)
{
    UIElement* result = nullptr;
    
    // Check each root element in reverse order (top-to-bottom)
    for (auto it = m_rootElements.rbegin(); it != m_rootElements.rend(); ++it) {
        FindElementAtRecursive(it->get(), x, y, result);
        if (result) {
            break;
        }
    }
    
    return result;
}

void UIManager::FindElementAtRecursive(UIElement* element, float x, float y, UIElement*& result)
{
    if (!element || !element->IsVisible() || !element->IsEnabled()) {
        return;
    }
    
    // Check children first (top-to-bottom)
    const auto& children = element->GetChildren();
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        FindElementAtRecursive(it->get(), x, y, result);
        if (result) {
            return;
        }
    }
    
    // Convert to absolute coordinates
    float absoluteX = element->GetX();
    float absoluteY = element->GetY();
    UIElement* parent = element->GetParent();
    while (parent) {
        absoluteX += parent->GetX();
        absoluteY += parent->GetY();
        parent = parent->GetParent();
    }
    
    // Check if point is inside this element
    if (x >= absoluteX && x < absoluteX + element->GetWidth() &&
        y >= absoluteY && y < absoluteY + element->GetHeight()) {
        result = element;
    }
}

} // namespace UI
} // namespace CardGameLib
