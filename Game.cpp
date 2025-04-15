#include "core/Game.h"
#include <algorithm>

namespace CardGameLib {
namespace Core {

Game::Game(const std::string& name, GameType type, int maxPlayers)
    : m_name(name)
    , m_type(type)
    , m_maxPlayers(maxPlayers)
    , m_state(GameState::WAITING_FOR_PLAYERS)
    , m_currentPlayerIndex(-1)
{
}

const std::string& Game::GetName() const
{
    return m_name;
}

GameType Game::GetType() const
{
    return m_type;
}

int Game::GetMaxPlayers() const
{
    return m_maxPlayers;
}

bool Game::AddPlayer(std::shared_ptr<Player> player)
{
    if (m_players.size() >= static_cast<size_t>(m_maxPlayers)) {
        return false;
    }
    
    // Check if player already exists
    for (const auto& existingPlayer : m_players) {
        if (existingPlayer->GetId() == player->GetId()) {
            return false;
        }
    }
    
    m_players.push_back(player);
    
    // If this is the first player, set them as current
    if (m_players.size() == 1) {
        m_currentPlayerIndex = 0;
    }
    
    return true;
}

bool Game::RemovePlayer(int playerId)
{
    auto it = std::find_if(m_players.begin(), m_players.end(),
                          [playerId](const std::shared_ptr<Player>& player) {
                              return player->GetId() == playerId;
                          });
    
    if (it == m_players.end()) {
        return false;
    }
    
    size_t index = std::distance(m_players.begin(), it);
    m_players.erase(it);
    
    // Adjust current player index if necessary
    if (m_players.empty()) {
        m_currentPlayerIndex = -1;
    } else if (static_cast<size_t>(m_currentPlayerIndex) == index) {
        m_currentPlayerIndex = m_currentPlayerIndex % m_players.size();
    } else if (static_cast<size_t>(m_currentPlayerIndex) > index) {
        m_currentPlayerIndex--;
    }
    
    return true;
}

std::shared_ptr<Player> Game::GetPlayer(int playerId) const
{
    auto it = std::find_if(m_players.begin(), m_players.end(),
                          [playerId](const std::shared_ptr<Player>& player) {
                              return player->GetId() == playerId;
                          });
    
    return (it != m_players.end()) ? *it : nullptr;
}

const std::vector<std::shared_ptr<Player>>& Game::GetPlayers() const
{
    return m_players;
}

GameState Game::GetState() const
{
    return m_state;
}

void Game::SetState(GameState state)
{
    m_state = state;
}

bool Game::IsGameOver() const
{
    return m_state == GameState::GAME_OVER;
}

int Game::GetCurrentPlayerIndex() const
{
    return m_currentPlayerIndex;
}

std::shared_ptr<Player> Game::GetCurrentPlayer() const
{
    if (m_currentPlayerIndex >= 0 && static_cast<size_t>(m_currentPlayerIndex) < m_players.size()) {
        return m_players[m_currentPlayerIndex];
    }
    return nullptr;
}

void Game::NextTurn()
{
    if (!m_players.empty()) {
        m_currentPlayerIndex = (m_currentPlayerIndex + 1) % m_players.size();
    }
}

} // namespace Core
} // namespace CardGameLib
