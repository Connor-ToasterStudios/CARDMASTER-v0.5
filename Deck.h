#pragma once

#include <vector>
#include <memory>
#include <random>
#include "core/Card.h"

namespace CardGameLib {
namespace Core {

class Deck {
public:
    // Create a standard 52-card deck
    Deck();
    
    // Create a deck with specified number of standard 52-card decks
    explicit Deck(int numberOfDecks);
    
    // Create a custom deck from a list of cards
    explicit Deck(const std::vector<Card>& cards);
    
    // Create an empty deck
    static Deck CreateEmpty();
    
    // Deck operations
    void Shuffle();
    Card Draw();
    bool IsEmpty() const;
    size_t Size() const;
    void AddCard(const Card& card);
    void AddCardToBottom(const Card& card);
    void Clear();
    
    // Access cards
    const Card& PeekTop() const;
    const Card& PeekAt(size_t index) const;
    const std::vector<Card>& GetCards() const;
    
private:
    std::vector<Card> m_cards;
    std::mt19937 m_rng;
    
    void InitializeStandardDeck(int numberOfDecks);
};

} // namespace Core
} // namespace CardGameLib
