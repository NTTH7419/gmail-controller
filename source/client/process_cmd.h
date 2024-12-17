#include "client_cmd.h"

class ProcessCommand {
    private:
        HWND& hwndMain;
        static const std::string directory;
        GmailAPI gmailapi;
        Message message;
        std::string response;
        std::string sender_query;
        std::unordered_map<std::string, Command*> commands;

        void updateSenderQuery();
        std::string getCommand() const;
        std::string getIP() const;
        std::string getParameter() const;
        void sendResponse(const std::string& response_string, const Attachment& attachment = {});
        void processResponse();
    public:
        void displayLog(const std::string& log);
        ProcessCommand(HWND& hwndMain);
        // ProcessCommand();
        ~ProcessCommand();
        void executeCommand(Client& client);
        bool getLatestMessage();
};