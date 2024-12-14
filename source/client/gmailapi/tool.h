#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <iostream>
#include <iomanip>
#include <sstream>

using json = nlohmann::json;

struct HTTPResponse {
	std::string headers;
	std::string body;
	
	std::string getHeader(const std::string& name);
	int getStatus();
	HTTPResponse(): headers(), body() {}
};

HTTPResponse makeRequest(const std::string& request_url, curl_slist *headers, const std::string& postFields, const std::string& type);
std::string getStringBetween(const std::string& str, const std::string& begin, const std::string& end);
std::string trim(std::string s);
std::string urlEncode(const std::string& value);