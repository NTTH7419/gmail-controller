#include <ftxui/dom/elements.hpp>    // FTXUI UI elements
#include <ftxui/screen/screen.hpp>   // For rendering the screen
#include <ftxui/component/component.hpp>  // For creating components
#include <ftxui/component/screen_interactive.hpp>  // For ScreenInteractive
#include <thread>                    // For multithreading
#include <mutex>                     // For mutex protection
#include <vector>                    // For storing log entries
#include <string>                    // For log messages
#include <chrono>                    // For simulating delays
#include <cstdlib>                   // For std::system() (to run shell commands)

using namespace ftxui;

std::vector<std::string> logs;         // Log storage
std::mutex log_mutex;                 // Mutex for thread safety

// Function to add new logs
void add_log(const std::string& log) {
    std::lock_guard<std::mutex> lock(log_mutex);
    logs.push_back(log);
    if (logs.size() > 100) {  // limit logs to 100 entries for performance
        logs.erase(logs.begin());
    }
}

// Log displayer with scrollable window
Component log_displayer() {
    return Renderer([&] {
        return vbox({
            text("Log Display") | center,  // Title
            separator(),
            vbox([&] {
                std::lock_guard<std::mutex> lock(log_mutex);  // Lock while accessing shared log
                std::vector<Element> log_entries;
                for (const auto& log : logs) {
                    log_entries.push_back(text(log));
                }
                return vbox(std::move(log_entries)) | vscroll_indicator | frame | border;
            }) | flex
        });
    });
}

// List of controlled computers (just displaying names, no buttons)
std::vector<std::string> computers = {"Computer1", "Computer2", "Computer3"};

Component list_computers() {
    return Renderer([&] {
        std::vector<Element> elements;
        for (const auto& computer : computers) {
            elements.push_back(text(computer));
        }
        return vbox(std::move(elements)) | border;
    });
}

// Function to open a file in Notepad (Windows-specific)
void open_file_in_notepad(const std::string& filename) {
    std::string command = "notepad.exe " + filename;
    std::system(command.c_str());  // This will open the file in Notepad
}

// File editing buttons that open files in Notepad
Component file_buttons() {
    return Container::Horizontal({
        Button("Edit abc.txt", []{ open_file_in_notepad("abc.txt"); }),
        Button("Edit xyz.json", []{ open_file_in_notepad("xyz.json"); })
    });
}

// Full layout of the UI
Component layout() {
    return Container::Vertical({
        log_displayer() | border,
		Container::Horizontal({
        	list_computers(),
        	file_buttons()
		})
    });
}

// Function to simulate background work that sends logs to UI
void background_worker() {
    int count = 0;
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));  // Simulate work delay
        add_log("Log entry #" + std::to_string(count++));
    }
}

int main() {
    // Create a separate thread to run the background worker
    std::thread worker_thread(background_worker);

    // Initialize FTXUI screen
    auto screen = ftxui::ScreenInteractive::Fullscreen();
    screen.Loop(layout());

    // Stop the background worker and join the thread before exiting
    worker_thread.detach();

    return 0;
}