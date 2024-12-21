#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <string>

using namespace ftxui;

std::vector<std::string> logs;
std::vector<std::string> ips;
std::mutex logs_mutex;

// Function to safely add a log
void AddLog(const std::string &log)
{
    {
        std::lock_guard<std::mutex> lock(logs_mutex);
        logs.push_back(log);
    }
}

void UpdateIP() {
    ips.clear();
    ips.push_back("192.168.1.1");
    ips.push_back("192.168.1.2");
    ips.push_back("192.168.1.3");
    ips.push_back("192.168.1.4");
    ips.push_back("192.168.1.5");
    ips.push_back("192.168.1.6");
    ips.push_back("192.168.1.7");
    ips.push_back("192.168.1.8");
    ips.push_back("192.168.1.9");
}


int main() {
    auto screen = ScreenInteractive::FitComponent();

    int depth = 0;

    // popup screen
    // IPs
    int selected_ip = 0;
    auto ip_entries = Menu(&ips, &selected_ip);

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
        UpdateIP();
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
        while (refresh_ui_continue) {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1s);
            screen.Post([] { AddLog("Refreshed at " + std::to_string(std::time(0))); });
            screen.Post(Event::Custom);
        }
    });

    screen.Loop(ui_renderer);
    refresh_ui_continue = false;
    refresh_ui.join();

    return 0;
}