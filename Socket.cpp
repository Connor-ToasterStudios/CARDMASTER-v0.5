#include "network/Socket.h"
#include <cstring>
#include <iostream>

namespace CardGameLib {
namespace Network {

Socket::Socket()
    : m_handle(INVALID_SOCKET_HANDLE)
    , m_isBlocking(true)
{
}

Socket::~Socket()
{
    Close();
}

bool Socket::InitializeSocketSystem()
{
#ifdef PLATFORM_WINDOWS
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
#else
    return true; // No initialization needed on Unix systems
#endif
}

void Socket::ShutdownSocketSystem()
{
#ifdef PLATFORM_WINDOWS
    WSACleanup();
#endif
}

bool Socket::Create(bool isTcp)
{
    // Close existing socket if any
    Close();
    
    // Create a new socket
    m_handle = socket(AF_INET, isTcp ? SOCK_STREAM : SOCK_DGRAM, 0);
    
    return m_handle != INVALID_SOCKET_HANDLE;
}

bool Socket::Bind(int port)
{
    if (!IsValid()) {
        return false;
    }
    
    // Set up the address structure
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(static_cast<uint16_t>(port));
    
    // Bind the socket
    int result = bind(m_handle, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    
    return result != -1;
}

bool Socket::Listen(int backlog)
{
    if (!IsValid()) {
        return false;
    }
    
    int result = listen(m_handle, backlog);
    
    return result != -1;
}

Socket* Socket::Accept()
{
    if (!IsValid()) {
        return nullptr;
    }
    
    sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    
    SocketHandle clientHandle = accept(m_handle, reinterpret_cast<sockaddr*>(&clientAddr), &addrLen);
    
    if (clientHandle == INVALID_SOCKET_HANDLE) {
        return nullptr;
    }
    
    Socket* clientSocket = new Socket();
    clientSocket->m_handle = clientHandle;
    
    return clientSocket;
}

bool Socket::Connect(const std::string& host, int port)
{
    if (!IsValid()) {
        return false;
    }
    
    // Resolve the hostname
    struct addrinfo hints, *servinfo;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    // Convert port to string
    std::string portStr = std::to_string(port);
    
    int result = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &servinfo);
    if (result != 0) {
        return false;
    }
    
    // Try to connect to the first address returned
    result = connect(m_handle, servinfo->ai_addr, static_cast<int>(servinfo->ai_addrlen));
    
    freeaddrinfo(servinfo);
    
    return result != -1;
}

int Socket::Send(const void* data, int size)
{
    if (!IsValid() || !data || size <= 0) {
        return -1;
    }
    
    return static_cast<int>(send(m_handle, static_cast<const char*>(data), size, 0));
}

int Socket::Send(const std::string& data)
{
    return Send(data.c_str(), static_cast<int>(data.size()));
}

int Socket::Receive(void* buffer, int size)
{
    if (!IsValid() || !buffer || size <= 0) {
        return -1;
    }
    
    return static_cast<int>(recv(m_handle, static_cast<char*>(buffer), size, 0));
}

std::string Socket::ReceiveString(int maxLength)
{
    if (!IsValid() || maxLength <= 0) {
        return "";
    }
    
    std::vector<char> buffer(maxLength);
    int bytesRead = Receive(buffer.data(), maxLength);
    
    if (bytesRead <= 0) {
        return "";
    }
    
    return std::string(buffer.data(), bytesRead);
}

void Socket::Close()
{
    if (IsValid()) {
#ifdef PLATFORM_WINDOWS
        closesocket(m_handle);
#else
        close(m_handle);
#endif
        m_handle = INVALID_SOCKET_HANDLE;
    }
}

bool Socket::SetNonBlocking(bool nonBlocking)
{
    if (!IsValid()) {
        return false;
    }
    
#ifdef PLATFORM_WINDOWS
    u_long mode = nonBlocking ? 1 : 0;
    int result = ioctlsocket(m_handle, FIONBIO, &mode);
    bool success = (result == 0);
#else
    int flags = fcntl(m_handle, F_GETFL, 0);
    if (flags == -1) {
        return false;
    }
    
    if (nonBlocking) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    
    bool success = (fcntl(m_handle, F_SETFL, flags) != -1);
#endif
    
    if (success) {
        m_isBlocking = !nonBlocking;
    }
    
    return success;
}

bool Socket::SetReuseAddr(bool reuse)
{
    int optVal = reuse ? 1 : 0;
    return SetSocketOption(SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));
}

bool Socket::IsValid() const
{
    return m_handle != INVALID_SOCKET_HANDLE;
}

SocketHandle Socket::GetHandle() const
{
    return m_handle;
}

std::string Socket::GetLocalAddress() const
{
    if (!IsValid()) {
        return "";
    }
    
    sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    
    if (getsockname(m_handle, reinterpret_cast<sockaddr*>(&addr), &addrLen) == -1) {
        return "";
    }
    
    char addrStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), addrStr, INET_ADDRSTRLEN);
    
    return std::string(addrStr);
}

