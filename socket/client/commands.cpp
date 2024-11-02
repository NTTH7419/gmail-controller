#include "commands.h"

ProcessCommand::ProcessCommand() : message(), response() {
    commands.insert({{"shutdown", new ShutdownCommand},
                     //{"restart", new RestartCommand},
                     //{"listapp", new ListAppCommand},
                     //{"startapp", new StartAppCommand}
                     //{"stopapp", new StopAppCommand},
                     //{"listser", new ListSerCommand},
                     //{"startser", new StartSerCommand}
                     //{"stopser", new StopSerCommand},
                     {"listfile", new ListFileCommand},
                     {"getfile", new GetFileCommand},
                     {"deletefile", new DeleteFileCommand},
                     //{"screenshot", new ScreenshotCommand},
                     //{"takephoto", new TakePhotoCommand},
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
    string ip = body.substr(0, body.find('\n'));
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
	string param = getParameter();
	string send_string = command + '\n' + param;
	SOCKET server_socket = client.getServerSocket();
    
	send(server_socket, send_string.c_str(), send_string.length(), 0);
    cout << "command sent: " << send_string << endl;

    commands[getCommand()]->execute(client, getParameter());
    message.clear();
}

bool ProcessCommand::getLatestMessage() {
	string query = "from:phungngoctuan5@gmail.com";
    message = gmailapi.getLatestMessage(query);
    if (message.isEmpty()) return false;
    gmailapi.setMessageRead(message.getID());
    return true;
}

void ProcessCommand::sendResponse() {
    Message response;
    response.setFromEmail(message.getToEmail());
    response.setToEmail(message.getFromEmail());
    response.setSubject(message.getSubject());
    response.setBody("Command executed");

    gmailapi.sendMessage(response, message.getThreadID());
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
        if (comma != string::npos){
            command = input.substr(0, comma);
            param = input.substr(comma + 1);
        }

        send(client.getServerSocket(), input.c_str(), input.length(), 0);
        cout << "command sent: " << input << endl;

        if (command == "end") return;

        if (commands.find(command) != commands.end()) {
            commands[command]->execute(client, param);
        }
        else cout << "Error: invalid command" << endl;
    }
}

//*Shutdown Command
void ShutdownCommand::execute(Client& client, const string& param){
    cout << "Shutting down server";
}

//*Get File Command
void GetFileCommand::execute(Client& client, const string& param){
    receiveFile(client);
}

//*Delete File Command
void DeleteFileCommand::execute(Client& client, const string& filepath){
    system(("del " + filepath).c_str());
    cout << "deleted file at: " << filepath << endl;
}

//* List File Command
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