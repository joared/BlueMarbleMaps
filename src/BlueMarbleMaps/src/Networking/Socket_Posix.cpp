#include "BlueMarbleMaps/Networking/Socket.h"

using namespace BlueMarbleMaps::Networking;

struct Socket::Impl 
{ 
    int fd = -1; 
};

// Socket::Socket(SocketType type) : m_impl(std::make_unique<Impl>()) 
// { /* ::socket(...) */ }

// Socket::~Socket() 
// { 
//     close(); 
// }
// Socket::Socket(Socket&&) noexcept = default;
// Socket& Socket::operator=(Socket&&) noexcept = default;