#include "gmail-api.h"

Attachment::Attachment(string file_path, string file_name) : file_path(file_path), file_name (file_name) {
    readFile();
}

Attachment::Attachment(string file_path) : file_path(file_path) {
    int idx = file_path.find_last_of("/");
    if (idx == string::npos) {
        idx = file_path.find_last_of("\\");
        if (idx == string::npos) {
            file_name = file_path;
            return;
        }
    }

    file_name = file_path.substr(idx + 1);
    readFile();
}

void Attachment::readFile() {
    if (exist()) return;
    ifstream fin (file_path, ios::binary);
    file_content = string(istreambuf_iterator<char>(fin), istreambuf_iterator<char>());
    fin.close();
}

string Attachment::getEncodedFileContent() const {
    return base64_encode(file_content);
}

void Attachment::setFileName(const string& file_name) {
    this->file_name = file_name;
}

string Attachment::getFileName() const {
    return file_name;
}

bool Attachment::exist() const {
    return (!file_name.empty() && !file_content.empty());
}

string Message::getID() const {
    return id;
}

string Message::getThreadID() const {
    return thread_id;
}

string Message::getFromEmail() const {
    return from;
}

string Message::getToEmail() const {
    return to;
}

string Message::getSubject() const {
    return subject;
}

string Message::getBody() const {
    return body;
}

void Message::setID(const string& id) {
    this->id = id;
}

void Message::setThreadID(const string& thread_id) {
    this->thread_id = thread_id;
}

void Message::setFromEmail(const string& from) {
    this->from = from;
}

void Message::setToEmail(const string& to) {
    this->to = to;
}

void Message::setSubject(const string& subject) {
    this->subject = subject;
}

void Message::setBody(const string& body) {
    this->body = body;
}

string Message::createMIME() const {
    string MIME_message =
            "From: " + from + "\r\n"
            "To: " + to + "\r\n"
            "Subject: " + subject + "\r\n"
            "Content-Type: text/plain; charset=UTF-8\r\n"
            "\r\n" // Empty line between headers and body
            + body + "\r\n";

    return MIME_message;
}

string Message::createMIME(const Attachment& attachment) const {
    string MIME_message =
            "MIME-Version: 1.0\r\n"
            "To: " + to + "\r\n"
            "From: " + from + "\r\n"
            "Subject: " + subject + "\r\n"
            "Content-Type: multipart/mixed; boundary=\"frontier\"\r\n\r\n"
            "--frontier\r\n"
            "Content-Type: text/plain; charset=\"UTF-8\"\r\n\r\n"
            + body + "\r\n\r\n"
            "--frontier\r\n"
            "Content-Type: application/octet-stream\r\n"
            "Content-Disposition: attachment; filename=\"" + attachment.getFileName() + "\"\r\n"
            "Content-Transfer-Encoding: base64\r\n\r\n"
            + attachment.getEncodedFileContent() + "\r\n\r\n"
            "--frontier--";
    
    return MIME_message;
}

string Message::getEncodedMessage() const {
    return base64_encode(createMIME());
}

string Message::getEncodedMessage(const Attachment& attachment) const {
    return base64_encode(createMIME(attachment));
}

bool Message::isEmpty() const {
    return (from.empty() && to.empty() && subject.empty() && body.empty());
}

void Message::clear() {
    id = "";
    thread_id = "";
    from = "";
    to = "";
    subject = "";
    body = "";
}

void GmailAPI::sendMessage(const Message& message, const string& thread_id) {
    string encoded_message = message.getEncodedMessage();
    
    string post_fields;
    if (thread_id.empty())
        post_fields = R"({"raw": ")" + encoded_message + R"("})";
    else
        post_fields = R"({"threadId": ")" + thread_id + R"(","raw": ")" + encoded_message + R"("})";

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + oauth.getAccessToken()).c_str());

    string url = "https://gmail.googleapis.com/gmail/v1/users/me/messages/send";

    makeRequest(url, headers, post_fields);
}

void GmailAPI::sendMessage(const Message& message, const Attachment& attachment, const string& thread_id) {
    string encoded_message = message.getEncodedMessage(attachment);
    
    string post_fields;
    if (thread_id.empty())
        post_fields = R"({"raw": ")" + encoded_message + R"("})";
    else
        post_fields = R"({"threadId": ")" + thread_id + R"(","raw": ")" + encoded_message + R"("})";

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + oauth.getAccessToken()).c_str());

    string url = "https://gmail.googleapis.com/gmail/v1/users/me/messages/send";

    makeRequest(url, headers, post_fields);
}

string GmailAPI::getLatestMessageID(const string& query) {
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + oauth.getAccessToken()).c_str());

    string url = "https://gmail.googleapis.com/gmail/v1/users/me/messages?maxResults=1&includeSpamTrash=true&q=\"" + query + '\"';

    string response = makeRequest(url, headers, "");

    json j = json::parse(response);
    if (j.contains("messages")) return j["messages"][0]["id"];
    else return "";
}

void GmailAPI::setMessageRead(const string& message_id) {
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + oauth.getAccessToken()).c_str());

    string post_fields = R"({"removeLabelIds": ["UNREAD"]})";

    string url = "https://gmail.googleapis.com/gmail/v1/users/me/messages/" + message_id + "/modify";

    makeRequest(url, headers, post_fields);
}

Message GmailAPI::getLatestMessage(const string& query) {
    Message message;

    string id = getLatestMessageID(query);
    if (id.empty()) {
        return message;
    }

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + oauth.getAccessToken()).c_str());

    string url = "https://gmail.googleapis.com/gmail/v1/users/me/messages/" + id + "?format=full";

    string response = makeRequest(url, headers, "");

    json j = json::parse(response);

    message.setID(j["id"]);
    message.setThreadID(j["threadId"]);

    auto response_headers = j["payload"]["headers"];
    for (auto header: response_headers) {
        if (header["name"] == "Delivered-To") message.setToEmail(header["value"]);
        if (header["name"] == "From") {
            if (string(header["value"]).find('\u003c') != string::npos)
                message.setFromEmail(getStringBetween(string(header["value"]), "\u003c", "\u003e"));
            else
                message.setFromEmail(header["value"]);
        }
        if (header["name"] == "Subject") message.setSubject(header["value"]);
    }

    message.setBody(trim(base64_decode(j["payload"]["parts"][0]["body"]["data"])));

    return message;
}