
#include "client.h"
#include "gmailapi/gmail-api.h"
#pragma comment(lib, "Psapi.lib")

class Command;
class ProcessCommand;
class ShutdownCommand;
class ListFileCommand;
class GetFileCommand;
class DeleteFileCommand;
class ListAppCommand;
class StartAppCommand;

class Command{
protected:
    string ip;
public:
    virtual void execute(Client& Client, const string& param) = 0;
    void setIP(string ip);
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
        void sendResponse(const string& response_string, const Attachment& attachment = {});

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


class ListAppCommand : public Command{
public:
    void execute(Client& client, const string& param) override;
};

class StartAppCommand : public Command{
public: 
    void execute(Client& client, const string& param) override;
};

class StopAppCommand : public Command{
public: 
    void execute(Client& client, const string& param) override;
};

class RestartCommand : public Command{
public: 
    void execute(Client& client, const string& param) override;
};

class ScreenshotCommand : public Command{
public: 
    void execute(Client& client, const string& param) override;
};



int receiveFile(Client& client, string ip);