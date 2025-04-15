#include "games/solitaire/FreeCell.h"
#include <sstream>
#include <algorithm>
#include <cmath>

namespace CardGameLib {
namespace Games {
namespace Solitaire {

FreeCell::FreeCell()
    : Core::Game("FreeCell", Core::GameType::SOLITAIRE_FREECELL, 1)
{
    // Initialize free cells to nullptr
    for (auto& cell : m_freeCells) {
        cell = nullptr;
    }
}

void FreeCell::Initialize()
{
    Reset();
}

bool FreeCell::Start()
{
    if (!CanStart()) {
        return false;
    }
    
    SetState(Core::GameState::STARTING);
    DealInitialLayout();
    SetState(Core::GameState::IN_PROGRESS);
    return true;
}

bool FreeCell::CanStart() const
{
    // FreeCell is a single-player game, so we only need one player
    return m_players.size() == 1;
}

void FreeCell::Reset()
{
    // Clean up any cards in free cells
    for (auto& cell : m_freeCells) {
        delete cell;
        cell = nullptr;
    }
    
    // Clear all other piles
    for (auto& foundation : m_foundations) {
        foundation.clear();
    }
    
    for (auto& pile : m_tableau) {
        pile.clear();
    }
    
    SetState(Core::GameState::WAITING_FOR_PLAYERS);
}

bool FreeCell::IsValidMove(const std::string& moveData)
{
    std::istringstream ss(moveData);
    std::string moveTypeStr;
    ss >> moveTypeStr;
    
    try {
        FreeCellMoveType moveType = static_cast<FreeCellMoveType>(std::stoi(moveTypeStr));
        
        switch (moveType) {
            case FreeCellMoveType::TABLEAU_TO_FREECELL: {
                int tableauIndex, freeCellIndex;
                ss >> tableauIndex >> freeCellIndex;
                
                if (tableauIndex < 0 || tableauIndex >= 8 || 
                    freeCellIndex < 0 || freeCellIndex >= 4 || 
                    m_tableau[tableauIndex].empty() || 
                    m_freeCells[freeCellIndex] != nullptr) {
                    return false;
                }
                
                return true;
            }
                
            case FreeCellMoveType::TABLEAU_TO_FOUNDATION: {
                int tableauIndex, foundationIndex;
                ss >> tableauIndex >> foundationIndex;
                
                if (tableauIndex < 0 || tableauIndex >= 8 || 
                    foundationIndex < 0 || foundationIndex >= 4 || 
                    m_tableau[tableauIndex].empty()) {
                    return false;
                }
                
                const Core::Card& card = m_tableau[tableauIndex].back();
                return IsValidCardForFoundation(card, m_foundations[foundationIndex]);
            }
                
            case FreeCellMoveType::TABLEAU_TO_TABLEAU: {
                int sourceIndex, targetIndex, cardCount;
                ss >> sourceIndex >> targetIndex >> cardCount;
                
                if (sourceIndex < 0 || sourceIndex >= 8 || 
                    targetIndex < 0 || targetIndex >= 8 || 
                    sourceIndex == targetIndex || 
                    m_tableau[sourceIndex].empty() || 
                    cardCount <= 0 || 
                    cardCount > static_cast<int>(m_tableau[sourceIndex].size()) ||
                    cardCount > GetMaxMovableCards()) {
                    return false;
                }
                
                // Check if the specified sequence is valid (descending alternating colors)
                for (int i = 1; i < cardCount; ++i) {
                    size_t cardIndex = m_tableau[sourceIndex].size() - cardCount + i;
                    size_t prevCardIndex = cardIndex - 1;
                    
                    const Core::Card& card = m_tableau[sourceIndex][cardIndex];
                    const Core::Card& prevCard = m_tableau[sourceIndex][prevCardIndex];
                    
                    if (card.GetColor() == prevCard.GetColor() || 
                        static_cast<int>(card.GetRank()) != static_cast<int>(prevCard.GetRank()) - 1) {
                        return false;
                    }
                }
                
                // Get the first card to move
                size_t cardIndex = m_tableau[sourceIndex].size() - cardCount;
                const Core::Card& card = m_tableau[sourceIndex][cardIndex];
                
                return IsValidTableauToTableauMove(card, m_tableau[targetIndex]);
            }
                
            case FreeCellMoveType::FREECELL_TO_FOUNDATION: {
                int freeCellIndex, foundationIndex;
                ss >> freeCellIndex >> foundationIndex;
                
                if (freeCellIndex < 0 || freeCellIndex >= 4 || 
                    foundationIndex < 0 || foundationIndex >= 4 || 
                    m_freeCells[freeCellIndex] == nullptr) {
                    return false;
                }
                
                const Core::Card& card = *m_freeCells[freeCellIndex];
                return IsValidCardForFoundation(card, m_foundations[foundationIndex]);
            }
                
            case FreeCellMoveType::FREECELL_TO_TABLEAU: {
                int freeCellIndex, tableauIndex;
                ss >> freeCellIndex >> tableauIndex;
                
                if (freeCellIndex < 0 || freeCellIndex >= 4 || 
                    tableauIndex < 0 || tableauIndex >= 8 || 
                    m_freeCells[freeCellIndex] == nullptr) {
                    return false;
                }
                
                const Core::Card& card = *m_freeCells[freeCellIndex];
                return IsValidTableauToTableauMove(card, m_tableau[tableauIndex]);
            }
                
            default:
                return false;
        }
    } catch (const std::exception&) {
        return false;
    }
    
    return false;
}

bool FreeCell::MakeMove(int playerId, const std::string& moveData)
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
        FreeCellMoveType moveType = static_cast<FreeCellMoveType>(std::stoi(moveTypeStr));
        
