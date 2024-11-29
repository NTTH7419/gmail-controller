#include "server_cmd.h"

const std::string Command::directory = "temp\\";

std::string toRawString(const std::string& input) {
    std::string raw;
    for (char c : input) {
        switch (c) {
            case '\\': raw += "\\\\"; break;
            case '\n': raw += "\\n"; break;
            case '\t': raw += "\\t"; break;
            case '\"': raw += "\\\""; break;
            default: raw += c;
        }
    }
    return raw;
}

std::string Command::createResponse() {
    std::string response = R"({"status": )" + std::to_string(status) + R"(, "file": ")" + file_name + R"(", "message": ")" + message + R"("})";
    status = -1;
    file_name = "";
    message = "";
    return response;
}


ReceiveCommand::ReceiveCommand() : command(), parameter() {
    commands.insert({{"shutdown", new ShutdownCommand},
                     {"restart", new RestartCommand},
                     {"listapp", new ListAppCommand},
                     {"startapp", new StartAppCommand},
                     {"stopapp", new StopAppCommand},
                     {"listser", new ListSerCommand},
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
	std::string receive_string = server.receive();

	int sep = receive_string.find('-');

    if (sep != std::string::npos) {
        command = receive_string.substr(0, sep);
    	parameter = receive_string.substr(sep + 1);
    }
}

void ReceiveCommand::executeCommand(Server& server) {
	if (command.empty()) return;
	if (commands.find(command) != commands.end()) {
        commands[command]->execute(server, parameter);
    }
    else{
        std::cout << "THUA" << std::endl;
    }

	command = "";
	parameter = "";
}

//* Shutdown
void ShutdownCommand::execute(Server& server, const std::string& param){
    std::cout << "con ga" << std::endl;
    status = SUCCESS;
    message = "Shutdown command has been received.";
    server.echo(createResponse());
    std::cout << "Shutting down server" << std::endl;
}

void RestartCommand::execute(Server& server, const std::string& param){
    status = SUCCESS;
    message = "Restart command has been received.";
    server.echo(createResponse());
    std::cout << "restarting server...." << std::endl;
}

//* Get file
void GetFileCommand::execute(Server& server, const std::string& param){
    int idx = param.find_last_of("\\/");
    if (idx != param.npos) {
        file_name = param.substr(idx + 1);
    }
    else {
        file_name = param;
    }


    status = sendFile(server, param);

    if (status == 0)
        message = "File at \\\"" + toRawString(param) + "\\\" was sent successfully.";  
    else
        message = "Get file error: Server could not send file at \\\"" + toRawString(param) + "\\\".";
        file_name = "";

    server.echo(createResponse());
}

//* List file
void ListFileCommand::execute(Server& server, const std::string& param){
    status = listFile(param);

    if (status == SUCCESS){
        status = sendFile(server, directory + '\\' + output_file);
        if (status == SUCCESS) {
            message = "Files at directory \\\"" + toRawString(param) + "\\\" were listed successfully.";
            file_name = output_file;
        }
        else message = "List file error: Server could not send files info.";
    }
    else{
        server.echo("error");
        message = "List file error: Could not list files at directory \\\"" + toRawString(param) + "\\\".";
    }
    
    server.echo(createResponse());
}

int ListFileCommand::listFile(const std::string& path){
    std::string command("dir /a-d ");
    output_file = "listfile.txt";
    command.append(path + " > " + directory + '\\' + output_file);
    system(command.c_str());

    std::ifstream fin(directory + '\\' + output_file);

    if (!fin.is_open()){
        std::cout << "cannot open file";
        return 1;
    }

    std::string temp;
    //skipping the headers
    for (int i = 0; i < 3; i++)
        getline(fin, temp);

    std::vector<std::string> files;
    while(getline(fin, temp)){
        files.push_back(temp + '\n');
    }
    fin.close();
    files.pop_back();

    std::ofstream fout(directory + '\\' + output_file);
    if (!fout.is_open()){
        std::cerr << "cannot open file write" << std::endl;
        return 2;
    }

    for (auto &line: files){
        fout << line;
    }

    fout.close();

    return 0;
}

//* Delete file
void DeleteFileCommand::execute(Server& server, const std::string& param){
    std::ifstream file(param);
    if (!file.good())
        status = SUCCESS;
    else{
        status = FAILURE;
        file.close();
        system(("del " + param).c_str());
    }
    
    if (status == 0)
        message = "File at \\\"" + toRawString(param) + "\\\" deleted successfully\n";
    else
        message = "Delete file error: File at \\\"" + toRawString(param) + "\\\" does not exist.";

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

void ListAppCommand::execute(Server& server, const std::string& param){
    file_name = "";
    std::string output_file = "runningAppList.txt";
    std::ofstream outFile(directory + '\\' + output_file);
    if (!outFile.is_open()) {
        std::cerr << "Unable to open file for writing.\n";
        status = 1;
        message = "List app error: Unable to open file for saving running applications.";
        server.echo("error");
        server.echo(createResponse());
        return;
    }

    //* Enumerate all top-level windows and write to the file
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&outFile));
    outFile.close();
    std::cout << "List of running applications saved to " << directory << '\\' << output_file << std::endl;


    //*sending applications to client
    SOCKET client_socket = server.getClientSocket();

    status = sendFile(server, directory + '\\' + output_file);
    
    if (status == SUCCESS) {
        file_name = output_file;
        message = "Running Applications listed successfully.";
    }
    else
        message = "List app error: Could not list running applications.";


    server.echo(createResponse());
}

void StartAppCommand::execute(Server& server, const std::string& param){
    std::string app_name = param;
    HINSTANCE hInstance = ShellExecute(NULL, "open", app_name.c_str(), NULL, NULL, SW_SHOWNORMAL);
    if (hInstance != NULL) {
        std::cout << "Started application: " << app_name << std::endl;
        status = 0;
        message = "Application \\\"" + toRawString(app_name) + "\\\" started successfully.";
    } else {
        std::cerr << "Failed to start application: " << app_name << " (Error: " << GetLastError() << ")\n";
        status = 1;
        message = "Start app error: Failed to start application \\\"" + toRawString(app_name) + "\\\".";
    }
    server.echo(createResponse());
}

void StopAppCommand::execute(Server& server, const std::string& param){
    DWORD processID = stoi(param);
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processID);
    if (hProcess) {
        if (TerminateProcess(hProcess, 0)) {
            std::cout << "Terminated process with PID: " << processID << "\n";
            CloseHandle(hProcess);
            status = SUCCESS;
            message = "Application with PID " + param + " stopped successfully.";
            server.echo(createResponse());
            return;
        } else {
            std::cerr << "Failed to terminate process with PID: " << processID << " (Error: " << GetLastError() << ")\n";
            CloseHandle(hProcess);
            status = ERROR;
            message = "Stop app error: Failed to stop application with PID " + param + '.'; 
            server.echo(createResponse());
            return;
        }
    } else {
        std::cerr << "Unable to open process with PID: " << processID << " (Error: " << GetLastError() << ")\n";
        status = ERROR;
        message = "Stop app error: Failed to stop application with PID " + param + '.'; 
        server.echo(createResponse());
        return;
    }
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


void ScreenshotCommand::execute(Server& server, const std::string& param){
    std::string output_file = "screenshot.png";

    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    if (GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) != GpStatus::Ok) {
        status = 1;
        message = "Screenshot error: GDI+ startup failed.";
        server.echo(createResponse());
        return;
    };

    // Set target capture resolution
    SetProcessDPIAware();
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Create a memory device context
    HDC hScreenDC = GetDC(NULL);
    if (!hScreenDC) {
        status = 1;
        message = "Screenshot error: cannot allocate memory.";
        server.echo(createResponse());
        GdiplusShutdown(gdiplusToken);
        return;
    }

    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    if (!hMemoryDC) {
        status = 1;
        message = "Screenshot error: cannot allocate memory.";
        server.echo(createResponse());
        ReleaseDC(NULL, hScreenDC);
        GdiplusShutdown(gdiplusToken);
        return;
    }

    // Create a bitmap to hold the screen capture at the desired resolution
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenWidth, screenHeight);
    if (!hBitmap) {
        status = 1;
        message = "Screenshot error: cannot allocate memory.";
        server.echo(createResponse());
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        GdiplusShutdown(gdiplusToken);
        return;
    }
    SelectObject(hMemoryDC, hBitmap);

    // Capture the screen and scale it to the desired resolution
    if (!BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, 0, 0, SRCCOPY)) {
        status = 1;
        message = "Screenshot error: cannot get screenshot.";
        server.echo(createResponse());
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        GdiplusShutdown(gdiplusToken);
        return;
    }

    // Convert the HBITMAP to a GDI+ Bitmap object for saving

    CLSID clsid;
    if (GetEncoderClsid(L"image/png", &clsid) == -1) {
        status = 1;
        message = "Screenshot error: cannot get encoder.";
        server.echo(createResponse());
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        GdiplusShutdown(gdiplusToken);
        return;
    }

    std::string tmp = directory + '\\' + output_file;
    Bitmap *bmp = new Bitmap(hBitmap, NULL);
    if (bmp->Save(std::wstring(tmp.begin(), tmp.end()).c_str(), &clsid, NULL) != Ok) {
        status = 1;
        message = "Screenshot error: cannot save screenshot.";
        server.echo(createResponse());
        delete bmp;
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        GdiplusShutdown(gdiplusToken);
        return;
    }

    std::cout << "sending screenshot" << std::endl;
    sendFile(server, directory + '\\' + output_file);
    status = 0;
    message = "Take screenshot successfully.";
    file_name = output_file;
    server.echo(createResponse());

    // Clean up
    delete bmp;
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);

    // Shutdown GDI+
    GdiplusShutdown(gdiplusToken);
}

