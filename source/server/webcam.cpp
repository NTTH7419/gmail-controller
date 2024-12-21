#include "webcam.h"
WebcamController* WebcamController::instance = nullptr;

WebcamController* WebcamController::getInstance(){
    if (!instance) instance = new WebcamController();

    return instance;
}

void WebcamController::deleteInstance(){
    if (instance) delete instance;
    instance = nullptr;
}

std::string WebcamController::detectWebcam(){
    std::string webcamName = "";

    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        // std::cerr << "Failed to initialize COM library." << std::endl;
        return webcamName;
    }

    ICreateDevEnum* pDevEnum = NULL;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pDevEnum);
    if (FAILED(hr)) {
        // std::cerr << "Failed to create device enumerator." << std::endl;
        CoUninitialize();
        return webcamName;
    }

    IEnumMoniker* pEnum = NULL;
    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
    if (hr == S_OK) {
        IMoniker* pMoniker = NULL;
        while (pEnum->Next(1, &pMoniker, NULL) == S_OK) {
            IPropertyBag* pPropBag;
            hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);
            if (hr == S_OK) {
                VARIANT var;
                VariantInit(&var);
                hr = pPropBag->Read(L"FriendlyName", &var, 0);
                if (hr == S_OK) {
                    std::wstring ws(var.bstrVal);
                    webcamName = std::string(ws.begin(), ws.end());
                    VariantClear(&var);
                }
                pPropBag->Release();
            }
            pMoniker->Release();
            break;
        }
    }

    if (pEnum) pEnum->Release();
    pDevEnum->Release();
    CoUninitialize();

    return webcamName;
}

std::string WebcamController::getWebcam(){
    return webcamName;
}

int WebcamController::takePhoto(std::string& message, std::string& file_name){
    if (webcamName.empty()){
        message = "Take photo error: Webcam not found";
        return FAILURE;
    }

    if (isRecording()) {
        message = "Take photo error: Recording is in progress.\\nStop record using \\\"stoprecord\\\" command before taking a photo";
        return FAILURE;
    }

    std::ostringstream cmd;
    cmd << "\"ffmpeg\" "
            << "-f dshow -i video=\"" << webcamName << "\" "
            << "-loglevel quiet -vframes 1 -rtbufsize 100M -y -update 1 "
            << directory + file_name;

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    LPSTR cmdLine = _strdup(cmd.str().c_str());

    // Create the process
    if (CreateProcess(
            NULL,
            cmdLine,
            NULL,
            NULL,
            FALSE,
            0,
            NULL,
            NULL,
            &si,
            &pi
    )) {
        message = "Snapshot captured successfully!";

        // Wait for the process to finish
        WaitForSingleObject(pi.hProcess, INFINITE);

        // Clean up handles
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        message = "Take photo error: Failed to capture snapshot";
        return FAILURE;
    }

    // Free the duplicated string
    free(cmdLine);
    return SUCCESS;
}

bool WebcamController::isRecording(){
    if (recordingProcess.hProcess)
        return true;

    return false;
}


int WebcamController::StartRecord(std::string& message){
    if (webcamName.empty()){
        message = "Start record error: Webcam not found";
        return FAILURE;
    }

    if (recordingProcess.hProcess){
        message = "Start record error: Recording already started.";
        return FAILURE;
    }

    STARTUPINFO si = { sizeof(si) };
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    // Create a pipe for communication with FFmpeg
    HANDLE hStdInRead = NULL;
    if (!CreatePipe(&hStdInRead, &hStdInWrite, &sa, 0)) {
        message = "Start record error: Failed to create pipe for communication.";
        recordingProcess = { 0 };
        return FAILURE;
    }

    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = hStdInRead;
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);

    std::ostringstream command;
    command << "\"ffmpeg\" -hide_banner -loglevel quiet "
            << "-f dshow -i video=\"" << webcamName << "\" "
            << "-c:v libx264 -fs 23M -pix_fmt yuv420p -preset ultrafast -movflags +faststart -y "
            << directory + "video.mp4\"";

    LPSTR cmdLine = _strdup(command.str().c_str());

    if (!CreateProcess(NULL, cmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &recordingProcess)) {
        message = "Start record error: Failed to start recording.";
        ZeroMemory(&recordingProcess, sizeof(recordingProcess)); // Ensure process information is zeroed out on failure
        return FAILURE;
    }

    CloseHandle(hStdInRead); // Close the read end of the pipe (not needed in the parent process)
    free(cmdLine);

    message = "Recording started successfully.\\nUse \\\"stoprecord\\\" command to stop recording and retreive record file.";
    return SUCCESS;
}

int WebcamController::StopRecord(std::string& message){
    if (hStdInWrite && recordingProcess.hProcess) {
        // Send 'q' to FFmpeg's stdin to stop recording gracefully
        DWORD bytesWritten;
        WriteFile(hStdInWrite, "q", 2, &bytesWritten, NULL);

        // Wait for the FFmpeg process to exit
        WaitForSingleObject(recordingProcess.hProcess, INFINITE);

        // Clean up
        CloseHandle(recordingProcess.hProcess);
        CloseHandle(recordingProcess.hThread);
        CloseHandle(hStdInWrite);

        message = "Recording stopped successfully.";
        ZeroMemory(&recordingProcess, sizeof(recordingProcess));
        hStdInWrite = NULL;
        return SUCCESS;
    } else {
        message = "No recording process found to stop.\\nUse \\\"startrecord\\\" command to start recording.";
        return FAILURE;
    }
}
