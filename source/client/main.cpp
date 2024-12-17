#include "process_cmd.h"
#include <atomic>

// int __cdecl main() {    
//     // Validate the parameters

//     Client c;
//     c.initialize();
//     c.sendDiscovery();
    
// 	ProcessCommand pc;

//     while (true) {
//         if (!pc.getLatestMessage()) {
//     		std::cout << "No new command, retry after 3 secs" << std::endl;
//     		Sleep(3000);
//             continue;	
//     	}
//     	pc.executeCommand(c);
//     }

//     return 0;
// }

class GUI {
public:
    HINSTANCE hInst;
    HWND hwndMain, hwndEdit, hwndShowIp;
    std::atomic<bool> stopFetching;
    Client client;
        

    ProcessCommand pc;

    GUI(HINSTANCE hInstance)
        : hInst(hInstance), hwndMain(nullptr), hwndEdit(nullptr), stopFetching(false), pc(hwndMain), client(hwndMain){
            
        }

    bool initialize() {
        WNDCLASS wc = {0};
        wc.lpfnWndProc = GUI::WndProc;
        wc.hInstance = hInst;
        wc.lpszClassName = TEXT("Remote Desktop Controller");
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

        if (!RegisterClass(&wc)) return false;

        hwndMain = CreateWindow(wc.lpszClassName, TEXT("Remote Desktop Controller"),
                                WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                                800, 600, nullptr, nullptr, hInst, this);

        

        if (!hwndMain) return false;

        ShowWindow(hwndMain, SW_SHOWNORMAL);
        UpdateWindow(hwndMain);

        return true;
    }

    void run() {
        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        GUI* gui;
        if (msg == WM_CREATE) {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            gui = (GUI*)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)gui);
        } else {
            gui = (GUI*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }

        switch (msg) {
        case WM_CREATE:
            gui->hwndEdit = CreateWindow(TEXT("EDIT"), nullptr,
                                         WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
                                         10, 10, 760, 400, hwnd, nullptr, gui->hInst, nullptr);
                                         
            gui->hwndShowIp = CreateWindow(TEXT("BUTTON"), TEXT("display available IPs"),
                               WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                               50, 450, 200, 30, hwnd, (HMENU)1, gui->hInst, nullptr);

            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_APP: {
            std::string* response = reinterpret_cast<std::string*>(lParam);
            gui->UpdateResponse(*response);
            delete response; 
            return 0;
        }
        
        
        case WM_COMMAND:
            switch (LOWORD(wParam)) {

            case 1: // Button 1 clicked
            {
                std::stringstream ss;
                
                for (const auto& item : gui->client.getIPList()) {
                    ss << item << "\n";
                }
                MessageBox(gui->hwndMain, ss.str().c_str(), TEXT("List of available IPs"), MB_OK);
                break;
            }

            default:
                return DefWindowProc(hwnd, msg, wParam, lParam);
            }

            break;

        // case WM_USER + 1: {
        //     std::wstring email = GetEmailFromUser(hwnd);
        //     if (!email.empty()) {
                
        //     }
        //     break;
        // }

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        return 0;
    }

    void UpdateResponse(const std::string& response) {
        int len = GetWindowTextLength(hwndEdit);
        SendMessage(hwndEdit, EM_SETSEL, len, len); 
        SendMessage(hwndEdit, EM_REPLACESEL, 0, (LPARAM)response.c_str());
    }

    HWND& getWindow() {
        return hwndMain;
    }
    
    void FetchServerResponse() {
        client.initialize();
        client.sendDiscovery();

        while (!stopFetching.load()) {
            if (!pc.getLatestMessage()) {
                // PostMessage(gui.getWindow(), WM_APP, , (LPARAM)new std::string("No new command, retry after 3 sec"));
                Sleep(3000);
                continue;
            }

            pc.executeCommand(client);
        }
    }
};


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    GUI gui(hInstance);

    if (!gui.initialize()) {
        return -1;
    }

    PostMessage(gui.getWindow(), WM_APP, 0, (LPARAM)new std::string("Program started, send emails to client.hcmus@gmail.com to control your computer(s)\n"));
    std::thread(&GUI::FetchServerResponse, std::ref(gui)).detach();

    gui.run();

    //Close client after closing GUI
    gui.stopFetching = true;
    return 0;
}
