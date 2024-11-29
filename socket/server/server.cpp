#include "server.h"

Server::Server(){

}

void Server::initialize(){
    int error = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (error != 0){
        cout << "WSAStartup failed, error: " << error << endl;
    }
    cout << "WSAStartup succeeded! " << endl;

    if (createAddressInfo() != 0) return;

    if (createSocket() != 0) return;

    if (bindSocket() != 0) return;

    freeaddrinfo(result);
    cout << "Initialization completed!" << endl;
    cout << "Server IP address: " << ip << endl;
    // broadcastIP();
    broadcastDiscovery();
}

void Server::broadcastIP(){
    SOCKET udp_send = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (udp_send == INVALID_SOCKET) {
        cerr << "UDP socket creation failed." << endl;
        return;
    }

    BOOL broadcast = TRUE;
    if (setsockopt(udp_send, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast, sizeof(broadcast)) == SOCKET_ERROR){
        std::cerr << "Set broadcast option failed." << std::endl;
        closesocket(udp_send);
        return;
    }

    sockaddr_in broadcastAddr;
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = DISCOVERY_PORT;
    broadcastAddr.sin_addr.s_addr = inet_addr("255.255.255.255");

    string server_info = string(hostname) + "|" + ip;

    if (sendto(udp_send, server_info.c_str(), server_info.length(), 0, (sockaddr*)&broadcastAddr, sizeof(broadcastAddr)) == SOCKET_ERROR) {
        cerr << "Broadcast failed: " << WSAGetLastError() << endl;
    }

    cout << "IP broadcasted successfully" << endl;
    closesocket(udp_send);

}

void Server::broadcastDiscovery(){
    SOCKET udp_discovery = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in serverAddr, clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = DISCOVERY_PORT;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Bind to all interfaces
    bind(udp_discovery, (sockaddr*)&serverAddr, sizeof(serverAddr));

    cout << "UDP Discovery Server is running..." << endl;

    int recvLen = recvfrom(udp_discovery, buffer, buffer_len ,0, (sockaddr*)&clientAddr, &clientAddrLen);
    if (recvLen > 0) {
        buffer[recvLen] = '\0';
        cout << "Received discovery request: " << buffer << endl;
        
        cout << "Sending server info" << endl;

        string response = string(hostname) + "|" + ip;
        sendto(udp_discovery, response.c_str(), response.size(), 0, (sockaddr*)&clientAddr, clientAddrLen);
    }

    // Cleanup
    closesocket(udp_discovery);
}


int Server::createAddressInfo(){
    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    
    if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
        cerr << "Error getting hostname: " << WSAGetLastError() << endl;
        ip = "";
        return 1;
    }

    int error = getaddrinfo(hostname, DEFAULT_PORT, &hints, &result);
    if (error != 0) {
        cout << "Get address failed, error: " << error << endl;
        WSACleanup();
        return error;
    }

    struct sockaddr_in* addr = (struct sockaddr_in*)result->ai_addr;
    ip = inet_ntoa(addr->sin_addr);

    cout << "Get address successfully! " << endl;
    return error;
}

int Server::createSocket(){
    server_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (server_socket == INVALID_SOCKET) {
        cout << "Create socket failed, error: " << WSAGetLastError() << endl;
        return 1;
    }
    cout << "Create socket successfully" << endl;
    return 0;
}

int Server::bindSocket(){
    int error = bind(server_socket, result->ai_addr, (int)result->ai_addrlen);
    if (error == SOCKET_ERROR) {
        cout << "Bind socket failed, error: " << WSAGetLastError() << endl;
        freeaddrinfo(result);
        closesocket(server_socket);
        return 1;
    }
    cout << "Bind socket successfully!" << endl;
    return 0;
}

int Server::listenForConnection(){
    if (listen(server_socket, 1) == SOCKET_ERROR) {
        cout << "listen for connection failed, error: " << WSAGetLastError() << endl;
        closesocket(server_socket);
        return 1;
    }
    cout << "Listening... " << endl;

    if (acceptConnection() != 0) return 1;

    return 0;
}

int Server::acceptConnection(){
    client_socket = accept(server_socket, NULL, NULL);
    if (client_socket == INVALID_SOCKET) {
        cout << "Failed to accept a connection, error: " << WSAGetLastError() << endl;
        closesocket(server_socket);
        return 1;
    }
    int flag = 1;
    setsockopt(client_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));
    cout << "Connected!" << endl;
    return 0;
}

string Server::receive(){
    int bytes_received = recv(client_socket, buffer, buffer_len, 0);
    if (bytes_received >= 0) {
        char* receive_message = new char[bytes_received + 1] {'\0'};
        strncpy(receive_message, buffer, bytes_received);
        string message = string(receive_message);
        delete [] receive_message;
        cout << "message received: " << message << endl;
        return message;
    } 
    else {
        cout << "receive message failed, error: " << WSAGetLastError() << endl;
    }

    return "error";
}

SOCKET& Server::getClientSocket(){
    return client_socket;
}

void Server::echo(const string& message){
    // Echo the buffer back to the sender
    int bytes_sent = send(client_socket, message.c_str(), (int)message.length(), 0);
    if (bytes_sent == SOCKET_ERROR) {
        cout << "Failed to send a response to the client, error: " << WSAGetLastError() << endl;
        return;
    }
}

Server::~Server(){

    freeaddrinfo(result);
    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();
}

int sendFile(Server& server, const string& filepath) {
    SOCKET client_socket = server.getClientSocket();

    string file_name;
    int last_slash = filepath.rfind('\\');
    if (last_slash != string::npos){
        file_name = filepath.substr(last_slash + 1);
    }
    else file_name = filepath;
    ifstream file(filepath.c_str(), ios::binary);
    if (!file) {
        cerr << "Failed to open file: " << filepath << endl;
        server.echo("error");
        return 1;
    }

    file.seekg(0, ios::end);
    int file_size = file.tellg();
    file.seekg(0, ios::beg);
    server.echo(file_name + '|' + to_string(file_size));
    const int buff_len = DEFAULT_BUFLEN;
    char send_buffer[buff_len];
    while (file.read(send_buffer, buff_len).gcount() > 0) {
        send(client_socket, send_buffer, static_cast<int>(file.gcount()), 0);
    }
    file.close();
    cout << "file sent" << endl;
    return 0;
}
