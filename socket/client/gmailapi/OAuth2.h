#pragma once

#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include "tool.h"

using namespace std;

struct Credential {
	string client_id;
	string auth_uri;
	string token_uri;
	string client_secret;
	string redirect_uri;
};

struct Token {
	string access_token;
	string refresh_token;
	long refresh_time;
};

class OAuth {
	private:
		Credential credential;
		string client_secret_file;

		Token token;
		string token_file;

		bool is_error = false;
		string error_message;

		void openGoogleLogin();
		string getAuthCode();
		string getTokenResponse(const string& auth_code);
		void writeTokenToFile();
		void refreshToken();
	
	public:
		OAuth();
		~OAuth();
		void login();
		string getAccessToken();
		bool good();
		string getErrorMessage();
};