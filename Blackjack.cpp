#include "Blackjack.h"
#include <algorithm>
#include <random>
#include <iostream>

namespace CardGameLib {
namespace Games {
namespace Blackjack {

// BlackjackGame implementation
BlackjackGame::BlackjackGame()
    : Core::Game("Blackjack", Core::GameType::BLACKJACK, 7)  // 7 players max in standard blackjack
    , m_deck(new Core::Deck())
    , m_currentPlayer(nullptr)
    , m_dealer(nullptr)
    , m_gameState(GameState::BETTING)
{
}

BlackjackGame::~BlackjackGame()
{
}

void BlackjackGame::Initialize()
{
    std::cout << "Initializing Blackjack game..." << std::endl;
    
    // Create a standard 52-card deck (create a new one since Initialize has no params)
    m_deck.reset(new Core::Deck());
    
    // Shuffle the deck
    m_deck->Shuffle();
    
    // Create players and dealer
    InitializePlayers();
    
    m_gameState = GameState::BETTING;
    
    // Set the game state
    SetState(Core::GameState::WAITING_FOR_PLAYERS);
}

bool BlackjackGame::Start() 
{
    if (!CanStart()) {
        return false;
    }
    
    // Start the game
    SetState(Core::GameState::IN_PROGRESS);
    DealInitialCards();
    return true;
}

bool BlackjackGame::CanStart() const
{
    // Check if we have at least one player
    return !m_players.empty();
}

void BlackjackGame::Reset()
{
    // Clear all players' hands
    for (auto& player : m_players) {
        player->Reset();
    }
    
    // Reset the deck
    m_deck.reset(new Core::Deck());
    m_deck->Shuffle();
    
    // Reset game state
    m_currentPlayer = m_players.empty() ? nullptr : m_players.front().get();
    m_gameState = GameState::BETTING;
    SetState(Core::GameState::WAITING_FOR_PLAYERS);
}

bool BlackjackGame::IsValidMove(const std::string& moveData)
{
    // Simple move validation for now
    if (moveData == "hit" || moveData == "stand" || moveData == "double" || 
        moveData == "split" || moveData == "surrender") {
        return true;
    }
    return false;
}

bool BlackjackGame::MakeMove(int playerId, const std::string& moveData)
{
    // Find the player
    BlackjackPlayer* player = nullptr;
    for (auto& p : m_players) {
        if (p->GetId() == playerId) {
            player = p.get();
            break;
        }
    }
    
    if (!player) {
        return false;
    }
    
    // Process the move
    if (moveData == "hit") {
        PlayerHit(player);
        return true;
    } else if (moveData == "stand") {
        PlayerStand(player);
        return true;
    } else if (moveData == "double") {
        PlayerDouble(player);
        return true;
    } else if (moveData == "split") {
        PlayerSplit(player);
        return true;
    } else if (moveData == "surrender") {
        PlayerSurrender(player);
        return true;
    }
    
    return false;
}

std::string BlackjackGame::SerializeGameState() const
{
    // Simple serialization for now
    return "Blackjack game state"; // Implement actual serialization if needed
}

bool BlackjackGame::DeserializeGameState(const std::string& data)
{
    // Simple deserialization for now
    return true; // Implement actual deserialization if needed
}

void BlackjackGame::InitializePlayers(int numPlayers)
{
    // Clear existing players
    m_players.clear();
    
    // Create human player
    m_players.push_back(std::make_unique<BlackjackPlayer>("Player 1", false));
    
    // Create dealer
    m_players.push_back(std::make_unique<BlackjackPlayer>("Dealer", true));
    
    // Set dealer reference
    m_dealer = m_players.back().get();
    
    // Set current player to first player
    m_currentPlayer = m_players.front().get();
    
    std::cout << "Created " << m_players.size() - 1 << " players and dealer" << std::endl;
}

void BlackjackGame::Update(float deltaTime)
{
    switch (m_gameState) {
        case GameState::BETTING:
            // In a real implementation, we would wait for player input
            // For simplicity, we'll automatically place a bet and move to dealing
            if (!m_currentPlayer->IsDealer()) {
                PlaceBet(m_currentPlayer, 10);  // Place a default bet of 10
            }
            m_gameState = GameState::DEALING;
            DealInitialCards();
            break;
            
        case GameState::DEALING:
            // Initial deal is complete, move to player turn
            m_gameState = GameState::PLAYER_TURN;
            break;
            
        case GameState::PLAYER_TURN:
            // Wait for player actions (handled via HandleInput)
            if (AllPlayersDone()) {
                m_gameState = GameState::DEALER_TURN;
                DealerPlay();
            }
            break;
            
        case GameState::DEALER_TURN:
            // Dealer play is done in DealerPlay method
            m_gameState = GameState::SETTLEMENT;
            SettleBets();
            break;
            
        case GameState::SETTLEMENT:
            // Bets have been settled, move to cleanup
            m_gameState = GameState::CLEANUP;
            break;
            
        case GameState::CLEANUP:
            // Prepare for next round
            for (auto& player : m_players) {
                player->Reset();
            }
            m_deck.reset(new Core::Deck());
            m_deck->Shuffle();
            m_currentPlayer = m_players.front().get();
            m_gameState = GameState::BETTING;
            break;
    }
}

void BlackjackGame::Render()
{
    // In a real implementation, this would render the game state graphically
    // For now, we'll just print the game state to the console
    
    std::cout << "\n--- Blackjack Game State ---\n";
    
    switch (m_gameState) {
        case GameState::BETTING:
            std::cout << "State: Betting Phase\n";
            break;
        case GameState::DEALING:
            std::cout << "State: Dealing Cards\n";
            break;
        case GameState::PLAYER_TURN:
            std::cout << "State: Player Turn\n";
            break;
        case GameState::DEALER_TURN:
            std::cout << "State: Dealer Turn\n";
            break;
        case GameState::SETTLEMENT:
            std::cout << "State: Settling Bets\n";
            break;
        case GameState::CLEANUP:
            std::cout << "State: Cleanup\n";
            break;
    }
    
    // Display player hands and bets
    for (auto& player : m_players) {
        std::cout << player->GetName() << (player->IsDealer() ? " (Dealer)" : "") << ":\n";
        
        // Show bet for non-dealer players
        if (!player->IsDealer()) {
            std::cout << "  Bet: " << player->GetBet() << "\n";
        }
        
        // Show hand
        std::cout << "  Hand: ";
        const auto& hand = player->GetHand();
        if (hand.empty()) {
            std::cout << "Empty";
        } else {
            for (auto card : hand) {
                // In dealer's first turn, hide the second card
                if (player->IsDealer() && m_gameState == GameState::PLAYER_TURN && card == hand[1]) {
                    std::cout << "[Hidden] ";
                } else {
                    std::cout << card->ToString() << " ";
                }
            }
            std::cout << "= " << CalculateHandValue(hand);
            if (IsBlackjack(hand)) {
                std::cout << " (Blackjack!)";
            } else if (IsBusted(hand)) {
                std::cout << " (Busted!)";
            }
        }
        std::cout << "\n";
        
        // Show split hand if it exists
        if (player->HasSplitHand()) {
            std::cout << "  Split Hand: ";
            const auto& splitHand = player->GetSplitHand();
            for (auto card : splitHand) {
                std::cout << card->ToString() << " ";
            }
            std::cout << "= " << CalculateHandValue(splitHand);
            if (IsBlackjack(splitHand)) {
                std::cout << " (Blackjack!)";
            } else if (IsBusted(splitHand)) {
                std::cout << " (Busted!)";
            }
            std::cout << "\n";
        }
    }
    
    // Show current player indicator
    if (m_currentPlayer) {
        std::cout << "\nCurrent Player: " << m_currentPlayer->GetName() << "\n";
    }
    
    std::cout << "-------------------------\n";
}

void BlackjackGame::HandleInput(int x, int y, bool isDown)
{
    // In a real implementation, this would handle mouse/touch input
    // For simplicity, we'll just use direct function calls from Update
    
    // Example of how it might work with UI buttons:
    // if (isDown && hitButton.Contains(x, y)) {
    //     PlayerHit(m_currentPlayer);
    // } else if (isDown && standButton.Contains(x, y)) {
    //     PlayerStand(m_currentPlayer);
    // }
}

void BlackjackGame::StartNewRound()
{
    // Reset game state for a new round
    for (auto& player : m_players) {
        player->Reset();
    }
    
    m_deck.reset(new Core::Deck());
    m_deck->Shuffle();
    
    m_currentPlayer = m_players.front().get();
    m_gameState = GameState::BETTING;
    
    std::cout << "Starting new round of Blackjack\n";
}

void BlackjackGame::DealInitialCards()
{
    std::cout << "Dealing initial cards...\n";
    
    // Deal two cards to each player
    for (int i = 0; i < 2; ++i) {
        for (auto& player : m_players) {
            DealCard(player.get());
        }
    }
}

Core::Card* BlackjackGame::DealCard(BlackjackPlayer* player)
{
    if (player == nullptr) {
        return nullptr;
    }
    
    // Draw a card from the deck
    if (m_deck->IsEmpty()) {
        // Deck is empty, create a new one and shuffle it
        std::cout << "Reshuffling deck...\n";
        m_deck.reset(new Core::Deck());
        m_deck->Shuffle();
    }
    
    // Create a heap card copy
    Core::Card* cardPtr = new Core::Card(m_deck->Draw());
    
    // Add the card to the player's hand
    if (player->m_playingSplitHand) {
        player->AddCardToSplitHand(cardPtr);
    } else {
        player->AddCard(cardPtr);
    }
    
    return cardPtr;
}

int BlackjackGame::CalculateHandValue(const std::vector<Core::Card*>& hand) const
{
    int value = 0;
    int aces = 0;
    
    for (auto card : hand) {
        Core::Rank rank = card->GetRank();
        int cardValue = static_cast<int>(rank);
        
        // Face cards (Jack, Queen, King) are worth 10
        if (rank == Core::Rank::JACK || rank == Core::Rank::QUEEN || rank == Core::Rank::KING) {
            value += 10;
        }
        // Aces can be worth 1 or 11, count them separately
        else if (rank == Core::Rank::ACE) {
            aces++;
            value += 11; // Initially count as 11
        }
        // Number cards are worth their face value
        else {
            value += cardValue;
        }
    }
    
    // Adjust aces from 11 to 1 as needed to avoid busting
    while (value > 21 && aces > 0) {
        value -= 10; // Change an ace from 11 to 1
        aces--;
    }
    
    return value;
}

bool BlackjackGame::IsBlackjack(const std::vector<Core::Card*>& hand) const
{
    // Blackjack is exactly two cards with a value of 21
    return hand.size() == 2 && CalculateHandValue(hand) == 21;
}

bool BlackjackGame::IsBusted(const std::vector<Core::Card*>& hand) const
{
    // Busted is a hand with value over 21
    return CalculateHandValue(hand) > 21;
}

void BlackjackGame::PlayerHit(BlackjackPlayer* player)
{
    if (player == nullptr || player->m_hasStood || player->m_hasSurrendered) {
        return;
    }
    
    std::cout << player->GetName() << " hits\n";
    
    // Deal a card to the player
    Core::Card* card = DealCard(player);
    
    // Check if player busted
    const auto& hand = player->m_playingSplitHand ? player->m_splitHand : player->m_hand;
    if (IsBusted(hand)) {
        std::cout << player->GetName() << " busted with " << CalculateHandValue(hand) << "\n";
        
        // If this is the split hand, move to the main hand or next player
        if (player->m_playingSplitHand) {
            player->m_playingSplitHand = false;
            
            // If both hands are done, move to next player
            if (player->m_hasStood || IsBusted(player->m_hand)) {
                NextPlayer();
            }
        } else {
            // Move to next player if not playing split hand
            NextPlayer();
        }
    }
}

void BlackjackGame::PlayerStand(BlackjackPlayer* player)
{
    if (player == nullptr || player->m_hasStood || player->m_hasSurrendered) {
        return;
    }
    
    std::cout << player->GetName() << " stands\n";
    
    // If playing split hand, switch to main hand
    if (player->m_playingSplitHand) {
        player->m_playingSplitHand = false;
        
        // If main hand is already done, move to next player
        if (player->m_hasStood || IsBusted(player->m_hand)) {
            NextPlayer();
        }
    } else {
        // Mark player as standing
        player->m_hasStood = true;
        
        // Move to next player
        NextPlayer();
    }
}

void BlackjackGame::PlayerDouble(BlackjackPlayer* player)
{
    if (player == nullptr || player->m_hasStood || player->m_hasSurrendered ||
        player->GetHand().size() > 2 || (player->m_playingSplitHand && player->GetSplitHand().size() > 2)) {
        return;
    }
    
    std::cout << player->GetName() << " doubles down\n";
    
    // Double the bet
    player->SetBet(player->GetBet() * 2);
    
    // Deal one more card
    Core::Card* card = DealCard(player);
    
    // Player automatically stands after doubling
    PlayerStand(player);
}

void BlackjackGame::PlayerSplit(BlackjackPlayer* player)
{
    if (player == nullptr || player->m_hasStood || player->m_hasSurrendered || 
        player->HasSplitHand() || player->GetHand().size() != 2) {
        return;
    }
    
    // Can only split if the two cards have the same value
    const auto& hand = player->GetHand();
    Core::Rank rank1 = hand[0]->GetRank();
    Core::Rank rank2 = hand[1]->GetRank();
    
    // Convert ranks to blackjack values
    int card1Value = (rank1 == Core::Rank::JACK || rank1 == Core::Rank::QUEEN || rank1 == Core::Rank::KING) ? 10 : 
                     (rank1 == Core::Rank::ACE) ? 1 : static_cast<int>(rank1);
                     
    int card2Value = (rank2 == Core::Rank::JACK || rank2 == Core::Rank::QUEEN || rank2 == Core::Rank::KING) ? 10 : 
                     (rank2 == Core::Rank::ACE) ? 1 : static_cast<int>(rank2);
    
    if (card1Value != card2Value) {
        std::cout << "Cannot split: cards must have the same value\n";
        return;
    }
    
    std::cout << player->GetName() << " splits\n";
    
    // Move second card to split hand
    Core::Card* splitCard = hand[1];
    player->m_hand.pop_back();
    player->m_splitHand.push_back(splitCard);
    
    // Deal one more card to each hand
    Core::Card* cardForMainHand = DealCard(player);
    player->m_playingSplitHand = true;
    Core::Card* cardForSplitHand = DealCard(player);
    player->m_playingSplitHand = false;
    
    // Player continues playing the first hand
}

void BlackjackGame::PlayerSurrender(BlackjackPlayer* player)
{
    if (player == nullptr || player->m_hasStood || player->m_hasSurrendered ||
        player->GetHand().size() > 2) {
        return;
    }
    
    std::cout << player->GetName() << " surrenders\n";
    
    // Player surrenders, loses half their bet
    player->m_hasSurrendered = true;
    player->SetBet(player->GetBet() / 2);
    
    // Move to next player
    NextPlayer();
}

void BlackjackGame::DealerPlay()
{
    std::cout << "Dealer plays\n";
    
    if (m_dealer == nullptr) {
        return;
    }
    
    // Dealer follows house rules: must hit on 16 or less, stand on 17 or more
    while (CalculateHandValue(m_dealer->GetHand()) < 17) {
        Core::Card* card = DealCard(m_dealer);
        std::cout << "Dealer draws " << card->ToString() << "\n";
    }
    
    int finalValue = CalculateHandValue(m_dealer->GetHand());
    std::cout << "Dealer final hand value: " << finalValue << "\n";
    
    if (finalValue > 21) {
        std::cout << "Dealer busts!\n";
    }
}

void BlackjackGame::PlaceBet(BlackjackPlayer* player, int amount)
{
    if (player == nullptr || player->IsDealer()) {
        return;
    }
    
    player->SetBet(amount);
    std::cout << player->GetName() << " bets " << amount << "\n";
}

void BlackjackGame::SettleBets()
{
    std::cout << "Settling bets...\n";
    
    if (m_dealer == nullptr) {
        return;
    }
    
    int dealerValue = CalculateHandValue(m_dealer->GetHand());
    bool dealerBusted = IsBusted(m_dealer->GetHand());
    bool dealerBlackjack = IsBlackjack(m_dealer->GetHand());
    
    for (auto& player : m_players) {
        if (player->IsDealer()) {
            continue;
        }
        
        // Process main hand
        SettleHand(player.get(), player->GetHand(), dealerValue, dealerBusted, dealerBlackjack);
        
        // Process split hand if it exists
        if (player->HasSplitHand()) {
            SettleHand(player.get(), player->GetSplitHand(), dealerValue, dealerBusted, dealerBlackjack);
        }
    }
}

void BlackjackGame::SettleHand(BlackjackPlayer* player, const std::vector<Core::Card*>& hand, 
                              int dealerValue, bool dealerBusted, bool dealerBlackjack)
{
    if (player->m_hasSurrendered) {
        std::cout << player->GetName() << " surrendered and loses half bet\n";
        player->Lose();
        return;
    }
    
    int playerValue = CalculateHandValue(hand);
    bool playerBusted = IsBusted(hand);
    bool playerBlackjack = IsBlackjack(hand);
    
    if (playerBusted) {
        std::cout << player->GetName() << " busted and loses\n";
        player->Lose();
    }
    else if (playerBlackjack && !dealerBlackjack) {
        std::cout << player->GetName() << " has blackjack and wins 3:2\n";
        player->Win(1.5f);
    }
    else if (!playerBlackjack && dealerBlackjack) {
        std::cout << player->GetName() << " loses to dealer's blackjack\n";
        player->Lose();
    }
    else if (playerBlackjack && dealerBlackjack) {
        std::cout << player->GetName() << " pushes with dealer's blackjack\n";
        player->Push();
    }
    else if (dealerBusted) {
        std::cout << player->GetName() << " wins as dealer busted\n";
        player->Win();
    }
    else if (playerValue > dealerValue) {
        std::cout << player->GetName() << " wins with " << playerValue << " vs dealer's " << dealerValue << "\n";
        player->Win();
    }
    else if (playerValue < dealerValue) {
        std::cout << player->GetName() << " loses with " << playerValue << " vs dealer's " << dealerValue << "\n";
        player->Lose();
    }
    else {
        std::cout << player->GetName() << " pushes with " << playerValue << " vs dealer's " << dealerValue << "\n";
        player->Push();
    }
}

void BlackjackGame::NextPlayer()
{
    // Find current player in the list
    auto it = std::find_if(m_players.begin(), m_players.end(), 
                          [this](const std::unique_ptr<BlackjackPlayer>& player) {
                              return player.get() == m_currentPlayer;
                          });
    
    if (it == m_players.end()) {
        return;
    }
    
    // Move to next player, skipping the dealer
    do {
        ++it;
        
        // If reached the end, all players are done
        if (it == m_players.end()) {
            // Check if all players are done
            if (AllPlayersDone()) {
                m_gameState = GameState::DEALER_TURN;
            }
            return;
        }
        
        m_currentPlayer = it->get();
    } while (m_currentPlayer->IsDealer());
}

bool BlackjackGame::AllPlayersDone() const
{
    for (const auto& player : m_players) {
        if (player->IsDealer()) {
            continue;
        }
        
        // Check if player has any active hands
        if (!player->m_hasStood && !player->m_hasSurrendered && !IsBusted(player->GetHand())) {
            return false;
        }
        
        // Check split hand if it exists
        if (player->HasSplitHand() && player->m_playingSplitHand && !IsBusted(player->GetSplitHand())) {
            return false;
        }
    }
    
    return true;
}

// BlackjackPlayer implementation
BlackjackPlayer::BlackjackPlayer(const std::string& name, bool isDealer)
    : Core::Player(name)
    , m_isDealer(isDealer)
    , m_currentBet(0)
    , m_hasSurrendered(false)
    , m_hasStood(false)
    , m_playingSplitHand(false)
{
}

BlackjackPlayer::~BlackjackPlayer()
{
}

void BlackjackPlayer::Reset()
{
    // Clear hands but don't delete cards as they belong to the deck
    m_hand.clear();
    m_splitHand.clear();
    
    // Reset player state
    m_currentBet = 0;
    m_hasSurrendered = false;
    m_hasStood = false;
    m_playingSplitHand = false;
}

void BlackjackPlayer::AddCard(Core::Card* card)
{
    if (card != nullptr) {
        m_hand.push_back(card);
    }
}

void BlackjackPlayer::AddCardToSplitHand(Core::Card* card)
{
    if (card != nullptr) {
        m_splitHand.push_back(card);
    }
}

void BlackjackPlayer::Win(float multiplier)
{
    int winnings = static_cast<int>(m_currentBet * multiplier);
    AddToScore(winnings);
    std::cout << GetName() << " wins " << winnings << " chips\n";
}

void BlackjackPlayer::Lose()
{
    AddToScore(-m_currentBet);
    std::cout << GetName() << " loses " << m_currentBet << " chips\n";
}

void BlackjackPlayer::Push()
{
    std::cout << GetName() << " pushes, bet returned\n";
    // Bet is returned, no score adjustment needed
}

} // namespace Blackjack
} // namespace Games
} // namespace CardGameLib