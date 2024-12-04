#include <iostream>
#include <windows.h>
#include <dshow.h>
#include <string>
#include <sstream>
// #include <thread>
#include <chrono>

#define SUCCESS 0
#define FAILURE 1
#define WEBCAM_NOT_DETECTED 30


#pragma comment(lib, "strmiids.lib")


class WebcamController{
private:
    static WebcamController* instance;
    std::string webcamName = detectWebcam();
    PROCESS_INFORMATION recordingProcess = { 0 };
    HANDLE hStdInWrite = NULL;
    std::string directory = "temp\\";

    std::string detectWebcam();
    WebcamController(){};
    
public:
    static void deleteInstance();
    static WebcamController* getInstance();
    std::string getWebcam();
    int takePhoto(std::string& message);
    bool isRecording();
    int StartRecord(std::string& message);
    int StopRecord(std::string& message);

};