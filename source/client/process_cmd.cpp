#include "process_cmd.h"

const std::string ProcessCommand::directory = "temp\\";

ProcessCommand::ProcessCommand() : message(), response(), gmailapi() {
    updateSenderQuery();
    if (!std::filesystem::exists(directory.substr(0, directory.size() - 1))) {
        std::filesystem::create_directory(directory.substr(0, directory.size() - 1));
    }
    commands.insert({{"shutdown", new ShutdownCommand},
                     {"getips", new GetIPsCommand},
                     {"restart", new RestartCommand},
                     {"listapps", new ListAppCommand},
                     {"startapp", new StartAppCommand},
                     {"stopapp", new StopAppCommand},
                     {"listsers", new ListSerCommand},
                     {"startser", new StartSerCommand},
                     {"stopser", new StopSerCommand},
                     {"listfiles", new ListFileCommand},
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
    
    try {
        gmailapi.initOAuth();
        addLog("Authenticate successfully");
    }
    catch (GmailError& ge) {
        addLog(ge.what());
    }
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

    addLog("Updated valid email list.");
}

std::string ProcessCommand::getCommand() const{
    std::string command = message.getSubject();
    if (commands.find(command) == commands.end()) return "invalid";
    return command;
}

std::string ProcessCommand::getIP() const{
    std::string body = message.getBody();
    std::string ip = body.substr(0, body.find("\r\n"));
    return ip;
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
        addLog("Received wrong command.");
        message.clear();
        return;
    }

    // commands that does not require connection to server
    if (command == "help" || command == "getips") {
        commands[command]->execute(client);
        response = commands[command]->getResponse();
        addLog("Command \"" + command + "\" has been executed.");
        processResponse("");
        message.clear();
        return;
    }

    // check ip
    std::string ip = getIP();
	SOCKET server_socket = client.getServerSocket(ip);
    if (server_socket == INVALID_SOCKET) {
        std::string error_response;
        error_response = "You have entered \"" + ip + "\", which is an invalid IP address.\n";
        error_response += "Please try again, or send command \"getips\" to get the list of available computers.";
        sendResponse(error_response);
        message.clear();
        addLog("Invalid IP: " + ip);
        return;
    }

    addLog("Command \"" + command + "\" has been sent to " + ip + ".");
    commands[command]->setCommandInfo(ip, command, param);
    commands[command]->execute(client);
    response = commands[command]->getResponse();
    processResponse(ip);
    message.clear();
}

bool ProcessCommand::getLatestMessage() {
    try {
        message = gmailapi.getLatestMessage(sender_query);
        if (message.isEmpty()) return false;
        gmailapi.markAsRead(message.getGmailID());
    }
    catch (GmailError& ge) {
        addLog(ge.what());
        return false;
    }
    return true;
}


void ProcessCommand::sendResponse(const std::string& response_string, const Attachment& attachment) {
    Message response;
    response.setSubject("Re: " + message.getSubject());
    response.setBody(response_string);

    try {
    gmailapi.replyMessage(message, response, attachment);
    }
    catch (GmailError& ge) {
        addLog(ge.what());
        if (ge.getCode() == ErrorCode::ATTACHMENT_TOO_LARGE) {
            response.setBody("Attachment is too large to send.");
            gmailapi.replyMessage(message, response);
            return;
        }
    }
    catch (std::runtime_error& re) {
        addLog(re.what());
    }
}

void ProcessCommand::processResponse(const std::string& ip) {
    json j;
    try {
        j = json::parse(response);
        if (j["status"] == -1) {
            sendResponse("An error has occur when executing the command (IP: " + ip + ").");
            addLog(ip + ": An error has occur when executing the command.");
            return;
        }
        std::string file_name = std::string(j["file"]);
        if (j["status"] == SUCCESS && !file_name.empty()) {
            sendResponse(j["message"], Attachment(directory + file_name, file_name));
            system(("del " + directory + file_name).c_str());
        }
        else {
            sendResponse(j["message"]);
        }
        addLog(ip + ": " + std::string(j["message"]));
    }
    catch (std::exception& e) {
        sendResponse("An error has occur when processing server response (IP: " + ip + ").");
        addLog(ip + ": An error has occur when processing server response.");
    }
}