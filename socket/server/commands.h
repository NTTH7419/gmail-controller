#include "server.h"
#include <unordered_map>

class Command{
public:
    virtual void execute(Server& server, const string& param) = 0;
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
    vector<string> listFile(const string& path);

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