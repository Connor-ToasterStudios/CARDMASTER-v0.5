#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <memory>

#include "network/Socket.h"

namespace CardGameLib {
namespace Network {

// Define message callback type for client
using ClientMessageCallback = std::function<void(const std::string& message)>;

class GameClient {
public:
    GameClient();
    ~GameClient();
    
    // Connect to a server
    bool Connect(const std::string& host, int port);
    
    // Disconnect from the server
    void Disconnect();
    
    // Check if connected to a server
    bool IsConnected() const;
    
    // Send a message to the server
    bool SendMessage(const std::string& message);
    
    // Set callback for incoming messages
    void SetMessageCallback(ClientMessageCallback callback);
    
    // Get client ID (assigned by server)
    int GetClientId() const { return m_clientId; }
    
    // Set client ID (called when server assigns an ID)
    void SetClientId(int id) { m_clientId = id; }
    
private:
    // Connection state
    std::atomic<bool> m_connected;
    std::atomic<bool> m_stopping;
    int m_clientId;
    
    // Server info
    std::string m_serverHost;
    int m_serverPort;
    
    // Socket for server communication
    std::shared_ptr<Socket> m_socket;
    
    // Message callback
    ClientMessageCallback m_messageCallback;
    
    // Receiving thread
    std::thread m_receiveThread;
    
    // Thread function for receiving messages
    void ReceiveLoop();
};

} // namespace Network
} // namespace CardGameLib
