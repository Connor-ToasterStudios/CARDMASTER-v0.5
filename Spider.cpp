#include "games/solitaire/Spider.h"
#include <sstream>
#include <algorithm>

namespace CardGameLib {
namespace Games {
namespace Solitaire {

Spider::Spider(SpiderDifficulty difficulty)
    : Core::Game("Spider", Core::GameType::SOLITAIRE_SPIDER, 1)
    , m_completedSuits(0)
    , m_difficulty(difficulty)
{
}

void Spider::Initialize()
{
    Reset();
}

bool Spider::Start()
{
    if (!CanStart()) {
        return false;
    }
    
    SetState(Core::GameState::STARTING);
    DealInitialLayout();
    SetState(Core::GameState::IN_PROGRESS);
    return true;
}

bool Spider::CanStart() const
{
    // Spider is a single-player game, so we only need one player
    return m_players.size() == 1;
}

void Spider::Reset()
{
    // Create a new specialized spider deck based on difficulty
    CreateSpiderDeck();
    
    // Clear tableau
    for (auto& pile : m_tableau) {
        pile.clear();
    }
    
    // Reset completed suits counter
    m_completedSuits = 0;
    
    SetState(Core::GameState::WAITING_FOR_PLAYERS);
}

bool Spider::IsValidMove(const std::string& moveData)
{
    std::istringstream ss(moveData);
    std::string moveTypeStr;
    ss >> moveTypeStr;
    
    try {
        SpiderMoveType moveType = static_cast<SpiderMoveType>(std::stoi(moveTypeStr));
        
        switch (moveType) {
            case SpiderMoveType::DEAL_CARDS:
                // Can deal cards if there are cards in the stock and no empty tableau piles
                return !m_stock.IsEmpty() && 
                       std::none_of(m_tableau.begin(), m_tableau.end(),
                                   [](const std::vector<Core::Card>& pile) { return pile.empty(); });
                
            case SpiderMoveType::TABLEAU_TO_TABLEAU: {
                int sourceIndex, targetIndex, cardCount;
                ss >> sourceIndex >> targetIndex >> cardCount;
                
                if (sourceIndex < 0 || sourceIndex >= 10 || 
                    targetIndex < 0 || targetIndex >= 10 || 
                    sourceIndex == targetIndex || 
                    m_tableau[sourceIndex].empty() ||
                    cardCount <= 0 || 
                    cardCount > static_cast<int>(m_tableau[sourceIndex].size())) {
                    return false;
                }
                
                // Check if the specified cards form a valid sequence
                size_t startIndex = m_tableau[sourceIndex].size() - cardCount;
                if (!IsDescendingSequence(m_tableau[sourceIndex], startIndex, cardCount)) {
                    return false;
                }
                
                // Get the card we're trying to move
                const Core::Card& card = m_tableau[sourceIndex][startIndex];
                
                // If target is empty, any card can be placed
                if (m_tableau[targetIndex].empty()) {
                    return true;
                }
                
                // For non-empty targets, the top card of the target pile must be one rank higher
                const Core::Card& targetCard = m_tableau[targetIndex].back();
                return static_cast<int>(card.GetRank()) == static_cast<int>(targetCard.GetRank()) - 1;
            }
                
            case SpiderMoveType::COLLECT_COMPLETED_SUIT:
                // This move is automatically checked and performed by the game
                return false;
                
            default:
                return false;
        }
    } catch (const std::exception&) {
        return false;
    }
    
    return false;
}

bool Spider::MakeMove(int playerId, const std::string& moveData)
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
        SpiderMoveType moveType = static_cast<SpiderMoveType>(std::stoi(moveTypeStr));
        
        switch (moveType) {
            case SpiderMoveType::DEAL_CARDS:
                return DealCards();
                
            case SpiderMoveType::TABLEAU_TO_TABLEAU: {
                int sourceIndex, targetIndex, cardCount;
                ss >> sourceIndex >> targetIndex >> cardCount;
                
                bool moveResult = MoveTableauToTableau(sourceIndex, targetIndex, cardCount);
                
                // After a successful move, check for completed suits
                if (moveResult) {
                    CheckAndRemoveCompletedSuits();
                }
                
                return moveResult;
            }
                
            case SpiderMoveType::COLLECT_COMPLETED_SUIT:
                // This should be handled automatically
                return false;
                
            default:
                return false;
        }
    } catch (const std::exception&) {
        return false;
    }
    
    return false;
}

std::string Spider::SerializeGameState() const
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
    
    // Serialize game state
    ss << "GAME_STATE " << m_completedSuits << " " << static_cast<int>(m_difficulty);
    
    return ss.str();
}

