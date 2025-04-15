#include "core/Player.h"
#include <stdexcept>

namespace CardGameLib {
namespace Core {

Player::Player(const std::string& name)
    : m_name(name)
    , m_id(-1)
    , m_connected(false)
    , m_ready(false)
    , m_score(0)
{
}

const std::string& Player::GetName() const
{
    return m_name;
}

int Player::GetId() const
{
    return m_id;
}

void Player::SetId(int id)
{
    m_id = id;
}

bool Player::IsConnected() const
{
    return m_connected;
}

void Player::SetConnected(bool connected)
{
    m_connected = connected;
}

bool Player::IsReady() const
{
    return m_ready;
}

void Player::SetReady(bool ready)
{
    m_ready = ready;
}

void Player::AddCardToHand(const Card& card)
{
    m_hand.push_back(card);
}

Card Player::RemoveCardFromHand(size_t index)
{
    if (index >= m_hand.size()) {
        throw std::out_of_range("Card index out of range");
    }
    
    Card removedCard = m_hand[index];
    m_hand.erase(m_hand.begin() + index);
    return removedCard;
}

const std::vector<Card>& Player::GetHand() const
{
    return m_hand;
}

void Player::ClearHand()
{
    m_hand.clear();
}

int Player::GetScore() const
{
    return m_score;
}

void Player::SetScore(int score)
{
    m_score = score;
}

void Player::AddToScore(int points)
{
    m_score += points;
}

} // namespace Core
} // namespace CardGameLib
