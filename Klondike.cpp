#include "games/solitaire/Klondike.h"
#include <sstream>
#include <stdexcept>

namespace CardGameLib {
namespace Games {
namespace Solitaire {

Klondike::Klondike()
    : Core::Game("Klondike", Core::GameType::SOLITAIRE_KLONDIKE, 1)
{
}

void Klondike::Initialize()
{
    Reset();
}

bool Klondike::Start()
{
    if (!CanStart()) {
        return false;
    }
    
    SetState(Core::GameState::STARTING);
    DealInitialLayout();
    SetState(Core::GameState::IN_PROGRESS);
    return true;
}

bool Klondike::CanStart() const
{
    // Klondike is a single-player game, so we only need one player
    return m_players.size() == 1;
}

void Klondike::Reset()
{
    // Create a new shuffled deck
    m_stock = Core::Deck();
    m_stock.Shuffle();
    
    // Clear all other piles
    m_waste.clear();
    
    for (auto& foundation : m_foundations) {
        foundation.clear();
    }
    
    for (auto& pile : m_tableau) {
        pile.clear();
    }
    
    SetState(Core::GameState::WAITING_FOR_PLAYERS);
}

bool Klondike::IsValidMove(const std::string& moveData)
{
    std::istringstream ss(moveData);
    std::string moveTypeStr;
    ss >> moveTypeStr;
    
    try {
        KlondikeMoveType moveType = static_cast<KlondikeMoveType>(std::stoi(moveTypeStr));
        
        switch (moveType) {
            case KlondikeMoveType::DRAW_FROM_STOCK:
                return !m_stock.IsEmpty();
                
            case KlondikeMoveType::WASTE_TO_TABLEAU: {
                int tableauIndex;
                ss >> tableauIndex;
                
                if (tableauIndex < 0 || tableauIndex >= 7 || m_waste.empty()) {
                    return false;
                }
                
                const Core::Card& card = m_waste.back();
                return IsValidTableauToTableauMove(card, m_tableau[tableauIndex]);
            }
                
            case KlondikeMoveType::WASTE_TO_FOUNDATION: {
                int foundationIndex;
                ss >> foundationIndex;
                
                if (foundationIndex < 0 || foundationIndex >= 4 || m_waste.empty()) {
                    return false;
                }
                
                const Core::Card& card = m_waste.back();
                return IsValidCardForFoundation(card, m_foundations[foundationIndex]);
            }
                
            case KlondikeMoveType::TABLEAU_TO_FOUNDATION: {
                int tableauIndex, foundationIndex;
                ss >> tableauIndex >> foundationIndex;
                
                if (tableauIndex < 0 || tableauIndex >= 7 || 
                    foundationIndex < 0 || foundationIndex >= 4 ||
                    m_tableau[tableauIndex].empty()) {
                    return false;
                }
                
                const Core::Card& card = m_tableau[tableauIndex].back();
                return card.IsFaceUp() && IsValidCardForFoundation(card, m_foundations[foundationIndex]);
            }
                
            case KlondikeMoveType::TABLEAU_TO_TABLEAU: {
                int sourceIndex, targetIndex, cardCount;
                ss >> sourceIndex >> targetIndex >> cardCount;
                
                if (sourceIndex < 0 || sourceIndex >= 7 || 
                    targetIndex < 0 || targetIndex >= 7 || 
                    sourceIndex == targetIndex ||
                    m_tableau[sourceIndex].empty() ||
                    cardCount <= 0 || cardCount > static_cast<int>(m_tableau[sourceIndex].size())) {
                    return false;
                }
                
                // Find the first face-up card in the source pile
                size_t firstFaceUpIndex = 0;
                for (size_t i = 0; i < m_tableau[sourceIndex].size(); ++i) {
                    if (m_tableau[sourceIndex][i].IsFaceUp()) {
                        firstFaceUpIndex = i;
                        break;
                    }
                }
                
                // Check if we're trying to move face-down cards
                if (m_tableau[sourceIndex].size() - cardCount < firstFaceUpIndex) {
                    return false;
                }
                
                // Get the card we're trying to move
                size_t cardIndex = m_tableau[sourceIndex].size() - cardCount;
                const Core::Card& card = m_tableau[sourceIndex][cardIndex];
                
                return IsValidTableauToTableauMove(card, m_tableau[targetIndex]);
            }
                
            case KlondikeMoveType::FOUNDATION_TO_TABLEAU: {
                int foundationIndex, tableauIndex;
                ss >> foundationIndex >> tableauIndex;
                
                if (foundationIndex < 0 || foundationIndex >= 4 || 
                    tableauIndex < 0 || tableauIndex >= 7 ||
                    m_foundations[foundationIndex].empty()) {
                    return false;
                }
                
                const Core::Card& card = m_foundations[foundationIndex].back();
                return IsValidTableauToTableauMove(card, m_tableau[tableauIndex]);
            }
                
            case KlondikeMoveType::RECYCLE_WASTE:
                return m_stock.IsEmpty() && !m_waste.empty();
                
            default:
                return false;
        }
    } catch (const std::exception&) {
        return false;
    }
    
    return false;
}

bool Klondike::MakeMove(int playerId, const std::string& moveData)
{
    // Only the current player can make moves
    if (m_players.empty() || m_players[0]->GetId() != playerId) {
        return false;
    }
    
    if (!IsValidMove(moveData)) {
        return false;
    }
    
    std::istringstream ss(moveData);
    std::string moveTypeStr;
    ss >> moveTypeStr;
    
    try {
        KlondikeMoveType moveType = static_cast<KlondikeMoveType>(std::stoi(moveTypeStr));
        
        switch (moveType) {
            case KlondikeMoveType::DRAW_FROM_STOCK:
                return DrawFromStock();
                
            case KlondikeMoveType::WASTE_TO_TABLEAU: {
                int tableauIndex;
                ss >> tableauIndex;
                return MoveWasteToTableau(tableauIndex);
            }
                
            case KlondikeMoveType::WASTE_TO_FOUNDATION: {
                int foundationIndex;
                ss >> foundationIndex;
                return MoveWasteToFoundation(foundationIndex);
            }
                
            case KlondikeMoveType::TABLEAU_TO_FOUNDATION: {
                int tableauIndex, foundationIndex;
                ss >> tableauIndex >> foundationIndex;
                return MoveTableauToFoundation(tableauIndex, foundationIndex);
            }
                
            case KlondikeMoveType::TABLEAU_TO_TABLEAU: {
                int sourceIndex, targetIndex, cardCount;
                ss >> sourceIndex >> targetIndex >> cardCount;
                return MoveTableauToTableau(sourceIndex, targetIndex, cardCount);
            }
                
            case KlondikeMoveType::FOUNDATION_TO_TABLEAU: {
                int foundationIndex, tableauIndex;
                ss >> foundationIndex >> tableauIndex;
                return MoveFoundationToTableau(foundationIndex, tableauIndex);
            }
                
            case KlondikeMoveType::RECYCLE_WASTE:
                return RecycleWaste();
                
            default:
                return false;
        }
    } catch (const std::exception&) {
        return false;
    }
    
    return false;
}

std::string Klondike::SerializeGameState() const
{
    std::stringstream ss;
    
    // Serialize stock
    ss << "STOCK " << m_stock.Size() << " ";
    for (size_t i = 0; i < m_stock.Size(); ++i) {
        const Core::Card& card = m_stock.PeekAt(i);
        ss << static_cast<int>(card.GetSuit()) << " " 
           << static_cast<int>(card.GetRank()) << " "
           << (card.IsFaceUp() ? 1 : 0) << " ";
    }
    
    // Serialize waste
    ss << "WASTE " << m_waste.size() << " ";
    for (const auto& card : m_waste) {
        ss << static_cast<int>(card.GetSuit()) << " " 
           << static_cast<int>(card.GetRank()) << " "
           << (card.IsFaceUp() ? 1 : 0) << " ";
    }
    
    // Serialize foundations
    ss << "FOUNDATIONS ";
    for (const auto& foundation : m_foundations) {
        ss << foundation.size() << " ";
        for (const auto& card : foundation) {
            ss << static_cast<int>(card.GetSuit()) << " " 
               << static_cast<int>(card.GetRank()) << " "
               << (card.IsFaceUp() ? 1 : 0) << " ";
        }
    }
    
    // Serialize tableau
    ss << "TABLEAU ";
    for (const auto& pile : m_tableau) {
        ss << pile.size() << " ";
        for (const auto& card : pile) {
            ss << static_cast<int>(card.GetSuit()) << " " 
               << static_cast<int>(card.GetRank()) << " "
               << (card.IsFaceUp() ? 1 : 0) << " ";
        }
    }
    
    return ss.str();
}

bool Klondike::DeserializeGameState(const std::string& data)
{
    std::istringstream ss(data);
    std::string token;
    
    // Clear current state
    Reset();
    
    // Deserialize stock
    ss >> token;
    if (token != "STOCK") return false;
    
    size_t stockSize;
    ss >> stockSize;
    
    m_stock = Core::Deck::CreateEmpty();
    for (size_t i = 0; i < stockSize; ++i) {
        int suit, rank, faceUp;
        ss >> suit >> rank >> faceUp;
        
        Core::Card card(static_cast<Core::Suit>(suit), static_cast<Core::Rank>(rank));
        if (faceUp) card.SetFaceUp(true);
        m_stock.AddCard(card);
    }
    
    // Deserialize waste
    ss >> token;
    if (token != "WASTE") return false;
    
    size_t wasteSize;
    ss >> wasteSize;
    
    for (size_t i = 0; i < wasteSize; ++i) {
        int suit, rank, faceUp;
        ss >> suit >> rank >> faceUp;
        
        Core::Card card(static_cast<Core::Suit>(suit), static_cast<Core::Rank>(rank));
        if (faceUp) card.SetFaceUp(true);
        m_waste.push_back(card);
    }
    
    // Deserialize foundations
    ss >> token;
    if (token != "FOUNDATIONS") return false;
    
    for (size_t i = 0; i < m_foundations.size(); ++i) {
        size_t foundationSize;
        ss >> foundationSize;
        
        for (size_t j = 0; j < foundationSize; ++j) {
            int suit, rank, faceUp;
            ss >> suit >> rank >> faceUp;
            
            Core::Card card(static_cast<Core::Suit>(suit), static_cast<Core::Rank>(rank));
            if (faceUp) card.SetFaceUp(true);
            m_foundations[i].push_back(card);
        }
    }
    
    // Deserialize tableau
    ss >> token;
    if (token != "TABLEAU") return false;
    
    for (size_t i = 0; i < m_tableau.size(); ++i) {
        size_t pileSize;
        ss >> pileSize;
        
        for (size_t j = 0; j < pileSize; ++j) {
            int suit, rank, faceUp;
            ss >> suit >> rank >> faceUp;
            
            Core::Card card(static_cast<Core::Suit>(suit), static_cast<Core::Rank>(rank));
            if (faceUp) card.SetFaceUp(true);
            m_tableau[i].push_back(card);
        }
    }
    
    // Set game state
    SetState(Core::GameState::IN_PROGRESS);
    
    // Check for win condition
    if (IsGameWon()) {
        SetState(Core::GameState::GAME_OVER);
    }
    
    return true;
}

bool Klondike::DrawFromStock()
{
    if (m_stock.IsEmpty()) {
        return false;
    }
    
    Core::Card card = m_stock.Draw();
    card.SetFaceUp(true);
    m_waste.push_back(card);
    
    return true;
}

bool Klondike::MoveWasteToTableau(int tableauIndex)
{
    if (tableauIndex < 0 || tableauIndex >= 7 || m_waste.empty()) {
        return false;
    }
    
    const Core::Card& card = m_waste.back();
    if (!IsValidTableauToTableauMove(card, m_tableau[tableauIndex])) {
        return false;
    }
    
    m_tableau[tableauIndex].push_back(m_waste.back());
    m_waste.pop_back();
    
    return true;
}

bool Klondike::MoveWasteToFoundation(int foundationIndex)
{
    if (foundationIndex < 0 || foundationIndex >= 4 || m_waste.empty()) {
        return false;
    }
    
    const Core::Card& card = m_waste.back();
    if (!IsValidCardForFoundation(card, m_foundations[foundationIndex])) {
        return false;
    }
    
    m_foundations[foundationIndex].push_back(m_waste.back());
    m_waste.pop_back();
    
    // Check for win condition
    if (IsGameWon()) {
        SetState(Core::GameState::GAME_OVER);
    }
    
    return true;
}

bool Klondike::MoveTableauToFoundation(int tableauIndex, int foundationIndex)
{
    if (tableauIndex < 0 || tableauIndex >= 7 || 
        foundationIndex < 0 || foundationIndex >= 4 ||
        m_tableau[tableauIndex].empty()) {
        return false;
    }
    
    const Core::Card& card = m_tableau[tableauIndex].back();
    if (!card.IsFaceUp() || !IsValidCardForFoundation(card, m_foundations[foundationIndex])) {
        return false;
    }
    
    m_foundations[foundationIndex].push_back(m_tableau[tableauIndex].back());
    m_tableau[tableauIndex].pop_back();
    
    // Flip the now-exposed card if any
    if (!m_tableau[tableauIndex].empty() && !m_tableau[tableauIndex].back().IsFaceUp()) {
        m_tableau[tableauIndex].back().SetFaceUp(true);
    }
    
    // Check for win condition
    if (IsGameWon()) {
        SetState(Core::GameState::GAME_OVER);
    }
    
    return true;
}

bool Klondike::MoveTableauToTableau(int sourceIndex, int targetIndex, int cardCount)
{
    if (sourceIndex < 0 || sourceIndex >= 7 || 
        targetIndex < 0 || targetIndex >= 7 || 
        sourceIndex == targetIndex ||
        m_tableau[sourceIndex].empty() ||
        cardCount <= 0 || cardCount > static_cast<int>(m_tableau[sourceIndex].size())) {
        return false;
    }
    
    // Find the first face-up card in the source pile
    size_t firstFaceUpIndex = 0;
    for (size_t i = 0; i < m_tableau[sourceIndex].size(); ++i) {
        if (m_tableau[sourceIndex][i].IsFaceUp()) {
            firstFaceUpIndex = i;
            break;
        }
    }
    
    // Check if we're trying to move face-down cards
    if (m_tableau[sourceIndex].size() - cardCount < firstFaceUpIndex) {
        return false;
    }
    
    // Get the card we're trying to move
    size_t cardIndex = m_tableau[sourceIndex].size() - cardCount;
    const Core::Card& card = m_tableau[sourceIndex][cardIndex];
    
    if (!IsValidTableauToTableauMove(card, m_tableau[targetIndex])) {
        return false;
    }
    
    // Move the cards
    std::vector<Core::Card> cardsToMove;
    cardsToMove.insert(cardsToMove.begin(), 
                      m_tableau[sourceIndex].begin() + cardIndex,
                      m_tableau[sourceIndex].end());
    
    m_tableau[targetIndex].insert(m_tableau[targetIndex].end(),
                                 cardsToMove.begin(), cardsToMove.end());
    
    m_tableau[sourceIndex].erase(m_tableau[sourceIndex].begin() + cardIndex,
                               m_tableau[sourceIndex].end());
    
    // Flip the now-exposed card if any
    if (!m_tableau[sourceIndex].empty() && !m_tableau[sourceIndex].back().IsFaceUp()) {
        m_tableau[sourceIndex].back().SetFaceUp(true);
    }
    
    return true;
}

bool Klondike::MoveFoundationToTableau(int foundationIndex, int tableauIndex)
{
    if (foundationIndex < 0 || foundationIndex >= 4 || 
        tableauIndex < 0 || tableauIndex >= 7 ||
        m_foundations[foundationIndex].empty()) {
        return false;
    }
    
    const Core::Card& card = m_foundations[foundationIndex].back();
    if (!IsValidTableauToTableauMove(card, m_tableau[tableauIndex])) {
        return false;
    }
    
    m_tableau[tableauIndex].push_back(m_foundations[foundationIndex].back());
    m_foundations[foundationIndex].pop_back();
    
    return true;
}

bool Klondike::RecycleWaste()
{
    if (!m_stock.IsEmpty() || m_waste.empty()) {
        return false;
    }
    
    // Move all waste cards back to stock in reverse order (maintaining their order)
    for (auto it = m_waste.rbegin(); it != m_waste.rend(); ++it) {
        Core::Card card = *it;
        card.SetFaceUp(false);
        m_stock.AddCard(card);
    }
    
    m_waste.clear();
    
    return true;
}

bool Klondike::IsGameWon() const
{
    // Game is won when all foundations have 13 cards each (A through K of each suit)
    for (const auto& foundation : m_foundations) {
        if (foundation.size() != 13) {
            return false;
        }
    }
    
    return true;
}

const Core::Deck& Klondike::GetStock() const
{
    return m_stock;
}

const std::vector<Core::Card>& Klondike::GetWaste() const
{
    return m_waste;
}

const std::array<std::vector<Core::Card>, 4>& Klondike::GetFoundations() const
{
    return m_foundations;
}

const std::array<std::vector<Core::Card>, 7>& Klondike::GetTableau() const
{
    return m_tableau;
}

bool Klondike::IsValidTableauToTableauMove(const Core::Card& card, const std::vector<Core::Card>& targetPile) const
{
    if (targetPile.empty()) {
        // Only Kings can be placed on empty tableau piles
        return card.GetRank() == Core::Rank::KING;
    } else {
        const Core::Card& targetCard = targetPile.back();
        
        // Cards must be face up, of opposite color, and 1 rank lower
        return targetCard.IsFaceUp() &&
               card.GetColor() != targetCard.GetColor() &&
               static_cast<int>(card.GetRank()) == static_cast<int>(targetCard.GetRank()) - 1;
    }
}

bool Klondike::IsValidCardForFoundation(const Core::Card& card, const std::vector<Core::Card>& foundation) const
{
    if (foundation.empty()) {
        // Only Aces can start a foundation pile
        return card.GetRank() == Core::Rank::ACE;
    } else {
        const Core::Card& topCard = foundation.back();
        
        // Cards must be of the same suit and next rank higher
        return card.GetSuit() == topCard.GetSuit() &&
               static_cast<int>(card.GetRank()) == static_cast<int>(topCard.GetRank()) + 1;
    }
}

void Klondike::DealInitialLayout()
{
    // Deal cards to tableau
    for (int i = 0; i < 7; ++i) {
        for (int j = i; j < 7; ++j) {
            Core::Card card = m_stock.Draw();
            
            // The top card of each pile should be face up
            if (j == i) {
                card.SetFaceUp(true);
            }
            
            m_tableau[j].push_back(card);
        }
    }
}

} // namespace Solitaire
} // namespace Games
} // namespace CardGameLib
