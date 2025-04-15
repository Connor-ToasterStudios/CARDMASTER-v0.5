#include "core/Deck.h"
#include <algorithm>
#include <stdexcept>
#include <chrono>

namespace CardGameLib {
namespace Core {

Deck::Deck()
    : m_rng(std::chrono::system_clock::now().time_since_epoch().count())
{
    InitializeStandardDeck(1);
}

Deck::Deck(int numberOfDecks)
    : m_rng(std::chrono::system_clock::now().time_since_epoch().count())
{
    InitializeStandardDeck(numberOfDecks);
}

Deck::Deck(const std::vector<Card>& cards)
    : m_cards(cards)
    , m_rng(std::chrono::system_clock::now().time_since_epoch().count())
{
}

Deck Deck::CreateEmpty()
{
    return Deck(std::vector<Card>());
}

void Deck::InitializeStandardDeck(int numberOfDecks)
{
    m_cards.clear();
    
    for (int deckIndex = 0; deckIndex < numberOfDecks; ++deckIndex) {
        for (int suit = 0; suit < 4; ++suit) {
            for (int rank = 1; rank <= 13; ++rank) {
                m_cards.emplace_back(static_cast<Suit>(suit), static_cast<Rank>(rank));
            }
        }
    }
}

void Deck::Shuffle()
{
    std::shuffle(m_cards.begin(), m_cards.end(), m_rng);
}

Card Deck::Draw()
{
    if (IsEmpty()) {
        throw std::runtime_error("Cannot draw from an empty deck");
    }
    
    Card drawnCard = m_cards.back();
    m_cards.pop_back();
    return drawnCard;
}

bool Deck::IsEmpty() const
{
    return m_cards.empty();
}

size_t Deck::Size() const
{
    return m_cards.size();
}

void Deck::AddCard(const Card& card)
{
    m_cards.push_back(card);
}

void Deck::AddCardToBottom(const Card& card)
{
    m_cards.insert(m_cards.begin(), card);
}

void Deck::Clear()
{
    m_cards.clear();
}

const Card& Deck::PeekTop() const
{
    if (IsEmpty()) {
        throw std::runtime_error("Cannot peek at an empty deck");
    }
    
    return m_cards.back();
}

const Card& Deck::PeekAt(size_t index) const
{
    if (index >= m_cards.size()) {
        throw std::out_of_range("Deck index out of range");
    }
    
    return m_cards[index];
}

const std::vector<Card>& Deck::GetCards() const
{
    return m_cards;
}

} // namespace Core
} // namespace CardGameLib
