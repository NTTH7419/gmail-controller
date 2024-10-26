#include "client.h"

void receiveFile(SOCKET sock, const char* filename) {
    char buffer[1024];
    int rec = recv(sock, buffer, sizeof(buffer), 0);
    char* recvc = new char[rec + 1] {'\0'};
    strncpy(recvc, buffer, rec);

    if (strcmp(recvc, "sending file") != 0) return;

    cout << "ready to received" << endl;
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to create file: " << filename << std::endl;
        return;
    }

    int bytesReceived;
    
    do{
        bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
        file.write(buffer, bytesReceived);
    }
    while(bytesReceived == 1024);

    if (bytesReceived == 0)
        cout << "con cac" << endl;


    file.close();
    cout << "file received" << endl;
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

    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            cout << "socket failed with error: " << WSAGetLastError();
            WSACleanup();
            return;
        }
        error = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);

        if (error == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }

        break;
    }

    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Unable to connect to server!" << endl;
        return;
    }
    cout << "Connected!" << endl;

    freeaddrinfo(result);      
}

void Client::process(){
    string input;
    while(true){
        getline(cin, input);
        send(ConnectSocket, input.c_str(), input.length(), 0);
        if (input == "end"){
            return;
        }

        if (input == "send file"){
            receiveFile(ConnectSocket, "adudu.png");
        }

        
    }
}

Client::~Client(){
    freeaddrinfo(result);
    closesocket(ConnectSocket);
    WSACleanup();
}


int __cdecl main(int argc, char **argv) 
{    
    // Validate the parameters
    if (argc != 2) {
        cout << "usage: client.exe [server-name]" << endl;
        return 1;
    }

    Client c;
    c.initialize();
    c.connectToServer(argv[1]);
    c.process();

    return 0;
}