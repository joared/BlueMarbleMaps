

#ifdef _WIN32
// Windows
#define NOMINMAX
#include <Winsock2.h>
#include <ws2tcpip.h>
#elif defined(__linux__)
// Linux
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <string>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <functional>
#include <sstream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <memory>
#include <list>

namespace BlueMarbleMaps {
namespace Networking {

class Socket
{
public:

#ifdef _WIN32
    // Windows
    using SocketHandle = SOCKET;
    #define InvalidSocket INVALID_SOCKET

    static bool initializeWinsock()
    {
        static bool initialized = []()
        {
            WSADATA wsaData;
            int iResult;
            iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
            if (iResult != 0) 
            {
                std::cout << "WSAStartup failed with error: " << iResult << "\n";
                return false;
            }

            return true;
        }();

        return initialized;
    }
#elif defined(__linux__)
    using SocketHandle = int;
    #define InvalidSocket -1
#endif

    enum class SocketType
    {
        Tcp,
        Udp
    };

    Socket(SocketType type = SocketType::Tcp)
        : m_sockfd(InvalidSocket)
    {
#ifdef _WIN32
        initializeWinsock();
        
        // Fixme: Maybe the instantiation should be in bind and connect
        m_sockfd = ::socket(
            AF_INET,
            type == SocketType::Tcp ? SOCK_STREAM : SOCK_DGRAM,
            type == SocketType::Tcp ? IPPROTO_TCP : IPPROTO_UDP
        );


#elif defined(__linux__)

        m_sockfd = socket(
            AF_INET, 
            type == SocketType::Tcp ? SOCK_STREAM : SOCK_DGRAM, 
        0);
#endif // defined(__linux__)
    }

    Socket(Socket&& other) noexcept
        : m_sockfd(other.m_sockfd)
    {
        other.m_sockfd = InvalidSocket;
    }

    Socket& operator=(Socket&& other) noexcept
    {
        if (this != &other)
        {
            close();
            m_sockfd = other.m_sockfd;
            other.m_sockfd = InvalidSocket;
        }
        return *this;
    }

    ~Socket()
    {
        close();
    }

    bool bind(int port)
    {
#ifdef _WIN32
        // Windows
        
        struct addrinfo* result = NULL,
            * ptr = NULL,
            hints;

        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        INT iResult = getaddrinfo("127.0.0.1", std::to_string(port).c_str(), &hints, &result);
        if (iResult != 0)
        {
            return false;
        }
        iResult = ::bind(m_sockfd, result->ai_addr, (int)result->ai_addrlen);

        freeaddrinfo(result);

        return iResult != SOCKET_ERROR;
#elif defined(__linux__)
        // Circumvent "Address already in use" error when restarting the server quickly
        int reuse = 1;
        ::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

        sockaddr_in serv_addr;
        std::memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(port);

        return 0 == ::bind(m_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
#endif // defined(__linux__)
    }

    bool listen(int backlog)
    {
#ifdef _WIN32
        // Windows
        return SOCKET_ERROR != ::listen(m_sockfd, SOMAXCONN);
#elif defined(__linux__)
        return 0 == ::listen(m_sockfd, backlog);
#endif
    }

    [[nodiscard]] Socket accept()
    {
#ifdef _WIN32
        // Windows
        SocketHandle clientSocket = ::accept(m_sockfd, NULL, NULL);
        return Socket(clientSocket);
#elif defined(__linux__)
        sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        int client_fd = ::accept(m_sockfd, (struct sockaddr *) &cli_addr, &clilen);
        
        if (client_fd == InvalidSocket)
        {
            throw std::runtime_error("Failed to accept");
        }

        return Socket(client_fd);
#endif
    }

    bool connect(const std::string& host, int port)
    {
#ifdef _WIN32
        // Windows
        
        struct addrinfo* result = NULL,
            * ptr = NULL,
            hints;

        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        INT iResult = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result);
        if (iResult != 0)
        {
            return false;
        }

        SOCKET connectSocket = InvalidSocket;
        for (ptr = result; ptr != NULL;ptr = ptr->ai_next) {

            // Create a SOCKET for connecting to server
            connectSocket = ::socket(ptr->ai_family, 
                                     ptr->ai_socktype,
                                     ptr->ai_protocol);
            if (connectSocket == InvalidSocket) 
            {
                printf("socket failed with error: %ld\n", WSAGetLastError());
                //WSACleanup();
                return false;
            }

            // Connect to server.
            iResult = ::connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
            if (iResult == SOCKET_ERROR) 
            {
                closesocket(connectSocket);
                connectSocket = InvalidSocket;
                continue;
            }
            break;
        }
        close(); // TODO: this is uggly. We should move the creation of the socket here (and in bind)
        m_sockfd = connectSocket;


        freeaddrinfo(result);

        return iResult != SOCKET_ERROR;

#elif defined(__linux__)
        // Create sockaddr initialized set to zero
        sockaddr_in serv_addr;
        std::memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        serv_addr.sin_addr.s_addr = inet_addr(host.c_str());

        return 0 == ::connect(m_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
#endif
    }
    void close()
    {
#ifdef _WIN32
        // Windows
        if (m_sockfd != InvalidSocket)
        {
            ::shutdown(m_sockfd, SD_SEND); // Not sure when to use this
            ::closesocket(m_sockfd);
        }
        m_sockfd = InvalidSocket;

#elif defined(__linux__)
        if (m_sockfd >= 0)
        {
            std::cout << "Socket::close()\n";
            ::close(m_sockfd);
            m_sockfd = InvalidSocket;
        }
#endif
    }

    bool isOpen() const
    {
        return m_sockfd >= 0;
    }

    int send(const char* data, size_t size)
    {
#ifdef _WIN32
        // Windows
        return ::send(m_sockfd, data, size, 0);
#elif defined(__linux__)
        return ::send(m_sockfd, data, size, 0);
#endif
    }

    int receive(char* buffer, size_t size)
    {
#ifdef _WIN32
        // Windows
        return ::recv(m_sockfd, buffer, size, 0);
#elif defined(__linux__)
        return ::recv(m_sockfd, buffer, size, 0);
#endif
    }

private:
    Socket(SocketHandle sockfd)
        : m_sockfd(sockfd)
    {
    }

    // disable copy constructor and assignment operator
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    SocketHandle m_sockfd;
};

class TcpAcceptor;
class TcpClient;
class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

class TcpConnection
{
public:
    bool isOpen() const
    {
        return m_socket.isOpen();
    }

