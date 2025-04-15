#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <memory>

#include "network/Socket.h"

namespace CardGameLib {
namespace Network {

// Define message callback type for server
using ServerMessageCallback = std::function<void(const std::string& message, int clientId)>;

// Client connection info
struct ClientInfo {
    int id;
    std::shared_ptr<Socket> socket;
    std::string address;
    int port;
    
    ClientInfo(int id, std::shared_ptr<Socket> socket, const std::string& address, int port)
        : id(id), socket(socket), address(address), port(port) {}
};

class GameServer {
public:
    GameServer();
    ~GameServer();
    
    // Start the server on the specified port
    bool Start(int port);
    
    // Stop the server
    void Stop();
    
    // Check if the server is running
    bool IsRunning() const;
    
    // Send a message to a specific client
    bool SendToClient(int clientId, const std::string& message);
    
    // Send a message to all connected clients
    bool SendToAllClients(const std::string& message);
    
    // Disconnect a specific client
    void DisconnectClient(int clientId);
    
    // Get a list of connected client IDs
    std::vector<int> GetConnectedClientIds() const;
    
    // Set callback for incoming messages
    void SetMessageCallback(ServerMessageCallback callback);
    
private:
    // Server state
    std::atomic<bool> m_running;
    std::atomic<bool> m_stopping;
    int m_port;
    
    // Socket for accepting connections
    std::shared_ptr<Socket> m_listenSocket;
    
    // Connected clients
    std::unordered_map<int, std::shared_ptr<ClientInfo>> m_clients;
    mutable std::mutex m_clientsMutex;  // mutable to allow locking in const methods
    int m_nextClientId;
    
    // Message callback
    ServerMessageCallback m_messageCallback;
    
    // Server thread
    std::thread m_serverThread;
    
    // Server loop
    void ServerLoop();
    
    // Accept new connections
    void AcceptConnections(std::vector<Socket*>& readySockets);
    
    // Receive messages from clients
    void ReceiveMessages(std::vector<Socket*>& readySockets);
    
    // Add a new client
    int AddClient(std::shared_ptr<Socket> socket);
    
    // Remove a client
    void RemoveClient(int clientId);
};

} // namespace Network
} // namespace CardGameLib
