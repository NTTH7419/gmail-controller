#include "client.h"

Client::Client(){
    commands.push_back(new ShutdownCommand);
    commands.push_back(new GetFileCommand);
    commands.push_back(new DeleteFileCommand);
    commands.push_back(new ListFileCommand);
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



void Client::process(){
    string input;
    while(true){
        cout << endl << "Input command (\"end\" to end the program): " << endl;
        getline(cin, input);
        int comma = input.find(' ');
        string command = input;
        string param = "";
        if (comma != string::npos){
            command = input.substr(0, comma);
            param = input.substr(comma + 1);
        }

        send(server_socket, input.c_str(), input.length(), 0);
        cout << "command sent: " << input << endl;

        if (command == "end") return;

        for (int i = 0; i < commands.size(); i++){
            if (commands[i]->isCommand(command)){
                commands[i]->execute(*this, param);
                cout << "command executed!" << endl;
                break;
            }
        }
        
    }
}

SOCKET& Client::getServerSocket(){
    return server_socket;
}


Client::~Client(){
    while(!commands.empty()){
        delete commands.back();
        commands.pop_back();
    }

    freeaddrinfo(result);
    closesocket(server_socket);
    WSACleanup();
}


//*Shutdown Command
bool ShutdownCommand::isCommand(const string& command){
    return command == "shutdown";
}

void ShutdownCommand::execute(Client& client, const string& param){

}

//*Get File Command
bool GetFileCommand::isCommand(const string& command){
    return command == "getf";
}

void GetFileCommand::execute(Client& client, const string& param){
    receiveFile(client, param);
}

void GetFileCommand::receiveFile(Client& client, const string& filepath){
    const int buffer_len = DEFAULT_BUFLEN;
    char buffer[buffer_len];
    SOCKET& server_socket = client.getServerSocket();
    string filename = filepath;

    int last_slash = filepath.rfind('\\');
    if (last_slash != string::npos){
        filename = filepath.substr(last_slash + 1);
    }


    int byte_received = recv(server_socket, buffer, sizeof(buffer), 0);
    buffer[byte_received] = '\0';

    if (strcmp(buffer, "sending file") != 0){
        cout << "Error: File could not be sent" << endl;
        return;
    }

    cout << "ready to receive file" << endl;
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to create file: " << filename << std::endl;
        return;
    }
    
    do{
        byte_received = recv(server_socket, buffer, sizeof(buffer), 0);
        file.write(buffer, byte_received);
    }
    while(byte_received == 1024);

    file.close();
    cout << "file received" << endl;
}

//*Delete File Command
bool DeleteFileCommand::isCommand(const string& command){
    return command == "deletef";
}

void DeleteFileCommand::execute(Client& client, const string& filepath){
    system(("del " + filepath).c_str());
    cout << "deleted file at: " << filepath << endl;
}

//* List File Command
bool ListFileCommand::isCommand(const string& command){
    return command == "listf";
}

void ListFileCommand::execute(Client& client, const string& param){
    const int buffer_len = DEFAULT_BUFLEN;
    char buffer[buffer_len];
    SOCKET& server_socket = client.getServerSocket();

    int byte_received = recv(server_socket, buffer, buffer_len, 0);
    buffer[byte_received] = '\0';
    cout << buffer << endl;

    if (strcmp(buffer, "listing file") != 0){
        cout << "Error! Initial response not received!" << endl;
        return;
    }

    cout << "current files in the directory " << param << ':' << endl;
    char* stop;
    do{
        byte_received = recv(server_socket, buffer, buffer_len, 0);
        buffer[byte_received] = '\0';
        stop = strstr(buffer, "FILE_LISTED");
        if(stop != NULL){
            buffer[stop - buffer] = '\0';
            cout << buffer;
            break;
        }
        cout << buffer;
    }
    while(byte_received > 0);

    cout << "file listed successfully !" << endl;
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