    bool send(const char* data, size_t size)
    {
        if (size == 0) 
        {
            throw std::runtime_error("Tried sending empty data");
        }

        return m_socket.send(data, size) > 0;
    }

    bool sendExact(const char* data, size_t bytesToSend)
    {
        size_t totalSent = 0;
        while (totalSent < bytesToSend) 
        {
            // Vi ber bara om det antal bytes som återstår upp till vår målstorlek
            size_t received = m_socket.send(data + totalSent, bytesToSend - totalSent);
            
            if (received < 0) 
            {
                std::cout << "sendExact() error\n";
                return false; // Error
            }
            if (received == 0) 
            {
                std::cout << "sendExact() disconnected\n";
                return false; // Disconnected
            }
            totalSent += received;
        }

        if (totalSent != bytesToSend)
        {
            throw std::runtime_error("Total sent != sent");
        }

        return true;
    }

    bool sendMessage(const std::string& message)
    {
        constexpr MessageSizeHeader maxAllowedSize = std::numeric_limits<MessageSizeHeader>::max();

        if (message.size() == 0)
        {
            std::cout << "Cant send an empty message!\n";
            return true;
        }

        if (message.size() > maxAllowedSize) 
        {
            std::cerr << "Message is too large!" << std::endl;
            return false;
        }

        // Send message size first
        MessageSizeHeader messageSize = static_cast<MessageSizeHeader>(message.size());
        if (!sendExact(reinterpret_cast<const char*>(&messageSize), sizeof(messageSize))) 
        {
            return false;
        }

        // 3. Skicka själva meddelandet direkt efter
        return sendExact(message.data(), message.size());
    }

    int receive(char* buffer, size_t size)
    {
        return m_socket.receive(buffer, size);
    }

    bool receiveExact(char* buffer, size_t bytesToRead)
    {
        size_t totalReceived = 0;
        while (totalReceived < bytesToRead) 
        {
            // Vi ber bara om det antal bytes som återstår upp till vår målstorlek
            size_t received = m_socket.receive(buffer + totalReceived, bytesToRead - totalReceived);
            
            if (received < 0) 
            {
                std::cout << "receiveExact() error\n";
                return false; // Error
            }
            if (received == 0) 
            {
                std::cout << "receiveExact() disconnected\n";
                return false; // Disconnected
            }
            totalReceived += received;
        }

        if (totalReceived != bytesToRead)
        {
            throw std::runtime_error("Total received != received");
        }

        return true;
    }

