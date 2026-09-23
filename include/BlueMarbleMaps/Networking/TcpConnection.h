#ifndef TCPCONNECTION
#define TCPCONNECTION

#include "Socket.h"

namespace BlueMarble {
namespace Networking {


class TcpAcceptor;
class TcpClient;
class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

class TcpConnection
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

} // namespace Networking
} // namespace BlueMarbleMaps

#endif /* TCPCONNECTION */
