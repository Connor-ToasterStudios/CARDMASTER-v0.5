#include "network/GameClient.h"
#include <iostream>
#include <vector>

namespace CardGameLib {
namespace Network {

GameClient::GameClient()
    : m_connected(false)
    , m_stopping(false)
    , m_clientId(-1)
    , m_serverPort(0)
{
}

GameClient::~GameClient()
{
    Disconnect();
}

bool GameClient::Connect(const std::string& host, int port)
{
    // Already connected?
    if (m_connected.load()) {
        return false;
    }
    
    m_serverHost = host;
    m_serverPort = port;
    
    // Create the socket
    m_socket = std::make_shared<Socket>();
    if (!m_socket->Create()) {
        std::cerr << "Failed to create client socket" << std::endl;
        return false;
    }
    
    // Connect to the server
    if (!m_socket->Connect(host, port)) {
        std::cerr << "Failed to connect to " << host << ":" << port << std::endl;
        m_socket.reset();
        return false;
    }
    
    // Set non-blocking mode
    m_socket->SetNonBlocking(true);
    
    // Start the receive thread
    m_connected = true;
    m_stopping = false;
    m_receiveThread = std::thread(&GameClient::ReceiveLoop, this);
    
    return true;
}

void GameClient::Disconnect()
{
    if (!m_connected.load()) {
        return;
    }
    
    // Signal the receive thread to stop
    m_stopping = true;
    
    // Wait for the receive thread to finish
    if (m_receiveThread.joinable()) {
        m_receiveThread.join();
    }
    
    // Close the socket
    if (m_socket) {
        m_socket->Close();
        m_socket.reset();
    }
    
    m_connected = false;
}

bool GameClient::IsConnected() const
{
    return m_connected.load();
}

bool GameClient::SendMessage(const std::string& message)
{
    if (!m_connected.load() || !m_socket) {
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
    int bytesSent = m_socket->Send(finalMessage);
    
    return bytesSent == static_cast<int>(finalMessage.size());
}

void GameClient::SetMessageCallback(ClientMessageCallback callback)
{
    m_messageCallback = callback;
}

void GameClient::ReceiveLoop()
{
    while (!m_stopping.load() && m_socket && m_socket->IsValid()) {
        // Set up the sockets for select
        std::vector<Socket*> readSockets = { m_socket.get() };
        std::vector<Socket*> writeSockets;
        
        // Wait for activity (with a timeout)
        bool activity = Socket::Select(readSockets, writeSockets, 0.1);
        
        if (activity && !readSockets.empty()) {
            // Read the message length (4 bytes)
            char lengthBuffer[4];
            int bytesRead = m_socket->Receive(lengthBuffer, 4);
            
            if (bytesRead <= 0) {
                // Server disconnected or error
                std::cout << "Disconnected from server" << std::endl;
                break;
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
                bytesRead = m_socket->Receive(messageBuffer.data(), messageLength);
                
                if (bytesRead == static_cast<int>(messageLength)) {
                    // Process the message
                    std::string message(messageBuffer.data(), messageLength);
                    
                    // Call the message callback
                    if (m_messageCallback) {
                        m_messageCallback(message);
                    }
                }
            }
        }
    }
    
    m_connected = false;
}

} // namespace Network
} // namespace CardGameLib