    bool receiveMessage(std::string& message)
    {
        MessageSizeHeader messageSize;
        if (!receiveExact(reinterpret_cast<char*>(&messageSize), sizeof(messageSize)))
        {
            std::cout << "Could not read message header\n";
            return false;
        }
        
        message.clear();
        message.resize(messageSize);
        std::cout << "Message size: " << messageSize << "\n";
        if (!receiveExact(message.data(), messageSize))
        {
            return false;
        }

        return true;
    }
    
private:
    using MessageSizeHeader = uint32_t;
    friend class TcpAcceptor;
    friend class TcpClient;
    TcpConnection()
        : m_socket(Socket::SocketType::Tcp)
    {
    }

    TcpConnection(Socket&& socket)
        : m_socket(std::move(socket))
    {
    }

    Socket m_socket;
};

class TcpAcceptor;
using TcpAcceptorPtr = std::shared_ptr<TcpAcceptor>;
class TcpAcceptor
{
public:

    TcpAcceptor()
        : m_socket(Socket::SocketType::Tcp)
    {
        
    }

    TcpAcceptor(int port)
        : m_socket(Socket::SocketType::Tcp)
        , m_port(port)
    {
        
    }

    bool bind()
    {
        return bind(m_port);
    }

    bool bind(int port)
    {
        if (!m_socket.bind(port))
        {
            throw std::runtime_error("Failed to bind to port " + std::to_string(port));
        }
        m_isBound = true;
        return true;
    }

    bool listen(int backlog)
    {
        if (!m_isBound)
        {
            throw std::runtime_error("Socket must be bound before listening");
        }
        if (!m_socket.listen(backlog))
        {
            throw std::runtime_error("Failed to listen on port " + std::to_string(m_port));
        }
        return true;
    }

    [[nodiscard]] TcpConnection accept()
    {
        return TcpConnection(std::move(m_socket.accept()));
    }

private:
    Socket m_socket;
    bool   m_isBound = false;
    int    m_port = 0;
};


class TcpClient
{
public:
    
    [[nodiscard]] static TcpConnection connect(const std::string& host, int port)
    {
        Socket socket;
        if (!socket.connect(host, port))
        {
            throw std::runtime_error("Failed to connect to " + host + ":" + std::to_string(port));
        }

        return TcpConnection(std::move(socket));
    }

};

class TcpServer
{
public:
    TcpServer(int port)
        : m_port(port)
    {
        if (!m_acceptor.bind(port))
        {
            throw std::runtime_error("Failed to bind to port " + std::to_string(port));
        }
        if (!m_acceptor.listen(5))
        {
            throw std::runtime_error("Failed to listen on port " + std::to_string(port));
        }
    }

    void run()
    {
        while (true)
        {
            TcpConnection accepted = m_acceptor.accept();
            if (!accepted.isOpen())
            {
                std::cerr << "Failed to accept connection\n";
                continue;
            }
            
            m_connectionsMutex.lock();
            TcpConnection& connection = m_connections.emplace_back(std::move(accepted));
            m_connectionsMutex.unlock();

            if (onClientConnected && !onClientConnected(connection))
            {
                disconnectClient(connection);
                continue;
            }

            std::thread thread = std::thread([this, &connection]
                {
                    for (;;)
                    {
                        std::string message;
                        if (!connection.receiveMessage(message))
                        {
                            // Client disconnected
                            disconnectClient(connection);
                            break;
                        }

                        if (onClientMessage && !onClientMessage(connection, message)) 
                        {
                            disconnectClient(connection);
                            break;
                        }
                    }
                }
            );
            thread.detach();
        }
    }

    int numClients()
    {
        return m_connections.size();
    }

    auto& clients() { return m_connections; }

    void disconnectClient(TcpConnection& connection)
    {
        for (auto it = m_connections.begin(); it != m_connections.end(); ++it)
        {
            if (&*it == &connection)
            {
                if (onClientDisconnected) 
                {
                    onClientDisconnected(connection);
                }
                m_connectionsMutex.lock();
                m_connections.erase(it);
                m_connectionsMutex.unlock();
                return;
            }
        }

        throw std::runtime_error("Tried to disconnect a client that doesnt exist");
    }

    bool sendMessageToClients(const std::string& message)
    {
        bool success = true;
        for (auto& c : m_connections)
        {
            success &= c.sendMessage(message);
        }

        return success;
    }

public:
    // On the "run" thread
    std::function<bool(TcpConnection& connection)>                             onClientConnected;
    std::function<void(TcpConnection& connection)>                             onClientDisconnected;
    // On background thread
    std::function<bool(TcpConnection& connection, const std::string& message)> onClientMessage;

private:

    int m_port;
    TcpAcceptor m_acceptor;
    std::mutex  m_connectionsMutex;
    std::list<TcpConnection> m_connections;

};

} // namespace Networking
} // namespace BlueMarbleMaps