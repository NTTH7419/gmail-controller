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

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "55555"

using namespace std;

int __cdecl main(void) {
    WSADATA wsaData;
    int iResult;

    struct addrinfo *result = NULL;
    struct addrinfo hints;
    
    SOCKET server_socket = INVALID_SOCKET;
    SOCKET client_socket = INVALID_SOCKET;

    char recvbuf[DEFAULT_BUFLEN];
    int iSendResult;
    int recvbuflen = DEFAULT_BUFLEN;

    
    


    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    cout << "WSAStartup succeeded, status: " <<  wsaData.szSystemStatus << endl;


    //Create address info
    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create and Initialize  the socket
    server_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (server_socket == INVALID_SOCKET) {
        cout << "socket() function failed, error: " << WSAGetLastError() << endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;

    }
    cout << "socket() function succeeded" << endl;


    // Bind the socket to a local address
    iResult = bind(server_socket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        cout << "bind() function failed, error: " << WSAGetLastError() << endl;
        freeaddrinfo(result);
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }
    
    cout << "bind() function succeeded" << endl;
    freeaddrinfo(result);

    //Listening for connections
    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "listen() function failed, error: " << WSAGetLastError() << endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }
    cout << "listening... " << endl;

    //accepting connections
    client_socket = accept(server_socket, NULL, NULL);
    if (client_socket == INVALID_SOCKET) {
        cout << "accept() function failed, error: " << WSAGetLastError() << endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }
    cout << "Accepted!" << endl;

    
    //*Receive and send data
    // Receive until the peer shuts down the connection
    do {

        iResult = recv(client_socket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            cout << "Bytes received: " << iResult << endl;
            char* contents = new char[iResult + 1] {'\0'};
            strncpy(contents, recvbuf, iResult);
            cout << "message received: " << contents << endl;
            delete [] contents;

            // Echo the buffer back to the sender
            iSendResult = send(client_socket, recvbuf, iResult, 0);
            if (iSendResult == SOCKET_ERROR) {
                cout << "send failed, error: " << WSAGetLastError() << endl;
                closesocket(client_socket);
                WSACleanup();
                return 1;
            }

            cout << "Bytes sent: " << iSendResult << endl;
        } else if (iResult == 0)
            cout << "Connection closing... " << endl;
        else {
            cout << "recv() function failed, error: " << WSAGetLastError() << endl;
            closesocket(client_socket);
            WSACleanup();
            return 1;
        }

    } while (iResult > 0);

    //*Shutdown
    // shutdown the send half of the connection since no more data will be sent
    iResult = shutdown(client_socket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        cout << "Shutdown failed, error: " << WSAGetLastError() << endl;    
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }


    closesocket(server_socket);
    WSACleanup();

    return 0;
}