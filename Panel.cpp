#include "ui/Panel.h"

namespace CardGameLib {
namespace UI {

Panel::Panel()
    : m_draggable(false)
    , m_resizable(false)
    , m_hasTitleBar(false)
    , m_title("Panel")
    , m_titleBarHeight(20.0f)
    , m_isDragging(false)
    , m_isResizing(false)
    , m_dragStartX(0)
    , m_dragStartY(0)
    , m_dragStartPanelX(0.0f)
    , m_dragStartPanelY(0.0f)
    , m_dragStartPanelWidth(0.0f)
    , m_dragStartPanelHeight(0.0f)
{
    // Default panel style
    
    // Background color (semi-transparent gray)
    m_bgColor[0] = 0.2f;
    m_bgColor[1] = 0.2f;
    m_bgColor[2] = 0.2f;
    m_bgColor[3] = 0.8f;
    
    // Border color (dark gray)
    m_borderColor[0] = 0.1f;
    m_borderColor[1] = 0.1f;
    m_borderColor[2] = 0.1f;
    m_borderColor[3] = 1.0f;
    
    // Title bar color (dark blue)
    m_titleBarColor[0] = 0.1f;
    m_titleBarColor[1] = 0.1f;
    m_titleBarColor[2] = 0.3f;
    m_titleBarColor[3] = 1.0f;
    
    // Set border width
    m_borderWidth = 1.0f;
    
    // Set default size
    m_width = 200.0f;
    m_height = 150.0f;
}

Panel::~Panel()
{
    // Nothing special to clean up
}

void Panel::Render(Graphics::Renderer* renderer)
{
    if (!renderer) {
        return;
    }
    
    // Calculate absolute position
    float absoluteX = m_x;
    float absoluteY = m_y;
    UIElement* parent = m_parent;
    while (parent) {
        absoluteX += parent->GetX();
        absoluteY += parent->GetY();
        parent = parent->GetParent();
    }
    
    float contentY = absoluteY;
    float contentHeight = m_height;
    
    // Draw title bar if needed
    if (m_hasTitleBar) {
        renderer->DrawQuad(absoluteX, contentY, m_width, m_titleBarHeight,
                         m_titleBarColor[0], m_titleBarColor[1], m_titleBarColor[2], m_titleBarColor[3]);
        
        // Draw title text
        float textX = absoluteX + 5.0f;
        float textY = contentY + (m_titleBarHeight - 20.0f) / 2.0f;
        renderer->DrawText(m_title, textX, textY, 1.0f, 1.0f, 1.0f, 1.0f);
        
        // Adjust content position and size
        contentY += m_titleBarHeight;
        contentHeight -= m_titleBarHeight;
    }
    
    // Draw panel background
    if (m_texture) {
        renderer->DrawTexturedQuad(absoluteX, contentY, m_width, contentHeight, m_texture);
    } else {
        renderer->DrawQuad(absoluteX, contentY, m_width, contentHeight,
                         m_bgColor[0], m_bgColor[1], m_bgColor[2], m_bgColor[3]);
    }
    
    // Draw border if needed
    if (m_borderWidth > 0.0f) {
        // Top border
        renderer->DrawQuad(absoluteX, absoluteY, m_width, m_borderWidth,
                         m_borderColor[0], m_borderColor[1], m_borderColor[2], m_borderColor[3]);
        
        // Bottom border
        renderer->DrawQuad(absoluteX, absoluteY + m_height - m_borderWidth, m_width, m_borderWidth,
                         m_borderColor[0], m_borderColor[1], m_borderColor[2], m_borderColor[3]);
        
        // Left border
        renderer->DrawQuad(absoluteX, absoluteY, m_borderWidth, m_height,
                         m_borderColor[0], m_borderColor[1], m_borderColor[2], m_borderColor[3]);
        
        // Right border
        renderer->DrawQuad(absoluteX + m_width - m_borderWidth, absoluteY, m_borderWidth, m_height,
                         m_borderColor[0], m_borderColor[1], m_borderColor[2], m_borderColor[3]);
    }
    
    // Draw resize handle if resizable
    if (m_resizable) {
        float handleSize = 10.0f;
        renderer->DrawQuad(absoluteX + m_width - handleSize, absoluteY + m_height - handleSize,
                         handleSize, handleSize, 
                         m_borderColor[0], m_borderColor[1], m_borderColor[2], m_borderColor[3]);
    }
    
    // Render children
    for (auto& child : m_children) {
        if (child->IsVisible()) {
            child->Render(renderer);
        }
    }
}

void Panel::SetTexture(std::shared_ptr<Graphics::Texture> texture)
{
    m_texture = texture;
}

std::shared_ptr<Graphics::Texture> Panel::GetTexture() const
{
    return m_texture;
}

void Panel::SetDraggable(bool draggable)
{
    m_draggable = draggable;
}

bool Panel::IsDraggable() const
{
    return m_draggable;
}

void Panel::SetResizable(bool resizable)
{
    m_resizable = resizable;
}

bool Panel::IsResizable() const
{
    return m_resizable;
}

void Panel::SetTitleBar(bool hasTitleBar)
{
    m_hasTitleBar = hasTitleBar;
}

bool Panel::HasTitleBar() const
{
    return m_hasTitleBar;
}

void Panel::SetTitle(const std::string& title)
{
    m_title = title;
}

const std::string& Panel::GetTitle() const
{
    return m_title;
}

void Panel::SetTitleBarHeight(float height)
{
    m_titleBarHeight = height;
}

float Panel::GetTitleBarHeight() const
{
    return m_titleBarHeight;
}

void Panel::SetTitleBarColor(float r, float g, float b, float a)
{
    m_titleBarColor[0] = r;
    m_titleBarColor[1] = g;
    m_titleBarColor[2] = b;
    m_titleBarColor[3] = a;
}

void Panel::GetTitleBarColor(float& r, float& g, float& b, float& a) const
{
    r = m_titleBarColor[0];
    g = m_titleBarColor[1];
    b = m_titleBarColor[2];
    a = m_titleBarColor[3];
}

void Panel::OnMouseDown(int x, int y, Input::MouseButton button)
{
    if (button == Input::MouseButton::LEFT) {
        // Check if clicking in title bar
        if (m_draggable && IsPointInTitleBar(static_cast<float>(x), static_cast<float>(y))) {
            m_isDragging = true;
            m_dragStartX = x;
            m_dragStartY = y;
            m_dragStartPanelX = m_x;
            m_dragStartPanelY = m_y;
        }
        // Check if clicking in resize area
        else if (m_resizable && IsPointInResizeArea(static_cast<float>(x), static_cast<float>(y))) {
            m_isResizing = true;
            m_dragStartX = x;
            m_dragStartY = y;
            m_dragStartPanelWidth = m_width;
            m_dragStartPanelHeight = m_height;
        }
    }
    
    // Call base class implementation to trigger events
    UIElement::OnMouseDown(x, y, button);
}

void Panel::OnMouseUp(int x, int y, Input::MouseButton button)
{
    if (button == Input::MouseButton::LEFT) {
        m_isDragging = false;
        m_isResizing = false;
    }
    
    // Call base class implementation to trigger events
    UIElement::OnMouseUp(x, y, button);
}

void Panel::OnMouseMove(int x, int y)
{
    if (m_isDragging) {
        // Update panel position
        float deltaX = static_cast<float>(x - m_dragStartX);
        float deltaY = static_cast<float>(y - m_dragStartY);
        
        SetPosition(m_dragStartPanelX + deltaX, m_dragStartPanelY + deltaY);
    }
    else if (m_isResizing) {
        // Update panel size
        float deltaX = static_cast<float>(x - m_dragStartX);
        float deltaY = static_cast<float>(y - m_dragStartY);
        
        float newWidth = m_dragStartPanelWidth + deltaX;
        float newHeight = m_dragStartPanelHeight + deltaY;
        
        // Enforce minimum size
        if (newWidth < 50.0f) newWidth = 50.0f;
        if (newHeight < 50.0f) newHeight = 50.0f;
        
        SetSize(newWidth, newHeight);
    }
    
    // Call base class implementation to trigger events
    UIElement::OnMouseMove(x, y);
}

bool Panel::IsPointInTitleBar(float x, float y) const
{
    if (!m_hasTitleBar) {
        return false;
    }
    
    // Calculate absolute position
    float absoluteX = m_x;
    float absoluteY = m_y;
    UIElement* parent = m_parent;
    while (parent) {
        absoluteX += parent->GetX();
        absoluteY += parent->GetY();
        parent = parent->GetParent();
    }
    
    return (x >= absoluteX && x < absoluteX + m_width &&
            y >= absoluteY && y < absoluteY + m_titleBarHeight);
}

bool Panel::IsPointInResizeArea(float x, float y) const
{
    if (!m_resizable) {
        return false;
    }
    
    // Calculate absolute position
    float absoluteX = m_x;
    float absoluteY = m_y;
    UIElement* parent = m_parent;
    while (parent) {
        absoluteX += parent->GetX();
        absoluteY += parent->GetY();
        parent = parent->GetParent();
    }
    
    float handleSize = 10.0f;
    return (x >= absoluteX + m_width - handleSize && x < absoluteX + m_width &&
            y >= absoluteY + m_height - handleSize && y < absoluteY + m_height);
}

} // namespace UI
} // namespace CardGameLib
