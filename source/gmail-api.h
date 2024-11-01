#pragma once

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
		string id;
		string thread_id;

		string from;
		string to;
		string subject;
		string body;
	
		string createMIME() const;
		string createMIME(const Attachment& attachment) const;

	public:
		Message() : from(""), to(""), subject(""), body("") {}
		Message(const string& from, const string& to, const string& subject, const string& body)
			: from(from), to(to), subject(subject), body(body) {};
		Message(const string& from, const string& to, const string& subject, const string& body)
			: from(from), to(to), subject(subject), body(body) {};
		~Message() {}

		string getEncodedMessage() const;
		string getEncodedMessage(const Attachment& attachment) const;
		string getID() const;
		string getThreadID() const;
		string getFromEmail() const;
		string getToEmail() const;
		string getSubject() const;
		string getBody() const;

		void setID(const string& id);
		void setThreadID(const string& thread_id);
		void setFromEmail(const string& from);
		void setToEmail(const string& to);
		void setSubject(const string& subject);
		void setBody(const string& body);
};

class GmailAPI {
	private:
		OAuth oauth;

		string getLatestMessageID();

	public:
		void sendMessage(const Message& message);
		void sendMessage(const Message& message, const Attachment& attachment);
		Message getLatestMessage();
};

 