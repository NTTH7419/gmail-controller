#include "commands.h"


const std::string Command::directory = "temp\\";
const std::string ProcessCommand::directory = "temp\\";


void Command::setCommandInfo(const std::string& ip, const std::string& command, const std::string& param){
    this->ip = ip;
    this->command = command;
    this->parameter = param;
}

bool Command::sendCommand(Client& client){
    SOCKET server_socket = client.getServerSocket(ip);
    std::string send_string = command + '-' + parameter;

    if (send(server_socket, send_string.c_str(), send_string.length(), 0) == SOCKET_ERROR){
        std::cerr << "Error: the server on the " << ip << " is no longer available" << std::endl;
        return 0;
    }
    
    std::cout << "command sent: " << send_string << std::endl; 
    
    return 1;
}  

std::string Command::getResponse(){
    std::string temp = response;
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
    std::ifstream fin("valid_email.txt");
    if (!fin.good()) {
        sender_query = "is:unread";
        return;
    }

    sender_query.clear();
    std::string email;
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

std::string ProcessCommand::getCommand() const{
    std::string command = message.getSubject();
    if (commands.find(command) == commands.end()) return "invalid";
    return command;
}

bool ProcessCommand::isValidIP(const std::string &ip) const{
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

std::string ProcessCommand::getIP() const{
    std::string body = message.getBody();
    std::string ip = body.substr(0, body.find("\r\n"));
    if (isValidIP(ip))
        return ip;
    else
        return "invalid";
}

std::string ProcessCommand::getParameter() const{
    std::string body = message.getBody();
    int idx = body.find("\r\n");
    if (idx == body.npos) {
        return "";
    }
    std::string param = body.substr(idx + 2);
    return param;
}

//! Remember to delete couts
void ProcessCommand::executeCommand(Client& client) {
    if (message.isEmpty()) return;

	std::string command = getCommand();
    std::string param = getParameter();

    // check command
    if (command == "invalid") {
        std::string error_response;
        error_response = "You have entered the wrong command.\n";
        error_response += "Please try again, or send command \"help\" to get the list of available commands.";
        sendResponse(error_response);
        message.clear();
        std::cerr << "Wrong command" << std::endl;
        return;
    }

    // commands that does not require connection to server
    if (command == "help"|| command == "getips") {
        commands[command]->execute(client);
        response = commands[command]->getResponse();
        processResponse();
        message.clear();
        std::cout << "Command \"" << command << "\" has been executed";
        return;
    }

    // check ip
    std::string ip = getIP();
	SOCKET server_socket = client.getServerSocket(ip);
    if (server_socket == INVALID_SOCKET) {
        std::string error_response;
        error_response = "You have entered an invalid IP address.\n";
        error_response += "Please try again, or send command \"getips\" to get the list of available computers.";
        sendResponse(error_response);
        message.clear();
        std::cerr << "Invalid socket for IP: " << ip << std::endl;
        response = R"({"status": )" + std::to_string(1) + R"(, "file": ")" + "" + R"(", "message": ")" + "Invalid socket for IP: " + ip + R"("})";
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


void ProcessCommand::sendResponse(const std::string& response_string, const Attachment& attachment) {
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
            sendResponse(j["message"], Attachment(directory + std::string(j["file"]), j["file"]));
        else
            sendResponse(j["message"]);

    }
    catch (std::exception& e) {
        sendResponse("An error has occur when processing server response.");
    }
}

//* for testing only
void ProcessCommand::process(Client& client){
    std::string input;
    while(true){
        std::cout << std::endl << "Input command (\"end\" to end the program): " << std::endl;
        std::getline(std::cin, input);
        int comma = input.find(' ');
        std::string command = input;
        std::string param = "";
        std::string ip = "";
        if (comma != std::string::npos) {
            command = input.substr(0, comma);
            param = input.substr(comma + 1);
        }

        if(send(client.getServerSocket(ip), input.c_str(), input.length(), 0) > 0)
            std::cout << "command sent: " << input << std::endl;
        else
            std::cerr << WSAGetLastError() << std::endl;

        if (command == "end") return;

        if (commands.find(command) != commands.end()) {
            // commands[command]->setIP(ip);
            commands[command]->execute(client);
        }
        else std::cout << "Error: invalid command" << std::endl;
    }
}

//*Shutdown Command
void ShutdownCommand::execute(Client& client){
    if (!sendCommand(client)){
        //* response
    }
    else{
        std::cout << "shutting down server from client..." << std::endl;
        response = client.receiveResponse(ip);
    }
    std::cout << response << std::endl;
}

bool ShutdownCommand::validateParameter() const {
    return true;
}

void RestartCommand::execute(Client& client){
    if (!sendCommand(client)){
        //* response
    }
    else{
        std::cout << "restarting server..." << std::endl;
        response = client.receiveResponse(ip);
    }
    std::cout << response << std::endl;
}

bool RestartCommand::validateParameter() const {
    return true;
}

//*Get File Command
void GetFileCommand::execute(Client& client){
    if (!validateParameter()){
        std::cout << "Invalid parameter" << std::endl;
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
        
    std::cout << response << std::endl;
}

bool GetFileCommand::validateParameter() const {
    if (parameter.find("C:\\") == 0 && parameter.find('/') == std::string::npos)
        return true;
    return false;
}

//*Delete File Command
void DeleteFileCommand::execute(Client& client){
    if (!validateParameter()){
        std::cout << "Invalid parameter" << std::endl;
        //* response
        return;
    }
        
    if (!sendCommand(client)){
        //* response
    }
    else{
        response = client.receiveResponse(ip);
    }
        
    std::cout << response << std::endl;
}

bool DeleteFileCommand::validateParameter() const {
    if (parameter.find("C:\\") == 0 && parameter.find('/') == std::string::npos)
        return true;
    return false;
}

//* List File Command
void ListFileCommand::execute(Client& client){
    if (!validateParameter()){
        std::cout << "Invalid parameter" << std::endl;
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

    std::cout << response << std::endl;
}

bool ListFileCommand::validateParameter() const {
    if (parameter.find("C:\\") == 0 && parameter.find('/') == std::string::npos)
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
    std::cout << response << std::endl;
}

bool ListAppCommand::validateParameter() const {
    return true;
}

void StartAppCommand::execute(Client& client){
    if (!validateParameter()){
        std::cout << "Invalid parameter" << std::endl;
        //* response
        return;
    }

    if (!sendCommand(client)){
        //*response
    }
    else{
        std::cout << "Starting application..." << std::endl;
        response = client.receiveResponse(ip);

    }

    std::cout << response << std::endl;
}

bool StartAppCommand::validateParameter() const {
    return true;
}

void StopAppCommand::execute(Client& client){
    if (!validateParameter()){
        std::cout << "Invalid parameter" << std::endl;
        //* response
        return;
    }
    
    if (!sendCommand(client)){
        //* response
    }
    else{
        std::cout << "Stopping applications..." << std::endl;
        response = client.receiveResponse(ip);

    }
    std::cout << response << std::endl;
}

bool StopAppCommand::validateParameter() const {
    return true;
}


void ScreenshotCommand::execute(Client& client){
    if (!validateParameter()){
        std::cout << "Invalid parameter" << std::endl;
        return;
    }

    if (!sendCommand(client)){
        //*response
    }

    else{
        receiveFile(client, ip, directory);
        response = client.receiveResponse(ip);
    }
    std::cout << response << std::endl;
}

bool ScreenshotCommand::validateParameter()  const {
    return true;
}


void HelpCommand::execute(Client& client) {
    int status = SUCCESS;
    std::stringstream ss;
    std::ifstream fin(directory + '\\' + "help.txt");
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
    std::string message;
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
        response = client.receiveResponse(ip);
    }
}

bool ListSerCommand::validateParameter() const{
    return true;
}

bool TakePhotoCommand::validateParameter() const{
    return true;
}

void TakePhotoCommand::execute(Client& client){
    if (!sendCommand(client)){
        //response
    }else{
        receiveFile(client, ip, directory);
        response = client.receiveResponse(ip);
    }
}