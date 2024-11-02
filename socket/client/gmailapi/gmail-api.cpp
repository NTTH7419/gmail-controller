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

string Message::getGmailID() const {
    return gmail_id;
}

string Message::getMessageID() const {
    return message_id;
}

string Message::getInReplyTo() const {
    return in_reply_to;
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

void Message::setGmailID(const string& gmail_id) {
    this->gmail_id = gmail_id;
}

void Message::setMessageID(const string& message_id) {
    this->message_id = message_id;
}

void Message::setInReplyTo(const string& in_reply_to) {
    this->in_reply_to = in_reply_to;
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

string Message::createMIME(const Attachment& attachment) const {
    string boundary = "boundary_string";
    stringstream mimeMessage;

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

string Message::getEncodedMessage(const Attachment& attachment) const {
    return base64_encode(createMIME(attachment));
}

bool Message::isEmpty() const {
    return (from.empty() && to.empty() && subject.empty() && body.empty());
}

void Message::clear() {
    gmail_id = "";
    in_reply_to = "";
    from = "";
    to = "";
    subject = "";
    body = "";
}

void GmailAPI::sendMessage(const Message& message, const Attachment& attachment) {
    string encoded_message = message.getEncodedMessage(attachment);
    
    string post_fields = R"({"raw": ")" + encoded_message + R"("})";

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

    string url = "https://gmail.googleapis.com/gmail/v1/users/me/messages";
           url += "?maxResults=1&includeSpamTrash=true&q=" + urlEncode(query);

    string response = makeRequest(url, headers, "");

    json j = json::parse(response);
    if (j.contains("messages")) return j["messages"][0]["id"];
    else return "";
}

void GmailAPI::markAsRead(const string& message_id) {
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

    message.setGmailID(j["id"]);

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
        if (header["name"] == "Message-ID") message.setMessageID(header["value"]);
    }

    message.setBody(trim(base64_decode(j["payload"]["parts"][0]["body"]["data"])));

    return message;
}

void GmailAPI::replyMessage(const Message& message, Message& reply_content, const Attachment& attachment) {
    reply_content.setInReplyTo(message.getMessageID());
    reply_content.setFromEmail(message.getToEmail());
    reply_content.setToEmail(message.getFromEmail());
    sendMessage(reply_content, attachment);
}