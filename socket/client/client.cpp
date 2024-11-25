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

void Client::sendDiscovery(){
    SOCKET udp_discovery = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    BOOL opt = TRUE;
    setsockopt(udp_discovery, SOL_SOCKET, SO_BROADCAST, (char*)&opt, sizeof(opt));
    sockaddr_in broadcastAddr, serverAddr;
    int serverAddrLen = sizeof(serverAddr);

    // Setup broadcast address
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = DISCOVERY_PORT;
    broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;

    string message = "DISCOVER_SERVER";

    // Broadcast the discovery message
    sendto(udp_discovery, message.c_str(), message.size(), 0, (sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
    int bytes_received;  
    

    timeval timeout;
    timeout.tv_sec = 10;  // 3 seconds timeout
    timeout.tv_usec = 0;
    setsockopt(udp_discovery, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));


    do{
        bytes_received = recvfrom(udp_discovery, buffer, buffer_len, 0, (sockaddr*)&serverAddr, &serverAddrLen);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            cout << "Computer: " << buffer << endl;
            string ip = string(buffer);
            ip = ip.substr(ip.find(": ") + 2);
            connectToServer(ip.c_str());
            cout << "connected to computer with ip: " << ip << endl;

        } 
        else{
            // Break the loop if no more responses or timeout occurs
            if (WSAGetLastError() == WSAETIMEDOUT) {
                cout << "Timeout reached. No more responses." << endl;
                break;
            }
        }
    }
    while(true);

    // Cleanup
    closesocket(udp_discovery);

}

void Client::connectToServer(const char* address){
    int error = getaddrinfo(address, DEFAULT_PORT, &hints, &result);
    if (error != 0 ) {
        cout << "getaddrinfo failed with error: " << error;
        return;
    }

    server_sockets[string(address)] = INVALID_SOCKET;
    for(ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        server_sockets[string(address)] = SOCKET(socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol));

        if (server_sockets[string(address)] == INVALID_SOCKET) {
            cout << "socket failed with error: " << WSAGetLastError();
            return;
        }
        error = connect(server_sockets[string(address)], ptr->ai_addr, (int)ptr->ai_addrlen);

        if (error == SOCKET_ERROR) {
            closesocket(server_sockets[string(address)]);
            continue;
        }

        break;
    }

    if (server_sockets[string(address)] == INVALID_SOCKET) {
        cout << "Unable to connect to server!" << endl;
        return;
    }
    cout << "Connected!" << endl;

    cout << address << endl;

    freeaddrinfo(result);
}

string Client::receiveResponse(string ip){
    int byte_received = recv(server_sockets[ip], buffer, buffer_len, 0);
    buffer[byte_received] = '\0';

    if (byte_received < 0){
        return "Error: response not received";
    }

    string response(buffer);

    return response;
}

SOCKET Client::getServerSocket(string ip){
    for (auto it = server_sockets.begin(); it != server_sockets.end(); it++){
        if (it->first == ip)
            return it->second;
    }
    return INVALID_SOCKET;
}


Client::~Client(){
    for (auto it = server_sockets.begin(); it != server_sockets.end(); it++)
        closesocket(it->second);

    server_sockets.clear();

    freeaddrinfo(result);
    WSACleanup();
}

//! Remember to delete couts
