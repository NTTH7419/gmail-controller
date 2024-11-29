#include "commands.h"


const string Command::directory = "temp\\";
const string ProcessCommand::directory = "temp\\";


void Command::setCommandInfo(const string& ip, const string& command, const string& param){
    this->ip = ip;
    this->command = command;
    this->parameter = param;
}

bool Command::sendCommand(Client& client){
    SOCKET server_socket = client.getServerSocket(ip);
    string send_string = command + '-' + parameter;

    if (send(server_socket, send_string.c_str(), send_string.length(), 0) == SOCKET_ERROR){
        cerr << "Error: the server on the " << ip << " is no longer available" << endl;
        return 0;
    }
    
    cout << "command sent: " << send_string << endl; 
    
    return 1;
}  

string Command::getResponse(){
    string temp = response;
    response = "";
    return temp;
}

ProcessCommand::ProcessCommand() : message(), response() {
    updateSenderQuery();
    commands.insert({{"shutdown", new ShutdownCommand},
                     {"getips", new GetIPsCommand},
                     {"restart", new RestartCommand},
                     {"listapp", new ListAppCommand},
                     {"startapp", new StartAppCommand},
                     {"stopapp", new StopAppCommand},
                     {"listser", new ListSerCommand},
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

string ProcessCommand::getCommand() const{
    string command = message.getSubject();
    if (commands.find(command) == commands.end()) return "invalid";
    return command;
}

bool ProcessCommand::isValidIP(const string &ip) const{
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

string ProcessCommand::getIP() const{
    string body = message.getBody();
    string ip = body.substr(0, body.find("\r\n"));
    if (isValidIP(ip))
        return ip;
    else
        return "invalid";
}

string ProcessCommand::getParameter() const{
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
    string param = getParameter();

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
    if (command == "help"|| command == "getips") {
        commands[command]->execute(client);
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
        response = R"({"status": )" + to_string(1) + R"(, "file": ")" + "" + R"(", "message": ")" + "Invalid socket for IP: " + ip + R"("})";
    }
    else{
        commands[command]->setCommandInfo(ip, command, param);
        commands[command]->execute(client);
        response = commands[command]->getResponse();
    }

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
            // commands[command]->setIP(ip);
            commands[command]->execute(client);
        }
        else cout << "Error: invalid command" << endl;
    }
}

//*Shutdown Command
void ShutdownCommand::execute(Client& client){
    if (!sendCommand(client)){
        //* response
    }
    else{
        cout << "shutting down server from client..." << endl;
        response = client.receiveResponse(ip);
    }
    cout << response << endl;
}

bool ShutdownCommand::validateParameter() const {
    return true;
}

void RestartCommand::execute(Client& client){
    if (!sendCommand(client)){
        //* response
    }
    else{
        cout << "restarting server..." << endl;
        response = client.receiveResponse(ip);
    }
    cout << response << endl;
}

bool RestartCommand::validateParameter() const {
    return true;
}

//*Get File Command
void GetFileCommand::execute(Client& client){
    if (!validateParameter()){
        cout << "Invalid parameter" << endl;
        //* response
        return;
    }
        
    if (!sendCommand(client)){
        //* response
    }
    else{
        receiveFile(client, ip, directory);
        response = client.receiveResponse(ip);
    }
        
    cout << response << endl;
}

bool GetFileCommand::validateParameter() const {
    if (parameter.find("C:\\") == 0 && parameter.find('/') == string::npos)
        return true;
    return false;
}

//*Delete File Command
void DeleteFileCommand::execute(Client& client){
    if (!validateParameter()){
        cout << "Invalid parameter" << endl;
        //* response
        return;
    }
        
    if (!sendCommand(client)){
        //* response
    }
    else{
        response = client.receiveResponse(ip);
    }
        
    cout << response << endl;
}

bool DeleteFileCommand::validateParameter() const {
    if (parameter.find("C:\\") == 0 && parameter.find('/') == string::npos)
        return true;
    return false;
}

//* List File Command
void ListFileCommand::execute(Client& client){
    if (!validateParameter()){
        cout << "Invalid parameter" << endl;
        //* response
        return;
    }

    if (!sendCommand(client)){
        //*response
    }
    else{
        receiveFile(client, ip, directory);
        response = client.receiveResponse(ip);
    }

    cout << response << endl;
}

bool ListFileCommand::validateParameter() const {
    if (parameter.find("C:\\") == 0 && parameter.find('/') == string::npos)
        return true;
    return false;
}

void ListAppCommand::execute(Client& client){
    if (!sendCommand(client)){
        //* response
    }
    else{
        receiveFile(client, ip, directory);
        response = client.receiveResponse(ip);
    }
    cout << response << endl;
}

bool ListAppCommand::validateParameter() const {
    return true;
}

void StartAppCommand::execute(Client& client){
    if (!validateParameter()){
        cout << "Invalid parameter" << endl;
        //* response
        return;
    }

    if (!sendCommand(client)){
        //*response
    }
    else{
        cout << "Starting application..." << endl;
        response = client.receiveResponse(ip);

    }

    cout << response << endl;
}

bool StartAppCommand::validateParameter() const {
    return true;
}

void StopAppCommand::execute(Client& client){
    if (!validateParameter()){
        cout << "Invalid parameter" << endl;
        //* response
        return;
    }
    
    if (!sendCommand(client)){
        //* response
    }
    else{
        cout << "Stopping applications..." << endl;
        response = client.receiveResponse(ip);

    }
    cout << response << endl;
}

bool StopAppCommand::validateParameter() const {
    return true;
}


void ScreenshotCommand::execute(Client& client){
    if (!validateParameter()){
        cout << "Invalid parameter" << endl;
        return;
    }

    if (!sendCommand(client)){
        //*response
    }

    else{
        receiveFile(client, ip, directory);
        response = client.receiveResponse(ip);
    }
    cout << response << endl;
}

bool ScreenshotCommand::validateParameter()  const {
    return true;
}


void HelpCommand::execute(Client& client) {
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

bool HelpCommand::validateParameter() const{
    return true;
}

bool GetIPsCommand::validateParameter() const{
    return true;
}

void GetIPsCommand::execute(Client& client){
    int status = SUCCESS;
    string message;
    message = "List of available IPs: \n";

    for (auto ip : client.getIPList()) {
        message += ip + "\n";
    }

    json j;
    j["status"] = status;
    j["message"] = message;
    j["file"] = "";

    response = j.dump();
}



void ListSerCommand::execute(Client& client) {
    if (!sendCommand(client)){
        //* response
    }
    else{
        receiveFile(client, ip, directory);
        client.receiveResponse(ip);
    }
}

bool ListSerCommand::validateParameter() const{
    return true;
}