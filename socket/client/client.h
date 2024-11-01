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
#include <fstream>
#include <istream>
#include <ostream>
#include <vector>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT "55555"

using namespace std;

#include <winsock2.h>
#include <iostream>
#include <fstream>

#pragma comment(lib, "Ws2_32.lib")

class Client;
class Command;
class ShutdownCommand;
class GetFileCommand;
class ListFileCommand;
class DeleteFileCommand;

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
    vector<Command*> commands;
public:
    Client();
    void initialize();
    SOCKET& getServerSocket();
    void connectToServer(char* address);
    void process();
    int listFiles(string dir);
    ~Client();

};

class Command{
public:
    virtual bool isCommand(const string& command) = 0;
    virtual void execute(Client& Client, const string& param) = 0;
};

class ShutdownCommand : public Command{
public:
    bool isCommand(const string& command) override;
    void execute(Client& client, const string& param) override;
};

class ListFileCommand : public Command{
private:
    vector<string> listFile(const string& path);

public:
    bool isCommand(const string& command) override;
    void execute(Client& client, const string& param) override;
};

class GetFileCommand : public Command{
private:
    void receiveFile(Client& client, const string& filename);
public:
    bool isCommand(const string& command) override;
    void execute(Client& client, const string& param) override;
};


class DeleteFileCommand : public Command{
private:
    void sendFile(Client& client, const string& filename);
public:
    bool isCommand(const string& command) override;
    void execute(Client& client, const string& param) override;
};