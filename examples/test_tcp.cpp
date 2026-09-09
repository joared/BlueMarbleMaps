#include "BlueMarbleMaps/Networking/Socket.h"
#include <iostream>

using namespace BlueMarbleMaps::Networking;

void testTcpServer()
{
    Socket socket;
    if (!socket.bind(8080))
    {
        std::cout << "Failed to bind to port 8080\n";
        return;
    }
    if (!socket.listen(5))
    {
        std::cout << "Failed to listen on port 8080\n";
        return;
    }
    Socket client_socket = socket.accept();
    if (!client_socket.isOpen())
    {
        std::cout << "Failed to accept connection\n";
        return;
    }
    char buffer[256];
    int n = client_socket.receive(buffer, sizeof(buffer));
    if (n < 0)
    {
        std::cout << "Failed to receive message from client\n";
        return;
    }

    buffer[n] = '\0'; // Null-terminate the received string
    std::cout << "Received message from client: " << buffer << "\n";

    std::string response = "Hello, client!";
    client_socket.send(response.c_str(), response.size()); // Send response back to client
}

void testTcpClient()
{
    Socket socket;
    if (!socket.connect("127.0.0.1", 8080))
    {
        std::cout << "Failed to connect to server\n";
        return;
    }
    std::string message = "Hello, server!";
    int n = socket.send(message.c_str(), message.size());
    if (n < 0)
    {
        std::cout << "Failed to send message to server\n";
        return;
    }

    while (true)
    {
        char buffer[256];
        n = socket.receive(buffer, sizeof(buffer));
        if (n < 0)
        {
            std::cout << "Failed to receive response from server\n";
            return;
        }
        if (n == 0)
        {
            std::cout << "Server closed the connection\n";
            return;
        }
        
        buffer[n] = '\0'; // Null-terminate the received string
        std::cout << "Received response from server: " << buffer << "\n";
    }
}

void testTcpServer2()
{
    TcpServer server(8080);

    server.onClientConnected = [&server](auto& connection)
    {
        std::cout << "Someone connected! (" << server.numClients() << ")\n";
        return true;
    };

    server.onClientDisconnected = [&server](auto& connection)
    {
        std::cout << "Someone disconnected! (" << server.numClients() << ")\n";
    };

    server.onClientMessage = [&server](auto& connection, const auto& message)
    {
        std::cout << "Client wrote: " << message << "\n";
        
        server.sendMessageToClients(message);

        return true;
    };

    std::cout << "Started server\n";
    server.run();
}

void testTcpClient2(std::string name)
{
    auto connection = TcpClient::connect("192.168.1.149", 8080);
    if (!connection.isOpen())
    {
        std::cout << "Failed to connect to server\n";
        return;
    }

    // Receive thread
    std::thread thread = std::thread([&connection]
    {
        for (;;)
        {
            std::string message;
            if (!connection.receiveMessage(message))
            {
                // Server disconnected
                std::cout << "Server disconnected\n";
                break;
            }

            std::cout << message << "\n";

        }
    });

    thread.detach();

    //std::cout << "\033[2J\033[H" << std::flush;
    std::cout << "Connected to server\n";
    
    for (;;)
    {
        //std::cout << "Write a message: ";
        std::string message;
        std::getline(std::cin, message);
        //std::cout << std::flush;

        if (message == "q")
        {
            break;
        }

        if (!connection.sendMessage(name + ": " + message))
        {
            std::cout << "Send message failed, server disconnected\n";
            break;
        }
    }
}

int main(int argc, char** argv)
{
    #ifndef COMPILE_TCP_SERVER
    // This needs to be defined in the CMakeLists.txt, throw an error if not defined
    #error "COMPILE_TCP_SERVER must be defined in CMakeLists.txt"
    #endif

    if (COMPILE_TCP_SERVER)
    {
        testTcpServer2();
    }
    else
    {
        std::string name = "Client";
        if (argc == 2)
        {
            name = argv[1];
        }
        testTcpClient2(name);
    }
    return 0;
}