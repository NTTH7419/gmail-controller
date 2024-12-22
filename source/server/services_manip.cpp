#include "server_cmd.h"

void ListSerCommand::listRunningServices() {
    SC_HANDLE hSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE);
    if (!hSCManager) {
        // std::cerr << "Failed to open Service Control Manager. Error: " << GetLastError() << std::endl;
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
        // std::cerr << "Failed to enumerate services. Error: " << GetLastError() << std::endl;
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
    }
    // else {
    //     std::cerr << "Failed to enumerate services. Error: " << GetLastError() << std::endl;
    // }

    fout.close();
    delete[] buffer;
    CloseServiceHandle(hSCManager);
}

void ListSerCommand::execute(Server& server, const std::string& param) {
    listRunningServices();
    std::string output_file = "runningServiceList.txt";
    status = sendFile(server, directory + output_file);
    
    file_name = "";
    if (status == SUCCESS) {
        file_name = output_file;
        message = "Running sservices listed successfully.";
        system(("del " + directory + output_file).c_str());
    }
    else
        message = "List service error: Could not list running services.";


    server.echo(createResponse());
}

int __stdcall DoStartSvc(std::string szSvcName, std::string& message){
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    // char szSvcName[256];
    SERVICE_STATUS_PROCESS ssStatus; 
    DWORD dwOldCheckPoint; 
    DWORD dwStartTickCount;
    DWORD dwWaitTime;
    DWORD dwBytesNeeded;

    schSCManager = OpenSCManager( 
        NULL,                    
        NULL,                   
        SC_MANAGER_ALL_ACCESS);

    if (NULL == schSCManager) 
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        message = "Failed to start service \\\"" + szSvcName + "\\\""; 
        return FAILURE;
    }

    // Get a handle to the service
    schService = OpenService( 
        schSCManager,            // SCM database 
        szSvcName.c_str(),               // name of service
        SERVICE_ALL_ACCESS);     // full access

    if (schService == NULL)
    { 
        printf("OpenService failed (%d)\n", GetLastError()); 
        CloseServiceHandle(schSCManager);
        message = "Failed to start service \\\"" + szSvcName + "\\\""; 
        CloseServiceHandle(schSCManager);
        return FAILURE;
    }

    // Check the status in case the service is not stopped
    if (!QueryServiceStatusEx( 
            schService,                     // handle to service
            SC_STATUS_PROCESS_INFO,         // information level
            (LPBYTE) &ssStatus,             // address of structure
            sizeof(SERVICE_STATUS_PROCESS), // size of structure
            &dwBytesNeeded ) )              // size needed if buffer is too small
    {
        printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
        message = "Failed to start service \\\"" + szSvcName + "\\\""; 
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
        return FAILURE;  
    }

    // Check if the service is already running
    if(ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING)
    {
        printf("Cannot start the service because it is already running\n");
        message = "Failed to start service \\\"" + szSvcName + "\\\"" + " because it is already running";
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
        return FAILURE; 
    }

    // Save the tick count and initial checkpoint
    dwStartTickCount = GetTickCount();
    dwOldCheckPoint = ssStatus.dwCheckPoint;

    // Wait for the service to stop before attempting to start it
    while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
    {
        // Do not wait longer than the wait hint
        dwWaitTime = ssStatus.dwWaitHint / 10;

        if (dwWaitTime < 1000)
            dwWaitTime = 1000;
        else if (dwWaitTime > 10000)
            dwWaitTime = 10000;

        Sleep(dwWaitTime);

        // Check the status until the service is no longer stop pending
        if (!QueryServiceStatusEx( 
                schService,                     // handle to service
                SC_STATUS_PROCESS_INFO,         // information level
                (LPBYTE) &ssStatus,             // address of structure
                sizeof(SERVICE_STATUS_PROCESS), // size of structure
                &dwBytesNeeded ) )              // size needed if buffer is too small
        {
            printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
            CloseServiceHandle(schService); 
            CloseServiceHandle(schSCManager);
            message = "Failed to start service \\\"" + szSvcName + "\\\""; 
            CloseServiceHandle(schService); 
            CloseServiceHandle(schSCManager);
            return FAILURE; 
        }

        if (ssStatus.dwCheckPoint > dwOldCheckPoint)
        {
            // Continue to wait and check
            dwStartTickCount = GetTickCount();
            dwOldCheckPoint = ssStatus.dwCheckPoint;
        }
        else
        {
            if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint)
            {
                printf("Timeout waiting for service to stop\n");
                CloseServiceHandle(schService); 
                CloseServiceHandle(schSCManager);
                message = "Failed to start service \\\"" + szSvcName + "\\\""; 
                CloseServiceHandle(schService); 
                CloseServiceHandle(schSCManager);
                return FAILURE; 
            }
        }
    }

    // Attempt to start the service
    if (!StartService(schService, 0, NULL))
    {
        printf("StartService failed (%d)\n", GetLastError());
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
        message = "Failed to start service \\\"" + szSvcName + "\\\""; 
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
        return FAILURE; 
    }
    else 
        printf("Service start pending...\n");

    // Check the status until the service is no longer start pending
    if (!QueryServiceStatusEx( 
            schService,                      
            SC_STATUS_PROCESS_INFO,         
            (LPBYTE) &ssStatus,              
            sizeof(SERVICE_STATUS_PROCESS),  
            &dwBytesNeeded ) )              
    {
        printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
        message = "Failed to start service \\\"" + szSvcName + "\\\""; 
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
        return FAILURE; 
    }

    // Save the tick count and initial checkpoint
    dwStartTickCount = GetTickCount();
    dwOldCheckPoint = ssStatus.dwCheckPoint;

    while (ssStatus.dwCurrentState == SERVICE_START_PENDING)
    {
        dwWaitTime = ssStatus.dwWaitHint / 10;

        if (dwWaitTime < 1000)
            dwWaitTime = 1000;
        else if (dwWaitTime > 10000)
            dwWaitTime = 10000;

        Sleep(dwWaitTime);

        if (!QueryServiceStatusEx( 
            schService,
            SC_STATUS_PROCESS_INFO,
            (LPBYTE) &ssStatus,
            sizeof(SERVICE_STATUS_PROCESS),
            &dwBytesNeeded ))
        {
            printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
            break;
        }

        if (ssStatus.dwCheckPoint > dwOldCheckPoint)
        {
            dwStartTickCount = GetTickCount();
            dwOldCheckPoint = ssStatus.dwCheckPoint;
        }
        else
        {
            if (GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint)
            {
                break;
            }
        }
    }

    // Determine whether the service is running
    if (ssStatus.dwCurrentState == SERVICE_RUNNING)
    {
        message = "Service: \\\"" + szSvcName + "\\\" started successfully";
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
        return SUCCESS;
    }
    else 
    { 
        // printf("Service not started. \n");
        // printf("  Current State: %d\n", ssStatus.dwCurrentState); 
        // printf("  Exit Code: %d\n", ssStatus.dwWin32ExitCode); 
        // printf("  Check Point: %d\n", ssStatus.dwCheckPoint); 
        // printf("  Wait Hint: %d\n", ssStatus.dwWaitHint); 
        message = "Failed to start service \\\"" + szSvcName + "\\\""; 
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
        return FAILURE;
    }
}