void ListSerCommand::listRunningServices() {
    SC_HANDLE hSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE);
    if (!hSCManager) {
        std::cerr << "Failed to open Service Control Manager. Error: " << GetLastError() << std::endl;
        return;
    }

    DWORD bytesNeeded = 0, servicesReturned = 0, resumeHandle = 0;
    DWORD bufferSize = 0;

    // First call to get the required buffer size
    EnumServicesStatusEx(
        hSCManager,
        SC_ENUM_PROCESS_INFO,
        SERVICE_WIN32,
        SERVICE_ACTIVE, // Only active (running) services
        nullptr,
        0,
        &bytesNeeded,
        &servicesReturned,
        &resumeHandle,
        nullptr);

    if (GetLastError() != ERROR_MORE_DATA) {
        std::cerr << "Failed to enumerate services. Error: " << GetLastError() << std::endl;
        CloseServiceHandle(hSCManager);
        return;
    }

    bufferSize = bytesNeeded;
    auto buffer = new BYTE[bufferSize];
    std::ofstream fout(directory + "runningServiceList.txt");

    if (EnumServicesStatusEx(
            hSCManager,
            SC_ENUM_PROCESS_INFO,
            SERVICE_WIN32,
            SERVICE_ACTIVE, // Only active (running) services
            buffer,
            bufferSize,
            &bytesNeeded,
            &servicesReturned,
            &resumeHandle,
            nullptr)) {
        LPENUM_SERVICE_STATUS_PROCESS services = reinterpret_cast<LPENUM_SERVICE_STATUS_PROCESS>(buffer);
        for (DWORD i = 0; i < servicesReturned; ++i) {
            fout << "Service Name: " << services[i].lpServiceName << std::endl;
            fout << "Display Name: " << services[i].lpDisplayName << std::endl;
            fout << "Status: Running" << std::endl;
            fout << "--------------------------------" << std::endl;
        }
    } else {
        std::cerr << "Failed to enumerate services. Error: " << GetLastError() << std::endl;
    }

    fout.close();
    delete[] buffer;
    CloseServiceHandle(hSCManager);
}

