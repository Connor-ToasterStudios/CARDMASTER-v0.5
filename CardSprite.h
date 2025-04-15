#pragma once

#include <memory>
#include <string>
#include "graphics/Texture.h"
#include "core/Card.h"

namespace CardGameLib {
namespace Graphics {

class CardSprite {
public:
    CardSprite();
    CardSprite(const Core::Card& card, float x, float y, float width, float height);
    ~CardSprite() = default;
    
    // Card state
    void SetCard(const Core::Card& card);
    const Core::Card& GetCard() const;
    
    // Position and size
    void SetPosition(float x, float y);
    void SetSize(float width, float height);
    
    float GetX() const { return m_x; }
    float GetY() const { return m_y; }
    float GetWidth() const { return m_width; }
    float GetHeight() const { return m_height; }
    
    // Texture handling
    void SetTexture(std::shared_ptr<Texture> texture);
    std::shared_ptr<Texture> GetTexture() const;
    void SetBackTexture(std::shared_ptr<Texture> texture);
    
    // Animation
    void SetFlipping(bool flipping);
    bool IsFlipping() const;
    void UpdateFlipAnimation(float deltaTime);
    float GetFlipProgress() const;
    
    // Interaction
    bool ContainsPoint(float x, float y) const;
    void SetDragging(bool dragging);
    bool IsDragging() const;
    
    // Rendering related
    void SetVisible(bool visible);
    bool IsVisible() const;
    
    // Card orientation
    void SetFaceUp(bool faceUp);
    bool IsFaceUp() const;
    
private:
    Core::Card m_card;
    float m_x;
    float m_y;
    float m_width;
    float m_height;
    
    std::shared_ptr<Texture> m_frontTexture;
    std::shared_ptr<Texture> m_backTexture;
    
    bool m_visible;
    bool m_faceUp;
    bool m_dragging;
    
    // Animation properties
    bool m_flipping;
    float m_flipProgress; // 0 to 1
    float m_flipSpeed;    // in seconds
};

} // namespace Graphics
} // namespace CardGameLib
