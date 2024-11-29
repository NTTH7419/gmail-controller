#undef UNICODE

#define WIN32_LEAN_AND_MEAN
// #define _WIN32_WINNT 0x501

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <string>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>
#include <tchar.h>
#include <psapi.h>
#include <shellapi.h>



#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Psapi.lib")

#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT "55555"
#define DISCOVERY_PORT 6666

typedef SHORT PROPID;
using namespace std;



class Server;

class Server{
private:
    WSADATA wsaData;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    char buffer[DEFAULT_BUFLEN];
    int buffer_len = DEFAULT_BUFLEN;
    SOCKET server_socket = INVALID_SOCKET;
    SOCKET client_socket = INVALID_SOCKET;

    string ip;
    char hostname[256];
    
    int createAddressInfo();
    int createSocket();
    int bindSocket();
    int acceptConnection();
    int getIP();
    

public:
    Server();
    void initialize();
    int listenForConnection();
    void broadcastIP();
    void broadcastDiscovery();

    SOCKET& getClientSocket();
    int process(string);
    string receive();
    void echo(const string& message);
    ~Server();
};


int sendFile(Server& server, const string& filepath);