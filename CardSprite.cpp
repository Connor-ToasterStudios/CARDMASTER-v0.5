#include "graphics/CardSprite.h"

namespace CardGameLib {
namespace Graphics {

CardSprite::CardSprite()
    : m_card(Core::Suit::HEARTS, Core::Rank::ACE) // Initialize with a default card
    , m_x(0.0f)
    , m_y(0.0f)
    , m_width(80.0f)
    , m_height(120.0f)
    , m_visible(true)
    , m_faceUp(false)
    , m_dragging(false)
    , m_flipping(false)
    , m_flipProgress(0.0f)
    , m_flipSpeed(0.5f)
{
}

CardSprite::CardSprite(const Core::Card& card, float x, float y, float width, float height)
    : m_card(card)
    , m_x(x)
    , m_y(y)
    , m_width(width)
    , m_height(height)
    , m_visible(true)
    , m_faceUp(card.IsFaceUp())
    , m_dragging(false)
    , m_flipping(false)
    , m_flipProgress(0.0f)
    , m_flipSpeed(0.5f)
{
}

void CardSprite::SetCard(const Core::Card& card)
{
    m_card = card;
    m_faceUp = card.IsFaceUp();
}

const Core::Card& CardSprite::GetCard() const
{
    return m_card;
}

void CardSprite::SetPosition(float x, float y)
{
    m_x = x;
    m_y = y;
}

void CardSprite::SetSize(float width, float height)
{
    m_width = width;
    m_height = height;
}

void CardSprite::SetTexture(std::shared_ptr<Texture> texture)
{
    m_frontTexture = texture;
}

std::shared_ptr<Texture> CardSprite::GetTexture() const
{
    if (m_faceUp || (m_flipping && m_flipProgress > 0.5f)) {
        return m_frontTexture;
    } else {
        return m_backTexture;
    }
}

void CardSprite::SetBackTexture(std::shared_ptr<Texture> texture)
{
    m_backTexture = texture;
}

void CardSprite::SetFlipping(bool flipping)
{
    m_flipping = flipping;
    if (flipping) {
        m_flipProgress = 0.0f;
    }
}

bool CardSprite::IsFlipping() const
{
    return m_flipping;
}

void CardSprite::UpdateFlipAnimation(float deltaTime)
{
    if (m_flipping) {
        m_flipProgress += deltaTime / m_flipSpeed;
        if (m_flipProgress >= 1.0f) {
            m_flipProgress = 1.0f;
            m_flipping = false;
            // Update the card's face-up state
            m_faceUp = !m_faceUp;
            m_card.SetFaceUp(m_faceUp);
        }
    }
}

float CardSprite::GetFlipProgress() const
{
    return m_flipProgress;
}

bool CardSprite::ContainsPoint(float x, float y) const
{
    return (x >= m_x && x <= m_x + m_width && y >= m_y && y <= m_y + m_height);
}

void CardSprite::SetDragging(bool dragging)
{
    m_dragging = dragging;
}

bool CardSprite::IsDragging() const
{
    return m_dragging;
}

void CardSprite::SetVisible(bool visible)
{
    m_visible = visible;
}

bool CardSprite::IsVisible() const
{
    return m_visible;
}

void CardSprite::SetFaceUp(bool faceUp)
{
    if (m_faceUp != faceUp) {
        m_faceUp = faceUp;
        m_card.SetFaceUp(faceUp);
    }
}

bool CardSprite::IsFaceUp() const
{
    return m_faceUp;
}

} // namespace Graphics
} // namespace CardGameLib
