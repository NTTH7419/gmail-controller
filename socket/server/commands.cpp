#include "commands.h"


ReceiveCommand::ReceiveCommand() : command(), parameter() {
    commands.insert({{"shutdown", new ShutdownCommand},
                     //{"restart", new RestartCommand},
                     //{"listapp", new ListAppCommand},
                     //{"startapp", new StartAppCommand}
                     //{"stopapp", new StopAppCommand},
                     //{"listser", new ListSerCommand},
                     //{"startser", new StartSerCommand}
                     //{"stopser", new StopSerCommand},
                     {"listfile", new ListFileCommand},
                     {"getfile", new GetFileCommand},
                     {"deletefile", new DeleteFileCommand},
                     //{"screenshot", new ScreenshotCommand},
                     //{"takephoto", new TakePhotoCommand},
                     //{"startrecord", new StartRecordCommand},
                     //{"stoprecord", new StopRecordCommand},
                     //{"record", new RecordCommand},
                     //{"startkeylog", new StartKeylogCommand},
                     //{"stopkeylog", new StopKeylogCommand},
                     //{"keylog", new KeylogCommand},
                     });
}

ReceiveCommand::~ReceiveCommand() {
    for (auto it = commands.begin(); it != commands.end(); it++) {
        delete it->second;
    }
	commands.clear();
}

void ReceiveCommand::getLatestCommand(Server& server) {
	string receive_string;
	server.receive(receive_string);
	int sep = receive_string.find('\n');
	command = receive_string.substr(0, sep);
	parameter = receive_string.substr(sep);
}

void ReceiveCommand::executeCommand(Server& server) {
	if (command.empty()) return;
	if (commands.find(command) != commands.end()) {
			commands[command]->execute(server, parameter);
		}

	command = "";
	parameter = "";
}

void ReceiveCommand::process(Server& server) {
    while(true){
        string message;
        int bytes_received = server.receive(message);
        if (message == "end") return;

        int comma = message.find(' ');
        string cmd = message;
        string param = "";
        if (comma != string::npos){
            cmd = message.substr(0, comma);
            param = message.substr(comma + 1);
        }

        if (commands.find(cmd) != commands.end()) {
			commands[cmd]->execute(server, param);
		}
        else cout << "Error: invalid command" << endl;
    }
}

//* Shutdown
void ShutdownCommand::execute(Server& server, const string& param){
    cout << "Shutting down server";
}

//* Send file
void GetFileCommand::execute(Server& server, const string& param){
    sendFile(server, param);
}

//* List file
void ListFileCommand::execute(Server& server, const string& param){
    vector<string> v(listFile(param));
    SOCKET& client_socket = server.getClientSocket();

    server.echo("listing file");
    for (auto file : v){
        send(client_socket, (file + "\n").c_str(), file.length() + 1, 0);
        cout << file << endl;
    }
    send(client_socket, "FILE_LISTED", 11, 0);
}

vector<string> ListFileCommand::listFile(const string& path){
    string command("dir /a-d ");
    command.append(path + " > listfile.txt");
    system(command.c_str());

    ifstream fin("listfile.txt");

    if (!fin.is_open()){
        cout << "cannot open file";
    }

    string temp;
    //skipping the headers
    for (int i = 0; i < 4; i++)
        getline(fin, temp);

    vector<string> files;
    while(getline(fin, temp)){
        files.push_back(temp);
    }
    fin.close();
    files.pop_back();
    files.pop_back();

    return files;
}

//* Delete file
void DeleteFileCommand::execute(Server& server, const string& param){
    
}