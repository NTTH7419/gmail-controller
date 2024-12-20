#include "process_cmd.h"

const std::string ProcessCommand::directory = "temp\\";

ProcessCommand::ProcessCommand(HWND& hwndMain) : message(), response(), hwndMain(hwndMain), gmailapi() {
    updateSenderQuery();
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
    }
    catch (GmailError& ge) {
        displayLog(ge.what());
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

void ProcessCommand::displayLog(const std::string& log){
    time_t curr_time;
	curr_time = time(NULL);

	tm *tm_local = localtime(&curr_time);
    LPARAM msg = (LPARAM)new std::string(
        "[" + std::to_string(tm_local->tm_hour) + ":" 
            + std::to_string(tm_local->tm_min) + ":" 
            + std::to_string(tm_local->tm_sec) + "] " 
            + log + '\n');

    PostMessage(hwndMain, WM_APP, 0, msg);
}

void ProcessCommand::executeCommand(Client& client) {
    if (message.isEmpty()) return;

	std::string command = getCommand();
    std::string param = getParameter();

    // check command
    if (command == "invalid") {
        std::string error_response;
        error_response = "You have entered the wrong command.";
        error_response += "Please try again, or send command \"help\" to get the list of available commands.";
        sendResponse(error_response);
        displayLog("Received wrong command.");
        message.clear();
        return;
    }

    // commands that does not require connection to server
    if (command == "help" || command == "getips") {
        commands[command]->execute(client);
        response = commands[command]->getResponse();
        displayLog("Command \"" + command + "\" has been executed.");
        processResponse();
        message.clear();
        return;
    }

    // check ip
    std::string ip = getIP();
	SOCKET server_socket = client.getServerSocket(ip);
    if (server_socket == INVALID_SOCKET) {
        std::string error_response;
        error_response = "You have entered an invalid IP address.";
        error_response += "Please try again, or send command \"getips\" to get the list of available computers.";
        sendResponse(error_response);
        message.clear();
        displayLog("Invalid IP: " + ip);
    }
    else{
        commands[command]->setCommandInfo(ip, command, param);
        commands[command]->execute(client);
        response = commands[command]->getResponse();
        processResponse();
    }


    json j = json::parse(response);
    displayLog(j["message"]);

    message.clear();
}

bool ProcessCommand::getLatestMessage() {
    try {
        message = gmailapi.getLatestMessage(sender_query);
        if (message.isEmpty()) return false;
        gmailapi.markAsRead(message.getGmailID());
    }
    catch (GmailError& ge) {
        displayLog(ge.what());
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
        displayLog(ge.what());
        if (ge.getCode() == ErrorCode::ATTACHMENT_TOO_LARGE) {
            response.setBody("Attachment is too large to send.");
            gmailapi.replyMessage(message, response);
            return;
        }
    }
}

void ProcessCommand::processResponse() {
    json j;
    try {
        j = json::parse(response);
        if (j["status"] == -1) {
            sendResponse("An error has occur when executing the command.");
            return;
        }
        if (j["status"] == SUCCESS && j["file"] != "")
            sendResponse(j["message"], Attachment(directory + std::string(j["file"]), j["file"]));
        else
            sendResponse(j["message"]);
    }
    catch (std::exception& e) {
        sendResponse("An error has occur when processing server response.");
        displayLog("An error has occur when processing server response.");
    }
}