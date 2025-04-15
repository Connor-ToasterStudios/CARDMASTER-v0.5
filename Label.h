#pragma once

#include "ui/UI.h"
#include <string>

namespace CardGameLib {
namespace UI {

class Label : public UIElement {
public:
    Label();
    Label(const std::string& text);
    virtual ~Label();
    
    // UIElement interface
    virtual UIElementType GetType() const override { return UIElementType::LABEL; }
    virtual void Render(Graphics::Renderer* renderer) override;
    
    // Label-specific methods
    void SetText(const std::string& text);
    const std::string& GetText() const;
    
    void SetTextColor(float r, float g, float b, float a);
    void GetTextColor(float& r, float& g, float& b, float& a) const;
    
    void SetTextScale(float scale);
    float GetTextScale() const;
    
    void SetTextAlignment(int alignment); // 0: Left, 1: Center, 2: Right
    int GetTextAlignment() const;
    
    void SetWordWrap(bool wordWrap);
    bool GetWordWrap() const;
    
private:
    std::string m_text;
    float m_textColor[4];
    float m_textScale;
    int m_textAlignment;
    bool m_wordWrap;
    
    // Helper function to split text for word wrapping
    std::vector<std::string> WrapText(const std::string& text, float maxWidth, float scale) const;
};

} // namespace UI
} // namespace CardGameLib
