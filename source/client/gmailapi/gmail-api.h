#pragma once

#include <exception>
#include <unordered_map>
#include "base64.h"
#include "tool.h"
#include "OAuth2.h"

class Attachment {
	private:
		std::string file_path;
		std::string file_name;
		std::string file_content;
		int file_size;

		void readFile();

	public:
		Attachment() : file_path(""), file_name(""), file_content(""), file_size(0) {}
		Attachment(std::string file_path, std::string file_name);
		Attachment(std::string file_path);
		~Attachment() {}

		void setFileName(const std::string& file_name);
		std::string getEncodedFileContent() const;
		std::string getFileName() const;
		int getFileSize() const;
		bool exist() const;
};

class Message {
	private:
		std::string gmail_id;
		std::string message_id;
		std::string in_reply_to;

		std::string from;
		std::string to;
		std::string subject;
		std::string body;
	
		std::string createMIME(const Attachment& attachment = {}) const;

	public:
		Message() : gmail_id(""), message_id(""), in_reply_to(""), from(""), to(""), subject(""), body("") {}
		Message(const std::string& from, const std::string& to, const std::string& subject, const std::string& body)
			: gmail_id(""), message_id(""), in_reply_to(""), from(from), to(to), subject(subject), body(body) {};
		~Message() {}

		std::string getMessage(const Attachment& attachment = {}) const;
		std::string getGmailID() const;
		std::string getMessageID() const;
		std::string getInReplyTo() const;
		std::string getFromEmail() const;
		std::string getToEmail() const;
		std::string getSubject() const;
		std::string getBody() const;

		void setGmailID(const std::string& gmail_id);
		void setMessageID(const std::string& message_id);
		void setInReplyTo(const std::string& in_reply_to);
		void setFromEmail(const std::string& from);
		void setToEmail(const std::string& to);
		void setSubject(const std::string& subject);
		void setBody(const std::string& body);

		bool isEmpty() const;
		void clear();
};

class GmailAPI {
	private:
		OAuth oauth;

		std::string getLatestMessageID(const std::string& query);
		void sendMessageWithAttachment(const Message& message, const Attachment& attachment);
		void sendMessageWithoutAttachment(const Message& message);

	public:
		GmailAPI();
		void initOAuth();
		void sendMessage(const Message& message, const Attachment& attachment = {});
		void markAsRead(const std::string& message_id);
		Message getLatestMessage(const std::string& query = "");
		void replyMessage(const Message& message, Message& reply_content, const Attachment& attachment = {});
};


enum ErrorCode {
	OK,
	ATTACHMENT_TOO_LARGE,
	CANNOT_SEND_MESSAGE,
	CANNOT_RETRIEVE_MESSAGE,
	CANNOT_RETRIEVE_MESSAGE_ID,
	CANNOT_INIT_UPLOAD,
	CANNOT_MARK_AS_READ,
	OAUTH_NOT_READY
};

std::unordered_map<ErrorCode, std::string> error_messages = {
	{ATTACHMENT_TOO_LARGE, "Attachment too large, limit is 25MB"},
	{CANNOT_SEND_MESSAGE, "Cannot send message"},
	{CANNOT_RETRIEVE_MESSAGE, "Cannot retrieve message"},
	{CANNOT_RETRIEVE_MESSAGE_ID, "Cannot retrieve message id"},
	{CANNOT_INIT_UPLOAD, "Cannot initialize upload"},
	{CANNOT_MARK_AS_READ, "Cannot mark message as read"},
	{OAUTH_NOT_READY, "OAuth not ready"}
};

class GmailError: public std::exception {
	private:
		std::string message;
		ErrorCode code;
		
	public:
		GmailError(ErrorCode code) : code(code), message(error_messages[code]) {}
		GmailError(ErrorCode code, const std::string& message) : code(code), message(message) {}
		const char* what() const noexcept override {
			return message.c_str();
		}
		ErrorCode getCode() const noexcept {
			return code;
		}
};