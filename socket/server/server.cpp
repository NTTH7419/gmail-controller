#include "server.h"

Server::Server(){
    // commands.push_back(new ShutdownCommand());
    commands.push_back(new ListFileCommand());
    commands.push_back(new GetFileCommand());

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

    cout << "message sent: " << message << endl;
}

int Server::process(string message){
    int comma = message.find(' ');
    string command = message;
    string param = "";
    if (comma != string::npos){
        command = message.substr(0, comma);
        param = message.substr(comma + 1);
    }

    //executing command
    for (int i = 0; i < commands.size(); i++){
        if (commands[i]->isCommand(command)){
            commands[i]->execute(*this, param);
            return 0;
        }
    }
    cout << "Error: invalid command" << endl;
    return 1;
}

Server::~Server(){
    while(!commands.empty()){
        delete commands.back();
        commands.pop_back();
    }

    freeaddrinfo(result);
    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();
}


//* Shutdown
bool ShutdownCommand::isCommand(const string& command){
    return command == "shutdown";
}
void ShutdownCommand::execute(Server& server, const string& param){

}


//* Send file
bool GetFileCommand::isCommand(const string& command){
    return command == "getf";
}
void GetFileCommand::execute(Server& server, const string& param){
    sendFile(server, param);
}
void GetFileCommand::sendFile(Server& server, const string& filename) {
    std::ifstream file(filename.c_str(), std::ios::binary);
    SOCKET client_socket = server.getClientSocket();
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        server.echo("Error");
        return;
    }

    server.echo("sending file");
    char send_buffer[DEFAULT_BUFLEN];
    while (file.read(send_buffer, DEFAULT_BUFLEN).gcount() > 0) {
        send(client_socket, send_buffer, static_cast<int>(file.gcount()), 0);
    }
    file.close();
    send(client_socket, send_buffer, 0, 0);
    cout << "file sent" << endl;
}


//* List file
bool ListFileCommand::isCommand(const string& command){
    return command == "listf";
}
void ListFileCommand::execute(Server& server, const string& param){
    vector<string> v(listFile(param));
    SOCKET& client_socket = server.getClientSocket();

    server.echo("listing file");
    for (auto file : v){
        send(client_socket, (file + "\n").c_str(), file.length() + 1, 0);
        cout << file << endl;
    }
    send(client_socket, "FILE_LISTED", 11, 0);
}
vector<string> ListFileCommand::listFile(const string& path){
    string command("dir /a-d ");
    command.append(path + " > listfile.txt");
    system(command.c_str());

    ifstream fin("listfile.txt");

    if (!fin.is_open()){
        cout << "cannot open file";
    }

    string temp;
    //skipping the headers
    for (int i = 0; i < 4; i++)
        getline(fin, temp);

    vector<string> files;
    while(getline(fin, temp)){
        files.push_back(temp);
    }
    fin.close();
    files.pop_back();
    files.pop_back();

    return files;
}

//* Delete file
bool DeleteFileCommand::isCommand(const string& command){
    return command == "deletef";
}

void DeleteFileCommand::execute(Server& server, const string& param){
    
}


int __cdecl main(void) {
    Server server;
    server.initialize();
    if (server.listenForConnection() == 0)
        server.listenForRequest();

    return 0;
}