bool Spider::DeserializeGameState(const std::string& data)
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
    
    // Deserialize game state
    ss >> token;
    if (token != "GAME_STATE") return false;
    
    int completedSuits, difficulty;
    ss >> completedSuits >> difficulty;
    
    m_completedSuits = completedSuits;
    m_difficulty = static_cast<SpiderDifficulty>(difficulty);
    
    // Set game state
    SetState(Core::GameState::IN_PROGRESS);
    
    // Check for win condition
    if (IsGameWon()) {
        SetState(Core::GameState::GAME_OVER);
    }
    
    return true;
}

bool Spider::DealCards()
{
    if (m_stock.IsEmpty()) {
        return false;
    }
    
    // Check if all tableau piles have at least one card
    if (std::any_of(m_tableau.begin(), m_tableau.end(), 
                   [](const std::vector<Core::Card>& pile) { return pile.empty(); })) {
        return false;
    }
    
    // Deal one card to each tableau pile
    for (auto& pile : m_tableau) {
        if (!m_stock.IsEmpty()) {
            Core::Card card = m_stock.Draw();
            card.SetFaceUp(true);
            pile.push_back(card);
        }
    }
    
    // Check for completed suits after dealing
    CheckAndRemoveCompletedSuits();
    
    return true;
}

bool Spider::MoveTableauToTableau(int sourceIndex, int targetIndex, int cardCount)
{
    if (sourceIndex < 0 || sourceIndex >= 10 || 
        targetIndex < 0 || targetIndex >= 10 || 
        sourceIndex == targetIndex || 
        m_tableau[sourceIndex].empty() ||
        cardCount <= 0 || 
        cardCount > static_cast<int>(m_tableau[sourceIndex].size())) {
        return false;
    }
    
    // Check if the specified cards form a valid sequence
    size_t startIndex = m_tableau[sourceIndex].size() - cardCount;
    if (!IsDescendingSequence(m_tableau[sourceIndex], startIndex, cardCount)) {
        return false;
    }
    
    // Get the card we're trying to move
    const Core::Card& card = m_tableau[sourceIndex][startIndex];
    
    // If target is empty, any card can be placed
    if (!m_tableau[targetIndex].empty()) {
        // For non-empty targets, the top card of the target pile must be one rank higher
        const Core::Card& targetCard = m_tableau[targetIndex].back();
        if (static_cast<int>(card.GetRank()) != static_cast<int>(targetCard.GetRank()) - 1) {
            return false;
        }
    }
    
    // Move the cards
    std::vector<Core::Card> cardsToMove;
    cardsToMove.insert(cardsToMove.begin(), 
                      m_tableau[sourceIndex].begin() + startIndex,
                      m_tableau[sourceIndex].end());
    
    m_tableau[targetIndex].insert(m_tableau[targetIndex].end(),
                                 cardsToMove.begin(), cardsToMove.end());
    
    m_tableau[sourceIndex].erase(m_tableau[sourceIndex].begin() + startIndex,
                               m_tableau[sourceIndex].end());
    
    // Turn over the top card of the source pile if needed
    if (!m_tableau[sourceIndex].empty() && !m_tableau[sourceIndex].back().IsFaceUp()) {
        m_tableau[sourceIndex].back().SetFaceUp(true);
    }
    
    return true;
}

bool Spider::CheckAndRemoveCompletedSuits()
{
    bool foundCompletedSuit = false;
    
    // Check each tableau pile for completed sequences (K-A of same suit)
    for (auto& pile : m_tableau) {
        if (pile.size() >= 13) {
            // Check from the end of the pile backwards
            size_t endPos = pile.size();
            while (endPos >= 13) {
                // Check if the 13 cards form a King-to-Ace sequence of the same suit
                if (IsKingToAceSequenceSameSuit(std::vector<Core::Card>(
                    pile.end() - 13, pile.end()))) {
                    // Remove the 13 cards
                    pile.erase(pile.end() - 13, pile.end());
                    
                    // Increment completed suits counter
                    m_completedSuits++;
                    foundCompletedSuit = true;
                    
                    // Turn over the top card of the pile if needed
                    if (!pile.empty() && !pile.back().IsFaceUp()) {
                        pile.back().SetFaceUp(true);
                    }
                    
                    // Check for win condition
                    if (IsGameWon()) {
                        SetState(Core::GameState::GAME_OVER);
                    }
                } else {
                    // Move to the next potential sequence
                    endPos--;
                }
            }
        }
    }
    
    return foundCompletedSuit;
}

void Spider::SetDifficulty(SpiderDifficulty difficulty)
{
    if (m_difficulty != difficulty) {
        m_difficulty = difficulty;
        Reset();
    }
}