void ListSerCommand::execute(Server& server, const std::string& param) {
    listRunningServices();
    std::string output_file = "runningServiceList.txt";
    status = sendFile(server, directory + '\\' + output_file);
    
    file_name = "";
    if (status == SUCCESS) {
        file_name = output_file;
        message = "Running Services listed successfully.";
    }
    else
        message = "List service error: Could not list running services.";


    server.echo(createResponse());
}

std::string TakePhotoCommand::detectWebcam(){
    std::string webcamName = "";

    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize COM library." << std::endl;
        return webcamName;
    }

    // Create the system device enumerator
    ICreateDevEnum* pDevEnum = NULL;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pDevEnum);
    if (FAILED(hr)) {
        std::cerr << "Failed to create device enumerator." << std::endl;
        CoUninitialize();
        return webcamName;
    }

    // Enumerate video capture devices (webcams)
    IEnumMoniker* pEnum = NULL;
    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
    if (hr == S_OK) {
        IMoniker* pMoniker = NULL;
        ULONG fetched;
        while (pEnum->Next(1, &pMoniker, &fetched) == S_OK) {
            IPropertyBag* pPropBag;
            hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);
            if (hr == S_OK) {
                VARIANT var;
                VariantInit(&var);
                hr = pPropBag->Read(L"FriendlyName", &var, 0);
                if (hr == S_OK) {
                    // Convert the webcam name from wide string to standard string
                    std::wstring ws(var.bstrVal);
                    webcamName = std::string(ws.begin(), ws.end());
                    VariantClear(&var);
                }
                pPropBag->Release();
            }
            pMoniker->Release();

            // Use the first webcam found
            break;
        }
    } else {
        std::cerr << "No webcams found." << std::endl;
    }

    if (pEnum) pEnum->Release();
    pDevEnum->Release();
    CoUninitialize();

    return webcamName;
}

