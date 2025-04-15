#pragma once

#include <string>
#include <vector>
#include <cstdint>

#ifdef PLATFORM_WINDOWS
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET SocketHandle;
    #define INVALID_SOCKET_HANDLE INVALID_SOCKET
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <netdb.h>
    typedef int SocketHandle;
    #define INVALID_SOCKET_HANDLE -1
#endif

namespace CardGameLib {
namespace Network {

class Socket {
public:
    Socket();
    ~Socket();
    
    // Socket initialization and cleanup
    static bool InitializeSocketSystem();
    static void ShutdownSocketSystem();
    
    // Create a socket
    bool Create(bool isTcp = true);
    
    // Bind to a local address and port
    bool Bind(int port);
    
    // Listen for incoming connections (server)
    bool Listen(int backlog = 5);
    
    // Accept a new connection (server)
    Socket* Accept();
    
    // Connect to a remote host (client)
    bool Connect(const std::string& host, int port);
    
    // Send data
    int Send(const void* data, int size);
    int Send(const std::string& data);
    
    // Receive data
    int Receive(void* buffer, int size);
    std::string ReceiveString(int maxLength = 4096);
    
    // Close the socket
    void Close();
    
    // Set socket options
    bool SetNonBlocking(bool nonBlocking);
    bool SetReuseAddr(bool reuse);
    
    // Socket state
    bool IsValid() const;
    
    // Get socket info
    SocketHandle GetHandle() const;
    std::string GetLocalAddress() const;
    int GetLocalPort() const;
    std::string GetRemoteAddress() const;
    int GetRemotePort() const;
    
    // Select for readability or writability with timeout
    static bool Select(std::vector<Socket*>& readSockets, 
                    std::vector<Socket*>& writeSockets,
                    double timeoutSeconds);
    
private:
    SocketHandle m_handle;
    bool m_isBlocking;
    
    // Helper function to set socket options
    bool SetSocketOption(int level, int optname, const void* optval, int optlen);
};

} // namespace Network
} // namespace CardGameLib
