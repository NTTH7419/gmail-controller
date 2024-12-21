#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <unordered_map>
#include <iostream>
#include <cstring>
#include <string>
#include <thread>
#include <fstream>
#include <tchar.h>
#include <psapi.h>
#include <vector>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT "55555"
#define DISCOVERY_PORT 6666

class Client;
void addLog(const std::string &log);

class Client{
private:
    WSADATA wsaData;
    std::unordered_map<std::string, SOCKET> server_sockets;
    std::vector<std::string> ips;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    char buffer[DEFAULT_BUFLEN];
    int buffer_len = DEFAULT_BUFLEN;

public:
    Client();
    void initialize();
    void sendDiscovery();
    SOCKET getServerSocket(std::string ip);
    bool connectToServer(const char* address);
    std::string receiveResponse(std::string ip);
    std::vector<std::string> getIPList();
    ~Client();
};

int receiveFile(Client& client, const std::string& ip, const std::string& directory);