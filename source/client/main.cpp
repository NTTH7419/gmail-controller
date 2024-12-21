#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <string>
#include "process_cmd.h"


// int __cdecl main() {    
//     // Validate the parameters

//     Client c;
//     c.initialize();
//     c.sendDiscovery();
    
// 	   ProcessCommand pc;

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


std::vector<std::string> logs;
std::mutex logs_mutex;  // to safely add a log


void addLog(const std::string &log) {
    time_t curr_time;
	curr_time = time(NULL);

    char time_str[20];
    strftime(time_str, 20, "[%H:%M:%S] ", localtime(&curr_time));

    {
        std::lock_guard<std::mutex> lock(logs_mutex);
        logs.push_back(time_str + log);
    }
}


int main() {
    using namespace ftxui;

    Client client;
    ProcessCommand pc;

    
    auto screen = ScreenInteractive::FitComponent();

    int depth = 0;

    // popup screen
    // IPs
    int selected_ip = 0;
    std::vector<std::string> ip_list;
    auto ip_entries = Menu(&ip_list, &selected_ip);

    // Button
    auto back_button = Button("Back", [&] {
        depth = 0;
    });


    // main screen
    // Logs
    int selected_log = 0;
    auto log_entries = Menu(&logs, &selected_log);

    // Button
    auto show_ips_button = Button("Show IPs", [&] {
        ip_list = client.getIPList();
        depth = 1;
    });

    // Main components
    auto main_components = Container::Vertical({
        log_entries,
        show_ips_button
    });

    // Popup components
    auto popup_components = Container::Vertical({
        ip_entries,
        back_button
    });

    // Main layout
    auto main_renderer = Renderer(main_components, [&] {
        return vbox({
            text("Gmail controller - Client") | bold | hcenter | color(Color::Green),
            separator(),
            window(
                text("Log"),
                log_entries->Render() | vscroll_indicator | frame
            ) | size(WIDTH, EQUAL, 100) | size(HEIGHT, EQUAL, 20),
            separator(),
            show_ips_button->Render() | hcenter,
        }) | border | center;
    });

    // Popup layout
    auto popup_renderer = Renderer(popup_components, [&] {
        return vbox({
            text("Available IP list") | bold | hcenter | color(Color::Green),
            separator(),
            ip_entries->Render() | vscroll_indicator | frame | size(WIDTH, EQUAL, 50) | size(HEIGHT, EQUAL, 7),
            separator(),
            back_button->Render() | center,
        }) | border | center;
    });

    // UI layout
    auto ui_component = Container::Tab({
        main_components,
        popup_components,
    }, &depth);
 
    // UI renderer
    auto ui_renderer = Renderer(ui_component, [&] {
        Element document = main_renderer->Render();
     
        if (depth == 1) {
            document = dbox({
                document,
                popup_renderer->Render() | clear_under | center,
            });
        }
            return document;
    });

    // Background thread
    std::atomic<bool> refresh_ui_continue = true;
    std::thread refresh_ui([&] {
        client.initialize();
        screen.Post(Event::Custom);
        client.sendDiscovery();
        screen.Post(Event::Custom);

        while (refresh_ui_continue) {
            using namespace std::chrono_literals;
            if (pc.getLatestMessage()) {
                screen.Post([&] { pc.executeCommand(client); });
                screen.Post(Event::Custom);
            }
            else {
                screen.Post(Event::Custom);
                std::this_thread::sleep_for(3s);
            }
        }
    });

    screen.Loop(ui_renderer);
    refresh_ui_continue = false;
    refresh_ui.join();

    return 0;
}




// class GUI {
// public:
//     HINSTANCE hInst;
//     HWND hwndMain, hwndEdit, hwndShowIp;
//     std::atomic<bool> stopFetching;
//     Client client;
        

//     ProcessCommand pc;

//     GUI(HINSTANCE hInstance)
//         : hInst(hInstance), hwndMain(nullptr), hwndEdit(nullptr), stopFetching(false), pc(hwndMain), client(hwndMain){
            
//         }

