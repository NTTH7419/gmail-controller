#include "commands.h"


const string Command::directory = "temp\\";
const string ProcessCommand::directory = "temp\\";


void Command::setIP(const string& ip){
    this->ip = ip;
}

string Command::getResponse() {
    string temp = response;
    response = "";
    return temp;
}

ProcessCommand::ProcessCommand() : message(), response() {
    updateSenderQuery();
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
                     //{"takephoto", new TakePhotoCommand},
                     //{"startrecord", new StartRecordCommand},
                     //{"stoprecord", new StopRecordCommand},
                     //{"startkeylog", new StartKeylogCommand},
                     //{"stopkeylog", new StopKeylogCommand},
                     {"help", new HelpCommand}
                     });
}

ProcessCommand::~ProcessCommand() {
    for (auto it = commands.begin(); it != commands.end(); it++) {
        delete it->second;
    }
    commands.clear();
}

void ProcessCommand::updateSenderQuery() {
    ifstream fin("valid_email.txt");
    if (!fin.good()) {
        sender_query = "is:unread";
        return;
    }

    sender_query.clear();
    string email;
    if (!fin.eof()) {
        fin >> email;
        if (email.empty()) {
            sender_query = "is:unread";
            fin.close();
            return;
        }
        sender_query += "from:" + email + ' ';
        email.clear();
    }
    while (!fin.eof()) {
        fin >> email;
        if (email.empty()) break;
        sender_query += "OR from:" + email + ' ';
        email.clear();
    }
    sender_query += "is:unread";
    fin.close();
}

string ProcessCommand::getCommand() {
    string command = message.getSubject();
    if (commands.find(command) == commands.end()) return "invalid";
    return command;
}

bool ProcessCommand::isValidIP(const string &ip) {
    // Define a regular expression for a valid IPv4 address
    const std::regex pattern("^(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})$");
    std::smatch match;

    // Check if IP matches the pattern
    if (regex_match(ip, match, pattern)) {
        // Validate each octet is within the range 0-255
        for (int i = 1; i <= 4; ++i) {
            int octet = std::stoi(match[i].str());
            if (octet < 0 || octet > 255) {
                return false;
            }
        }
        return true;
    }
    return false;
}

string ProcessCommand::getIP() {
    string body = message.getBody();
    string ip = body.substr(0, body.find("\r\n"));
    if (isValidIP(ip))
        return ip;
    else
        return "invalid";
}

string ProcessCommand::getParameter() {
    string body = message.getBody();
    int idx = body.find("\r\n");
    if (idx == body.npos) {
        return "";
    }
    string param = body.substr(idx + 2);
    return param;
}

//! Remember to delete couts
void ProcessCommand::executeCommand(Client& client) {
    if (message.isEmpty()) return;

	string command = getCommand();

    // check command
    if (command == "invalid") {
        string error_response;
        error_response = "You have entered the wrong command.\n";
        error_response += "Please try again, or send command \"help\" to get the list of available commands.";
        sendResponse(error_response);
        message.clear();
        cerr << "Wrong command" << endl;
        return;
    }

    // commands that does not require connection to server
    if (command == "help") {
        commands[command]->execute(client, "");
        response = commands[command]->getResponse();
        processResponse();
        message.clear();
        cout << "Command \"" << command << "\" has been executed";
        return;
    }

    // check ip
    string ip = getIP();
	SOCKET server_socket = client.getServerSocket(ip);
    if (server_socket == INVALID_SOCKET) {
        string error_response;
        error_response = "You have entered an invalid IP address.\n";
        error_response += "Please try again, or send command \"getips\" to get the list of available computers.";
        sendResponse(error_response);
        message.clear();
        cerr << "Invalid socket for IP: " << ip << endl;
        return;
    }
    
    string param = getParameter();
	string send_string = command + '-' + param;
	if (send(server_socket, send_string.c_str(), send_string.length(), 0) == SOCKET_ERROR){
        cerr << "Error: " << WSAGetLastError() << endl;
        return;
    }
    else
        cout << "command sent: " << send_string << endl;

    // execute command
    commands[command]->setIP(ip);
    commands[command]->execute(client, getParameter());
    response = commands[command]->getResponse();
    processResponse();
    message.clear();
}

bool ProcessCommand::getLatestMessage() {
    message = gmailapi.getLatestMessage(sender_query);
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

void ProcessCommand::processResponse() {
    json j;
    try {
        j = json::parse(response);
        if (j["status"] == -1) {
            sendResponse("An error has occur when executing the command");
            return;
        }
        if (j["status"] == SUCCESS && j["file"] != "")
            sendResponse(j["message"], Attachment(directory + string(j["file"]), j["file"]));
        else
            sendResponse(j["message"]);

    }
    catch (exception& e) {
        sendResponse("An error has occur when processing server response.");
    }
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
        string ip = "";
        if (comma != string::npos) {
            command = input.substr(0, comma);
            param = input.substr(comma + 1);
        }

        if(send(client.getServerSocket(ip), input.c_str(), input.length(), 0) > 0)
            cout << "command sent: " << input << endl;
        else
            cerr << WSAGetLastError() << endl;

        if (command == "end") return;

        if (commands.find(command) != commands.end()) {
            commands[command]->setIP(ip);
            commands[command]->execute(client, param);
        }
        else cout << "Error: invalid command" << endl;
    }
}

//*Shutdown Command
void ShutdownCommand::execute(Client& client, const string& param){
    cout << "Shutting down server from client" << endl;
    response = client.receiveResponse(ip);
    cout << response << endl;
}

void RestartCommand::execute(Client& client, const string& param){
    cout << "restarting server..." << endl;
    response = client.receiveResponse(ip);
    cout << response << endl;
}

//*Get File Command
void GetFileCommand::execute(Client& client, const string& param){
    receiveFile(client, ip, directory);
    response = client.receiveResponse(ip);
    cout << response << endl;
}

//*Delete File Command
void DeleteFileCommand::execute(Client& client, const string& param){
    response = client.receiveResponse(ip);
    cout << response << endl;
}

//* List File Command
void ListFileCommand::execute(Client& client, const string& param){
    receiveFile(client, ip, directory);
    response = client.receiveResponse(ip);

    cout << response << endl;
}

void ListAppCommand::execute(Client& client, const string& param){
    receiveFile(client, ip, directory);
    response = client.receiveResponse(ip);
    cout << response << endl;
}

void StartAppCommand::execute(Client& client, const string& param){
    cout << "Starting application..." << endl;
    response = client.receiveResponse(ip);

    cout << response << endl;
}

void StopAppCommand::execute(Client& client, const string& param){
    cout << "Stopping applications..." << endl;
    response = client.receiveResponse(ip);
    cout << response << endl;
}


void ScreenshotCommand::execute(Client& client, const string& param){
    receiveFile(client, ip, directory);
    response = client.receiveResponse(ip);
    cout << response << endl;
}


void HelpCommand::execute(Client& client, const string& param) {
    int status = SUCCESS;
    stringstream ss;
    ifstream fin(directory + '\\' + "help.txt");
    if (fin.good()) {
        ss << fin.rdbuf();
        fin.close();
    }
    else {
        status = FAILURE;
        ss << "Help error: Cannot access the content of the help file.";
    }

    json j;
    j["status"] = status;
    j["message"] = ss.str();
    j["file"] = "";
    response = j.dump();
}