#pragma once

#include <curl/curl.h>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>

using namespace std;

struct ClientSecrect {
	static string client_id;
	static string auth_uri;
	static string token_uri;
	static string auth_provider_x509_cert_url;
	static string client_secret;
	static string redirect_uri;
};

struct Token {
	string access_token = "";
	string refresh_token = "";
};

Token login();