#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <string>
#include "process_cmd.h"


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
                text(" Logs "),
                log_entries->Render() | vscroll_indicator | frame
            ) | size(WIDTH, EQUAL, 150) | size(HEIGHT, EQUAL, 20),
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
    auto screen = ScreenInteractive::FitComponent();
    std::atomic<bool> refresh_ui_continue = true;
    std::thread refresh_ui([&] {
        client.initialize();
        screen.Post(Event::Custom);
        client.sendDiscovery();
        screen.Post(Event::Custom);

        while (refresh_ui_continue) {
            using namespace std::chrono_literals;
            if (pc.getLatestMessage()) {
                pc.executeCommand(client);
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