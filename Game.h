#pragma once

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include "core/Player.h"
#include "core/Deck.h"

namespace CardGameLib {
namespace Core {

enum class GameState {
    WAITING_FOR_PLAYERS,
    STARTING,
    IN_PROGRESS,
    GAME_OVER
};

enum class GameType {
    SOLITAIRE_KLONDIKE,
    SOLITAIRE_SPIDER,
    SOLITAIRE_FREECELL,
    BLACKJACK,
    POKER
    // Add more as needed
};

// Base class for all card games
class Game {
public:
    // Constructor
    Game(const std::string& name, GameType type, int maxPlayers);
    
    // Game identification
    const std::string& GetName() const;
    GameType GetType() const;
    int GetMaxPlayers() const;
    
    // Virtual destructor
    virtual ~Game() = default;
    
    // Player management
    virtual bool AddPlayer(std::shared_ptr<Player> player);
    virtual bool RemovePlayer(int playerId);
    virtual std::shared_ptr<Player> GetPlayer(int playerId) const;
    virtual const std::vector<std::shared_ptr<Player>>& GetPlayers() const;
    
    // Game state
    virtual GameState GetState() const;
    virtual void SetState(GameState state);
    virtual bool IsGameOver() const;
    
    // Game initialization
    virtual void Initialize() = 0;
    
    // Game actions
    virtual bool Start() = 0;
    virtual bool CanStart() const = 0;
    virtual void Reset() = 0;
    
    // Turn management
    virtual int GetCurrentPlayerIndex() const;
    virtual std::shared_ptr<Player> GetCurrentPlayer() const;
    virtual void NextTurn();
    
    // Game-specific actions (to be implemented by derived classes)
    virtual bool IsValidMove(const std::string& moveData) = 0;
    virtual bool MakeMove(int playerId, const std::string& moveData) = 0;
    
    // Game state serialization (for networking)
    virtual std::string SerializeGameState() const = 0;
    virtual bool DeserializeGameState(const std::string& data) = 0;
    
protected:
    std::string m_name;
    GameType m_type;
    int m_maxPlayers;
    std::vector<std::shared_ptr<Player>> m_players;
    GameState m_state;
    int m_currentPlayerIndex;
};

} // namespace Core
} // namespace CardGameLib
