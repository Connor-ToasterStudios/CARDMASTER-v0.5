#pragma once

#include "../../core/Game.h"
#include "../../core/Card.h"
#include "../../core/Deck.h"
#include "../../core/Player.h"

#include <vector>
#include <string>
#include <memory>

namespace CardGameLib {
namespace Games {
namespace Blackjack {

// Forward declarations
class BlackjackPlayer;

// Enum for possible actions in Blackjack
enum class BlackjackAction {
    HIT,      // Draw another card
    STAND,    // End turn, keep current hand
    DOUBLE,   // Double bet, take one more card, then stand
    SPLIT,    // Split pair into two hands (when holding a pair)
    SURRENDER // Give up hand, receive half the bet back
};

// Class for Blackjack game implementation
class BlackjackGame : public Core::Game {
public:
    // Constructor and destructor
    BlackjackGame();
    virtual ~BlackjackGame();
    
    // Game interface implementation
    virtual void Initialize() override;
    virtual bool Start() override;
    virtual bool CanStart() const override;
    virtual void Reset() override;
    virtual bool IsValidMove(const std::string& moveData) override;
    virtual bool MakeMove(int playerId, const std::string& moveData) override;
    virtual std::string SerializeGameState() const override;
    virtual bool DeserializeGameState(const std::string& data) override;
    
    // Blackjack-specific methods
    void Update(float deltaTime);
    void Render();
    void HandleInput(int x, int y, bool isDown);
    
    // Blackjack specific methods
    void StartNewRound();
    void DealInitialCards();
    int CalculateHandValue(const std::vector<Core::Card*>& hand) const;
    bool IsBlackjack(const std::vector<Core::Card*>& hand) const;
    bool IsBusted(const std::vector<Core::Card*>& hand) const;
    
    // Player actions
    void PlayerHit(BlackjackPlayer* player);
    void PlayerStand(BlackjackPlayer* player);
    void PlayerDouble(BlackjackPlayer* player);
    void PlayerSplit(BlackjackPlayer* player);
    void PlayerSurrender(BlackjackPlayer* player);
    
    // Dealer actions
    void DealerPlay();
    
    // Betting
    void PlaceBet(BlackjackPlayer* player, int amount);
    void SettleBets();
    void SettleHand(BlackjackPlayer* player, const std::vector<Core::Card*>& hand, 
                   int dealerValue, bool dealerBusted, bool dealerBlackjack);
    
private:
    // Game state
    enum class GameState {
        BETTING,    // Players placing bets
        DEALING,    // Initial cards being dealt
        PLAYER_TURN, // Player making decisions
        DEALER_TURN, // Dealer making decisions
        SETTLEMENT, // Determining winners and paying out
        CLEANUP     // Preparing for next round
    };
    
    // Member variables
    std::unique_ptr<Core::Deck> m_deck;
    std::vector<std::unique_ptr<BlackjackPlayer>> m_players;
    BlackjackPlayer* m_currentPlayer;
    BlackjackPlayer* m_dealer;
    GameState m_gameState;
    
    // Helper methods
    void InitializePlayers(int numPlayers = 1);
    Core::Card* DealCard(BlackjackPlayer* player);
    void NextPlayer();
    bool AllPlayersDone() const;
};

// Class for Blackjack players
class BlackjackPlayer : public Core::Player {
public:
    // Constructor and destructor
    BlackjackPlayer(const std::string& name, bool isDealer = false);
    virtual ~BlackjackPlayer();
    
    // Player actions
    void Reset();
    void AddCard(Core::Card* card);
    void AddCardToSplitHand(Core::Card* card);
    
    // Getters and setters
    const std::vector<Core::Card*>& GetHand() const { return m_hand; }
    const std::vector<Core::Card*>& GetSplitHand() const { return m_splitHand; }
    bool HasSplitHand() const { return !m_splitHand.empty(); }
    bool IsDealer() const { return m_isDealer; }
    
    // Betting methods
    void SetBet(int amount) { m_currentBet = amount; }
    int GetBet() const { return m_currentBet; }
    void Win(float multiplier = 1.0f);
    void Lose();
    void Push(); // Tie with dealer
    
private:
    std::vector<Core::Card*> m_hand;
    std::vector<Core::Card*> m_splitHand;
    bool m_isDealer;
    int m_currentBet;
    bool m_hasSurrendered;
    bool m_hasStood;
    bool m_playingSplitHand;  // True if currently playing the split hand
    
    friend class BlackjackGame;
};

} // namespace Blackjack
} // namespace Games
} // namespace CardGameLib