bool Spider::IsGameWon() const
{
    // Spider is won when all 8 suits are completed
    return m_completedSuits == 8;
}

const Core::Deck& Spider::GetStock() const
{
    return m_stock;
}

const std::array<std::vector<Core::Card>, 10>& Spider::GetTableau() const
{
    return m_tableau;
}

int Spider::GetCompletedSuits() const
{
    return m_completedSuits;
}

SpiderDifficulty Spider::GetDifficulty() const
{
    return m_difficulty;
}

bool Spider::IsValidTableauToTableauMove(const Core::Card& card, const std::vector<Core::Card>& targetPile) const
{
    if (targetPile.empty()) {
        // Any card can be placed on an empty pile in Spider
        return true;
    } else {
        const Core::Card& targetCard = targetPile.back();
        
        // In Spider, the card must be one rank lower than the target card
        // (suit doesn't matter for basic moves)
        return static_cast<int>(card.GetRank()) == static_cast<int>(targetCard.GetRank()) - 1;
    }
}

bool Spider::IsDescendingSequence(const std::vector<Core::Card>& cards, size_t startIndex, size_t count) const
{
    // Check if all cards are face up
    for (size_t i = startIndex; i < startIndex + count; ++i) {
        if (!cards[i].IsFaceUp()) {
            return false;
        }
    }
    
    // Check if the cards form a descending sequence
    for (size_t i = startIndex; i < startIndex + count - 1; ++i) {
        if (static_cast<int>(cards[i].GetRank()) != static_cast<int>(cards[i+1].GetRank()) + 1) {
            return false;
        }
    }
    
    return true;
}

bool Spider::IsSameSuitSequence(const std::vector<Core::Card>& cards, size_t startIndex, size_t count) const
{
    if (count <= 1) {
        return true;
    }
    
    Core::Suit suit = cards[startIndex].GetSuit();
    
    for (size_t i = startIndex + 1; i < startIndex + count; ++i) {
        if (cards[i].GetSuit() != suit) {
            return false;
        }
    }
    
    return true;
}

bool Spider::IsKingToAceSequenceSameSuit(const std::vector<Core::Card>& cards) const
{
    if (cards.size() != 13) {
        return false;
    }
    
    // Check if all cards are of the same suit
    Core::Suit suit = cards[0].GetSuit();
    for (const auto& card : cards) {
        if (card.GetSuit() != suit) {
            return false;
        }
    }
    
    // Check if the sequence is King to Ace
    for (int i = 0; i < 13; i++) {
        if (static_cast<int>(cards[i].GetRank()) != 13 - i) {
            return false;
        }
    }
    
    return true;
}

void Spider::CreateSpiderDeck()
{
    // Spider uses 2 decks (104 cards)
    std::vector<Core::Card> cards;
    
    switch (m_difficulty) {
        case SpiderDifficulty::ONE_SUIT:
            // Use 8 suits of Spades
            for (int i = 0; i < 8; i++) {
                for (int rank = 1; rank <= 13; rank++) {
                    cards.emplace_back(Core::Suit::SPADES, static_cast<Core::Rank>(rank));
                }
            }
            break;
            
        case SpiderDifficulty::TWO_SUITS:
            // Use 4 suits of Spades and 4 suits of Hearts
            for (int i = 0; i < 4; i++) {
                for (int rank = 1; rank <= 13; rank++) {
                    cards.emplace_back(Core::Suit::SPADES, static_cast<Core::Rank>(rank));
                    cards.emplace_back(Core::Suit::HEARTS, static_cast<Core::Rank>(rank));
                }
            }
            break;
            
        case SpiderDifficulty::FOUR_SUITS:
            // Use 2 standard decks (all 4 suits)
            for (int i = 0; i < 2; i++) {
                for (int suit = 0; suit < 4; suit++) {
                    for (int rank = 1; rank <= 13; rank++) {
                        cards.emplace_back(static_cast<Core::Suit>(suit), static_cast<Core::Rank>(rank));
                    }
                }
            }
            break;
    }
    
    m_stock = Core::Deck(cards);
    m_stock.Shuffle();
}

void Spider::DealInitialLayout()
{
    // First deal: 4 cards to each tableau pile
    for (int deal = 0; deal < 4; deal++) {
        for (auto& pile : m_tableau) {
            Core::Card card = m_stock.Draw();
            if (deal == 3) {
                card.SetFaceUp(true);
            }
            pile.push_back(card);
        }
    }
    
    // Second deal: 1 card to first 4 piles
    for (int i = 0; i < 4; i++) {
        Core::Card card = m_stock.Draw();
        card.SetFaceUp(true);
        m_tableau[i].push_back(card);
    }
}

} // namespace Solitaire
} // namespace Games
} // namespace CardGameLib
