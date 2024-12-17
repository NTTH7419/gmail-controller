#include "gmail-api.h"

Attachment::Attachment(std::string file_path, std::string file_name) : file_path(file_path), file_name (file_name) {
    readFile();
    file_size = file_content.size();
}

Attachment::Attachment(std::string file_path) : file_path(file_path) {
    int idx = file_path.find_last_of("/");
    if (idx == std::string::npos) {
        idx = file_path.find_last_of("\\");
        if (idx == std::string::npos) {
            file_name = file_path;
            readFile();
            return;
        }
    }

    file_name = file_path.substr(idx + 1);
    readFile();
    file_size = file_content.size();
}

void Attachment::readFile() {
    if (exist()) return;
    std::ifstream fin (file_path, std::ios::binary);
    file_content = std::string(std::istreambuf_iterator<char>(fin), std::istreambuf_iterator<char>());
    fin.close();
}

std::string Attachment::getEncodedFileContent() const {
    return base64_encode(file_content);
}

void Attachment::setFileName(const std::string& file_name) {
    this->file_name = file_name;
}

std::string Attachment::getFileName() const {
    return file_name;
}

int Attachment::getFileSize() const {
    return file_size;
}

bool Attachment::exist() const {
    return (!file_name.empty() && !file_content.empty());
}





std::string Message::getGmailID() const {
    return gmail_id;
}

std::string Message::getMessageID() const {
    return message_id;
}

std::string Message::getInReplyTo() const {
    return in_reply_to;
}

std::string Message::getFromEmail() const {
    return from;
}

std::string Message::getToEmail() const {
    return to;
}

std::string Message::getSubject() const {
    return subject;
}

std::string Message::getBody() const {
    return body;
}

void Message::setGmailID(const std::string& gmail_id) {
    this->gmail_id = gmail_id;
}

void Message::setMessageID(const std::string& message_id) {
    this->message_id = message_id;
}

void Message::setInReplyTo(const std::string& in_reply_to) {
    this->in_reply_to = in_reply_to;
}

void Message::setFromEmail(const std::string& from) {
    this->from = from;
}

void Message::setToEmail(const std::string& to) {
    this->to = to;
}

void Message::setSubject(const std::string& subject) {
    this->subject = subject;
}

void Message::setBody(const std::string& body) {
    this->body = body;
}

std::string Message::createMIME(const Attachment& attachment) const {
    std::string boundary = "boundary_string";
    std::stringstream mimeMessage;

    // Common headers for all message types
    mimeMessage << "MIME-Version: 1.0\r\n";
    mimeMessage << "To: " << to << "\r\n";
    mimeMessage << "From: " << from << "\r\n";
    mimeMessage << "Subject: " << subject << "\r\n";

    // Add reply headers if present
    if (!in_reply_to.empty()) {
        mimeMessage << "In-Reply-To: " << in_reply_to << "\r\n";
    }

    // Determine message type based on whether there's an attachment
    if (!attachment.exist()) {
        // Case 1: Normal message or reply without attachment
        mimeMessage << "Content-Type: text/plain; charset=\"UTF-8\"\r\n\r\n";
        mimeMessage << body << "\r\n";
    } else {
        // Case 2: Message with attachment or reply with attachment
        mimeMessage << "Content-Type: multipart/mixed; boundary=\"" << boundary << "\"\r\n\r\n";
        mimeMessage << "--" << boundary << "\r\n";
        mimeMessage << "Content-Type: text/plain; charset=\"UTF-8\"\r\n\r\n";
        mimeMessage << body << "\r\n\r\n";

        // Add the attachment
        mimeMessage << "--" << boundary << "\r\n";
        mimeMessage << "Content-Type: application/octet-stream\r\n";
        mimeMessage << "Content-Disposition: attachment; filename=\"" << attachment.getFileName() << "\"\r\n";
        mimeMessage << "Content-Transfer-Encoding: base64\r\n\r\n";
        mimeMessage << attachment.getEncodedFileContent() << "\r\n";
        mimeMessage << "--" << boundary << "--\r\n";
    }

    return mimeMessage.str();
}

std::string Message::getMessage(const Attachment& attachment) const {
    return createMIME(attachment);
}

bool Message::isEmpty() const {
    return (from.empty() && to.empty() && subject.empty() && body.empty());
}

void Message::clear() {
    gmail_id.clear();
    in_reply_to.clear();
    from.clear();
    to.clear();
    subject.clear();
    body.clear();
}






