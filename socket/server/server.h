#undef UNICODE

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x501

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <string>
#include <istream>
#include <ostream>
#include <fstream>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT "55555"

using namespace std;



class Server{
private:
    WSADATA wsaData;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    char receive_buffer[DEFAULT_BUFLEN];
    int receive_buffer_len = DEFAULT_BUFLEN;
    SOCKET server_socket = INVALID_SOCKET;
    SOCKET client_socket = INVALID_SOCKET;

public:
    void initialize();
    int createAddressInfo();
    int createSocket();
    int bindSocket();
    int listenForConnection();
    void listenForRequest();
    int process(string);
    int acceptConnection();
    int receive(string&);
    int echo(string);
    void sendFile(string);
    ~Server();


};