#include "client.h"

Client::Client(){

}

void Client::initialize(){
    int error = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (error != 0) {
        cout << "WSAStartup failed with error: " << error << endl;
        return;
    }
    cout << "WSAStartup succeeded" << endl;

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
}

void Client::connectToServer(char* address){
    int error = getaddrinfo(address, DEFAULT_PORT, &hints, &result);
    if (error != 0 ) {
        cout << "getaddrinfo failed with error: " << error;
        return;
    }

    for(ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        server_socket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (server_socket == INVALID_SOCKET) {
            cout << "socket failed with error: " << WSAGetLastError();
            WSACleanup();
            return;
        }
        error = connect( server_socket, ptr->ai_addr, (int)ptr->ai_addrlen);

        if (error == SOCKET_ERROR) {
            closesocket(server_socket);
            server_socket = INVALID_SOCKET;
            continue;
        }

        break;
    }

    if (server_socket == INVALID_SOCKET) {
        cout << "Unable to connect to server!" << endl;
        return;
    }
    cout << "Connected!" << endl;

    freeaddrinfo(result);      
}

SOCKET& Client::getServerSocket(){
    return server_socket;
}


Client::~Client(){
    freeaddrinfo(result);
    closesocket(server_socket);
    WSACleanup();
}

//! Remember to delete couts
bool receiveFile(Client& client){
    const int buffer_len = DEFAULT_BUFLEN;
    char buffer[buffer_len];
    SOCKET& server_socket = client.getServerSocket();

    int byte_received = recv(server_socket, buffer, sizeof(buffer), 0);
    buffer[byte_received] = '\0';

    if (strcmp(buffer, "Error") == 0){
        cout << "Error: File could not be sent" << endl;
        return false;
    }

    string file_name = string(buffer);

    cout << "ready to receive file" << endl;
    std::ofstream file(file_name, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to create file: " << file_name << std::endl;
        return false;
    }
    
    do{
        byte_received = recv(server_socket, buffer, sizeof(buffer), 0);
        file.write(buffer, byte_received);
    }
    while(byte_received == 1024);

    file.close();
    cout << "file received" << endl;
    return true;
}