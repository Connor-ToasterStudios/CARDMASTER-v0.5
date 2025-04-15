#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <queue>

#include "network/Socket.h"
#include "network/GameServer.h"
#include "network/GameClient.h"

namespace CardGameLib {
namespace Network {

enum class NetworkMode {
    NONE,
    CLIENT,
    SERVER
};

// Message callback type
using NetworkMessageCallback = std::function<void(const std::string& message, int clientId)>;

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();
    
    // Initialization
    bool Initialize();
    void Shutdown();
    
    // Server management
    bool StartServer(int port);
    void StopServer();
    bool IsServerRunning() const;
    
    // Client management
    bool Connect(const std::string& host, int port);
    void Disconnect();
    bool IsConnected() const;
    
    // Send messages
    bool SendToServer(const std::string& message);
    bool SendToClient(int clientId, const std::string& message);
    bool SendToAllClients(const std::string& message);
    
    // Message handling
    void RegisterMessageCallback(NetworkMessageCallback callback);
    void Update(); // Process incoming messages
    
    // State access
    NetworkMode GetMode() const;
    
    // Client info
    int GetClientId() const;
    std::vector<int> GetConnectedClientIds() const;
    
private:
    NetworkMode m_mode;
    std::unique_ptr<GameServer> m_server;
    std::unique_ptr<GameClient> m_client;
    
    // Callback for network messages
    std::vector<NetworkMessageCallback> m_messageCallbacks;
    
    // Message queue for thread-safe message handling
    std::queue<std::pair<std::string, int>> m_messageQueue;
    std::mutex m_messageMutex;
    
    // Internal callback handlers
    void OnServerMessage(const std::string& message, int clientId);
    void OnClientMessage(const std::string& message);
};

} // namespace Network
} // namespace CardGameLib
