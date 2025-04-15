#pragma once

#include <vector>
#include <array>
#include <memory>
#include "core/Game.h"
#include "core/Deck.h"

namespace CardGameLib {
namespace Games {
namespace Solitaire {

// Move types for Klondike Solitaire
enum class KlondikeMoveType {
    DRAW_FROM_STOCK,       // Draw from stock to waste
    STOCK_TO_TABLEAU,      // Move from stock directly to tableau
    WASTE_TO_TABLEAU,      // Move from waste to tableau
    WASTE_TO_FOUNDATION,   // Move from waste to foundation
    TABLEAU_TO_FOUNDATION, // Move from tableau to foundation
    TABLEAU_TO_TABLEAU,    // Move from tableau to tableau
    FOUNDATION_TO_TABLEAU, // Move from foundation back to tableau
    RECYCLE_WASTE          // Turn waste pile back into stock
};

// Klondike game implementation
class Klondike : public Core::Game {
public:
    // Constructor
    Klondike();
    
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
    
    // Klondike-specific methods
    bool DrawFromStock();
    bool MoveWasteToTableau(int tableauIndex);
    bool MoveWasteToFoundation(int foundationIndex);
    bool MoveTableauToFoundation(int tableauIndex, int foundationIndex);
    bool MoveTableauToTableau(int sourceIndex, int targetIndex, int cardCount);
    bool MoveFoundationToTableau(int foundationIndex, int tableauIndex);
    bool RecycleWaste();
    
    // Game state queries
    bool IsGameWon() const;
    
    // Access game state
    const Core::Deck& GetStock() const;
    const std::vector<Core::Card>& GetWaste() const;
    const std::array<std::vector<Core::Card>, 4>& GetFoundations() const;
    const std::array<std::vector<Core::Card>, 7>& GetTableau() const;
    
private:
    // Game components
    Core::Deck m_stock;                    // Stock/draw pile
    std::vector<Core::Card> m_waste;       // Waste pile (drawn cards)
    std::array<std::vector<Core::Card>, 4> m_foundations; // 4 foundation piles (A to K by suit)
    std::array<std::vector<Core::Card>, 7> m_tableau;     // 7 tableau piles
    
    // Helper methods
    bool IsValidTableauToTableauMove(const Core::Card& card, const std::vector<Core::Card>& targetPile) const;
    bool IsValidCardForFoundation(const Core::Card& card, const std::vector<Core::Card>& foundation) const;
    void DealInitialLayout();
};

} // namespace Solitaire
} // namespace Games
} // namespace CardGameLib
