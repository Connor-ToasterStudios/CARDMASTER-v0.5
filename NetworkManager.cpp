#include "network/NetworkManager.h"

namespace CardGameLib {
namespace Network {

NetworkManager::NetworkManager()
    : m_mode(NetworkMode::NONE)
{
}

NetworkManager::~NetworkManager()
{
    Shutdown();
}

bool NetworkManager::Initialize()
{
    // Initialize socket subsystem
    if (!Socket::InitializeSocketSystem()) {
        return false;
    }
    
    return true;
}

void NetworkManager::Shutdown()
{
    // Stop any active connections
    StopServer();
    Disconnect();
    
    // Clean up socket subsystem
    Socket::ShutdownSocketSystem();
}

bool NetworkManager::StartServer(int port)
{
    // Cannot start server if already in client mode
    if (m_mode == NetworkMode::CLIENT) {
        return false;
    }
    
    // Create server
    m_server = std::make_unique<GameServer>();
    
    // Set up server callback
    m_server->SetMessageCallback([this](const std::string& message, int clientId) {
        OnServerMessage(message, clientId);
    });
    
    // Start server
    if (!m_server->Start(port)) {
        m_server.reset();
        return false;
    }
    
    m_mode = NetworkMode::SERVER;
    return true;
}

void NetworkManager::StopServer()
{
    if (m_mode == NetworkMode::SERVER && m_server) {
        m_server->Stop();
        m_server.reset();
        m_mode = NetworkMode::NONE;
    }
}

bool NetworkManager::IsServerRunning() const
{
    return m_mode == NetworkMode::SERVER && m_server && m_server->IsRunning();
}

bool NetworkManager::Connect(const std::string& host, int port)
{
    // Cannot connect if already in server mode
    if (m_mode == NetworkMode::SERVER) {
        return false;
    }
    
    // Create client
    m_client = std::make_unique<GameClient>();
    
    // Set up client callback
    m_client->SetMessageCallback([this](const std::string& message) {
        OnClientMessage(message);
    });
    
    // Connect to server
    if (!m_client->Connect(host, port)) {
        m_client.reset();
        return false;
    }
    
    m_mode = NetworkMode::CLIENT;
    return true;
}

void NetworkManager::Disconnect()
{
    if (m_mode == NetworkMode::CLIENT && m_client) {
        m_client->Disconnect();
        m_client.reset();
        m_mode = NetworkMode::NONE;
    }
}

bool NetworkManager::IsConnected() const
{
    return m_mode == NetworkMode::CLIENT && m_client && m_client->IsConnected();
}

bool NetworkManager::SendToServer(const std::string& message)
{
    if (m_mode != NetworkMode::CLIENT || !m_client) {
        return false;
    }
    
    return m_client->SendMessage(message);
}

bool NetworkManager::SendToClient(int clientId, const std::string& message)
{
    if (m_mode != NetworkMode::SERVER || !m_server) {
        return false;
    }
    
    return m_server->SendToClient(clientId, message);
}

bool NetworkManager::SendToAllClients(const std::string& message)
{
    if (m_mode != NetworkMode::SERVER || !m_server) {
        return false;
    }
    
    return m_server->SendToAllClients(message);
}

void NetworkManager::RegisterMessageCallback(NetworkMessageCallback callback)
{
    m_messageCallbacks.push_back(callback);
}

void NetworkManager::Update()
{
    // Process any queued messages
    std::queue<std::pair<std::string, int>> messagesToProcess;
    
    {
        std::lock_guard<std::mutex> lock(m_messageMutex);
        messagesToProcess.swap(m_messageQueue);
    }
    
    // Process all messages
    while (!messagesToProcess.empty()) {
        const auto& [message, clientId] = messagesToProcess.front();
        
        // Notify all registered callbacks
        for (const auto& callback : m_messageCallbacks) {
            callback(message, clientId);
        }
        
        messagesToProcess.pop();
    }
}

NetworkMode NetworkManager::GetMode() const
{
    return m_mode;
}

int NetworkManager::GetClientId() const
{
    if (m_mode == NetworkMode::CLIENT && m_client) {
        return m_client->GetClientId();
    }
    return -1;
}

std::vector<int> NetworkManager::GetConnectedClientIds() const
{
    if (m_mode == NetworkMode::SERVER && m_server) {
        return m_server->GetConnectedClientIds();
    }
    return {};
}

void NetworkManager::OnServerMessage(const std::string& message, int clientId)
{
    // Queue the message for processing in the main thread
    std::lock_guard<std::mutex> lock(m_messageMutex);
    m_messageQueue.push(std::make_pair(message, clientId));
}

void NetworkManager::OnClientMessage(const std::string& message)
{
    // Queue the message for processing in the main thread
    // clientId -1 indicates it's from the server
    std::lock_guard<std::mutex> lock(m_messageMutex);
    m_messageQueue.push(std::make_pair(message, -1));
}

} // namespace Network
} // namespace CardGameLib
