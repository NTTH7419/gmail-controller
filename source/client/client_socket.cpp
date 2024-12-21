#include "client_socket.h"

Client::Client(){}

void Client::initialize(){
    int error = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (error != 0) {
        addLog("WSAStartup failed with error: " + std::to_string(error) + ".");
        return;
    }

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

    std::string message = "DISCOVER_SERVER";

    addLog("Scanning for available computer servers...");
    sendto(udp_discovery, message.c_str(), message.size(), 0, (sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
    int bytes_received;  
    
    timeval timeout;
    timeout.tv_sec = 5000; 
    timeout.tv_usec = 0;
    setsockopt(udp_discovery, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));


    do{
        bytes_received = recvfrom(udp_discovery, buffer, buffer_len, 0, (sockaddr*)&serverAddr, &serverAddrLen);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            std::string ip = std::string(buffer);
            ip = ip.substr(ip.find(": ") + 2);
            if (connectToServer(ip.c_str()) == true){
                ips.push_back(std::string(buffer));
            }
            

        } 
        else{
            if (WSAGetLastError() == WSAETIMEDOUT) {
                // std::cout << "Timeout reached. No more responses." << std::endl;
                addLog("Timeout reached. Stop finding computer.");
                break;
            }
        }
    }
    while(true);


    // Cleanup
    closesocket(udp_discovery);
}

std::vector<std::string> Client::getIPList(){ 
    std::vector<std::string> result;

    for (int i = 0; i < ips.size(); i++) {
        std::string ip = (ips[i]).substr(ips[i].find(": ") + 2);
        if (send(server_sockets[ip], "", 0, 0) == SOCKET_ERROR){
            ips.erase(ips.begin() + i);
        }
    }

    return ips;
}

bool Client::connectToServer(const char* address){
    int error = getaddrinfo(address, DEFAULT_PORT, &hints, &result);
    if (error != 0 ) {
        addLog("getaddrinfo failed with error" + std::to_string(error) + ".");
        return false;
    }
    
    server_sockets[std::string(address)] = INVALID_SOCKET;
    for(ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        server_sockets[std::string(address)] = SOCKET(socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol));

        if (server_sockets[std::string(address)] == INVALID_SOCKET) {
            addLog("socket failed with error: " +  std::to_string(WSAGetLastError()));
            return false;
        }
        error = connect(server_sockets[std::string(address)], ptr->ai_addr, (int)ptr->ai_addrlen);

        if (error == SOCKET_ERROR) {
            closesocket(server_sockets[std::string(address)]);
            continue;
        }

        break;
    }

    if (server_sockets[std::string(address)] == INVALID_SOCKET) {
        addLog("Unable to connect to server(s).");
        return false;
    }
    addLog("Connected to computer with IP: " + std::string(address) + ".");

    freeaddrinfo(result);
    return true;
}

std::string Client::receiveResponse(std::string ip){
    int byte_received = recv(server_sockets[ip], buffer, buffer_len, 0);
    buffer[byte_received] = '\0';

    if (byte_received < 0){
        return "Error: response not received";
    }

    std::string response(buffer);
    // std::cout << "message: " << response << " received" << std::endl;

    return response;
}

SOCKET Client::getServerSocket(std::string ip){
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
int receiveFile(Client& client, const std::string& ip, const std::string& directory){
    const int buffer_len = DEFAULT_BUFLEN;
    char buffer[buffer_len];
    SOCKET server_socket = client.getServerSocket(ip);

    int byte_received = recv(server_socket, buffer, sizeof(buffer), 0);
    buffer[byte_received] = '\0';

    if (strcmp(buffer, "error") == 0){
        addLog(ip + ": Error, File could not be sent");
        return 1;
    }

    char* sep = strchr(buffer, '|');
    std::string file_name = std::string(buffer).substr(0, sep - buffer);
    int file_size = stoi(std::string(buffer).substr(sep - buffer + 1));
    int count = 0;

    std::ofstream file("temp\\" + file_name, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to create file: " << file_name << std::endl;
        return 1;
    }
    
    do{
        if (file_size - count > buffer_len)
            byte_received = recv(server_socket, buffer, buffer_len, 0);
        else
            byte_received = recv(server_socket, buffer, file_size - count, 0);

        file.write(buffer, byte_received);
        count += byte_received;
    }
    while(count < file_size);

    file.close();
    // std::cout << "file received" << std::endl;

    return 0;
}