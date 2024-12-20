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

// Globals for logging
std::vector<std::string> logs;
std::mutex logs_mutex;
std::atomic<bool> running(true);  // Flag to control the background thread

// Function to safely add a log
void AddLog(const std::string& log) {
    {
        std::lock_guard<std::mutex> lock(logs_mutex);
        logs.push_back(log);
    }
}

// Background task
void BackgroundTask() {
    int counter = 0;
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));  // Simulate work
        AddLog("Background task iteration " + std::to_string(++counter));
    }
}

// Function to create a basic text editor component
ftxui::Component CreateTextEditor(const std::string& filename, ftxui::ScreenInteractive& screen) {
    static std::string file_content;
    static std::mutex editor_mutex;

    {
        std::ifstream file(filename);
        std::lock_guard<std::mutex> lock(editor_mutex);
        file_content.assign((std::istreambuf_iterator<char>(file)),
                            (std::istreambuf_iterator<char>()));
    }

    auto input = ftxui::Input(&file_content, "Edit here...");
    auto save_button = ftxui::Button("Save", [=] {
        std::ofstream file(filename);
        std::lock_guard<std::mutex> lock(editor_mutex);
        file << file_content;
    });

    auto exit_button = ftxui::Button("Exit", screen.ExitLoopClosure());

    return ftxui::Container::Vertical({
        input,
        save_button,
        exit_button,
    });
}

// Main log displayer with buttons
ftxui::Component CreateLogDisplayer(ftxui::ScreenInteractive& screen) {
    int selected_log = 0;
    auto log_entries = ftxui::Menu(&logs, &selected_log);

    // Buttons
    auto show_ips_button = ftxui::Button("Show IPs", [] {
        AddLog("192.168.1.1");
        AddLog("192.168.1.2");
    });

    auto edit_abc_button = ftxui::Button("Edit abc.json", [&] {
        auto editor = CreateTextEditor("abc.json", screen);
        screen.Loop(editor);
    });

    auto edit_xyz_button = ftxui::Button("Edit xyz.txt", [&] {
        auto editor = CreateTextEditor("xyz.txt", screen);
        screen.Loop(editor);
    });

    auto buttons = ftxui::Container::Vertical({
        show_ips_button,
        edit_abc_button,
        edit_xyz_button,
    });

    // Main layout
    auto renderer = ftxui::Renderer([&] {
        auto log_table = ftxui::vbox({
            ftxui::center(ftxui::text("Log Viewer") | ftxui::bold),
            log_entries->Render() | ftxui::frame | ftxui::border,
        });

        auto button_column = buttons->Render() | ftxui::vcenter | ftxui::border;

        return ftxui::hbox({
            log_table,
            button_column,
        });
    });

    return ftxui::Container::Vertical({
        log_entries,
        buttons,
        renderer,
    });
}

#endif // LOG_DISPLAYER_HPP