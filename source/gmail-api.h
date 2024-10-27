#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <ctime>
#include <curl/curl.h>
#include "base64.h"
#include "tool.h"
#include "OAuth2.h"

using namespace std;

class Attachment {
	private:
		string file_path;
		string file_name;
		string file_content;

		void readFile();

	public:
		Attachment(string file_path, string file_name);
		Attachment(string file_path);
		~Attachment() {}

		void setFileName(const string& file_name);
		string getEncodedFileContent() const;
		string getFileName() const;
		bool exist() const;
};

class Message {
	private:
		string from;
		string to;
		string subject;
		string body;

		Attachment attachment;
	
		string createMIME() const;
		string createMIMEWithAttachment() const;

	public:
		Message() : from(""), to(""), subject(""), body(""), attachment("", "") {}
		Message(const string& from, const string& to, const string& subject, const string& body, const Attachment& attachment) : 
		from(from), to(to), subject(subject), body(body), attachment(attachment) {};
		Message(const string& from, const string& to, const string& subject, const string& body) : 
		from(from), to(to), subject(subject), body(body), attachment("", "") {};
		~Message() {}


		string getEncodedMessage() const;
		string getFromEmail() const;
		string getToEmail() const;
		string getSubject() const;
		string getBody() const;
		// Attachment getAttachment() const;

		void setFromEmail(const string& from);
		void setToEmail(const string& to);
		void setSubject(const string& subject);
		void setBody(const string& body);
		void setAttachment(const Attachment& attachment);
};

class GmailAPI {
	private:
		OAuth oauth;

		string getLatestMessageID();

	public:
		void sendMessage(const Message& message);
		Message getLatestMessage();
};

 