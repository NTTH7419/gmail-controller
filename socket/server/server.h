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
#include <vector>
#include <chrono>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT "55555"

using namespace std;


class Server;
class Command;
class ShutdownCommand;
class GetFileCommand;
class ListFileCommand;
class DeleteFileCommand;

class Server{
private:
    WSADATA wsaData;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    char receive_buffer[DEFAULT_BUFLEN];
    int receive_buffer_len = DEFAULT_BUFLEN;
    SOCKET server_socket = INVALID_SOCKET;
    SOCKET client_socket = INVALID_SOCKET;

    vector<Command*> commands;
public:
    Server();
    void initialize();
    int createAddressInfo();
    int createSocket();
    int bindSocket();
    int listenForConnection();
    void listenForRequest();
    int acceptConnection();

    SOCKET& getClientSocket();
    int process(string);
    int receive(string&);
    void echo(const string& message);
    ~Server();


};


class Command{
public:
    virtual bool isCommand(const string& command) = 0;
    virtual void execute(Server& server, const string& param) = 0;
};

class ShutdownCommand : public Command{
public:
    bool isCommand(const string& command) override;
    void execute(Server& server, const string& param) override;
};

class ListFileCommand : public Command{
private:
    vector<string> listFile(const string& path);

public:
    bool isCommand(const string& command) override;
    void execute(Server& server, const string& param) override;
};

class GetFileCommand : public Command{
private:
    void sendFile(Server& server, const string& filename);
public:
    bool isCommand(const string& command) override;
    void execute(Server& server, const string& param) override;
};


class DeleteFileCommand : public Command{
private:
    void deleteFile(Server& server, const string& filename);
public:
    bool isCommand(const string& command) override;
    void execute(Server& server, const string& param) override;
};