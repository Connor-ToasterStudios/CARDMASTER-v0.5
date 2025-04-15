#include "ui/Button.h"

namespace CardGameLib {
namespace UI {

Button::Button()
    : m_text("Button")
    , m_textAlignment(1) // Center by default
    , m_isPressed(false)
    , m_isHovered(false)
{
    // Set default button style
    
    // Default background color (light gray)
    m_bgColor[0] = 0.8f;
    m_bgColor[1] = 0.8f;
    m_bgColor[2] = 0.8f;
    m_bgColor[3] = 1.0f;
    
    // Hover background color (slightly lighter)
    m_hoverBgColor[0] = 0.9f;
    m_hoverBgColor[1] = 0.9f;
    m_hoverBgColor[2] = 0.9f;
    m_hoverBgColor[3] = 1.0f;
    
    // Pressed background color (slightly darker)
    m_pressedBgColor[0] = 0.7f;
    m_pressedBgColor[1] = 0.7f;
    m_pressedBgColor[2] = 0.7f;
    m_pressedBgColor[3] = 1.0f;
    
    // Disabled background color (darker gray)
    m_disabledBgColor[0] = 0.6f;
    m_disabledBgColor[1] = 0.6f;
    m_disabledBgColor[2] = 0.6f;
    m_disabledBgColor[3] = 1.0f;
    
    // Text color (black)
    m_textColor[0] = 0.0f;
    m_textColor[1] = 0.0f;
    m_textColor[2] = 0.0f;
    m_textColor[3] = 1.0f;
    
    // Set border
    m_borderWidth = 1.0f;
}

Button::Button(const std::string& text)
    : Button()
{
    m_text = text;
}

Button::~Button()
{
    // Nothing special to clean up
}

void Button::Render(Graphics::Renderer* renderer)
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
    
    // Choose the appropriate background color based on button state
    float* bgColor = m_bgColor;
    
    if (!m_enabled) {
        bgColor = m_disabledBgColor;
    }
    else if (m_isPressed) {
        bgColor = m_pressedBgColor;
    }
    else if (m_isHovered) {
        bgColor = m_hoverBgColor;
    }
    
    // Draw button background
    renderer->DrawQuad(absoluteX, absoluteY, m_width, m_height, 
                     bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
    
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
    
    // Draw texture if available
    if (m_texture) {
        renderer->DrawTexturedQuad(absoluteX, absoluteY, m_width, m_height, m_texture);
    }
    
    // Draw text
    if (!m_text.empty()) {
        float textX = absoluteX;
        
        // Text alignment
        if (m_textAlignment == 1) { // Center
            textX += m_width / 2.0f - (m_text.length() * 8.0f) / 2.0f; // Rough estimate of text width
        }
        else if (m_textAlignment == 2) { // Right
            textX += m_width - (m_text.length() * 8.0f) - 5.0f; // Rough estimate with padding
        }
        else { // Left
            textX += 5.0f; // Padding
        }
        
        float textY = absoluteY + (m_height - 20.0f) / 2.0f; // Center vertically
        
        renderer->DrawText(m_text, textX, textY, 1.0f, 
                         m_textColor[0], m_textColor[1], m_textColor[2]);
    }
    
    // Render children
    for (auto& child : m_children) {
        if (child->IsVisible()) {
            child->Render(renderer);
        }
    }
}

void Button::SetText(const std::string& text)
{
    m_text = text;
}

const std::string& Button::GetText() const
{
    return m_text;
}

void Button::SetTexture(std::shared_ptr<Graphics::Texture> texture)
{
    m_texture = texture;
}

std::shared_ptr<Graphics::Texture> Button::GetTexture() const
{
    return m_texture;
}

void Button::SetTextColor(float r, float g, float b, float a)
{
    m_textColor[0] = r;
    m_textColor[1] = g;
    m_textColor[2] = b;
    m_textColor[3] = a;
}

void Button::GetTextColor(float& r, float& g, float& b, float& a) const
{
    r = m_textColor[0];
    g = m_textColor[1];
    b = m_textColor[2];
    a = m_textColor[3];
}

void Button::SetHoverBackgroundColor(float r, float g, float b, float a)
{
    m_hoverBgColor[0] = r;
    m_hoverBgColor[1] = g;
    m_hoverBgColor[2] = b;
    m_hoverBgColor[3] = a;
}

void Button::GetHoverBackgroundColor(float& r, float& g, float& b, float& a) const
{
    r = m_hoverBgColor[0];
    g = m_hoverBgColor[1];
    b = m_hoverBgColor[2];
    a = m_hoverBgColor[3];
}

void Button::SetPressedBackgroundColor(float r, float g, float b, float a)
{
    m_pressedBgColor[0] = r;
    m_pressedBgColor[1] = g;
    m_pressedBgColor[2] = b;
    m_pressedBgColor[3] = a;
}

void Button::GetPressedBackgroundColor(float& r, float& g, float& b, float& a) const
{
    r = m_pressedBgColor[0];
    g = m_pressedBgColor[1];
    b = m_pressedBgColor[2];
    a = m_pressedBgColor[3];
}

void Button::SetDisabledBackgroundColor(float r, float g, float b, float a)
{
    m_disabledBgColor[0] = r;
    m_disabledBgColor[1] = g;
    m_disabledBgColor[2] = b;
    m_disabledBgColor[3] = a;
}

void Button::GetDisabledBackgroundColor(float& r, float& g, float& b, float& a) const
{
    r = m_disabledBgColor[0];
    g = m_disabledBgColor[1];
    b = m_disabledBgColor[2];
    a = m_disabledBgColor[3];
}

void Button::SetTextAlignment(int alignment)
{
    m_textAlignment = alignment;
}

int Button::GetTextAlignment() const
{
    return m_textAlignment;
}

void Button::OnMouseDown(int x, int y, Input::MouseButton button)
{
    if (button == Input::MouseButton::LEFT) {
        m_isPressed = true;
    }
    
    // Call base class implementation to trigger events
    UIElement::OnMouseDown(x, y, button);
}

void Button::OnMouseUp(int x, int y, Input::MouseButton button)
{
    if (button == Input::MouseButton::LEFT && m_isPressed) {
        m_isPressed = false;
        
        // Check if mouse is still inside the button
        if (IsPointInside(static_cast<float>(x), static_cast<float>(y))) {
            // Trigger click event
            UIEvent event;
            event.type = UIEventType::CLICK;
            event.element = this;
            event.x = x;
            event.y = y;
            
            TriggerEvent(event);
        }
    }
}

void Button::OnMouseMove(int x, int y)
{
    bool wasHovered = m_isHovered;
    m_isHovered = IsPointInside(static_cast<float>(x), static_cast<float>(y));
    
    // Only trigger hover event if the hover state changed
    if (m_isHovered != wasHovered) {
        UIEvent event;
        event.type = UIEventType::HOVER;
        event.element = this;
        event.x = x;
        event.y = y;
        
        TriggerEvent(event);
    }
}

} // namespace UI
} // namespace CardGameLib
