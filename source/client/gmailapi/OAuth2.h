#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include "tool.h"

struct Credential {
	std::string client_id;
	std::string auth_uri;
	std::string token_uri;
	std::string client_secret;
	std::string redirect_uri;
};

struct Token {
	std::string access_token;
	std::string refresh_token;
	long refresh_time;
};

class OAuth {
	private:
		Credential credential;
		std::string client_secret_file;

		Token token;
		std::string token_file;

		bool is_error;
		std::string error_message;

		std::string openLoginUI();
		std::string getAuthCode(const std::string& url);
		std::string getTokenResponse(const std::string& auth_code);
		void writeTokenToFile();
		void refreshToken();
		void login();
	
	public:
		OAuth();
		~OAuth();
		void init();
		std::string getAccessToken();
		bool good();
		std::string getErrorMessage();
};