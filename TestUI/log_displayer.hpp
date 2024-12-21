#ifndef LOG_DISPLAYER_HPP
#define LOG_DISPLAYER_HPP

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <fstream>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <string>
#include <functional>

using namespace ftxui;

// Globals for logging
std::vector<std::string> logs;
std::mutex logs_mutex;

// Function to safely add a log
void AddLog(const std::string& log) {
    {
        std::lock_guard<std::mutex> lock(logs_mutex);
        logs.push_back(log);
    }
}


// Main log displayer with buttons
Component CreateLogDisplayer(ScreenInteractive& screen) {
    int selected_log = 0;
    auto log_entries = Menu(&logs, &selected_log);

    // Buttons
    auto show_ips_button = Button("Show IPs", [] {
        AddLog("192.168.1.1");
        AddLog("192.168.1.2");
    });

    // Main layout
    auto renderer = Renderer([&] {
        return vbox({
            text("Log Viewer") | bold | hcenter,
            separator(),
            log_entries->Render() | frame | size(WIDTH, LESS_THAN, 100) | size(HEIGHT, LESS_THAN, 20),
            separator(),
            show_ips_button->Render() | hcenter,
        }) | border;
    });

    return renderer;
}

#endif // LOG_DISPLAYER_HPP