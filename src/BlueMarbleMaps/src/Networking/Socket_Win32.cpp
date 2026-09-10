#include "BlueMarbleMaps/Networking/Socket.h"

#define NOMINMAX
#include <Winsock2.h>
#include <ws2tcpip.h>

using namespace BlueMarble::Networking;

struct Socket::Impl
{
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

    SOCKET m_sockFd = INVALID_SOCKET;
    SocketType m_type = SocketType::Tcp;
};

Socket::Socket(SocketType type)
    : m_impl(std::make_unique<Impl>())
{
    Impl::initializeWinsock();

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
#ifdef _WIN32
    // Windows

    int port = endPoint.port;
    auto address = endPoint.address;

    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = m_impl->m_type == SocketType::Tcp ? SOCK_STREAM : SOCK_DGRAM;
    hints.ai_protocol = m_impl->m_type == SocketType::Tcp ? IPPROTO_TCP : IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE; // All local IPv4 interfaces

    INT iResult = getaddrinfo(address.c_str(), std::to_string(port).c_str(), &hints, &result);
    if (iResult != 0)
    {
        return false;
    }
    iResult = ::bind(m_impl->m_sockFd, result->ai_addr, (int)result->ai_addrlen);

    freeaddrinfo(result);

    return iResult != SOCKET_ERROR;
#elif defined(__linux__)
    // Circumvent "Address already in use" error when restarting the server quickly
    int reuse = 1;
    ::setsockopt(m_sockFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr_in serv_addr;
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    return 0 == ::bind(m_sockFd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
#endif // defined(__linux__)
}

bool Socket::listen(int backlog)
{
#ifdef _WIN32
    // Windows
    if (backlog == -1) backlog = SOMAXCONN;
    return SOCKET_ERROR != ::listen(m_impl->m_sockFd, backlog);
#elif defined(__linux__)
    if (backlog == -1) backlog = 5; // TODO: what to do?
    return 0 == ::listen(m_sockFd, backlog);
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
    int client_fd = ::accept(m_sockFd, (struct sockaddr*)&cli_addr, &clilen);

    if (client_fd == InvalidSocket)
    {
        throw std::runtime_error("Failed to accept");
    }

    return Socket(client_fd);
#endif
}


bool Socket::connect(const EndPoint& endPoint)
{
#ifdef _WIN32
    // Windows
    // We allow "double connect" by closing the socket if its open
    close();

    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = m_impl->m_type == SocketType::Tcp ? SOCK_STREAM : SOCK_DGRAM;
    hints.ai_protocol = m_impl->m_type == SocketType::Tcp ? IPPROTO_TCP : IPPROTO_UDP;
    hints.ai_flags = 0;

    INT iResult = getaddrinfo(endPoint.address.c_str(), std::to_string(endPoint.port).c_str(), &hints, &result);
    if (iResult != 0)
    {
        return false;
    }

    SOCKET connectSocket = INVALID_SOCKET;
    for (ptr = result; ptr != NULL;ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        connectSocket = ::socket(ptr->ai_family,
            ptr->ai_socktype,
            ptr->ai_protocol);
        if (connectSocket == INVALID_SOCKET)
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
            connectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }
    
    m_impl->m_sockFd = connectSocket;


    freeaddrinfo(result);

    return iResult != SOCKET_ERROR;

#elif defined(__linux__)
    // Create sockaddr initialized set to zero

    // TODO: make it possible to use host eg "example.com"
    // #include <netdb.h>
    /*struct addrinfo hints {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo* result = nullptr;

    int error = getaddrinfo(host.c_str(), std::to_string(port).c_str(),
        &hints, &result);*/

    sockaddr_in serv_addr;
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = inet_addr(host.c_str());

    return 0 == ::connect(m_sockFd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
#endif
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
    if (m_sockFd >= 0)
    {
        std::cout << "Socket::close()\n";
        ::close(m_sockFd);
        m_sockFd = InvalidSocket;
    }
#endif
}


bool Socket::isOpen() const
{
    return m_impl->m_sockFd != INVALID_SOCKET;
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
    return ::send(m_sockFd, data, size, 0);
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
    return ::recv(m_sockFd, buffer, size, 0);
#endif
}

int Socket::sendTo(const char* data, size_t size, const EndPoint& endPoint)
{
    SOCKET s = m_impl->m_sockFd; // If this is unbound, it will be done implicitly by the os
    
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = m_impl->m_type == SocketType::Tcp ? SOCK_STREAM : SOCK_DGRAM;
    hints.ai_protocol = m_impl->m_type == SocketType::Tcp ? IPPROTO_TCP : IPPROTO_UDP;
    hints.ai_flags = 0;


    INT iResult = getaddrinfo(endPoint.address.c_str(), std::to_string(endPoint.port).c_str(), &hints, &result);
    if (iResult != 0)
    {
        return false;
    }

    int res = ::sendto(s, data, (int)size, 0, result->ai_addr, (int)result->ai_addrlen);

    freeaddrinfo(result);

    if (res == SOCKET_ERROR)
    {
        return -1;
    }
    return res;
}

int Socket::receiveFrom(char* buffer, size_t size, EndPoint& senderEndPoint)
{
    SOCKET s = m_impl->m_sockFd;
    sockaddr_in addr{};
    int addrLen = sizeof(addr);

    int nBytesReceived = ::recvfrom(s, buffer, (int)size, 0, reinterpret_cast<sockaddr*>(&addr), &addrLen);

    if (nBytesReceived == SOCKET_ERROR)
    {
        return -1;
    }

    if (nBytesReceived > 0)
    {
        if (addr.sin_family != AF_INET)
        {
            throw std::runtime_error("receiveFrom() received data from unssuported address family: " + std::to_string(addr.sin_family) + "\n");
        }

        sockaddr_in* addripv4 = reinterpret_cast<sockaddr_in*>(&addr);

        char address[INET_ADDRSTRLEN];

        inet_ntop(
            AF_INET,
            &addripv4->sin_addr,
            address,
            sizeof(address)
        );

        senderEndPoint = { address, ntohs(addripv4->sin_port) };
    }

    return nBytesReceived;
}

bool Socket::setSocketOptions(const SocketOptions& options)
{
    if (!isOpen())
    {
        std::cout << "Socket::setSocketOptions() called on a closed socket\n";
        return false;
    }
    int res;

    SOCKET s = m_impl->m_sockFd;
    BOOL enable = options.broadcastEnabled ? TRUE : FALSE;
    res = setsockopt(
        s, 
        SOL_SOCKET, 
        SO_BROADCAST,
        reinterpret_cast<const char*>(&enable),
        sizeof(enable));

    if (res == SOCKET_ERROR)
    {
        std::cout << "Socket::setSocketOptions() failed to toggle broadcast\n";
        return false;
    }

    BOOL reuse = options.reuseAddressEnabled ? TRUE : FALSE;;
    res = setsockopt(
        s,
        SOL_SOCKET,
        SO_REUSEADDR,
        reinterpret_cast<const char*>(&reuse),
        sizeof(reuse));

    if (res == SOCKET_ERROR)
    {
        std::cout << "Socket::setSocketOptions() failed to toggle reuse\n";
        return false;
    }

    return true;
}
