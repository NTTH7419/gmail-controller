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

void Client::receiveAvailableIP(){
    SOCKET udp_recv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_recv == INVALID_SOCKET) {
        cerr << "Socket creation failed!" << endl;
        return;
    }

    int broadcastEnable = 1;
    if (setsockopt(udp_recv, SOL_SOCKET, SO_BROADCAST, (char*)&broadcastEnable, sizeof(broadcastEnable)) == SOCKET_ERROR) {
        std::cerr << "setsockopt failed to enable broadcast!" << std::endl;
        closesocket(udp_recv);
        return;
    }

    sockaddr_in localAddr;
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = 6666;       // Port to listen to
    localAddr.sin_addr.s_addr = INADDR_ANY; // Listen on all network interfaces

    if (bind(udp_recv, (struct sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed!" << std::endl;
        closesocket(udp_recv);
        WSACleanup();
        return;
    }

    sockaddr_in senderAddr;
    int senderAddrSize = sizeof(senderAddr);
    
    while (true) {
        int bytes_received = recvfrom(udp_recv, recvbuf, recvbuflen, 0, (struct sockaddr*)&senderAddr, &senderAddrSize);
        if (bytes_received == SOCKET_ERROR) {
            cerr << "recvfrom failed!" << endl;
            break;
        }

        if (bytes_received > 0){
            recvbuf[bytes_received] = '\0';
            string ip(inet_ntoa(senderAddr.sin_addr));

            cout << "Computer: " << recvbuf << endl;
            break;
        }
    }


    closesocket(udp_recv);
}

void Client::connectToServer(const char* address){
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

string Client::receiveResponse(){
    int byte_received = recv(server_socket, recvbuf, recvbuflen, 0);
    recvbuf[byte_received] = '\0';

    if (byte_received < 0){
        return "Error";
    }

    string response(recvbuf);

    return response;
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
void receiveFile(Client& client){
    const int buffer_len = DEFAULT_BUFLEN;
    char buffer[buffer_len];
    SOCKET& server_socket = client.getServerSocket();

    int byte_received = recv(server_socket, buffer, sizeof(buffer), 0);
    buffer[byte_received] = '\0';

    if (strcmp(buffer, "error") == 0){
        cout << "Error: File could not be sent" << endl;
        return;
    }

    string file_name = string(buffer);
    cout << file_name << endl;;
    cout << "ready to receive file" << endl;
    ofstream file(file_name, ios::binary);
    if (!file) {
        cerr << "Failed to create file: " << file_name << endl;
        return;
    }
    
    do{
        byte_received = recv(server_socket, buffer, buffer_len, 0);
        file.write(buffer, byte_received);
    }
    while(byte_received == buffer_len);

    file.close();
    cout << "file received" << endl;
}