#include "server_socket.h"

Server::Server(){

}

void Server::initialize(){
    int error = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (error != 0){
        std::cout << "WSAStartup failed, error: " << error << std::endl;
    }
    std::cout << "WSAStartup succeeded! " << std::endl;

    if (createAddressInfo() != 0) return;

    if (createSocket() != 0) return;

    if (bindSocket() != 0) return;

    freeaddrinfo(result);
    std::cout << "Initialization completed!" << std::endl;
    std::cout << "Server IP address: " << ip << std::endl;
    // broadcastIP();
    broadcastDiscovery();
}

void Server::broadcastDiscovery(){
    SOCKET udp_discovery = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in serverAddr, clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = DISCOVERY_PORT;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Bind to all interfaces
    bind(udp_discovery, (sockaddr*)&serverAddr, sizeof(serverAddr));

    std::cout << "UDP Discovery Server is running..." << std::endl;

    int recvLen = recvfrom(udp_discovery, buffer, buffer_len ,0, (sockaddr*)&clientAddr, &clientAddrLen);
    if (recvLen > 0) {
        buffer[recvLen] = '\0';
        std::cout << "Received discovery request: " << buffer << std::endl;
        
        std::cout << "Sending server info" << std::endl;

        std::string response = std::string(hostname) + ": " + ip;
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
        std::cerr << "Error getting hostname: " << WSAGetLastError() << std::endl;
        ip = "";
        return 1;
    }

    int error = getaddrinfo(hostname, DEFAULT_PORT, &hints, &result);
    if (error != 0) {
        std::cout << "Get address failed, error: " << error << std::endl;
        WSACleanup();
        return error;
    }

    struct sockaddr_in* addr = (struct sockaddr_in*)result->ai_addr;
    ip = inet_ntoa(addr->sin_addr);

    std::cout << "Get address successfully! " << std::endl;
    return error;
}

int Server::createSocket(){
    server_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (server_socket == INVALID_SOCKET) {
        std::cout << "Create socket failed, error: " << WSAGetLastError() << std::endl;
        return 1;
    }
    std::cout << "Create socket successfully" << std::endl;
    return 0;
}

int Server::bindSocket(){
    int error = bind(server_socket, result->ai_addr, (int)result->ai_addrlen);
    if (error == SOCKET_ERROR) {
        std::cout << "Bind socket failed, error: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        closesocket(server_socket);
        return 1;
    }
    std::cout << "Bind socket successfully!" << std::endl;
    return 0;
}

int Server::listenForConnection(){
    if (listen(server_socket, 1) == SOCKET_ERROR) {
        std::cout << "listen for connection failed, error: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        return 1;
    }
    std::cout << "Listening... " << std::endl;

    if (acceptConnection() != 0) return 1;

    return 0;
}

int Server::acceptConnection(){
    client_socket = accept(server_socket, NULL, NULL);
    if (client_socket == INVALID_SOCKET) {
        std::cout << "Failed to accept a connection, error: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        return 1;
    }
    int flag = 1;
    setsockopt(client_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));
    std::cout << "Connected!" << std::endl;
    return 0;
}

std::string Server::receive(){
    int bytes_received = recv(client_socket, buffer, buffer_len, 0);
    if (bytes_received >= 0) {
        char* receive_message = new char[bytes_received + 1] {'\0'};
        strncpy(receive_message, buffer, bytes_received);
        std::string message = std::string(receive_message);
        delete [] receive_message;
        std::cout << "message received: " << message << std::endl;
        return message;
    } 
    else {
        std::cout << "receive message failed, error: " << WSAGetLastError() << std::endl;
    }

    return "error";
}

SOCKET& Server::getClientSocket(){
    return client_socket;
}

void Server::echo(const std::string& message){
    // Echo the buffer back to the sender
    int bytes_sent = send(client_socket, message.c_str(), (int)message.length(), 0);
    if (bytes_sent == SOCKET_ERROR) {
        std::cout << "Failed to send a response to the client, error: " << WSAGetLastError() << std::endl;
        return;
    }
    std::cout << "message: " <<  message << " was sent successfully" << std::endl;
}

bool Server::isClientAlive(){
    if (recv(client_socket, buffer, buffer_len, MSG_PEEK) <= 0){
        return false;
    }
    return true;
}

Server::~Server(){

    freeaddrinfo(result);
    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();
}

int sendFile(Server& server, const std::string& filepath) {
    SOCKET client_socket = server.getClientSocket();

    std::string file_name;
    int last_slash = filepath.rfind('\\');
    if (last_slash != std::string::npos){
        file_name = filepath.substr(last_slash + 1);
    }
    else file_name = filepath;
    std::ifstream file(filepath.c_str(), std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        server.echo("error");
        return 1;
    }

    file.seekg(0, std::ios::end);
    int file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    server.echo(file_name + '|' + std::to_string(file_size));
    const int buff_len = DEFAULT_BUFLEN;
    char send_buffer[buff_len];
    while (file.read(send_buffer, buff_len).gcount() > 0) {
        send(client_socket, send_buffer, static_cast<int>(file.gcount()), 0);
    }
    file.close();
    std::cout << "file sent" << std::endl;
    return 0;
}

