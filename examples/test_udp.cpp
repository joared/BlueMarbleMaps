#include "BlueMarbleMaps/Networking/Socket.h"
#include <iostream>
#include <algorithm>

using namespace BlueMarble::Networking;

void testUdpServer2()
{
    
}

void testUdpBroadCast(std::string name)
{
    constexpr size_t MAX_MESSAGE_SIZE = 256;
    auto txEndPoint = EndPoint{ "255.255.255.255", 8080 }; // Local network
    auto rxEndPoint = EndPoint{ "0.0.0.0", 8080 };

    Socket rxSocket = Socket(Socket::SocketType::Udp);
    

    std::thread rxThread([&rxSocket, &rxEndPoint, &txEndPoint]()
        {
            rxSocket.setSocketOptions({ .reuseAddressEnabled = true });
            if (!rxSocket.bind(rxEndPoint))
            {
                std::cout << "Filed to bind receive socket\n";
                return;
            }
            std::cout << "Started receiving thread. Listening on " + rxEndPoint.toString()  + "\n";
            for (;;)
            {
                EndPoint sender;
                std::string message;
                message.resize(MAX_MESSAGE_SIZE);
                int nReceived = rxSocket.receiveFrom(message.data(),
                    message.size(),
                    sender);

                if (nReceived <= 0)
                {
                    std::cout << "Receive failed or socket closed\n";
                    return;
                }
                
                message.resize(nReceived);

                if (sender == txEndPoint)// FIXME: This doesnt really work, this is not the actual endpoint
                {
                    std::cout << "Got my own message!\n";
                }
                else
                {
                    std::cout << "Received: " << message << " (from: " << sender.address << " : " << std::to_string(sender.port) << ") \n";
                }
            }
        }
    );

    Socket txSocket = Socket(Socket::SocketType::Udp);
    txSocket.setSocketOptions({ .broadcastEnabled = true });

    for (;;)
    {
        std::string message;
        std::getline(std::cin, message);
        message = name + ": " + message;

        message.resize(std::min(message.size(), MAX_MESSAGE_SIZE));

        int nSent = txSocket.sendTo(message.data(), message.size(), txEndPoint);
        
        if (nSent == 0)
        {
            std::cout << "Send socket closed\n";
            return;
        }

        if (nSent != message.size())
        {
            throw std::runtime_error("THIS SHOULD NEVER HAPPEN FOR UDP!");
        }

        std::cout << "Sent " << nSent << " bytes\n";
    }
}

void testUdpClient2(std::string name)
{
    
}

int main(int argc, char** argv)
{
    #ifndef COMPILE_UDP_SERVER
    // This needs to be defined in the CMakeLists.txt, throw an error if not defined
    #error "COMPILE_UDP_SERVER must be defined in CMakeLists.txt"
    #endif

    if (COMPILE_UDP_SERVER)
    {
        //testUdpServer2();
        std::string name = "Client";
        if (argc == 2)
        {
            name = argv[1];
        }
        testUdpBroadCast(name);
    }
    else
    {
        std::string name = "Client";
        if (argc == 2)
        {
            name = argv[1];
        }
        testUdpBroadCast(name);
    }
    return 0;
}