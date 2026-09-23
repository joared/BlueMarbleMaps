#ifndef BLUEMARBLE_SOCKET
#define BLUEMARBLE_SOCKET

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

    struct SocketOptions
    {
        bool broadcastEnabled = false;      // Allows socket to Send to a broadcast address
        bool reuseAddressEnabled = false;   // Allows multiple sockets to use the same local port
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
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    explicit Socket(std::unique_ptr<Impl> impl);    
};

} // namespace Networking
} // namespace BlueMarbleMaps

#endif // BLUEMARBLE_SOCKET