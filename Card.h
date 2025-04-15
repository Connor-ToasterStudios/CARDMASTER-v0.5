#pragma once

#include <string>

namespace CardGameLib {
namespace Core {

enum class Suit {
    HEARTS,
    DIAMONDS,
    CLUBS,
    SPADES
};

enum class Rank {
    ACE = 1,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    TEN,
    JACK,
    QUEEN,
    KING
};

enum class Color {
    RED,
    BLACK
};

class Card {
public:
    Card(Suit suit, Rank rank);
    
    // Copy constructor and assignment operator
    Card(const Card& other);
    Card& operator=(const Card& other);
    
    // Card properties
    Suit GetSuit() const;
    Rank GetRank() const;
    Color GetColor() const;
    bool IsFaceUp() const;
    
    // Card actions
    void Flip();
    void SetFaceUp(bool faceUp);
    
    // Utility functions
    bool IsRed() const;
    bool IsBlack() const;
    std::string ToString() const;
    
    // Comparison operators
    bool operator==(const Card& other) const;
    bool operator!=(const Card& other) const;
    
private:
    Suit m_suit;
    Rank m_rank;
    bool m_faceUp;
};

} // namespace Core
} // namespace CardGameLib
