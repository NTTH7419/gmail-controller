#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <ctime>
#include <curl/curl.h>
#include "base64.h"
#include "tool.h"

using namespace std;

struct Message {
	string from;
	string to;
	string subject;
	string body;
};

void sendEmail(const string& accessToken, const Message& m);
Message getLatestMessage(const string& accessToken);