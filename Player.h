#pragma once

#include <string>
#include <memory>
#include <vector>
#include "core/Card.h"

namespace CardGameLib {
namespace Core {

class Player {
public:
    // Create a new player with a given name
    explicit Player(const std::string& name);
    
    // Player identification
    const std::string& GetName() const;
    int GetId() const;
    void SetId(int id);
    
    // Network status
    bool IsConnected() const;
    void SetConnected(bool connected);
    
    // Player state
    bool IsReady() const;
    void SetReady(bool ready);
    
    // Hand management
    void AddCardToHand(const Card& card);
    Card RemoveCardFromHand(size_t index);
    const std::vector<Card>& GetHand() const;
    void ClearHand();
    
    // Game-specific data (can be overridden by derived classes)
    virtual int GetScore() const;
    virtual void SetScore(int score);
    virtual void AddToScore(int points);
    
private:
    std::string m_name;
    int m_id;
    bool m_connected;
    bool m_ready;
    std::vector<Card> m_hand;
    int m_score;
};

} // namespace Core
} // namespace CardGameLib