int TakePhotoCommand::takePhoto(){
    std::string webcamName = detectWebcam();
    if (webcamName.empty()) {
        std::cerr << "Error: No webcam detected." << std::endl;
        return FAILURE;
    }

    std::cout << "Detected Webcam: " << webcamName << std::endl;

    // Construct the ffmpeg command with the detected webcam name
    std::ostringstream cmd;
    cmd << "\"ffmpeg.exe\" " //replace with your ffmpeg bin path
            << "-f dshow -i video=\"" << webcamName << "\" "
            << "-vframes 1 -rtbufsize 100M -y -update 1 "
            << directory + "snapshot.png"; //replace with your path that you want to save as

    // Initialize STARTUPINFO and PROCESS_INFORMATION structs
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    // Convert the string command to a C-string (LPSTR)
    LPSTR cmdLine = _strdup(cmd.str().c_str());

    // Create the process
    if (CreateProcess(
            NULL,        // Application name
            cmdLine,     // Command line
            NULL,        // Process security attributes
            NULL,        // Thread security attributes
            FALSE,       // Inherit handles
            0,           // Creation flags
            NULL,        // Environment block
            NULL,        // Current directory
            &si,         // Startup information
            &pi          // Process information
    )) {
        std::cout << "Snapshot captured successfully!" << std::endl;

        // Wait for the process to finish
        WaitForSingleObject(pi.hProcess, INFINITE);

        // Clean up handles
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        std::cerr << "Error: Failed to capture snapshot. Error code: " << GetLastError() << std::endl;
        return FAILURE;
    }

    // Free the duplicated string
    free(cmdLine);
    return SUCCESS;
}

void TakePhotoCommand::execute(Server& server, const std::string& param){
    status = takePhoto();
    file_name = "";
    std::string outfile = "snapshot.png";

    if (status == SUCCESS){
        status = sendFile(server, directory + '\\' + outfile);
        if (status == SUCCESS){
            file_name = "snapshot.png";
            message = "Photo taken successfully\n\n";
        }
        else{
            message = "Failed to send the photo to client\n\n";
        }

    }
    else{
        server.echo("error");
        message = "Failed to take a photo\n\n";
    }

    server.echo(createResponse());
}



