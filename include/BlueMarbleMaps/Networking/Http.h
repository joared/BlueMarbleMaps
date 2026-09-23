#ifndef HTTP
#define HTTP

#include "TcpConnection.h"
#include <string>
#include <vector>

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

#endif /* HTTP */
