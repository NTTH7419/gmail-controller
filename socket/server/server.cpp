#include "server.h"

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
}

int Server::createAddressInfo(){
    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    int error = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (error != 0) {
        cout << "Get address failed, error: " << error << endl;
        WSACleanup();
        return error;
    }
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
    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "listen for connection failed, error: " << WSAGetLastError() << endl;
        closesocket(server_socket);
        return 1;
    }
    cout << "Listening... " << endl;

    if (acceptConnection() != 0) return 1;

    return 0;
}

void Server::listenForRequest(){
    while(true){
        string message;
        int bytes_received = receive(message);
        if (string(message) == "end") return;

        process(string(message));
    }
}

int Server::process(string message){
    if (message == "send file")
        sendFile("memaybeo.png");
        
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

int Server::receive(string &message){
    int bytes_received = recv(client_socket, receive_buffer, receive_buffer_len, 0);
    if (bytes_received > 0) {
        cout << "Bytes received: " << bytes_received << endl;
        char* receive_message = new char[bytes_received + 1] {'\0'};
        strncpy(receive_message, receive_buffer, bytes_received);
        message = string(receive_message); 
        delete [] receive_message;
        cout << "message received: " << message << endl;
    } 
    else {
        cout << "receive message failed, error: " << WSAGetLastError() << endl;
        closesocket(client_socket);
        WSACleanup();
    }

    return bytes_received;
}

Server::~Server(){
    freeaddrinfo(result);
    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();
}

void Server::sendFile(string filename) {
    std::ifstream file(filename.c_str(), std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        echo("Error");
        return;
    }

    echo("sending file");
    char send_buffer[DEFAULT_BUFLEN];
    while (file.read(send_buffer, DEFAULT_BUFLEN).gcount() > 0) {
        send(client_socket, send_buffer, static_cast<int>(file.gcount()), 0);
    }
    file.close();
    send(client_socket, send_buffer, 0, 0);
    cout << "file sent" << endl;
}

int Server::echo(string message){
    // Echo the buffer back to the sender
    int bytes_sent = send(client_socket, message.c_str(), (int)message.length(), 0);
    if (bytes_sent == SOCKET_ERROR) {
        cout << "Failed to send a response to the client, error: " << WSAGetLastError() << endl;
        closesocket(client_socket);
        WSACleanup();
    }

    cout << "Bytes sent: " << bytes_sent << endl;
    cout << "message sent: " << message << endl;
    return bytes_sent;
}

int __cdecl main(void) {
    Server server;
    server.initialize();
    server.listenForConnection();
    server.listenForRequest();



    return 0;
}