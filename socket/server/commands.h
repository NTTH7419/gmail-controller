#include "server.h"
#include <unordered_map>
#include <gdiplus.h>
#include <thread>
#pragma comment(lib, "gdiplus.lib")

#define SUCCESS 0
#define FAILURE 1

using namespace Gdiplus;

typedef SHORT PROPID;

class Command;
class ReceiveCommand;
class ShutdownCommand;
class ListFileCommand;
class GetFileCommand;
class ListAppCommand;
class StartAppCommand;
class Command{
protected:
    static const string directory;
    int status;
    string file_name;   // name of the return file
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
    string output_file;
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