        switch (moveType) {
            case FreeCellMoveType::TABLEAU_TO_FREECELL: {
                int tableauIndex, freeCellIndex;
                ss >> tableauIndex >> freeCellIndex;
                return MoveTableauToFreeCell(tableauIndex, freeCellIndex);
            }
                
            case FreeCellMoveType::TABLEAU_TO_FOUNDATION: {
                int tableauIndex, foundationIndex;
                ss >> tableauIndex >> foundationIndex;
                return MoveTableauToFoundation(tableauIndex, foundationIndex);
            }
                
            case FreeCellMoveType::TABLEAU_TO_TABLEAU: {
                int sourceIndex, targetIndex, cardCount;
                ss >> sourceIndex >> targetIndex >> cardCount;
                return MoveTableauToTableau(sourceIndex, targetIndex, cardCount);
            }
                
            case FreeCellMoveType::FREECELL_TO_FOUNDATION: {
                int freeCellIndex, foundationIndex;
                ss >> freeCellIndex >> foundationIndex;
                return MoveFreeCellToFoundation(freeCellIndex, foundationIndex);
            }
                
            case FreeCellMoveType::FREECELL_TO_TABLEAU: {
                int freeCellIndex, tableauIndex;
                ss >> freeCellIndex >> tableauIndex;
                return MoveFreeCellToTableau(freeCellIndex, tableauIndex);
            }
                
            default:
                return false;
        }
    } catch (const std::exception&) {
        return false;
    }
    
