#include "BlueMarbleMaps/Networking/Socket.h"

using namespace BlueMarble::Networking;

struct Socket::Impl
{
    int fd = -1;
};

Socket::Socket(SocketType type)
    : m_sockfd(InvalidSocket)
{
    initializeWinsock();

    // Fixme: Maybe the instantiation should be in bind and connect
    m_sockfd = socket(
        AF_INET,
        type == SocketType::Tcp ? SOCK_STREAM : SOCK_DGRAM,
        0);
}

Socket::~Socket()
{
    close();
}





// Socket::Socket(SocketType type) : m_impl(std::make_unique<Impl>()) 
// { /* ::socket(...) */ }

// Socket::~Socket() 
// { 
//     close(); 
// }
// Socket::Socket(Socket&&) noexcept = default;
// Socket& Socket::operator=(Socket&&) noexcept = default;