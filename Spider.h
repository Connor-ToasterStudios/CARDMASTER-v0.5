#pragma once

#include <array>
#include <vector>
#include <string>
#include "core/Game.h"
#include "core/Deck.h"

namespace CardGameLib {
namespace Games {
namespace Solitaire {

// Different difficulty levels (number of suits)
enum class SpiderDifficulty {
    ONE_SUIT,   // Easiest - all cards are the same suit
    TWO_SUITS,  // Medium - two suits
    FOUR_SUITS  // Hardest - all four suits
};

// Move types for Spider Solitaire
enum class SpiderMoveType {
    DEAL_CARDS,          // Deal cards from stock to tableau
    TABLEAU_TO_TABLEAU,  // Move from tableau to tableau
    COLLECT_COMPLETED_SUIT // Automatic move of a completed suit to foundation
};

class Spider : public Core::Game {
public:
    // Constructor, default to ONE_SUIT difficulty
    Spider(SpiderDifficulty difficulty = SpiderDifficulty::ONE_SUIT);
    
    // Game setup
    virtual void Initialize() override;
    virtual bool Start() override;
    virtual bool CanStart() const override;
    virtual void Reset() override;
    
    // Game moves
    virtual bool IsValidMove(const std::string& moveData) override;
    virtual bool MakeMove(int playerId, const std::string& moveData) override;
    
    // Game state serialization
    virtual std::string SerializeGameState() const override;
    virtual bool DeserializeGameState(const std::string& data) override;
    
    // Spider-specific methods
    bool DealCards();
    bool MoveTableauToTableau(int sourceIndex, int targetIndex, int cardCount);
    bool CheckAndRemoveCompletedSuits();
    void SetDifficulty(SpiderDifficulty difficulty);
    
    // Game state queries
    bool IsGameWon() const;
    
    // Access game state
    const Core::Deck& GetStock() const;
    const std::array<std::vector<Core::Card>, 10>& GetTableau() const;
    int GetCompletedSuits() const;
    SpiderDifficulty GetDifficulty() const;
    
private:
    // Game components
    Core::Deck m_stock;                       // Stock/draw pile
    std::array<std::vector<Core::Card>, 10> m_tableau; // 10 tableau piles
    int m_completedSuits;                     // Counter for completed suits (0-8)
    SpiderDifficulty m_difficulty;            // Game difficulty
    
    // Helper methods
    bool IsValidTableauToTableauMove(const Core::Card& card, const std::vector<Core::Card>& targetPile) const;
    bool IsDescendingSequence(const std::vector<Core::Card>& cards, size_t startIndex, size_t count) const;
    bool IsSameSuitSequence(const std::vector<Core::Card>& cards, size_t startIndex, size_t count) const;
    bool IsKingToAceSequenceSameSuit(const std::vector<Core::Card>& cards) const;
    void CreateSpiderDeck();
    void DealInitialLayout();
};

} // namespace Solitaire
} // namespace Games
} // namespace CardGameLib
