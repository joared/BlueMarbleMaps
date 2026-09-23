#ifndef TCPSERVER
#define TCPSERVER

#include "TcpConnection.h"
#include "TcpAcceptor.h"

namespace BlueMarble {
namespace Networking {

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

    size_t numClients()
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
} // namespace BlueMarble

#endif /* TCPSERVER */
