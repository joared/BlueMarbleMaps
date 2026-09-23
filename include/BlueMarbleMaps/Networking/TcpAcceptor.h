#ifndef TCPACCEPTOR
#define TCPACCEPTOR

#include "Socket.h"

#include <memory.h>

namespace BlueMarble {
namespace Networking {

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
        if (!m_socket.bind({ "0.0.0.0", port}))
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

} // namespace Networking
} // namespace BlueMarble

#endif /* TCPACCEPTOR */