void StartSerCommand::execute(Server& server, const std::string& param){
    status = DoStartSvc(param, message);
    file_name = "";

    if (status == SUCCESS){
        message = "Service \\\"" + param + "\\\" started successfully";
    }

    server.echo(createResponse());
}


// Function to stop the service
int __stdcall DoStopSvc(std::string szSvcName)
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    SERVICE_STATUS_PROCESS ssStatus; 
    DWORD dwBytesNeeded;
    DWORD dwStartTickCount;
    DWORD dwWaitTime;

    // Get a handle to the SCM database
    schSCManager = OpenSCManager( 
        NULL,
        NULL,
        SC_MANAGER_ALL_ACCESS);

    if (NULL == schSCManager) 
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return FAILURE;
    }

    // Get a handle to the service
    schService = OpenService( 
        schSCManager,
        szSvcName.c_str(),
        SERVICE_ALL_ACCESS);

    if (schService == NULL)
    { 
        // printf("OpenService failed (%d)\n", GetLastError()); 
        CloseServiceHandle(schSCManager);
        return FAILURE;
    }

    // Query the service status
    if (!QueryServiceStatusEx( 
            schService,
            SC_STATUS_PROCESS_INFO,
            (LPBYTE) &ssStatus,
            sizeof(SERVICE_STATUS_PROCESS),
            &dwBytesNeeded ))
    {
        // printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
        return FAILURE; 
    }

    // Check if the service is already stopped
    if (ssStatus.dwCurrentState == SERVICE_STOPPED)
    {
        // printf("The service is already stopped.\n");
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
        return FAILURE;
    }

    // Attempt to stop the service
    if (!ControlService(schService, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS) &ssStatus))
    {
        printf("ControlService failed (%d)\n", GetLastError());
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
        return FAILURE;
    }
    else 
        // printf("Service stop pending...\n");

    // Wait for the service to stop
    dwStartTickCount = GetTickCount();
    DWORD dwMaxWaitTime = 60000; // Max wait time set to 60 seconds

    while (ssStatus.dwCurrentState != SERVICE_STOPPED)
    {
        dwWaitTime = ssStatus.dwWaitHint / 10;
        if (dwWaitTime < 1000)
            dwWaitTime = 1000;
        else if (dwWaitTime > 10000)
            dwWaitTime = 10000;

        Sleep(dwWaitTime);

        // Query the service status again
        if (!QueryServiceStatusEx( 
            schService,
            SC_STATUS_PROCESS_INFO,
            (LPBYTE) &ssStatus,
            sizeof(SERVICE_STATUS_PROCESS),
            &dwBytesNeeded))
        {
            // printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
            break;
        }

        // Check for timeout or state changes
        if (GetTickCount() - dwStartTickCount > dwMaxWaitTime)
        {
            // printf("Timeout waiting for service to stop.\n");
            break;
        }
    }

    // Final status of the service
    if (ssStatus.dwCurrentState == SERVICE_STOPPED)
    {
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
        return SUCCESS;
    }
    else 
    {
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
        return FAILURE;
    }
}

void StopSerCommand::execute(Server& server, const std::string& param){
    status = DoStopSvc(param);
    file_name = "";

    if (status == FAILURE){
        message = "Stop service error: Could not stop service: \\\"" + param + "\\\""; 
    }
    else{
        message = "Service \\\"" + param + "\\\" stopped successfully";
    }

    server.echo(createResponse());
}