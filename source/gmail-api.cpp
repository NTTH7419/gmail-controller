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
    if (!exist()) return;
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
    return (file_name.length() && file_content.length());
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

// Attachment Message::getAttachment() const {
//     return attachment;
// }

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

void Message::setAttachment(const Attachment& attachment) {
    this->attachment = attachment;
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

string Message::createMIMEWithAttachment() const {
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
    if (attachment.exist()) return base64_encode(createMIMEWithAttachment());
    return base64_encode(createMIME());
}

void GmailAPI::sendMessage(const Message& message) {
    string encoded_message = message.getEncodedMessage();
    
    const string post_fields = R"({"raw": ")" + encoded_message + R"("})";

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + oauth.getAccessToken()).c_str());

    string url = "https://gmail.googleapis.com/gmail/v1/users/me/messages/send";

    makeRequest(url, headers, post_fields);
}

string GmailAPI::getLatestMessageID() {
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + oauth.getAccessToken()).c_str());

    string url = "https://gmail.googleapis.com/gmail/v1/users/me/messages?maxResults=1&includeSpamTrash=true";

    string response = makeRequest(url, headers, "");

    json j = json::parse(response);
    return j["messages"][0]["id"];
}

Message GmailAPI::getLatestMessage() {
    Message message;

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + oauth.getAccessToken()).c_str());

    string id = getLatestMessageID();
    string url = "https://gmail.googleapis.com/gmail/v1/users/me/messages/" + id + "?format=full";

    string response = makeRequest(url, headers, "");

    json j = json::parse(response);

    auto response_headers = j["payload"]["headers"];
    for (auto header: response_headers) {
        if (header["name"] == "Delivered-To") message.setToEmail(header["value"]);
        if (header["name"] == "From") {
            if (string(header["value"]).find('\u003c') != string::npos) message.setFromEmail(getStringBetween(string(header["value"]), "\u003c", "\u003e"));
            else message.setFromEmail(header["value"]);
        }
        if (header["name"] == "Subject") message.setSubject(header["value"]);
    }

    message.setBody(trim(base64_decode(j["payload"]["parts"][0]["body"]["data"])));

    return message;
}