#pragma once

#include "ui/UI.h"
#include <string>
#include <memory>
#include "graphics/Texture.h"

namespace CardGameLib {
namespace UI {

class Button : public UIElement {
public:
    Button();
    Button(const std::string& text);
    virtual ~Button();
    
    // UIElement interface
    virtual UIElementType GetType() const override { return UIElementType::BUTTON; }
    virtual void Render(Graphics::Renderer* renderer) override;
    
    // Button-specific methods
    void SetText(const std::string& text);
    const std::string& GetText() const;
    
    void SetTexture(std::shared_ptr<Graphics::Texture> texture);
    std::shared_ptr<Graphics::Texture> GetTexture() const;
    
    void SetTextColor(float r, float g, float b, float a);
    void GetTextColor(float& r, float& g, float& b, float& a) const;
    
    void SetHoverBackgroundColor(float r, float g, float b, float a);
    void GetHoverBackgroundColor(float& r, float& g, float& b, float& a) const;
    
    void SetPressedBackgroundColor(float r, float g, float b, float a);
    void GetPressedBackgroundColor(float& r, float& g, float& b, float& a) const;
    
    void SetDisabledBackgroundColor(float r, float g, float b, float a);
    void GetDisabledBackgroundColor(float& r, float& g, float& b, float& a) const;
    
    void SetTextAlignment(int alignment); // 0: Left, 1: Center, 2: Right
    int GetTextAlignment() const;
    
protected:
    // Override mouse event handlers
    virtual void OnMouseDown(int x, int y, Input::MouseButton button) override;
    virtual void OnMouseUp(int x, int y, Input::MouseButton button) override;
    virtual void OnMouseMove(int x, int y) override;
    
private:
    std::string m_text;
    std::shared_ptr<Graphics::Texture> m_texture;
    float m_textColor[4];
    float m_hoverBgColor[4];
    float m_pressedBgColor[4];
    float m_disabledBgColor[4];
    int m_textAlignment;
    
    bool m_isPressed;
    bool m_isHovered;
};

} // namespace UI
} // namespace CardGameLib
