#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x501

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
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

using namespace std;

class Client;

class Client{
private:
    WSADATA wsaData;
    SOCKET server_socket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    char *sendbuf;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

public:
    Client();
    void initialize();
    void receiveAvailableIP();
    SOCKET& getServerSocket();
    void connectToServer(const char* address);
    string receiveResponse();
    ~Client();
};


void receiveFile(Client& client);