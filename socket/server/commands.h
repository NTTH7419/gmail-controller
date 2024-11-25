#include "server.h"
#include <unordered_map>
typedef SHORT PROPID;
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

#define SUCCESS 0
#define FAILURE 1

using namespace Gdiplus;


class Command;
class ReceiveCommand;
class ShutdownCommand;
class ListFileCommand;
class GetFileCommand;
class ListAppCommand;
class StartAppCommand;
class Command{
protected:
    int status;
    string file_path;
    string message;
public:
    virtual void execute(Server& server, const string& param) = 0;
    string createResponse();
};

class ReceiveCommand {
    private:
        string command;
        string parameter;

        
        unordered_map<string, Command*> commands;

    public:
        ReceiveCommand();
        ~ReceiveCommand();
        void getLatestCommand(Server& server);
        void executeCommand(Server& server);
		void process(Server& server); //* for testing only  
};

class ShutdownCommand : public Command{
public:
    void execute(Server& server, const string& param) override;
};

class ListFileCommand : public Command{
private:
    int listFile(const string& path);

public:
    void execute(Server& server, const string& param) override;
};

class GetFileCommand : public Command{
public:
    void execute(Server& server, const string& param) override;
};


class DeleteFileCommand : public Command{
private:
    void deleteFile(Server& server, const string& filename);
public:
    void execute(Server& server, const string& param) override;
};

class ListAppCommand : public Command{
    public:
        void execute(Server& server, const string& param) override;
};


class StartAppCommand : public Command{
    public:
        void execute(Server& server, const string& param) override;
};


class StopAppCommand : public Command{
    public:
        void execute(Server& server, const string& param) override;
};

class RestartCommand : public Command{
    public:
        void execute(Server& server, const string& param) override;
};

class ScreenshotCommand : public Command{
    public:
        void execute(Server& server, const string& param) override;
};


