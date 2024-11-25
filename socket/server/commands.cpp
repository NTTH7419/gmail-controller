#include "commands.h"
#include <thread>

string Command::createResponse() {
    return R"({"status": )" + to_string(status) + R"(, "file": ")" + file_path + R"(", "message": ")" + message + R"("})";
}

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
	string receive_string = server.receive();

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
        string message = server.receive();
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

//* Get file
void GetFileCommand::execute(Server& server, const string& param){
    file_path = param + '\n';
    status = sendFile(server, param);

    if (status == 0)
        message = "File at " + param + " was sent successfully\n";  
    else
        message = "Could not send file at " + param + '\n';

    server.echo(createResponse());
}

//* List file
void ListFileCommand::execute(Server& server, const string& param){
    status = listFile(param);
    file_path = param + '\n';

    if (status == SUCCESS){
        status = sendFile(server, "temp\\listfile.txt");
        if (status == SUCCESS) message = "Files at directory: " + param + " were listed successfully\n";
        else message = "Could not list files at directory " + param + '\n';
    }
    else{
        server.echo("error");
        message = "An error occured while listing files\n";
    }
    
    server.echo(createResponse());
}

int ListFileCommand::listFile(const string& path){
    string command("dir /a-d ");
    command.append(path + " > temp\\listfile.txt");
    system(command.c_str());

    ifstream fin("temp\\listfile.txt");

    if (!fin.is_open()){
        cout << "cannot open file";
        return 1;
    }

    string temp;
    //skipping the headers
    for (int i = 0; i < 5; i++)
        getline(fin, temp);

    vector<string> files;
    while(getline(fin, temp)){
        files.push_back(temp + '\n');
    }
    fin.close();
    files.pop_back();
    files.pop_back();

    ofstream fout("temp\\listfile.txt");
    if (!fout.is_open()){
        cerr << "cannot open file write" << endl;
        return 2;
    }

    for (auto &line: files){
        fout << line;
    }

    fout.close();

    return 0;
}

//* Delete file
void DeleteFileCommand::execute(Server& server, const string& param){
    ifstream file(param);
    if (!file)
        status = 1;
    else{
        status = 0;
        file.close();
        system(("del " + param).c_str());
    }
    
    file_path = param + '\n';
    if (status == 0)
        message = "File at " + param + " deleted successfully\n";
    else
        message = "Could not delete file at " + param + '\n';

    server.echo(createResponse());
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
    file_path = "";
    ofstream outFile("temp\\runningAppList.txt");
    if (!outFile.is_open()) {
        cerr << "Unable to open file for writing.\n";
        message = "Unable to open file for saving running applications\n";
        status = 1;
        server.echo("error");
        server.echo(createResponse());
        return;
    }

    //* Enumerate all top-level windows and write to the file
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&outFile));
    outFile.close();
    cout << "List of running applications saved to temp\\RunningAppList.txt\n";


    //*sending applications to client
    SOCKET client_socket = server.getClientSocket();

    status = sendFile(server, "temp\\runningAppList.txt");
    
    if (status == SUCCESS)
        message = "Running Applications listed successfully\n";
    else
        message = "Could not list running applications\n";


    server.echo(createResponse());
}

void StartAppCommand::execute(Server& server, const string& param){
    file_path = "";
    string app_name = param;
    HINSTANCE hInstance = ShellExecute(NULL, "open", app_name.c_str(), NULL, NULL, SW_SHOWNORMAL);
    if (hInstance != NULL) {
        cout << "Started application: " << app_name << endl;
        status = 0;
        message = "Application " + app_name + " started successfully\n";
    } else {
        cerr << "Failed to start application: " << app_name << " (Error: " << GetLastError() << ")\n";
        server.echo("failure");
        status = 1;
        message = "Failed to start application " + app_name + '\n';
    }
    server.echo(createResponse());
}

void StopAppCommand::execute(Server& server, const string& param){
    file_path = "";
    DWORD processID = stoi(param);
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processID);
    if (hProcess) {
        if (TerminateProcess(hProcess, 0)) {
            cout << "Terminated process with PID: " << processID << "\n";
            CloseHandle(hProcess);
            status = SUCCESS;
            message = "Application " + param + " stopped successfully\n";
            server.echo(createResponse());
            return;
        } else {
            cerr << "Failed to terminate process with PID: " << processID << " (Error: " << GetLastError() << ")\n";
            CloseHandle(hProcess);
            status = ERROR;
            message = "Failed to stop application " + param + '\n'; 
            server.echo(createResponse());
            return;
        }
    } else {
        cerr << "Unable to open process with PID: " << processID << " (Error: " << GetLastError() << ")\n";
        status = ERROR;
        message = "Failed to stop application " + param + '\n'; 
        server.echo(createResponse());
        return;
    }
}

