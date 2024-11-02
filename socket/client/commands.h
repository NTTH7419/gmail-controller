#include "client.h"
#include "gmailapi/gmail-api.h"

class Command{
public:
    virtual void execute(Client& Client, const string& param) = 0;
};

class ProcessCommand {
    private:
        GmailAPI gmailapi;
        Message message;
        string response;
        unordered_map<string, Command*> commands;
        string getCommand();
        string getIP();
        string getParameter();

    public:
        ProcessCommand();
        ~ProcessCommand();
        void executeCommand(Client& client);
        bool getLatestMessage();
        void sendResponse(const Attachment& attachment = {});

		void process(Client& c); //* for testing only  
};

class ShutdownCommand : public Command{
public:
    void execute(Client& client, const string& param) override;
};


class ListFileCommand : public Command{
private:
    vector<string> listFile(const string& path);

public:
    void execute(Client& client, const string& param) override;
};

class GetFileCommand : public Command{
public:
    void execute(Client& client, const string& param) override;
};


class DeleteFileCommand : public Command{
public:
    void execute(Client& client, const string& param) override;
};