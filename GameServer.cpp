#include "network/GameServer.h"
#include <iostream>

namespace CardGameLib {
namespace Network {

GameServer::GameServer()
    : m_running(false)
    , m_stopping(false)
    , m_port(0)
    , m_nextClientId(1)
{
}

GameServer::~GameServer()
{
    Stop();
}

bool GameServer::Start(int port)
{
    // Already running?
    if (m_running.load()) {
        return false;
    }
    
    m_port = port;
    
    // Create the listen socket
    m_listenSocket = std::make_shared<Socket>();
    if (!m_listenSocket->Create()) {
        std::cerr << "Failed to create listen socket" << std::endl;
        return false;
    }
    
    // Set socket options
    m_listenSocket->SetReuseAddr(true);
    m_listenSocket->SetNonBlocking(true);
    
    // Bind to the port
    if (!m_listenSocket->Bind(port)) {
        std::cerr << "Failed to bind listen socket to port " << port << std::endl;
        m_listenSocket.reset();
        return false;
    }
    
    // Start listening
    if (!m_listenSocket->Listen()) {
        std::cerr << "Failed to listen on port " << port << std::endl;
        m_listenSocket.reset();
        return false;
    }
    
    // Start the server thread
    m_running = true;
    m_stopping = false;
    m_serverThread = std::thread(&GameServer::ServerLoop, this);
    
    return true;
}

void GameServer::Stop()
{
    if (!m_running.load()) {
        return;
    }
    
    // Signal the server thread to stop
    m_stopping = true;
    
    // Wait for the server thread to finish
    if (m_serverThread.joinable()) {
        m_serverThread.join();
    }
    
    // Close the listen socket
    if (m_listenSocket) {
        m_listenSocket->Close();
        m_listenSocket.reset();
    }
    
    // Close all client connections
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        for (auto& client : m_clients) {
            if (client.second && client.second->socket) {
                client.second->socket->Close();
            }
        }
        m_clients.clear();
    }
    
    m_running = false;
}

bool GameServer::IsRunning() const
{
    return m_running.load();
}

bool GameServer::SendToClient(int clientId, const std::string& message)
{
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    
    auto it = m_clients.find(clientId);
    if (it == m_clients.end() || !it->second || !it->second->socket) {
        return false;
    }
    
    // Prepend message length as a 4-byte integer
    uint32_t length = static_cast<uint32_t>(message.size());
    std::string finalMessage;
    finalMessage.resize(4 + message.size());
    
    // Store the length in network byte order (big endian)
    finalMessage[0] = static_cast<char>((length >> 24) & 0xFF);
    finalMessage[1] = static_cast<char>((length >> 16) & 0xFF);
    finalMessage[2] = static_cast<char>((length >> 8) & 0xFF);
    finalMessage[3] = static_cast<char>(length & 0xFF);
    
    // Copy the message
    std::copy(message.begin(), message.end(), finalMessage.begin() + 4);
    
    // Send the message
    int bytesSent = it->second->socket->Send(finalMessage);
    
    return bytesSent == static_cast<int>(finalMessage.size());
}

bool GameServer::SendToAllClients(const std::string& message)
{
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    
    bool allSent = true;
    
    for (auto& client : m_clients) {
        if (client.second && client.second->socket) {
            if (!SendToClient(client.first, message)) {
                allSent = false;
            }
        }
    }
    
    return allSent;
}

void GameServer::DisconnectClient(int clientId)
{
    RemoveClient(clientId);
}

std::vector<int> GameServer::GetConnectedClientIds() const
{
    std::vector<int> clientIds;
    
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    
    for (const auto& client : m_clients) {
        clientIds.push_back(client.first);
    }
    
    return clientIds;
}

void GameServer::SetMessageCallback(ServerMessageCallback callback)
{
    m_messageCallback = callback;
}

void GameServer::ServerLoop()
{
    // Main server loop
    while (!m_stopping.load()) {
        // Set up the sockets for select
        std::vector<Socket*> readSockets;
        std::vector<Socket*> writeSockets;
        
        // Add the listen socket for reading
        readSockets.push_back(m_listenSocket.get());
        
        // Add all client sockets for reading
        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);
            for (auto& client : m_clients) {
                if (client.second && client.second->socket) {
                    readSockets.push_back(client.second->socket.get());
                }
            }
        }
        