    return false;
}

std::string FreeCell::SerializeGameState() const
{
    std::stringstream ss;
    
    // Serialize free cells
    ss << "FREECELLS ";
    for (const auto& cell : m_freeCells) {
        if (cell == nullptr) {
            ss << "0 ";
        } else {
            ss << "1 " << static_cast<int>(cell->GetSuit()) << " " 
               << static_cast<int>(cell->GetRank()) << " ";
        }
    }
    
    // Serialize foundations
    ss << "FOUNDATIONS ";
    for (const auto& foundation : m_foundations) {
        ss << foundation.size() << " ";
        for (const auto& card : foundation) {
            ss << static_cast<int>(card.GetSuit()) << " " 
               << static_cast<int>(card.GetRank()) << " ";
        }
    }
    
    // Serialize tableau
    ss << "TABLEAU ";
    for (const auto& pile : m_tableau) {
        ss << pile.size() << " ";
        for (const auto& card : pile) {
            ss << static_cast<int>(card.GetSuit()) << " " 
               << static_cast<int>(card.GetRank()) << " ";
        }
    }
    
    return ss.str();
}

bool FreeCell::DeserializeGameState(const std::string& data)
{
    std::istringstream ss(data);
    std::string token;
    
    // Clean up current state
    Reset();
    
    // Deserialize free cells
    ss >> token;
    if (token != "FREECELLS") return false;
    
    for (auto& cell : m_freeCells) {
        int hasCard;
        ss >> hasCard;
        
        if (hasCard == 1) {
            int suit, rank;
            ss >> suit >> rank;
            cell = new Core::Card(static_cast<Core::Suit>(suit), static_cast<Core::Rank>(rank));
            cell->SetFaceUp(true);
        }
    }
    
    // Deserialize foundations
    ss >> token;
    if (token != "FOUNDATIONS") return false;
    
    for (size_t i = 0; i < m_foundations.size(); ++i) {
        size_t foundationSize;
        ss >> foundationSize;
        
        for (size_t j = 0; j < foundationSize; ++j) {
            int suit, rank;
            ss >> suit >> rank;
            
            Core::Card card(static_cast<Core::Suit>(suit), static_cast<Core::Rank>(rank));
            card.SetFaceUp(true);
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
            int suit, rank;
            ss >> suit >> rank;
            
            Core::Card card(static_cast<Core::Suit>(suit), static_cast<Core::Rank>(rank));
            card.SetFaceUp(true);
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

bool FreeCell::MoveTableauToFreeCell(int tableauIndex, int freeCellIndex)
{
    if (tableauIndex < 0 || tableauIndex >= 8 || 
        freeCellIndex < 0 || freeCellIndex >= 4 || 
        m_tableau[tableauIndex].empty() || 
        m_freeCells[freeCellIndex] != nullptr) {
        return false;
    }
    
    m_freeCells[freeCellIndex] = new Core::Card(m_tableau[tableauIndex].back());
    m_freeCells[freeCellIndex]->SetFaceUp(true);
    m_tableau[tableauIndex].pop_back();
    
    return true;
}

bool FreeCell::MoveTableauToFoundation(int tableauIndex, int foundationIndex)
{
    if (tableauIndex < 0 || tableauIndex >= 8 || 
        foundationIndex < 0 || foundationIndex >= 4 || 
        m_tableau[tableauIndex].empty()) {
        return false;
    }
    
    const Core::Card& card = m_tableau[tableauIndex].back();
    if (!IsValidCardForFoundation(card, m_foundations[foundationIndex])) {
        return false;
    }
    
    m_foundations[foundationIndex].push_back(m_tableau[tableauIndex].back());
    m_tableau[tableauIndex].pop_back();
    
    // Check for win condition
    if (IsGameWon()) {
        SetState(Core::GameState::GAME_OVER);
    }
    
    return true;
}

bool FreeCell::MoveTableauToTableau(int sourceIndex, int targetIndex, int cardCount)
{
    if (sourceIndex < 0 || sourceIndex >= 8 || 
        targetIndex < 0 || targetIndex >= 8 || 
        sourceIndex == targetIndex || 
        m_tableau[sourceIndex].empty() || 
        cardCount <= 0 || 
        cardCount > static_cast<int>(m_tableau[sourceIndex].size()) ||
        cardCount > GetMaxMovableCards()) {
        return false;
    }
    
    // Check if the specified sequence is valid (descending alternating colors)
    for (int i = 1; i < cardCount; ++i) {
        size_t cardIndex = m_tableau[sourceIndex].size() - cardCount + i;
        size_t prevCardIndex = cardIndex - 1;
        
        const Core::Card& card = m_tableau[sourceIndex][cardIndex];
        const Core::Card& prevCard = m_tableau[sourceIndex][prevCardIndex];
        
        if (card.GetColor() == prevCard.GetColor() || 
            static_cast<int>(card.GetRank()) != static_cast<int>(prevCard.GetRank()) - 1) {
            return false;
        }
    }
    
    // Get the first card to move
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
    
    return true;
}

bool FreeCell::MoveFreeCellToFoundation(int freeCellIndex, int foundationIndex)
{
    if (freeCellIndex < 0 || freeCellIndex >= 4 || 
        foundationIndex < 0 || foundationIndex >= 4 || 
        m_freeCells[freeCellIndex] == nullptr) {
        return false;
    }
    
    const Core::Card& card = *m_freeCells[freeCellIndex];
    if (!IsValidCardForFoundation(card, m_foundations[foundationIndex])) {
        return false;
    }
    
    m_foundations[foundationIndex].push_back(*m_freeCells[freeCellIndex]);
    delete m_freeCells[freeCellIndex];
    m_freeCells[freeCellIndex] = nullptr;
    
    // Check for win condition
    if (IsGameWon()) {
        SetState(Core::GameState::GAME_OVER);
    }
    
    return true;
}

bool FreeCell::MoveFreeCellToTableau(int freeCellIndex, int tableauIndex)
{
    if (freeCellIndex < 0 || freeCellIndex >= 4 || 
        tableauIndex < 0 || tableauIndex >= 8 || 
        m_freeCells[freeCellIndex] == nullptr) {
        return false;
    }
    
    const Core::Card& card = *m_freeCells[freeCellIndex];
    if (!IsValidTableauToTableauMove(card, m_tableau[tableauIndex])) {
        return false;
    }
    
    m_tableau[tableauIndex].push_back(*m_freeCells[freeCellIndex]);
    delete m_freeCells[freeCellIndex];
    m_freeCells[freeCellIndex] = nullptr;
    
    return true;
}

bool FreeCell::IsGameWon() const
{
    // Game is won when all foundations have 13 cards each (A through K of each suit)
    for (const auto& foundation : m_foundations) {
        if (foundation.size() != 13) {
            return false;
        }
    }
    
    return true;
}

int FreeCell::GetMaxMovableCards() const
{
    // The formula for max movable cards is: (1 + # of empty free cells) * 2^(# of empty tableau columns)
    int emptyFreeCells = CountEmptyFreeCells();
    int emptyTableau = CountEmptyTableauPiles();
    
    return (1 + emptyFreeCells) * (1 << emptyTableau);
}

const std::array<Core::Card*, 4>& FreeCell::GetFreeCells() const
{
    return m_freeCells;
}

const std::array<std::vector<Core::Card>, 4>& FreeCell::GetFoundations() const
{
    return m_foundations;
}

const std::array<std::vector<Core::Card>, 8>& FreeCell::GetTableau() const
{
    return m_tableau;
}

bool FreeCell::IsValidTableauToTableauMove(const Core::Card& card, const std::vector<Core::Card>& targetPile) const
{
    if (targetPile.empty()) {
        // Any card can be placed on an empty tableau pile in FreeCell
        return true;
    } else {
        const Core::Card& targetCard = targetPile.back();
        
        // Cards must be of opposite color and 1 rank lower
        return card.GetColor() != targetCard.GetColor() &&
               static_cast<int>(card.GetRank()) == static_cast<int>(targetCard.GetRank()) - 1;
    }
}

bool FreeCell::IsValidCardForFoundation(const Core::Card& card, const std::vector<Core::Card>& foundation) const
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

int FreeCell::CountEmptyFreeCells() const
{
    int count = 0;
    for (const auto& cell : m_freeCells) {
        if (cell == nullptr) {
            count++;
        }
    }
    return count;
}

int FreeCell::CountEmptyTableauPiles() const
{
    int count = 0;
    for (const auto& pile : m_tableau) {
        if (pile.empty()) {
            count++;
        }
    }
    return count;
}

void FreeCell::DealInitialLayout()
{
    // Create a new shuffled deck
    Core::Deck deck;
    deck.Shuffle();
    
    // Deal all cards to the tableau piles
    int currentPile = 0;
    while (!deck.IsEmpty()) {
        Core::Card card = deck.Draw();
        card.SetFaceUp(true);
        m_tableau[currentPile].push_back(card);
        currentPile = (currentPile + 1) % 8;
    }
}

} // namespace Solitaire
} // namespace Games
} // namespace CardGameLib
