#pragma once

#include <string>
#include "json.hpp"
#include <curl/curl.h>
#include <iostream>

using namespace std;
using json = nlohmann::json;

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
string makeRequest(const string& request_url, struct curl_slist *headers, const string& postFields);
string getStringBetween(const string& str, const string& begin, const string& end);
string trim(string s);