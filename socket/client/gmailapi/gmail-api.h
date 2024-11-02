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
		Attachment() : file_path(""), file_name(""), file_content("") {}
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
		string gmail_id;
		string message_id;
		string in_reply_to;

		string from;
		string to;
		string subject;
		string body;
	
		string createMIME(const Attachment& attachment = {}) const;

	public:
		Message() : gmail_id(""), message_id(""), in_reply_to(""), from(""), to(""), subject(""), body("") {}
		Message(const string& from, const string& to, const string& subject, const string& body)
			: gmail_id(""), message_id(""), in_reply_to(""), from(from), to(to), subject(subject), body(body) {};
		~Message() {}

		string getEncodedMessage(const Attachment& attachment = {}) const;
		string getGmailID() const;
		string getMessageID() const;
		string getInReplyTo() const;
		string getFromEmail() const;
		string getToEmail() const;
		string getSubject() const;
		string getBody() const;

		void setGmailID(const string& gmail_id);
		void setMessageID(const string& message_id);
		void setInReplyTo(const string& in_reply_to);
		void setFromEmail(const string& from);
		void setToEmail(const string& to);
		void setSubject(const string& subject);
		void setBody(const string& body);

		bool isEmpty() const;
		void clear();
};

class GmailAPI {
	private:
		OAuth oauth;

		string getLatestMessageID(const string& query);

	public:
		void sendMessage(const Message& message, const Attachment& attachment = {});
		void markAsRead(const string& message_id);
		Message getLatestMessage(const string& query = "");
		void replyMessage(const Message& message, Message& reply_content, const Attachment& attachment = {});
};