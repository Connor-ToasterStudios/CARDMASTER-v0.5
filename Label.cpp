#include "ui/Label.h"
#include <sstream>

namespace CardGameLib {
namespace UI {

Label::Label()
    : m_text("Label")
    , m_textScale(1.0f)
    , m_textAlignment(0) // Left by default
    , m_wordWrap(false)
{
    // Set default label style
    
    // Make background transparent
    m_bgColor[0] = 1.0f;
    m_bgColor[1] = 1.0f;
    m_bgColor[2] = 1.0f;
    m_bgColor[3] = 0.0f;
    
    // Text color (white)
    m_textColor[0] = 1.0f;
    m_textColor[1] = 1.0f;
    m_textColor[2] = 1.0f;
    m_textColor[3] = 1.0f;
    
    // No border
    m_borderWidth = 0.0f;
    
    // Default size
    m_width = 100.0f;
    m_height = 20.0f;
}

Label::Label(const std::string& text)
    : Label()
{
    m_text = text;
}

Label::~Label()
{
    // Nothing special to clean up
}

void Label::Render(Graphics::Renderer* renderer)
{
    if (!renderer || !m_visible) {
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
    
    // Draw background if not transparent
    if (m_bgColor[3] > 0.0f) {
        renderer->DrawQuad(absoluteX, absoluteY, m_width, m_height,
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
    
    // Draw text
    if (!m_text.empty()) {
        // Estimate character width (very rough approximation)
        float charWidth = 8.0f * m_textScale;
        
        if (m_wordWrap) {
            // Split text into lines based on word wrapping
            std::vector<std::string> lines = WrapText(m_text, m_width, m_textScale);
            
            float lineHeight = 20.0f * m_textScale;
            float textY = absoluteY;
            
            for (const auto& line : lines) {
                float textX = absoluteX;
                
                // Text alignment
                if (m_textAlignment == 1) { // Center
                    textX += m_width / 2.0f - (line.length() * charWidth) / 2.0f;
                }
                else if (m_textAlignment == 2) { // Right
                    textX += m_width - (line.length() * charWidth) - 5.0f;
                }
                else { // Left
                    textX += 5.0f; // Padding
                }
                
                renderer->DrawText(line, textX, textY, m_textScale,
                                 m_textColor[0], m_textColor[1], m_textColor[2]);
                
                textY += lineHeight;
            }
        }
        else {
            // Single line text
            float textX = absoluteX;
            
            // Text alignment
            if (m_textAlignment == 1) { // Center
                textX += m_width / 2.0f - (m_text.length() * charWidth) / 2.0f;
            }
            else if (m_textAlignment == 2) { // Right
                textX += m_width - (m_text.length() * charWidth) - 5.0f;
            }
            else { // Left
                textX += 5.0f; // Padding
            }
            
            float textY = absoluteY + (m_height - 20.0f * m_textScale) / 2.0f; // Center vertically
            
            renderer->DrawText(m_text, textX, textY, m_textScale,
                             m_textColor[0], m_textColor[1], m_textColor[2]);
        }
    }
    
    // Render children
    for (auto& child : m_children) {
        if (child->IsVisible()) {
            child->Render(renderer);
        }
    }
}

void Label::SetText(const std::string& text)
{
    m_text = text;
}

const std::string& Label::GetText() const
{
    return m_text;
}

void Label::SetTextColor(float r, float g, float b, float a)
{
    m_textColor[0] = r;
    m_textColor[1] = g;
    m_textColor[2] = b;
    m_textColor[3] = a;
}

void Label::GetTextColor(float& r, float& g, float& b, float& a) const
{
    r = m_textColor[0];
    g = m_textColor[1];
    b = m_textColor[2];
    a = m_textColor[3];
}

void Label::SetTextScale(float scale)
{
    m_textScale = scale;
}

float Label::GetTextScale() const
{
    return m_textScale;
}

void Label::SetTextAlignment(int alignment)
{
    m_textAlignment = alignment;
}

int Label::GetTextAlignment() const
{
    return m_textAlignment;
}

void Label::SetWordWrap(bool wordWrap)
{
    m_wordWrap = wordWrap;
}

bool Label::GetWordWrap() const
{
    return m_wordWrap;
}

std::vector<std::string> Label::WrapText(const std::string& text, float maxWidth, float scale) const
{
    std::vector<std::string> lines;
    
    // If text is empty, return empty vector
    if (text.empty()) {
        return lines;
    }
    
    // Estimate character width (very rough approximation)
    float charWidth = 8.0f * scale;
    
    // Maximum characters per line
    int maxCharsPerLine = static_cast<int>((maxWidth - 10.0f) / charWidth); // 10.0f for padding
    
    if (maxCharsPerLine <= 0) {
        maxCharsPerLine = 1; // Ensure at least 1 character per line
    }
    
    // Split text into words
    std::istringstream iss(text);
    std::vector<std::string> words;
    std::string word;
    while (iss >> word) {
        words.push_back(word);
    }
    
    // Build lines
    std::string currentLine;
    for (const auto& word : words) {
        // If adding this word would exceed the line width
        if (!currentLine.empty() && 
            (currentLine.length() + 1 + word.length()) > static_cast<size_t>(maxCharsPerLine)) {
            // Add current line to lines
            lines.push_back(currentLine);
            currentLine = word;
        }
        else {
            // Add word to current line
            if (!currentLine.empty()) {
                currentLine += " ";
            }
            currentLine += word;
        }
    }
    
    // Add the last line
    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }
    
    return lines;
}

} // namespace UI
} // namespace CardGameLib
