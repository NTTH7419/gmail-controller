#include "client_cmd.h"


const std::string Command::directory = "temp\\";


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