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

namespace BlueMarble {
namespace Networking {


struct EndPoint
{
    // TODO: make this integer of sort, and have a "resolver" that can convert host names to actuall end points
    // This is currently interpreted as "host" and automatically resolves end points technically
    std::string address = "0.0.0.0"; 
    int port = -1;

    bool operator==(const EndPoint& other) const
    {
        return address == other.address &&
               port == other.port;
    }

    std::string toString() const
    {
        return address + " : " + std::to_string(port);
    }
};

/*
* Support TCP or UDP using explicitly the IPv4 address family
* Typcal TCP server: bind() -> listen() -> accept()
* TCP Client: connect() 
* UDP Server: bind() -> sendTo()/receiveFrom()
* UDP Client unconnected peer: bind() (optional) -> sendTo()/receiveFrom()
* UDP Client with connected peer: connect() -> send()/receive()
*/
class Socket
{
public:
    
//#ifdef _WIN32
//    // Windows
//    using NativeSocketHandle = SOCKET;
//#elif defined(__linux__)
//    using NativeSocketHandle = int;
//#endif

    struct SocketOptions
    {
        bool broadcastEnabled = false; // Allows socket to Send to a broadcast address
        bool reuseAddressEnabled = false; // Allows multiple sockets to use the same local port
    };

    enum class SocketType
    {
        Tcp,
        Udp
    };

    Socket(SocketType type = SocketType::Tcp);

    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    // disable copy constructor and assignment operator
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    ~Socket();
    
    // Binds a socket 0.0.0.0 and accepts all traffic on any interface
    bool bind(const EndPoint& endPoint);
    // Start listening to to a currently bound socket. 
    // backlog defines the os queue size for connections, -1 a default will be chosen
    bool listen(int backlog=-1);
    // Accept incoming connections on the currently bound and listen socket.
    [[nodiscard]] Socket accept();
    // Connect to a host
    // TCP/UDP
    bool connect(const EndPoint& endPoint);
    void close();

    // Checks if the socket is valid
    bool isOpen() const;

    // Sends at maximum number of bytes (size). Returns the actual number of bytes sent
    // Used for sockets that are connected
    int send(const char* data, size_t size);
    // Receives at maximum number of bytes (size). Returns the actual number of bytes read.
    // Used for sockets that are connected.
    int receive(char* buffer, size_t size);

    int sendTo(const char* data, size_t size, const EndPoint& endPoint);
    int receiveFrom(char* buffer, size_t size, EndPoint& senderEndPoint);

    // Options. If socket is not open, this will fail and return false
    bool setSocketOptions(const SocketOptions& options);

private:
    struct Impl;                     // incomplete here — defined in the .cpp
    std::unique_ptr<Impl> m_impl;
    explicit Socket(std::unique_ptr<Impl> impl);    
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
            int received = m_socket.send(data + totalSent, bytesToSend - totalSent);
            
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
            throw std::runtime_error("Total sent != to send (" + std::to_string(totalSent) + " != " + std::to_string(bytesToSend));
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
            int received = m_socket.receive(buffer + totalReceived, bytesToRead - totalReceived);
            
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


class TcpClient
{
public:
    
    [[nodiscard]] static TcpConnection connect(const std::string& host, int port)
    {
        Socket socket;
        if (!socket.connect({ host, port }))
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

struct HTTPMessage 
{
    std::string start_line; // e.g., "GET / HTTP/1.1" or "HTTP/1.1 200 OK"
    std::unordered_map<std::string, std::string> headers;
    std::vector<char> body;
};

// Helper to read exactly one line (until \r\n) from your socket abstraction
inline std::string readLine(TcpConnection& socket) 
{
    std::string line;
    char c;
    while (socket.receive(&c, 1) > 0) {
        line.push_back(c);
        if (line.size() >= 2 && line[line.size() - 2] == '\r' && line[line.size() - 1] == '\n') {
            line.pop_back(); // Remove \n
            line.pop_back(); // Remove \r
            break;
        }
    }
    return line;
}

} // namespace Networking
} // namespace BlueMarbleMaps