#include "BlueMarbleMaps/Networking/Socket.h"

#include <sys/types.h> 
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace BlueMarble::Networking;

struct Socket::Impl
{
    int m_sockFd = -1;
    SocketType m_type = SocketType::Tcp;
};

Socket::Socket(SocketType type)
    : m_impl(std::make_unique<Impl>())
{

    // Fixme: Maybe the instantiation should be in bind and connect
    *m_impl = {
        .m_sockFd = ::socket(AF_INET,
                             type == SocketType::Tcp ? SOCK_STREAM : SOCK_DGRAM,
                             type == SocketType::Tcp ? IPPROTO_TCP : IPPROTO_UDP),
        .m_type = type
    };
}

Socket::Socket(std::unique_ptr<Impl> impl)
    : m_impl(std::move(impl))
{

}

Socket::~Socket()
{
    close();
}

Socket::Socket(Socket&& other) noexcept
    : m_impl(std::move(other.m_impl))
{
}

Socket& Socket::operator=(Socket&& other) noexcept
{
    if (this != &other)
    {
        close();
        m_impl = std::move(other.m_impl);
    }

    return *this;
}

bool Socket::bind(const EndPoint& endPoint)
{
    // Circumvent "Address already in use" error when restarting the server quickly
    int reuse = 1;
    
    ::setsockopt(m_impl->m_sockFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    int port = endPoint.port;
    auto address = endPoint.address;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = m_impl->m_type == SocketType::Tcp ? SOCK_STREAM : SOCK_DGRAM;
    hints.ai_protocol = m_impl->m_type == SocketType::Tcp ? IPPROTO_TCP : IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE; // All local IPv4 interfaces

    int iResult = ::getaddrinfo(address.c_str(), std::to_string(port).c_str(), &hints, &result);
    if (iResult != 0)
    {
        return false;
    }
    freeaddrinfo(result);

    return 0 == ::bind(m_impl->m_sockFd, result->ai_addr, (int)result->ai_addrlen);

    // getaddrinfo()
    // sockaddr_in serv_addr;
    // std::memset(&serv_addr, 0, sizeof(serv_addr));
    // serv_addr.sin_family = AF_INET;
    // serv_addr.sin_addr.s_addr = INADDR_ANY;
    // serv_addr.sin_port = htons(endPoint.port);

    // return 0 == ::bind(m_impl->m_sockFd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
}

bool Socket::listen(int backlog)
{
#ifdef _WIN32
    // Windows
    if (backlog == -1) backlog = SOMAXCONN;
    return SOCKET_ERROR != ::listen(m_impl->m_sockFd, backlog);
#elif defined(__linux__)
    if (backlog == -1) backlog = 5; // TODO: what to do?
    return 0 == ::listen(m_impl->m_sockFd, backlog);
#endif
}


[[nodiscard]] Socket Socket::accept()
{
#ifdef _WIN32
    // Windows
    SOCKET clientSocket = ::accept(m_impl->m_sockFd, NULL, NULL);

    auto impl = std::make_unique<Impl>(clientSocket, m_impl->m_type);

    return Socket(std::move(impl));
#elif defined(__linux__)
    sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    int clientFd = ::accept(m_impl->m_sockFd, (struct sockaddr*)&cli_addr, &clilen);

    if (clientFd == -1)
    {
        throw std::runtime_error("Failed to accept");
    }

    auto impl = std::make_unique<Impl>(Impl{clientFd, m_impl->m_type});

    return Socket(std::move(impl));
#endif
}


bool Socket::connect(const EndPoint& endPoint)
{
    // We allow "double connect" by closing the socket if its open
    close();

    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = m_impl->m_type == SocketType::Tcp ? SOCK_STREAM : SOCK_DGRAM;
    hints.ai_protocol = m_impl->m_type == SocketType::Tcp ? IPPROTO_TCP : IPPROTO_UDP;
    hints.ai_flags = 0;

    int iResult = getaddrinfo(endPoint.address.c_str(), std::to_string(endPoint.port).c_str(), &hints, &result);
    if (iResult != 0)
    {
        return false;
    }

    int connectSocket = -1;
    for (ptr = result; ptr != NULL;ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        connectSocket = ::socket(ptr->ai_family,
            ptr->ai_socktype,
            ptr->ai_protocol);
        if (connectSocket == -1)
        {
            // printf("socket failed with error: %ld\n", WSAGetLastError());
            //WSACleanup();
            return false;
        }

        // Connect to server.
        iResult = ::connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == -1)
        {
            ::close(connectSocket);
            connectSocket = -1;
            continue;
        }
        break;
    }
    
    m_impl->m_sockFd = connectSocket;


    freeaddrinfo(result);

    return iResult == 0;

    // sockaddr_in serv_addr;
    // std::memset(&serv_addr, 0, sizeof(serv_addr));
    // serv_addr.sin_family = AF_INET;
    // serv_addr.sin_port = htons(endPoint.port);
    // serv_addr.sin_addr.s_addr = inet_addr(endPoint.address.c_str());

    // return 0 == ::connect(m_impl->m_sockFd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
}

