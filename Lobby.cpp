#include "network/Lobby.h"
#include "games/solitaire/Klondike.h"
#include "games/solitaire/FreeCell.h"
#include "games/solitaire/Spider.h"
#include <sstream>
#include <iostream>
#include <nlohmann/json.hpp>

// Using nlohmann json for serialization
using json = nlohmann::json;

namespace CardGameLib {
namespace Network {

// GameInfo serialization
std::string GameInfo::Serialize() const
{
    json j;
    j["id"] = id;
    j["name"] = name;
    j["type"] = static_cast<int>(type);
    j["maxPlayers"] = maxPlayers;
    j["currentPlayerCount"] = currentPlayerCount;
    j["inProgress"] = inProgress;
    
    return j.dump();
}

GameInfo GameInfo::Deserialize(const std::string& data)
{
    GameInfo info;
    try {
        json j = json::parse(data);
        info.id = j["id"];
        info.name = j["name"];
        info.type = static_cast<Core::GameType>(j["type"].get<int>());
        info.maxPlayers = j["maxPlayers"];
        info.currentPlayerCount = j["currentPlayerCount"];
        info.inProgress = j["inProgress"];
    } catch (const std::exception& e) {
        std::cerr << "Error parsing GameInfo: " << e.what() << std::endl;
    }
    
    return info;
}

Lobby::Lobby(NetworkManager* networkManager)
    : m_networkManager(networkManager)
    , m_currentGameId(-1)
    , m_isHost(false)
    , m_nextGameId(1)
{
}

Lobby::~Lobby()
{
    Disconnect();
}

void Lobby::Initialize()
{
    // Register for network messages
    if (m_networkManager) {
        m_networkManager->RegisterMessageCallback([this](const std::string& message, int clientId) {
            HandleNetworkMessage(message, clientId);
        });
    }
}

bool Lobby::StartServer(int port)
{
    if (!m_networkManager) {
        return false;
    }
    
    // Start the network server
    if (!m_networkManager->StartServer(port)) {
        return false;
    }
    
    // Clear existing games
    {
        std::lock_guard<std::mutex> lock(m_gamesMutex);
        m_games.clear();
        m_nextGameId = 1;
    }
    
    return true;
}

bool Lobby::ConnectToServer(const std::string& host, int port)
{
    if (!m_networkManager) {
        return false;
    }
    
    // Connect to the server
    if (!m_networkManager->Connect(host, port)) {
        return false;
    }
    
    // Reset local state
    m_currentGameId = -1;
    m_isHost = false;
    m_localPlayer.reset();
    
    // Clear players and current game
    {
        std::lock_guard<std::mutex> lock(m_playersMutex);
        m_playersInGame.clear();
        m_currentGame.reset();
    }
    
    return true;
}

void Lobby::Disconnect()
{
    // Leave any current game
    LeaveGame();
    
    // Disconnect from server
    if (m_networkManager) {
        m_networkManager->Disconnect();
    }
}

int Lobby::CreateGame(const std::string& name, Core::GameType type, int maxPlayers)
{
    // Server mode only
    if (m_networkManager->GetMode() != NetworkMode::SERVER) {
        return -1;
    }
    
    // Create the game info
    GameInfo gameInfo;
    gameInfo.id = m_nextGameId++;
    gameInfo.name = name;
    gameInfo.type = type;
    gameInfo.maxPlayers = maxPlayers;
    gameInfo.currentPlayerCount = 0;
    gameInfo.inProgress = false;
    
    // Add to games list
    {
        std::lock_guard<std::mutex> lock(m_gamesMutex);
        m_games[gameInfo.id] = gameInfo;
    }
    
    // Notify all clients about the new game
    SendGameList();
    
    return gameInfo.id;
}

bool Lobby::JoinGame(int gameId, const std::string& playerName)
{
    if (!m_networkManager) {
        return false;
    }
    
    // Create local player
    m_localPlayer = std::make_shared<Core::Player>(playerName);
    
    if (m_networkManager->GetMode() == NetworkMode::CLIENT) {
        // Send join request to server
        json j;
        j["command"] = "join_game";
        j["game_id"] = gameId;
        j["player_name"] = playerName;
        
        return m_networkManager->SendToServer(j.dump());
    }
    else if (m_networkManager->GetMode() == NetworkMode::SERVER) {
        // Server joining a local game
        std::lock_guard<std::mutex> lock(m_gamesMutex);
        
        auto it = m_games.find(gameId);
        if (it == m_games.end()) {
            return false;
        }
        
        GameInfo& gameInfo = it->second;
        
        // Check if game is full or in progress
        if (gameInfo.currentPlayerCount >= gameInfo.maxPlayers || gameInfo.inProgress) {
            return false;
        }
        
        // Set player ID
        m_localPlayer->SetId(0); // Server player always has ID 0
        
        // Create game instance if needed
        if (!m_currentGame) {
            m_currentGame = CreateGameInstance(gameInfo.type);
            if (!m_currentGame) {
                return false;
            }
        }
        
        // Add player to game
        m_currentGame->AddPlayer(m_localPlayer);
        
        // Update player list
        {
            std::lock_guard<std::mutex> playersLock(m_playersMutex);
            m_playersInGame.clear();
            m_playersInGame.push_back(m_localPlayer);
        }
        
        // Update game info
        gameInfo.currentPlayerCount++;
        
        // Set as current game
        m_currentGameId = gameId;
        m_isHost = true; // Server is always host
        
        // Notify clients
        SendGameList();
        SendPlayerList(gameId);
        
        // Trigger callback
        if (m_lobbyUpdateCallback) {
            m_lobbyUpdateCallback();
        }
        
        return true;
    }
    
    return false;
}

void Lobby::LeaveGame()
{
    if (!m_networkManager || m_currentGameId < 0) {
        return;
    }
    
    if (m_networkManager->GetMode() == NetworkMode::CLIENT) {
        // Send leave request to server
        json j;
        j["command"] = "leave_game";
        j["game_id"] = m_currentGameId;
        
        m_networkManager->SendToServer(j.dump());
    }
    else if (m_networkManager->GetMode() == NetworkMode::SERVER) {
        // Server leaving a local game
        std::lock_guard<std::mutex> lock(m_gamesMutex);
        
        auto it = m_games.find(m_currentGameId);
        if (it != m_games.end()) {
            GameInfo& gameInfo = it->second;
            
            // Update game info
            gameInfo.currentPlayerCount--;
            
            // If last player, remove the game
            if (gameInfo.currentPlayerCount <= 0) {
                m_games.erase(it);
            }
            else {
                // Notify clients about player leaving
                SendPlayerList(m_currentGameId);
            }
            
            // Notify clients about game list update
            SendGameList();
        }
    }
    
    // Reset local state
    m_currentGameId = -1;
    m_isHost = false;
    
    // Clear players and current game
    {
        std::lock_guard<std::mutex> lock(m_playersMutex);
        m_playersInGame.clear();
        m_currentGame.reset();
    }
    
    // Trigger callback
    if (m_lobbyUpdateCallback) {
        m_lobbyUpdateCallback();
    }
}

bool Lobby::StartGame()
{
    if (!m_networkManager || m_currentGameId < 0 || !m_isHost || !m_currentGame) {
        return false;
    }
    
    // Start the game
    if (!m_currentGame->Start()) {
        return false;
    }
    
    if (m_networkManager->GetMode() == NetworkMode::SERVER) {
        // Update game info
        std::lock_guard<std::mutex> lock(m_gamesMutex);
        
        auto it = m_games.find(m_currentGameId);
        if (it != m_games.end()) {
            GameInfo& gameInfo = it->second;
            gameInfo.inProgress = true;
            
            // Notify clients about game starting
            json j;
            j["command"] = "start_game";
            j["game_id"] = m_currentGameId;
            j["game_state"] = m_currentGame->SerializeGameState();
            
            m_networkManager->SendToAllClients(j.dump());
            
            // Notify clients about game list update
            SendGameList();
        }
    }
    else if (m_networkManager->GetMode() == NetworkMode::CLIENT) {
        // Send start request to server
        json j;
        j["command"] = "start_game";
        j["game_id"] = m_currentGameId;
        
        m_networkManager->SendToServer(j.dump());
    }
    
    // Trigger game start callback
    if (m_gameStartCallback) {
        m_gameStartCallback(m_currentGameId, m_currentGame);
    }
    
    return true;
}

void Lobby::SetReady(bool ready)
{
    if (!m_localPlayer || m_currentGameId < 0) {
        return;
    }
    
    m_localPlayer->SetReady(ready);
    
    if (m_networkManager->GetMode() == NetworkMode::CLIENT) {
        // Send ready status to server
        json j;
        j["command"] = "set_ready";
        j["game_id"] = m_currentGameId;
        j["ready"] = ready;
        
        m_networkManager->SendToServer(j.dump());
    }
    else if (m_networkManager->GetMode() == NetworkMode::SERVER) {
        // Notify clients about player ready status
        SendPlayerList(m_currentGameId);
    }
}

std::shared_ptr<Core::Game> Lobby::GetCurrentGame() const
{
    std::lock_guard<std::mutex> lock(m_playersMutex);
    return m_currentGame;
}

std::vector<GameInfo> Lobby::GetAvailableGames() const
{
    std::vector<GameInfo> games;
    
    std::lock_guard<std::mutex> lock(m_gamesMutex);
    
    for (const auto& pair : m_games) {
        games.push_back(pair.second);
    }
    
    return games;
}

std::vector<std::shared_ptr<Core::Player>> Lobby::GetPlayersInGame() const
{
    std::lock_guard<std::mutex> lock(m_playersMutex);
    return m_playersInGame;
}

std::shared_ptr<Core::Player> Lobby::GetLocalPlayer() const
{
    return m_localPlayer;
}

bool Lobby::IsHost() const
{
    return m_isHost;
}

void Lobby::Update()
{
    // Process network messages
    if (m_networkManager) {
        m_networkManager->Update();
    }
}

void Lobby::SetLobbyUpdateCallback(LobbyUpdateCallback callback)
{
    m_lobbyUpdateCallback = callback;
}

void Lobby::SetGameStartCallback(GameStartCallback callback)
{
    m_gameStartCallback = callback;
}

void Lobby::HandleNetworkMessage(const std::string& message, int clientId)
{
    try {
        json j = json::parse(message);
        std::string command = j["command"];
        
        if (command == "get_games") {
            // Client requesting game list
            if (m_networkManager->GetMode() == NetworkMode::SERVER) {
                SendGameList(clientId);
            }
        }
        else if (command == "game_list") {
            // Server sending game list
            if (m_networkManager->GetMode() == NetworkMode::CLIENT) {
                std::lock_guard<std::mutex> lock(m_gamesMutex);
                m_games.clear();
                
                json games = j["games"];
                for (const auto& gameJson : games) {
                    GameInfo gameInfo = GameInfo::Deserialize(gameJson.dump());
                    m_games[gameInfo.id] = gameInfo;
                }
                
                // Trigger callback
                if (m_lobbyUpdateCallback) {
                    m_lobbyUpdateCallback();
                }
            }
        }
        else if (command == "join_game") {
            // Client requesting to join a game
            if (m_networkManager->GetMode() == NetworkMode::SERVER) {
                int gameId = j["game_id"];
                std::string playerName = j["player_name"];
                
                std::lock_guard<std::mutex> lock(m_gamesMutex);
                
                auto it = m_games.find(gameId);
                if (it != m_games.end()) {
                    GameInfo& gameInfo = it->second;
                    
                    // Check if game is full or in progress
                    if (gameInfo.currentPlayerCount < gameInfo.maxPlayers && !gameInfo.inProgress) {
                        // Create game instance if needed
                        if (!m_currentGame || m_currentGameId != gameId) {
                            m_currentGame = CreateGameInstance(gameInfo.type);
                            m_currentGameId = gameId;
                        }
                        
                        if (m_currentGame) {
                            // Create player
                            auto player = std::make_shared<Core::Player>(playerName);
                            player->SetId(clientId);
                            player->SetConnected(true);
                            
                            // Add player to game
                            m_currentGame->AddPlayer(player);
                            
                            // Update player list
                            {
                                std::lock_guard<std::mutex> playersLock(m_playersMutex);
                                m_playersInGame.push_back(player);
                            }
                            
                            // Update game info
                            gameInfo.currentPlayerCount++;
                            
                            // Send join success response
                            json response;
                            response["command"] = "join_game_response";
                            response["success"] = true;
                            response["game_id"] = gameId;
                            response["player_id"] = clientId;
                            
                            m_networkManager->SendToClient(clientId, response.dump());
                            
                            // Notify all clients about updated game and player list
                            SendGameList();
                            SendPlayerList(gameId);
                        }
                    }
                    else {
                        // Send join failure response
                        json response;
                        response["command"] = "join_game_response";
                        response["success"] = false;
                        response["game_id"] = gameId;
                        response["error"] = gameInfo.inProgress ? "Game in progress" : "Game is full";
                        
                        m_networkManager->SendToClient(clientId, response.dump());
                    }
                }
                else {
                    // Send join failure response - game not found
                    json response;
                    response["command"] = "join_game_response";
                    response["success"] = false;
                    response["game_id"] = gameId;
                    response["error"] = "Game not found";
                    
                    m_networkManager->SendToClient(clientId, response.dump());
                }
            }
        }
        else if (command == "join_game_response") {
            // Server response to join request
            if (m_networkManager->GetMode() == NetworkMode::CLIENT) {
                bool success = j["success"];
                int gameId = j["game_id"];
                
                if (success) {
                    int playerId = j["player_id"];
                    
                    // Set player ID
                    if (m_localPlayer) {
                        m_localPlayer->SetId(playerId);
                        m_localPlayer->SetConnected(true);
                    }
                    
                    // Set current game
                    m_currentGameId = gameId;
                    m_isHost = false;
                    
                    // Request player list
                    json requestPlayers;
                    requestPlayers["command"] = "get_players";
                    requestPlayers["game_id"] = gameId;
                    
                    m_networkManager->SendToServer(requestPlayers.dump());
                }
                else {
                    std::string error = j["error"];
                    std::cerr << "Failed to join game: " << error << std::endl;
                    
                    // Reset local player
                    m_localPlayer.reset();
                }
                
                // Trigger callback
                if (m_lobbyUpdateCallback) {
                    m_lobbyUpdateCallback();
                }
            }
        }
        else if (command == "leave_game") {
            // Client leaving a game
            if (m_networkManager->GetMode() == NetworkMode::SERVER) {
                int gameId = j["game_id"];
                
                std::lock_guard<std::mutex> lock(m_gamesMutex);
                
                auto it = m_games.find(gameId);
                if (it != m_games.end()) {
                    GameInfo& gameInfo = it->second;
                    
                    // Remove player from game
                    if (m_currentGame && m_currentGameId == gameId) {
                        m_currentGame->RemovePlayer(clientId);
                        
                        // Update player list
                        {
                            std::lock_guard<std::mutex> playersLock(m_playersMutex);
                            m_playersInGame.erase(
                                std::remove_if(m_playersInGame.begin(), m_playersInGame.end(),
                                             [clientId](const std::shared_ptr<Core::Player>& player) {
                                                 return player->GetId() == clientId;
                                             }),
                                m_playersInGame.end());
                        }
                        
                        // Update game info
                        gameInfo.currentPlayerCount--;
                        
                        // If no players left, remove the game
                        if (gameInfo.currentPlayerCount <= 0) {
                            m_games.erase(it);
                            m_currentGame.reset();
                            m_currentGameId = -1;
                        }
                        else {
                            // Notify clients about player leaving
                            SendPlayerList(gameId);
                        }
                        
                        // Notify clients about game list update
                        SendGameList();
                    }
                }
            }
        }
        else if (command == "get_players") {
            // Client requesting player list
            if (m_networkManager->GetMode() == NetworkMode::SERVER) {
                int gameId = j["game_id"];
                SendPlayerList(gameId, clientId);
            }
        }
        else if (command == "player_list") {
            // Server sending player list
            if (m_networkManager->GetMode() == NetworkMode::CLIENT) {
                int gameId = j["game_id"];
                
                if (gameId == m_currentGameId) {
                    // Parse player list
                    std::lock_guard<std::mutex> lock(m_playersMutex);
                    m_playersInGame.clear();
                    
                    json players = j["players"];
                    for (const auto& playerJson : players) {
                        std::string name = playerJson["name"];
                        int id = playerJson["id"];
                        bool ready = playerJson["ready"];
                        bool isHost = playerJson["host"];
                        
                        auto player = std::make_shared<Core::Player>(name);
                        player->SetId(id);
                        player->SetConnected(true);
                        player->SetReady(ready);
                        
                        m_playersInGame.push_back(player);
                        
                        // Check if local player is host
                        if (id == m_localPlayer->GetId() && isHost) {
                            m_isHost = true;
                        }
                    }
                    
                    // Create game instance if needed
                    if (!m_currentGame) {
                        std::lock_guard<std::mutex> gamesLock(m_gamesMutex);
                        auto it = m_games.find(gameId);
                        if (it != m_games.end()) {
                            m_currentGame = CreateGameInstance(it->second.type);
                            
                            // Add all players to the game
                            for (const auto& player : m_playersInGame) {
                                m_currentGame->AddPlayer(player);
                            }
                        }
                    }
                    
                    // Trigger callback
                    if (m_lobbyUpdateCallback) {
                        m_lobbyUpdateCallback();
                    }
                }
            }
        }
        else if (command == "set_ready") {
            // Client setting ready status
            if (m_networkManager->GetMode() == NetworkMode::SERVER) {
                int gameId = j["game_id"];
                bool ready = j["ready"];
                
                if (m_currentGameId == gameId && m_currentGame) {
                    auto player = m_currentGame->GetPlayer(clientId);
                    if (player) {
                        player->SetReady(ready);
                        
                        // Notify all clients
                        SendPlayerList(gameId);
                    }
                }
            }
        }
        else if (command == "start_game") {
            // Game start request or notification
            if (m_networkManager->GetMode() == NetworkMode::SERVER) {
                // Client requesting to start a game
                int gameId = j["game_id"];
                
                if (m_currentGameId == gameId && m_currentGame) {
                    // Check if all players are ready
                    bool allReady = true;
                    for (const auto& player : m_currentGame->GetPlayers()) {
                        if (!player->IsReady() && player->GetId() != 0) { // Skip the host
                            allReady = false;
                            break;
                        }
                    }
                    
                    if (allReady) {
                        // Start the game
                        if (m_currentGame->Start()) {
                            // Update game info
                            std::lock_guard<std::mutex> lock(m_gamesMutex);
                            
                            auto it = m_games.find(gameId);
                            if (it != m_games.end()) {
                                GameInfo& gameInfo = it->second;
                                gameInfo.inProgress = true;
                                
                                // Notify clients about game starting
                                json response;
                                response["command"] = "start_game";
                                response["game_id"] = gameId;
                                response["game_state"] = m_currentGame->SerializeGameState();
                                
                                m_networkManager->SendToAllClients(response.dump());
                                
                                // Notify clients about game list update
                                SendGameList();
                                
                                // Trigger game start callback
                                if (m_gameStartCallback) {
                                    m_gameStartCallback(gameId, m_currentGame);
                                }
                            }
                        }
                    }
                }
            }
            else if (m_networkManager->GetMode() == NetworkMode::CLIENT) {
                // Server notifying game has started
                int gameId = j["game_id"];
                std::string gameState = j["game_state"];
                
                if (m_currentGameId == gameId && m_currentGame) {
                    // Deserialize game state
                    m_currentGame->DeserializeGameState(gameState);
                    
                    // Trigger game start callback
                    if (m_gameStartCallback) {
                        m_gameStartCallback(gameId, m_currentGame);
                    }
                }
            }
        }
        else if (command == "game_move") {
            // Game move
            int gameId = j["game_id"];
            int playerId = j["player_id"];
            std::string moveData = j["move_data"];
            
            if (m_currentGameId == gameId && m_currentGame) {
                if (m_networkManager->GetMode() == NetworkMode::SERVER) {
                    // Client sent a move to the server
                    if (m_currentGame->IsValidMove(moveData)) {
                        // Process the move
                        if (m_currentGame->MakeMove(playerId, moveData)) {
                            // Broadcast move to all clients including the sender
                            json moveNotification;
                            moveNotification["command"] = "game_move";
                            moveNotification["game_id"] = gameId;
                            moveNotification["player_id"] = playerId;
                            moveNotification["move_data"] = moveData;
                            moveNotification["game_state"] = m_currentGame->SerializeGameState();
                            
                            m_networkManager->SendToAllClients(moveNotification.dump());
                        }
                    }
                }
                else if (m_networkManager->GetMode() == NetworkMode::CLIENT) {
                    // Server broadcasting a move
                    std::string gameState = j["game_state"];
                    
                    // Update local game state
                    m_currentGame->DeserializeGameState(gameState);
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error processing network message: " << e.what() << std::endl;
    }
}

void Lobby::SendGameList(int clientId)
{
    if (m_networkManager->GetMode() != NetworkMode::SERVER) {
        return;
    }
    
    // Prepare the game list
    json j;
    j["command"] = "game_list";
    
    json games = json::array();
    
    {
        std::lock_guard<std::mutex> lock(m_gamesMutex);
        for (const auto& pair : m_games) {
            games.push_back(json::parse(pair.second.Serialize()));
        }
    }
    
    j["games"] = games;
    
    // Send to specific client or all clients
    if (clientId >= 0) {
        m_networkManager->SendToClient(clientId, j.dump());
    } else {
        m_networkManager->SendToAllClients(j.dump());
    }
}

void Lobby::SendPlayerList(int gameId, int clientId)
{
    if (m_networkManager->GetMode() != NetworkMode::SERVER || m_currentGameId != gameId) {
        return;
    }
    
    // Prepare the player list
    json j;
    j["command"] = "player_list";
    j["game_id"] = gameId;
    
    json players = json::array();
    
    {
        std::lock_guard<std::mutex> lock(m_playersMutex);
        for (const auto& player : m_playersInGame) {
            json playerJson;
            playerJson["id"] = player->GetId();
            playerJson["name"] = player->GetName();
            playerJson["ready"] = player->IsReady();
            playerJson["host"] = (player->GetId() == 0); // Server player is always host
            
            players.push_back(playerJson);
        }
    }
    
    j["players"] = players;
    
    // Send to specific client or all clients
    if (clientId >= 0) {
        m_networkManager->SendToClient(clientId, j.dump());
    } else {
        m_networkManager->SendToAllClients(j.dump());
    }
}

void Lobby::SendGameState(int gameId, int clientId)
{
    if (m_networkManager->GetMode() != NetworkMode::SERVER || m_currentGameId != gameId || !m_currentGame) {
        return;
    }
    
    // Prepare the game state
    json j;
    j["command"] = "game_state";
    j["game_id"] = gameId;
    j["game_state"] = m_currentGame->SerializeGameState();
    
    // Send to specific client or all clients
    if (clientId >= 0) {
        m_networkManager->SendToClient(clientId, j.dump());
    } else {
        m_networkManager->SendToAllClients(j.dump());
    }
}

std::shared_ptr<Core::Game> Lobby::CreateGameInstance(Core::GameType type)
{
    switch (type) {
        case Core::GameType::SOLITAIRE_KLONDIKE:
            return std::make_shared<Games::Solitaire::Klondike>();
            
        case Core::GameType::SOLITAIRE_FREECELL:
            return std::make_shared<Games::Solitaire::FreeCell>();
            
        case Core::GameType::SOLITAIRE_SPIDER:
            return std::make_shared<Games::Solitaire::Spider>();
            
        default:
            return nullptr;
    }
}

} // namespace Network
} // namespace CardGameLib
