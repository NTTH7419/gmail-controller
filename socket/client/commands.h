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
class StopAppCommand;
class ListSerCommand;
class StartSerCommand;
class StopSerCommand;
class TakePhotoCommand;
class StartRecordCommand;
class StopRecordCommand;
class GetIPsCommand;

class Command{
protected:
    static const std::string directory;
    std::string response;
    std::string command;
    std::string ip;
    std::string parameter;
public:
    virtual void execute(Client& Client) = 0;
    void setCommandInfo(const std::string& ip, const std::string& command, const std::string& parameter);
    virtual bool validateParameter() const = 0;
    bool sendCommand(Client& Client);
    std::string getResponse();
};

class ProcessCommand {
    private:
        static const std::string directory;
        GmailAPI gmailapi;
        Message message;
        std::string response;
        std::string sender_query;
        std::unordered_map<std::string, Command*> commands;

        void updateSenderQuery();
        std::string getCommand() const;
        bool isValidIP(const std::string &ip) const;
        std::string getIP() const;
        std::string getParameter() const;
        void sendResponse(const std::string& response_string, const Attachment& attachment = {});
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
    bool validateParameter() const override;
    void execute(Client& client) override;
};


class ListFileCommand : public Command{
private:
    std::vector<std::string> listFile(const std::string& path);
    

public:
    void execute(Client& client) override;
    bool validateParameter() const override;
};

class GetFileCommand : public Command{
public:
    void execute(Client& client) override;
    bool validateParameter() const override;
};


class DeleteFileCommand : public Command{
public:
    void execute(Client& client) override;
    bool validateParameter() const override;
};


class ListAppCommand : public Command{
public:
    void execute(Client& client) override;
    bool validateParameter() const override;
};

class StartAppCommand : public Command{
public: 
    void execute(Client& client) override;
    bool validateParameter() const override;
};

class StopAppCommand : public Command{
public: 
    void execute(Client& client) override;
    bool validateParameter() const override;
};

class RestartCommand : public Command{
public: 
    void execute(Client& client) override;
    bool validateParameter() const override;
};

class ScreenshotCommand : public Command{
public: 
    void execute(Client& client) override;
    bool validateParameter() const override;
};

class StartSerCommand : public Command{
    public:
        void execute(Client& client) override;
        bool validateParameter() const override;
};

class StopSerCommand : public Command{
    public:
        void execute(Client& client) override;
        bool validateParameter() const override;
};
class StartRecordCommand : public Command{
    public:
        void execute(Client& client) override;
        bool validateParameter() const override;
};

class StopRecordCommand : public Command{
    public:
        void execute(Client& client) override;
        bool validateParameter() const override;
};

class GetIPsCommand : public Command{
    public:
        void execute(Client& client) override;
        bool validateParameter() const override;
};

class HelpCommand: public Command {
    public:
        void execute(Client& client) override;
        bool validateParameter() const override;
};

class ListSerCommand: public Command {
public:
    void execute(Client& client) override;
    bool validateParameter() const override;
};

class TakePhotoCommand : public Command{
    public:
        void execute(Client& client) override;
        bool validateParameter() const override;
};