void GmailAPI::sendMessageWithAttachment(const Message& message, const Attachment& attachment) {
    std::string text_message = message.getMessage(attachment);
    int message_length = text_message.length();

    if (message_length > 25 * 1024 * 1024) {// 25 MB
        throw std::runtime_error("Attachment too large, limit is 25MB");
    }

    std::string token = oauth.getAccessToken();

    // init upload
    std::string url = "https://www.googleapis.com/upload/gmail/v1/users/me/messages/send?uploadType=resumable";
    curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Content-Length: 0");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
    headers = curl_slist_append(headers, "X-Upload-Content-Type: message/rfc822");
    headers = curl_slist_append(headers, ("X-Upload-Content-Length: " + std::to_string(message_length)).c_str());

    HTTPResponse response = makeRequest(url, headers, "", "POST");
    json j = response.body;
    if (j.contains("error")) {
        throw std::runtime_error(j["error"]["errors"]["message"]);
    }

    std::string session_uri = response.getHeader("Location");
    if (session_uri.empty()) {
        throw std::runtime_error("Cannot init uploading");
    }

    // uploading
    headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
    headers = curl_slist_append(headers, "Content-Type: message/rfc822");
    headers = curl_slist_append(headers, ("Content-Length: " + std::to_string(message_length)).c_str());

    response = makeRequest(session_uri, headers, text_message, "PUT");
    if (response.getStatus() != 200) {
        throw std::runtime_error("Cannot send message");
    }

}

void GmailAPI::sendMessageWithoutAttachment(const Message& message) {
    std::string token = oauth.getAccessToken();

    std::string encoded_message = base64_encode(message.getMessage());
    
    std::string post_fields = R"({"raw": ")" + encoded_message + R"("})";

    curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());

    std::string url = "https://gmail.googleapis.com/gmail/v1/users/me/messages/send";

    HTTPResponse response = makeRequest(url, headers, post_fields, "POST");
    if (response.getStatus() != 200) {
        throw std::runtime_error("Cannot send message");
    }
}

void GmailAPI::sendMessage(const Message& message, const Attachment& attachment) {
    if (attachment.exist()) {
        sendMessageWithAttachment(message, attachment);
    }
    else {
        sendMessageWithoutAttachment(message);
    }
}

std::string GmailAPI::getLatestMessageID(const std::string& query) {
    std::string token = oauth.getAccessToken();

    curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());

    std::string url = "https://gmail.googleapis.com/gmail/v1/users/me/messages";
           url += "?maxResults=1&includeSpamTrash=true&q=" + urlEncode(query);

    HTTPResponse response = makeRequest(url, headers, "", "GET");
    if (response.getStatus() != 200) {
        throw std::runtime_error("Cannot retrieve message id.");
    }

    json j = json::parse(response.body);
    if (j.contains("messages")) return j["messages"][0]["id"];
    else return "";
}

void GmailAPI::markAsRead(const std::string& message_id) {
    if(message_id.empty()) return;
    
    std::string token = oauth.getAccessToken();

    curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());

    std::string post_fields = R"({"removeLabelIds": ["UNREAD"]})";

    std::string url = "https://gmail.googleapis.com/gmail/v1/users/me/messages/" + message_id + "/modify";

    HTTPResponse response = makeRequest(url, headers, post_fields, "POST");
    if (response.getStatus() != 200) {
        throw std::runtime_error("Cannot mark message as read.");
    }
}

Message GmailAPI::getLatestMessage(const std::string& query) {
    std::string token = oauth.getAccessToken();

    Message message;

    std::string id = getLatestMessageID(query);
    if (id.empty()) {
        return message;
    }

    curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());

    std::string url = "https://gmail.googleapis.com/gmail/v1/users/me/messages/" + id + "?format=full";

    HTTPResponse response = makeRequest(url, headers, "", "GET");
    if (response.getStatus() != 200) {
        throw std::runtime_error("Cannot retrive message.");
    }

    try {
        json j = json::parse(response.body);

        message.setGmailID(j["id"]);

        auto response_headers = j["payload"]["headers"];
        for (auto header: response_headers) {
            if (header["name"] == "Delivered-To") message.setToEmail(header["value"]);
            if (header["name"] == "From") {
                if (std::string(header["value"]).find('\u003c') != std::string::npos)
                    message.setFromEmail(getStringBetween(std::string(header["value"]), "\u003c", "\u003e"));
                else
                    message.setFromEmail(header["value"]);
            }
            if (header["name"] == "Subject") message.setSubject(header["value"]);
            if (header["name"] == "Message-ID") message.setMessageID(header["value"]);
        }

        message.setBody(trim(base64_decode(j["payload"]["parts"][0]["body"]["data"])));
    }
    catch (...) {
        throw std::runtime_error("Cannot retrieve message.");
    }

    return message;
}

void GmailAPI::replyMessage(const Message& message, Message& reply_content, const Attachment& attachment) {
    reply_content.setInReplyTo(message.getMessageID());
    reply_content.setFromEmail(message.getToEmail());
    reply_content.setToEmail(message.getFromEmail());
    sendMessage(reply_content, attachment);
}

GmailAPI::GmailAPI() : oauth() {}

void GmailAPI::initOAuth() {
    oauth.init();
    if (!oauth.good()) {
        throw std::runtime_error(oauth.getErrorMessage());
    }
}