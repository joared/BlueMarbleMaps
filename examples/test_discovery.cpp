#include "BlueMarbleMaps/Networking/Socket.h"
#include "BlueMarbleMaps/Core/Serialization/Json/JsonValue.h"
#include <iostream>
#include <algorithm>

using namespace BlueMarble::Networking;

static std::string hostName = "MyDiscovery";
constexpr size_t MAX_MESSAGE_SIZE = 256;

std::string getWhoAmIMessage(const EndPoint& endpoint)
{
    auto json = BlueMarble::JsonValue::Object();
    json["addr"] = endpoint.address;
    json["port"] = endpoint.port;

    return BlueMarble::JsonValue(json).toString();
}

EndPoint parseWhoAmIMessage(const std::string& message)
{
    auto val = BlueMarble::JsonValue::fromString(message);
    if (!val.hasValue())
    {
        std::cout << "failed to parse who am i message\n";
    }

    auto& json = val.asObject();

    return EndPoint{json["addr"].asString(), (int)json["port"].asInteger()};
}

void testDiscoveryClient(std::string name)
{
    Socket txSocket = Socket(Socket::SocketType::Udp);
    txSocket.setSocketOptions({ 
                .broadcastEnabled = true,
                .reuseAddressEnabled = true });

    auto txEndPoint = EndPoint{ "255.255.255.255", 8080 }; // Local network
    auto rxEndPoint = EndPoint{ "0.0.0.0", 8080 };
    for (;;)
    {
        std::string message = BlueMarble::JsonValue({{ "query", hostName }}).toString();

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

        EndPoint sender;
        std::string response;
        response.resize(MAX_MESSAGE_SIZE);
        int nReceived = txSocket.receiveFrom(response.data(),
            response.size(),
            sender);

        if (nReceived <= 0)
        {
            std::cout << "Receive failed or socket closed\n";
            return;
        }

        std::cout << "Received: " << response << "\n";
        break;
    }
}

void testDiscoveryServer(std::string name)
{
    auto rxEndPoint = EndPoint{ "0.0.0.0", 8080 };

    Socket rxSocket = Socket(Socket::SocketType::Udp);
    
    rxSocket.setSocketOptions({ 
                .broadcastEnabled = true,
                .reuseAddressEnabled = true });
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

        std::cout << "Received: " << message << " (from: " << sender.address << " : " << std::to_string(sender.port) << ") \n";

        auto value = BlueMarble::JsonValue::fromString(message);

        if (!value.isObject())
        {
            std::cout << "Received something that is not an object\n";
            continue;
        }
        auto& json = value.asObject();

        if (json.find("query") == json.end())
        {
            std::cout << "got a message without query, ignoring...";
            continue;
        }

        if (json["query"].asString() != hostName)
        {
            std::cout << "received invalid request\n";
            continue;
        }

        auto response = getWhoAmIMessage(EndPoint{"192.168.1.149", 8080});
        rxSocket.sendTo(response.data(), response.size(), sender);
    };
}


int main(int argc, char** argv)
{
    #ifndef COMPILE_SERVER
    // This needs to be defined in the CMakeLists.txt, throw an error if not defined
    #error "COMPILE_SERVER must be defined in CMakeLists.txt"
    #endif

    if (COMPILE_SERVER)
    {
        //testUdpServer2();
        std::string name = "Client";
        if (argc == 2)
        {
            name = argv[1];
        }
        testDiscoveryServer(name);
    }
    else
    {
        std::string name = "Client";
        if (argc == 2)
        {
            name = argv[1];
        }
        testDiscoveryClient(name);
    }
    return 0;
}