void RestartCommand::execute(Server& server, const string& param){
    cout << "restarting server...." << endl;

}

int GetEncoderClsid(const wchar_t* format, CLSID* pClsid) {
    UINT num = 0;
    UINT size = 0;
    ImageCodecInfo* pImageCodecInfo = NULL;

    // Get the number of image encoders and their size
    GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;

    // Allocate memory for the encoders
    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL) return -1;

    // Get the available encoders
    GetImageEncoders(num, size, pImageCodecInfo);

    // Search for the required encoder (e.g., PNG)
    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }

    free(pImageCodecInfo);
    return -1;
}

void GetPrimaryScreenResolution(int &width, int &height) {
    // Get the device context for the screen
    HDC hScreenDC = GetDC(NULL);

    // Use EnumDisplayDevices to retrieve information about the primary display
    DISPLAY_DEVICE dd;
    ZeroMemory(&dd, sizeof(dd));
    dd.cb = sizeof(dd);

    if (EnumDisplayDevices(NULL, 0, &dd, 0)) {  // Get the primary display device
        // Get display settings for the primary monitor
        DEVMODE devmode;
        ZeroMemory(&devmode, sizeof(devmode));
        devmode.dmSize = sizeof(devmode);

        if (EnumDisplaySettings(dd.DeviceName, ENUM_CURRENT_SETTINGS, &devmode)) {
            width = devmode.dmPelsWidth;
            height = devmode.dmPelsHeight;
        } else {
            std::cerr << "Error: Unable to retrieve display settings." << std::endl;
        }
    }

    ReleaseDC(NULL, hScreenDC);
}

void ScreenshotCommand::execute(Server& server, const string& param){
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Get the primary screen resolution (unscaled) using EnumDisplayDevices
    int screenWidth = 0, screenHeight = 0;
    GetPrimaryScreenResolution(screenWidth, screenHeight);

    std::cout << "Primary screen resolution: " << screenWidth << "x" << screenHeight << std::endl;

    // Create the device context for the screen
    HDC hScreenDC = GetDC(NULL);

    // Create a memory device context for screen capture
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    // Create a bitmap to hold the screen capture at the correct screen size
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenWidth, screenHeight);
    SelectObject(hMemoryDC, hBitmap);

    // Capture the entire screen (using the screen size retrieved)
    BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, 0, 0, SRCCOPY);

    // Convert the HBITMAP to a GDI+ Bitmap object for saving
    Bitmap bmp(hBitmap, NULL);
    CLSID clsid;
    GetEncoderClsid(L"image/png", &clsid);  // Save as PNG format
    bmp.Save(L"temp\\screen.png", &clsid, NULL);

    // Clean up
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);

    // Shutdown GDI+
    GdiplusShutdown(gdiplusToken);

    status = sendFile(server, "temp\\screen.png");
    if (status == SUCCESS)
        message = "screen shot taken\n";
    else    
        message = "Failed to take a screen shot\n";
    file_path = "";
    server.echo(createResponse()); 
}

int sendFile(Server& server, const string& filepath) {
    SOCKET client_socket = server.getClientSocket();

    string file_name;
    int last_slash = filepath.rfind('\\');
    if (last_slash != string::npos){
        file_name = filepath.substr(last_slash + 1);
    }
    else file_name = filepath;
    ifstream file(filepath.c_str(), ios::binary);
    if (!file) {
        cerr << "Failed to open file: " << filepath << endl;
        server.echo("error");
        return 1;
    }

    file.seekg(0, ios::end);
    int file_size = file.tellg();
    file.seekg(0, ios::beg);
    server.echo(file_name + '|' + to_string(file_size));
    const int buff_len = DEFAULT_BUFLEN;
    char send_buffer[buff_len];
    while (file.read(send_buffer, buff_len).gcount() > 0) {
        send(client_socket, send_buffer, static_cast<int>(file.gcount()), 0);
    }
    file.close();
    cout << "file sent" << endl;
    return 0;
}