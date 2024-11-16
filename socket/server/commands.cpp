#include "commands.h"


ReceiveCommand::ReceiveCommand() : command(), parameter() {
    commands.insert({{"shutdown", new ShutdownCommand},
                     {"restart", new RestartCommand},
                     {"listapp", new ListAppCommand},
                     {"startapp", new StartAppCommand},
                     {"stopapp", new StopAppCommand},
                     //{"listser", new ListSerCommand},
                     //{"startser", new StartSerCommand},
                     //{"stopser", new StopSerCommand},
                     {"listfile", new ListFileCommand},
                     {"getfile", new GetFileCommand},
                     {"deletefile", new DeleteFileCommand},
                     {"screenshot", new ScreenshotCommand},
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

    if (sep != string::npos) {
        command = receive_string.substr(0, sep);
    	parameter = receive_string.substr(sep + 1);
    }
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
    cout << "Shutting down server" << endl;
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
    command.append(path + " > temp\\listfile.txt");
    system(command.c_str());

    ifstream fin("temp\\listfile.txt");

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
    system(("del " + param).c_str());
    server.echo("File deleted");
}


BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam){
    std::ofstream* outFile = reinterpret_cast<std::ofstream*>(lParam);

    DWORD processID;
    TCHAR windowTitle[MAX_PATH];
    TCHAR processName[MAX_PATH] = TEXT("<unknown>"); // Default value if process name cannot be retrieved

    // Check if the window is visible and has a title
    if (IsWindowVisible(hwnd) && GetWindowText(hwnd, windowTitle, MAX_PATH) && _tcslen(windowTitle) > 0) {
        // Get the process ID associated with this window
        GetWindowThreadProcessId(hwnd, &processID);

        // Open the process to retrieve the executable name
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
        if (hProcess) {
            HMODULE hMod;
            DWORD cbNeeded;
            
            // Attempt to retrieve the executable name associated with the process.
            if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
                // Retrieve the base name of the module, which is the process name
                GetModuleBaseName(hProcess, hMod, processName, sizeof(processName) / sizeof(TCHAR));
            }
            CloseHandle(hProcess);
        }

        // Write the window title and process name to the file
        *outFile << "Application: " << processName 
                 << " (Window Title: " << windowTitle 
                 << ") (PID: " << processID << ")\n";
    }
    return TRUE; // Continue enumeration
}

void ListAppCommand::execute(Server& server, const string& param){
    ofstream outFile("temp\\runningAppList.txt");
    if (!outFile.is_open()) {
        cerr << "Unable to open file for writing.\n";
        return;
    }

    // Enumerate all top-level windows and write to the file
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&outFile));
    outFile.close();
    cout << "List of running applications saved to RunningAppList.txt\n";


    //*sending applications to client
    SOCKET client_socket = server.getClientSocket();

    
    //* reading file
    ifstream fin("temp\\runningAppList.txt");

    if (!fin.is_open()){
        cout << "cannot open file";
        server.echo("Error");
        return;
    }
    server.echo("listing applications");

    string temp;
    vector<string> apps;
    while(getline(fin, temp)){
        apps.push_back(temp);
    }
    fin.close();

    //*sending
    for (auto app : apps){
        send(client_socket, (app + "\n").c_str(), app.length() + 1, 0);
        cout << app << endl;
    }
    send(client_socket, "APP_LISTED", 11, 0);
    cout << "Apllications listed successfully" << endl;;
}

void StartAppCommand::execute(Server& server, const string& param){
    string app_name = param;
    HINSTANCE hInstance = ShellExecute(NULL, "open", app_name.c_str(), NULL, NULL, SW_SHOWNORMAL);
    if ((int)hInstance > 32) {
        cout << "Started application: " << app_name << endl;
        server.echo("success");
        return;
    } else {
        cerr << "Failed to start application: " << app_name << " (Error: " << GetLastError() << ")\n";
        server.echo("failure");
        return;
    }
}

void StopAppCommand::execute(Server& server, const string& param){
    DWORD processID = stoi(param);
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processID);
    if (hProcess) {
        if (TerminateProcess(hProcess, 0)) {
            cout << "Terminated process with PID: " << processID << "\n";
            CloseHandle(hProcess);
            server.echo("success");
            return;
        } else {
            cerr << "Failed to terminate process with PID: " << processID << " (Error: " << GetLastError() << ")\n";
            CloseHandle(hProcess);
            server.echo("failure");
            return;
        }
    } else {
        cerr << "Unable to open process with PID: " << processID << " (Error: " << GetLastError() << ")\n";
        server.echo("error");
        return;
    }
}

void RestartCommand::execute(Server& server, const string& param){
    cout << "restarting server...." << endl;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT num = 0;
	UINT size = 0;
	ImageCodecInfo* pImage = NULL;
	
	GetImageEncodersSize(&num, &size);
	if (size == 0) return -1;
	
	pImage = (ImageCodecInfo*)(malloc(size));
	if (pImage == NULL) return -1;
	
	GetImageEncoders(num, size, pImage);
	
	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImage[j].MimeType, format) == 0)
		{
			*pClsid = pImage[j].Clsid;
			free(pImage);
			return j;
		}
	}
	
	free(pImage);
	return -1;
}

void ScreenshotCommand::execute(Server& server, const string& param){
    GdiplusStartupInput gdip;
	ULONG_PTR gdipToken;
	GdiplusStartup(&gdipToken, &gdip, NULL);
	
	// Get the screen dimensions
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	
	// Create device contexts
	HDC hdcScreen = GetDC(NULL);
	HDC hdcMemDC = CreateCompatibleDC(hdcScreen);
	HBITMAP hbmScreen = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);
	SelectObject(hdcMemDC, hbmScreen);
	
	// Capture the entire screen
	BitBlt(hdcMemDC, 0, 0, screenWidth, screenHeight, hdcScreen, 0, 0, SRCCOPY);
	
	// Save the captured screen as a PNG
	CLSID encoderID;
	GetEncoderClsid(L"image/png", &encoderID);
	Bitmap* bmp = new Bitmap(hbmScreen, (HPALETTE)0);
	bmp->Save(L"temp/screen.png", &encoderID, NULL);

    sendFile(server, "temp\\screen.png");
    cout << "sending screenshot" << endl;
	
	// Cleanup
	GdiplusShutdown(gdipToken);
	DeleteObject(hbmScreen);
	DeleteDC(hdcMemDC);
	ReleaseDC(NULL, hdcScreen);
	delete bmp;

}