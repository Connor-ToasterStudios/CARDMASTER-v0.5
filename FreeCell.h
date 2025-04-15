#pragma once

#include <array>
#include <vector>
#include <string>
#include "core/Game.h"
#include "core/Deck.h"

namespace CardGameLib {
namespace Games {
namespace Solitaire {

// Move types for FreeCell Solitaire
enum class FreeCellMoveType {
    TABLEAU_TO_FREECELL,    // Move from tableau to free cell
    TABLEAU_TO_FOUNDATION,  // Move from tableau to foundation
    TABLEAU_TO_TABLEAU,     // Move from tableau to tableau
    FREECELL_TO_FOUNDATION, // Move from free cell to foundation
    FREECELL_TO_TABLEAU     // Move from free cell to tableau
};

class FreeCell : public Core::Game {
public:
    // Constructor
    FreeCell();
    
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
    
    // FreeCell-specific methods
    bool MoveTableauToFreeCell(int tableauIndex, int freeCellIndex);
    bool MoveTableauToFoundation(int tableauIndex, int foundationIndex);
    bool MoveTableauToTableau(int sourceIndex, int targetIndex, int cardCount);
    bool MoveFreeCellToFoundation(int freeCellIndex, int foundationIndex);
    bool MoveFreeCellToTableau(int freeCellIndex, int tableauIndex);
    
    // Game state queries
    bool IsGameWon() const;
    int GetMaxMovableCards() const; // Calculate max cards that can be moved at once
    
    // Access game state
    const std::array<Core::Card*, 4>& GetFreeCells() const;
    const std::array<std::vector<Core::Card>, 4>& GetFoundations() const;
    const std::array<std::vector<Core::Card>, 8>& GetTableau() const;
    
private:
    // Game components
    std::array<Core::Card*, 4> m_freeCells;        // 4 free cells (single card spaces)
    std::array<std::vector<Core::Card>, 4> m_foundations; // 4 foundation piles (A to K by suit)
    std::array<std::vector<Core::Card>, 8> m_tableau;     // 8 tableau piles
    
    // Helper methods
    bool IsValidTableauToTableauMove(const Core::Card& card, const std::vector<Core::Card>& targetPile) const;
    bool IsValidCardForFoundation(const Core::Card& card, const std::vector<Core::Card>& foundation) const;
    int CountEmptyFreeCells() const;
    int CountEmptyTableauPiles() const;
    void DealInitialLayout();
};

} // namespace Solitaire
} // namespace Games
} // namespace CardGameLib
