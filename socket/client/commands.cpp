#include "commands.h"
#define SUCCESS 0
#define FAILURE 1

void Command::setIP(string ip){
    this->ip = ip;
}

ProcessCommand::ProcessCommand() : message(), response() {
    commands.insert({{"shutdown", new ShutdownCommand},
                     {"restart", new RestartCommand},
                     {"listapp", new ListAppCommand},
                     {"startapp", new StartAppCommand},
                     {"stopapp", new StopAppCommand},
                     //{"listser", new ListSerCommand},
                     //{"startser", new StartSerCommand},
                     //{"stopser", new StopSerCommand},
                     {"listfile", new ListFileCommand},
                     {"getfile", new GetFileCommand},
                     {"deletefile", new DeleteFileCommand},
                     {"screenshot", new ScreenshotCommand},
                    //  {"takephoto", new TakePhotoCommand},
                     //{"startrecord", new StartRecordCommand},
                     //{"stoprecord", new StopRecordCommand},
                     //{"record", new RecordCommand},
                     //{"startkeylog", new StartKeylogCommand},
                     //{"stopkeylog", new StopKeylogCommand},
                     //{"keylog", new KeylogCommand},
                     //{"error", new HandleErrorCommand}
                     });
}

ProcessCommand::~ProcessCommand() {
    for (auto it = commands.begin(); it != commands.end(); it++) {
        delete it->second;
    }
    commands.clear();
}

string ProcessCommand::getCommand() {
    string command = message.getSubject();
    if (commands.find(command) == commands.end()) return "error";
    return command;
}

string ProcessCommand::getIP() {
    string body = message.getBody();
    string ip = body.substr(0, body.find("\r\n"));
    return ip;
}

string ProcessCommand::getParameter() {
    string body = message.getBody();
    string param = body.substr(body.find('\n') + 1);
    return param;
}

//! Remember to delete couts
void ProcessCommand::executeCommand(Client& client) {
    if (message.isEmpty()) return;
	string command = getCommand();
    string ip = getIP();
	string param = getParameter();
	string send_string = command + "\n" + param;
	SOCKET server_socket = client.getServerSocket(ip);

    if (server_socket == INVALID_SOCKET) {
        cerr << "Invalid socket for IP: " << ip << endl;
        return;
    }
    
    int error_code = 0;
	if (send(server_socket, send_string.c_str(), send_string.length(), 0) == SOCKET_ERROR){
        cerr << "Error: " << WSAGetLastError() << endl;
        return;
    }
    else
        cout << "command sent: " << send_string << endl;

    commands[command]->setIP(ip);
    commands[getCommand()]->execute(client, getParameter());
    
    sendResponse("Command executed");
    message.clear();
}

bool ProcessCommand::getLatestMessage() {
	// string query = "from:phungngoctuan5@gmail.com is:unread";
    string query = "from:quangthangngo181@gmail.com is:unread";
    message = gmailapi.getLatestMessage(query);
    if (message.isEmpty()) return false;
    gmailapi.markAsRead(message.getGmailID());
    return true;
}

void ProcessCommand::sendResponse(const string& response_string, const Attachment& attachment) {
    Message response;
    response.setSubject("Re: " + message.getSubject());
    response.setBody(response_string);

    gmailapi.replyMessage(message, response, attachment);
}

//* for testing only
void ProcessCommand::process(Client& client){
    string input;
    while(true){
        cout << endl << "Input command (\"end\" to end the program): " << endl;
        getline(cin, input);
        int comma = input.find(' ');
        string command = input;
        string param = "";
        string ip = ";";
        if (comma != string::npos) {
            command = input.substr(0, comma);
            param = input.substr(comma + 1);
        }

        if(send(client.getServerSocket(ip), input.c_str(), input.length(), 0) == 0)
            cout << "command sent: " << input << endl;
        else
            cerr << WSAGetLastError() << endl;

        if (command == "end") return;

        if (commands.find(command) != commands.end()) {

            commands[command]->execute(client, param);
        }
        else cout << "Error: invalid command" << endl;
    }
}

//*Shutdown Command
void ShutdownCommand::execute(Client& client, const string& param){
    cout << "Shutting down server from client" << endl;
}

//*Get File Command
void GetFileCommand::execute(Client& client, const string& param){
    receiveFile(client, ip);
    string response = client.receiveResponse(ip);
    cout << response << endl;
}

//*Delete File Command
void DeleteFileCommand::execute(Client& client, const string& param){
    string response = client.receiveResponse(ip);
    cout << response << endl;
}

//* List File Command
void ListFileCommand::execute(Client& client, const string& param){
    receiveFile(client, ip);
    string response = client.receiveResponse(ip);

    cout << response << endl;
}

void ListAppCommand::execute(Client& client, const string& param){
    receiveFile(client, ip);
    string response = client.receiveResponse(ip);
    cout << response << endl;
}

void StartAppCommand::execute(Client& client, const string& param){
    cout << "Starting application..." << endl;
    string response = client.receiveResponse(ip);

    cout << response << endl;
}

void StopAppCommand::execute(Client& client, const string& param){
    cout << "Stopping applications..." << endl;
    string response = client.receiveResponse(ip);
    cout << response << endl;
}

void RestartCommand::execute(Client& client, const string& param){
    cout << "restarting server..." << endl;
}


void ScreenshotCommand::execute(Client& client, const string& param){
    receiveFile(client, ip);
    string response = client.receiveResponse(ip);
    cout << response << endl;
}

int receiveFile(Client& client, string ip){
    const int buffer_len = DEFAULT_BUFLEN;
    char buffer[buffer_len];
    SOCKET server_socket = client.getServerSocket(ip);

    int byte_received = recv(server_socket, buffer, sizeof(buffer), 0);
    buffer[byte_received] = '\0';

    if (strcmp(buffer, "error") == 0){
        cout << "Error: File could not be sent" << endl;
        return 1;
    }

    char* sep = strchr(buffer, '|');
    string file_name = string(buffer).substr(0, sep - buffer);
    int file_size = stoi(string(buffer).substr(sep - buffer + 1));
    cout << "ready to receive file" << endl;
    int count = 0;

    ofstream file("temp\\" + file_name, ios::binary);
    if (!file) {
        cerr << "Failed to create file: " << file_name << endl;
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
    cout << "file received" << endl;
    return 0;
}