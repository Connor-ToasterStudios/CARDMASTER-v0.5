#include "core/Card.h"

namespace CardGameLib {
namespace Core {

Card::Card(Suit suit, Rank rank)
    : m_suit(suit)
    , m_rank(rank)
    , m_faceUp(false)
{
}

Card::Card(const Card& other)
    : m_suit(other.m_suit)
    , m_rank(other.m_rank)
    , m_faceUp(other.m_faceUp)
{
}

Card& Card::operator=(const Card& other)
{
    if (this != &other) {
        m_suit = other.m_suit;
        m_rank = other.m_rank;
        m_faceUp = other.m_faceUp;
    }
    return *this;
}

Suit Card::GetSuit() const
{
    return m_suit;
}

Rank Card::GetRank() const
{
    return m_rank;
}

Color Card::GetColor() const
{
    return (m_suit == Suit::HEARTS || m_suit == Suit::DIAMONDS) ? Color::RED : Color::BLACK;
}

bool Card::IsFaceUp() const
{
    return m_faceUp;
}

void Card::Flip()
{
    m_faceUp = !m_faceUp;
}

void Card::SetFaceUp(bool faceUp)
{
    m_faceUp = faceUp;
}

bool Card::IsRed() const
{
    return GetColor() == Color::RED;
}

bool Card::IsBlack() const
{
    return GetColor() == Color::BLACK;
}

std::string Card::ToString() const
{
    const char* rankStr;
    switch (m_rank) {
        case Rank::ACE:   rankStr = "A"; break;
        case Rank::TWO:   rankStr = "2"; break;
        case Rank::THREE: rankStr = "3"; break;
        case Rank::FOUR:  rankStr = "4"; break;
        case Rank::FIVE:  rankStr = "5"; break;
        case Rank::SIX:   rankStr = "6"; break;
        case Rank::SEVEN: rankStr = "7"; break;
        case Rank::EIGHT: rankStr = "8"; break;
        case Rank::NINE:  rankStr = "9"; break;
        case Rank::TEN:   rankStr = "10"; break;
        case Rank::JACK:  rankStr = "J"; break;
        case Rank::QUEEN: rankStr = "Q"; break;
        case Rank::KING:  rankStr = "K"; break;
        default:          rankStr = "?";
    }
    
    const char* suitStr;
    switch (m_suit) {
        case Suit::HEARTS:   suitStr = "♥"; break;
        case Suit::DIAMONDS: suitStr = "♦"; break;
        case Suit::CLUBS:    suitStr = "♣"; break;
        case Suit::SPADES:   suitStr = "♠"; break;
        default:             suitStr = "?";
    }
    
    return std::string(rankStr) + suitStr;
}

bool Card::operator==(const Card& other) const
{
    return m_suit == other.m_suit && m_rank == other.m_rank;
}

bool Card::operator!=(const Card& other) const
{
    return !(*this == other);
}

} // namespace Core
} // namespace CardGameLib
