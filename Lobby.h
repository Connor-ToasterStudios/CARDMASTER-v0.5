#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <mutex>

#include "core/Player.h"
#include "core/Game.h"
#include "network/NetworkManager.h"

namespace CardGameLib {
namespace Network {

// Game info for the lobby
struct GameInfo {
    int id;
    std::string name;
    Core::GameType type;
    int maxPlayers;
    int currentPlayerCount;
    bool inProgress;
    
    GameInfo() : id(0), type(Core::GameType::SOLITAIRE_KLONDIKE), maxPlayers(1), currentPlayerCount(0), inProgress(false) {}
    
    GameInfo(int id, const std::string& name, Core::GameType type, int maxPlayers, int currentPlayerCount, bool inProgress)
        : id(id), name(name), type(type), maxPlayers(maxPlayers), currentPlayerCount(currentPlayerCount), inProgress(inProgress) {}
    
    // Serialize to string
    std::string Serialize() const;
    
    // Deserialize from string
    static GameInfo Deserialize(const std::string& data);
};

// Define callback types
using LobbyUpdateCallback = std::function<void()>;
using GameStartCallback = std::function<void(int gameId, std::shared_ptr<Core::Game> game)>;

class Lobby {
public:
    Lobby(NetworkManager* networkManager);
    ~Lobby();
    
    // Initialize lobby
    void Initialize();
    
    // Start a game server
    bool StartServer(int port);
    
    // Connect to a game server
    bool ConnectToServer(const std::string& host, int port);
    
    // Disconnect from server
    void Disconnect();
    
    // Create a new game (server only)
    int CreateGame(const std::string& name, Core::GameType type, int maxPlayers);
    
    // Join a game
    bool JoinGame(int gameId, const std::string& playerName);
    
    // Leave current game
    void LeaveGame();
    
    // Start current game (host only)
    bool StartGame();
    
    // Set ready status
    void SetReady(bool ready);
    
    // Get current game
    std::shared_ptr<Core::Game> GetCurrentGame() const;
    
    // Get list of available games
    std::vector<GameInfo> GetAvailableGames() const;
    
    // Get list of players in current game
    std::vector<std::shared_ptr<Core::Player>> GetPlayersInGame() const;
    
    // Get local player
    std::shared_ptr<Core::Player> GetLocalPlayer() const;
    
    // Is local player host of current game
    bool IsHost() const;
    
    // Process network messages
    void Update();
    
    // Set callbacks
    void SetLobbyUpdateCallback(LobbyUpdateCallback callback);
    void SetGameStartCallback(GameStartCallback callback);
    
private:
    // Network manager
    NetworkManager* m_networkManager;
    
    // Local player info
    std::shared_ptr<Core::Player> m_localPlayer;
    int m_currentGameId;
    bool m_isHost;
    
    // Game info (server only)
    std::unordered_map<int, GameInfo> m_games;
    int m_nextGameId;
    
    // Current game (if joined)
    std::shared_ptr<Core::Game> m_currentGame;
    std::vector<std::shared_ptr<Core::Player>> m_playersInGame;
    
    // Synchronization
    mutable std::mutex m_gamesMutex;
    mutable std::mutex m_playersMutex;
    
    // Callbacks
    LobbyUpdateCallback m_lobbyUpdateCallback;
    GameStartCallback m_gameStartCallback;
    
    // Message handling
    void HandleNetworkMessage(const std::string& message, int clientId);
    
    // Message sending helpers
    void SendGameList(int clientId = -1);
    void SendPlayerList(int gameId, int clientId = -1);
    void SendGameState(int gameId, int clientId = -1);
    
    // Create a new game instance based on type
    std::shared_ptr<Core::Game> CreateGameInstance(Core::GameType type);
};

} // namespace Network
} // namespace CardGameLib