void Socket::close()
{
#ifdef _WIN32
    if (!m_impl)
    {
        return;
    }
    std::cout << "Closing socket: " << m_impl->m_sockFd << "\n";
    // Windows
    if (m_impl->m_sockFd != INVALID_SOCKET)
    {
        if (m_impl->m_type == SocketType::Tcp)
            ::shutdown(m_impl->m_sockFd, SD_SEND); // Not sure when to use this
        ::closesocket(m_impl->m_sockFd);
    }
    m_impl->m_sockFd = INVALID_SOCKET;

#elif defined(__linux__)
    if (!m_impl)
    {
        return;
    }
    if (m_impl->m_sockFd >= 0)
    {
        std::cout << "Socket::close()\n";
        ::close(m_impl->m_sockFd);
        m_impl->m_sockFd = -1;
    }
#endif
}


bool Socket::isOpen() const
{
    return m_impl && m_impl->m_sockFd != -1;
}


int Socket::send(const char* data, size_t size)
{
#ifdef _WIN32
    // Windows
    int res = ::send(m_impl->m_sockFd, data, (int)size, 0);
    if (res == SOCKET_ERROR)
    {
        return -1;
    }
    return res;
#elif defined(__linux__)
    return ::send(m_impl->m_sockFd, data, size, 0);
#endif
}

int Socket::receive(char* buffer, size_t size)
{
#ifdef _WIN32
    // Windows
    int res = ::recv(m_impl->m_sockFd, buffer, (int)size, 0);
    if (res == SOCKET_ERROR)
    {
        return -1;
    }
    return res;
#elif defined(__linux__)
    return ::recv(m_impl->m_sockFd, buffer, size, 0);
#endif
}

int Socket::sendTo(const char* data, size_t size, const EndPoint& endPoint)
{
    // SOCKET s = m_impl->m_sockFd; // If this is unbound, it will be done implicitly by the os
    
    // struct addrinfo* result = NULL,
    //     * ptr = NULL,
    //     hints;

    // ZeroMemory(&hints, sizeof(hints));
    // hints.ai_family = AF_INET;
    // hints.ai_socktype = m_impl->m_type == SocketType::Tcp ? SOCK_STREAM : SOCK_DGRAM;
    // hints.ai_protocol = m_impl->m_type == SocketType::Tcp ? IPPROTO_TCP : IPPROTO_UDP;
    // hints.ai_flags = 0;


    // INT iResult = getaddrinfo(endPoint.address.c_str(), std::to_string(endPoint.port).c_str(), &hints, &result);
    // if (iResult != 0)
    // {
    //     return false;
    // }

    // int res = ::sendto(s, data, (int)size, 0, result->ai_addr, (int)result->ai_addrlen);

    // freeaddrinfo(result);

    // if (res == SOCKET_ERROR)
    // {
    //     return -1;
    // }
    // return res;

    sockaddr_in serv_addr;
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    socklen_t len = sizeof(serv_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(endPoint.port);
    serv_addr.sin_addr.s_addr = inet_addr(endPoint.address.c_str());

    int res = ::sendto(m_impl->m_sockFd, data, size, 0, (sockaddr*)&serv_addr, len);

    return res;
}

int Socket::receiveFrom(char* buffer, size_t size, EndPoint& senderEndPoint)
{
    // SOCKET s = m_impl->m_sockFd;
    // sockaddr_in addr{};
    // int addrLen = sizeof(addr);

    // int nBytesReceived = ::recvfrom(s, buffer, (int)size, 0, reinterpret_cast<sockaddr*>(&addr), &addrLen);

    // if (nBytesReceived == SOCKET_ERROR)
    // {
    //     return -1;
    // }

    // if (nBytesReceived > 0)
    // {
    //     if (addr.sin_family != AF_INET)
    //     {
    //         throw std::runtime_error("receiveFrom() received data from unssuported address family: " + std::to_string(addr.sin_family) + "\n");
    //     }

    //     sockaddr_in* addripv4 = reinterpret_cast<sockaddr_in*>(&addr);

    //     char address[INET_ADDRSTRLEN];

    //     inet_ntop(
    //         AF_INET,
    //         &addripv4->sin_addr,
    //         address,
    //         sizeof(address)
    //     );

    //     senderEndPoint = { address, ntohs(addripv4->sin_port) };
    // }

    // return nBytesReceived;

    sockaddr_in serv_addr;
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    socklen_t len = sizeof(serv_addr);

    int res = ::recvfrom(m_impl->m_sockFd, buffer, size, 0, (sockaddr*)&serv_addr, &len);

    if (res == -1) return -1;

    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, 
              (void*)&serv_addr.sin_addr,
              buf, 
              sizeof(buf));

    senderEndPoint.address = buf;
    senderEndPoint.port = ntohs(serv_addr.sin_port);

    return res;
}

bool Socket::setSocketOptions(const SocketOptions& options)
{
    if (!isOpen())
    {
        std::cout << "Socket::setSocketOptions() called on a closed socket\n";
        return false;
    }
    int res;
    int s = m_impl->m_sockFd;

    int enable = options.broadcastEnabled ? 1 : 0;
    res = ::setsockopt(
        s, 
        SOL_SOCKET, 
        SO_BROADCAST,
        reinterpret_cast<const char*>(&enable),
        sizeof(enable));

    if (res == -1)
    {
        std::cout << "Socket::setSocketOptions() failed to toggle broadcast\n";
        return false;
    }

    int reuse = options.reuseAddressEnabled ? 1 : 0;
    res = ::setsockopt(m_impl->m_sockFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    if (res == -1)
    {
        std::cout << "Socket::setSocketOptions() failed to toggle reuse\n";
        return false;
    }

    return true;
}