        // Wait for activity (with a timeout)
        bool activity = Socket::Select(readSockets, writeSockets, 0.1);
        
        if (activity) {
            // Check for new connections
            AcceptConnections(readSockets);
            
            // Check for client messages
            ReceiveMessages(readSockets);
        }
    }
    
    m_running = false;
}

void GameServer::AcceptConnections(std::vector<Socket*>& readySockets)
{
    // Check if the listen socket is ready for reading (new connection)
    auto it = std::find(readySockets.begin(), readySockets.end(), m_listenSocket.get());
    if (it != readySockets.end()) {
        // Accept the new connection
        Socket* clientSocket = m_listenSocket->Accept();
        if (clientSocket) {
            // Set non-blocking mode
            clientSocket->SetNonBlocking(true);
            
            // Add the client
            int clientId = AddClient(std::shared_ptr<Socket>(clientSocket));
            
            std::cout << "New client connected: ID=" << clientId 
                    << ", Address=" << clientSocket->GetRemoteAddress() 
                    << ", Port=" << clientSocket->GetRemotePort() << std::endl;
        }
    }
}

void GameServer::ReceiveMessages(std::vector<Socket*>& readySockets)
{
    // Check for messages from clients
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    
    for (auto& client : m_clients) {
        if (!client.second || !client.second->socket) {
            continue;
        }
        
        Socket* socket = client.second->socket.get();
        
        // Check if this socket is ready for reading
        auto it = std::find(readySockets.begin(), readySockets.end(), socket);
        if (it != readySockets.end()) {
            // Read the message length (4 bytes)
            char lengthBuffer[4];
            int bytesRead = socket->Receive(lengthBuffer, 4);
            
            if (bytesRead <= 0) {
                // Client disconnected or error
                std::cout << "Client disconnected: ID=" << client.first << std::endl;
                // Mark for removal
                client.second->socket.reset();
                continue;
            }
            
            if (bytesRead == 4) {
                // Extract the message length
                uint32_t messageLength = 
                    (static_cast<uint32_t>(lengthBuffer[0] & 0xFF) << 24) |
                    (static_cast<uint32_t>(lengthBuffer[1] & 0xFF) << 16) |
                    (static_cast<uint32_t>(lengthBuffer[2] & 0xFF) << 8) |
                    (static_cast<uint32_t>(lengthBuffer[3] & 0xFF));
                
                // Read the message content
                std::vector<char> messageBuffer(messageLength);
                bytesRead = socket->Receive(messageBuffer.data(), messageLength);
                
                if (bytesRead == static_cast<int>(messageLength)) {
                    // Process the message
                    std::string message(messageBuffer.data(), messageLength);
                    
                    // Call the message callback
                    if (m_messageCallback) {
                        m_messageCallback(message, client.first);
                    }
                }
            }
        }
    }
    
    // Remove disconnected clients
    for (auto it = m_clients.begin(); it != m_clients.end();) {
        if (!it->second || !it->second->socket) {
            it = m_clients.erase(it);
        } else {
            ++it;
        }
    }
}

int GameServer::AddClient(std::shared_ptr<Socket> socket)
{
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    
    int clientId = m_nextClientId++;
    std::string address = socket->GetRemoteAddress();
    int port = socket->GetRemotePort();
    
    m_clients[clientId] = std::make_shared<ClientInfo>(clientId, socket, address, port);
    
    return clientId;
}

void GameServer::RemoveClient(int clientId)
{
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    
    auto it = m_clients.find(clientId);
    if (it != m_clients.end()) {
        if (it->second && it->second->socket) {
            it->second->socket->Close();
        }
        m_clients.erase(it);
    }
}

} // namespace Network
} // namespace CardGameLib