//     bool initialize() {
//         WNDCLASS wc = {0};
//         wc.lpfnWndProc = GUI::WndProc;
//         wc.hInstance = hInst;
//         wc.lpszClassName = TEXT("Remote Desktop Controller");
//         wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

//         if (!RegisterClass(&wc)) return false;

//         hwndMain = CreateWindow(wc.lpszClassName, TEXT("Remote Desktop Controller"),
//                                 WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
//                                 800, 600, nullptr, nullptr, hInst, this);

        

//         if (!hwndMain) return false;

//         ShowWindow(hwndMain, SW_SHOWNORMAL);
//         UpdateWindow(hwndMain);

//         return true;
//     }

//     void run() {
//         MSG msg;
//         while (GetMessage(&msg, nullptr, 0, 0)) {
//             TranslateMessage(&msg);
//             DispatchMessage(&msg);
//         }
//     }

//     static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
//         GUI* gui;
//         if (msg == WM_CREATE) {
//             CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
//             gui = (GUI*)pCreate->lpCreateParams;
//             SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)gui);
//         } else {
//             gui = (GUI*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
//         }

//         switch (msg) {
//         case WM_CREATE:
//             gui->hwndEdit = CreateWindow(TEXT("EDIT"), nullptr,
//                                          WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
//                                          10, 10, 760, 400, hwnd, nullptr, gui->hInst, nullptr);
                                         
//             gui->hwndShowIp = CreateWindow(TEXT("BUTTON"), TEXT("display available IPs"),
//                                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
//                                50, 450, 200, 30, hwnd, (HMENU)1, gui->hInst, nullptr);

//             break;

//         case WM_DESTROY:
//             PostQuitMessage(0);
//             break;

//         case WM_APP: {
//             std::string* response = reinterpret_cast<std::string*>(lParam);
//             gui->UpdateResponse(*response);
//             delete response; 
//             return 0;
//         }
        
        
//         case WM_COMMAND:
//             switch (LOWORD(wParam)) {

//             case 1: // Button 1 clicked
//             {
//                 std::stringstream ss;
                
//                 for (const auto& item : gui->client.getIPList()) {
//                     ss << item << "\n";
//                 }
//                 MessageBox(gui->hwndMain, ss.str().c_str(), TEXT("List of available IPs"), MB_OK);
//                 break;
//             }

//             default:
//                 return DefWindowProc(hwnd, msg, wParam, lParam);
//             }

//             break;

//         // case WM_USER + 1: {
//         //     std::wstring email = GetEmailFromUser(hwnd);
//         //     if (!email.empty()) {
                
//         //     }
//         //     break;
//         // }

//         default:
//             return DefWindowProc(hwnd, msg, wParam, lParam);
//         }
//         return 0;
//     }

//     void UpdateResponse(const std::string& response) {
//         int len = GetWindowTextLength(hwndEdit);
//         SendMessage(hwndEdit, EM_SETSEL, len, len); 
//         SendMessage(hwndEdit, EM_REPLACESEL, 0, (LPARAM)response.c_str());
//     }

//     HWND& getWindow() {
//         return hwndMain;
//     }
    
//     void FetchServerResponse() {
//         client.initialize();
//         client.sendDiscovery();

//         while (!stopFetching.load()) {
//             if (!pc.getLatestMessage()) {
//                 // PostMessage(gui.getWindow(), WM_APP, , (LPARAM)new std::string("No new command, retry after 3 sec"));
//                 Sleep(3000);
//                 continue;
//             }

//             pc.executeCommand(client);
//         }
//     }
// };


// int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
//     // FreeConsole();
//     GUI gui(hInstance);

//     if (!gui.initialize()) {
//         return -1;
//     }

//     PostMessage(gui.getWindow(), WM_APP, 0, (LPARAM)new std::string("Program started, send emails to client.hcmus@gmail.com to control your computer(s)\n"));
//     std::thread(&GUI::FetchServerResponse, std::ref(gui)).detach();

//     gui.run();

//     //Close client after closing GUI
//     gui.stopFetching = true;
//     return 0;
// }