int Socket::GetLocalPort() const
{
    if (!IsValid()) {
        return -1;
    }
    
    sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    
    if (getsockname(m_handle, reinterpret_cast<sockaddr*>(&addr), &addrLen) == -1) {
        return -1;
    }
    
    return ntohs(addr.sin_port);
}

std::string Socket::GetRemoteAddress() const
{
    if (!IsValid()) {
        return "";
    }
    
    sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    
    if (getpeername(m_handle, reinterpret_cast<sockaddr*>(&addr), &addrLen) == -1) {
        return "";
    }
    
    char addrStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), addrStr, INET_ADDRSTRLEN);
    
    return std::string(addrStr);
}

int Socket::GetRemotePort() const
{
    if (!IsValid()) {
        return -1;
    }
    
    sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    
    if (getpeername(m_handle, reinterpret_cast<sockaddr*>(&addr), &addrLen) == -1) {
        return -1;
    }
    
    return ntohs(addr.sin_port);
}

bool Socket::Select(std::vector<Socket*>& readSockets, 
                  std::vector<Socket*>& writeSockets,
                  double timeoutSeconds)
{
    // Set up the file descriptor sets
    fd_set readSet, writeSet;
    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);
    
    // Find the largest socket handle
    SocketHandle maxHandle = 0;
    
    // Add read sockets to the read set
    for (Socket* socket : readSockets) {
        if (socket && socket->IsValid()) {
            FD_SET(socket->GetHandle(), &readSet);
            if (socket->GetHandle() > maxHandle) {
                maxHandle = socket->GetHandle();
            }
        }
    }
    
    // Add write sockets to the write set
    for (Socket* socket : writeSockets) {
        if (socket && socket->IsValid()) {
            FD_SET(socket->GetHandle(), &writeSet);
            if (socket->GetHandle() > maxHandle) {
                maxHandle = socket->GetHandle();
            }
        }
    }
    
    // If no valid sockets, return
    if (maxHandle == 0) {
        readSockets.clear();
        writeSockets.clear();
        return false;
    }
    
    // Set up the timeout
    struct timeval timeout;
    struct timeval* timeoutPtr = nullptr;
    
    if (timeoutSeconds >= 0) {
        timeout.tv_sec = static_cast<long>(timeoutSeconds);
        timeout.tv_usec = static_cast<long>((timeoutSeconds - timeout.tv_sec) * 1000000);
        timeoutPtr = &timeout;
    }
    
    // Perform the select
    int result = select(static_cast<int>(maxHandle + 1), &readSet, &writeSet, nullptr, timeoutPtr);
    
    // Check for errors
    if (result == -1) {
        readSockets.clear();
        writeSockets.clear();
        return false;
    }
    
    // Update read sockets (remove sockets that aren't ready)
    for (auto it = readSockets.begin(); it != readSockets.end();) {
        if (!(*it) || !(*it)->IsValid() || !FD_ISSET((*it)->GetHandle(), &readSet)) {
            it = readSockets.erase(it);
        } else {
            ++it;
        }
    }
    
    // Update write sockets (remove sockets that aren't ready)
    for (auto it = writeSockets.begin(); it != writeSockets.end();) {
        if (!(*it) || !(*it)->IsValid() || !FD_ISSET((*it)->GetHandle(), &writeSet)) {
            it = writeSockets.erase(it);
        } else {
            ++it;
        }
    }
    
    return (result > 0);
}

bool Socket::SetSocketOption(int level, int optname, const void* optval, int optlen)
{
    if (!IsValid()) {
        return false;
    }
    
    int result = setsockopt(m_handle, level, optname, static_cast<const char*>(optval), optlen);
    
    return result != -1;
}

} // namespace Network
} // namespace CardGameLib
