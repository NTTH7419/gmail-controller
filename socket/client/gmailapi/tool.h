#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;
using json = nlohmann::json;

struct HTTPResponse {
	string headers;
	string body;
	
	string getHeader(const string& name);
};

HTTPResponse makeRequest(const string& request_url, curl_slist *headers, const string& postFields, const string& type);
string getStringBetween(const string& str, const string& begin, const string& end);
string trim(string s);
string urlEncode(const string& value);