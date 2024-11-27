#include "client.h"
#include "gmailapi/gmail-api.h"
#include <nlohmann/json.hpp>
#include <regex>
#pragma comment(lib, "Psapi.lib")

#define SUCCESS 0
#define FAILURE 1

using json = nlohmann::json;


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
    static const string directory;
    string response;
    string ip;
public:
    virtual void execute(Client& Client, const string& param) = 0;
    void setIP(const string& ip);
    string getResponse();
};

class ProcessCommand {
    private:
        static const string directory;
        GmailAPI gmailapi;
        Message message;
        string response;
        unordered_map<string, Command*> commands;
        string getCommand();
        bool isValidIP(const string &ip);
        string getIP();
        string getParameter();
        void sendResponse(const string& response_string, const Attachment& attachment = {});
        void processResponse();
    public:
        ProcessCommand();
        ~ProcessCommand();
        void executeCommand(Client& client);
        bool getLatestMessage();
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

class HandleErrorCommand: public Command {
    public:
        void execute(Client& client, const string& param) override;
};