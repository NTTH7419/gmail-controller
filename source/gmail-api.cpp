#include "gmail-api.h"

string getCurrentDate() {
    time_t now = time(nullptr);
    struct tm tstruct;
    char buf[80];
    tstruct = *gmtime(&now);
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %z", &tstruct);
    return string(buf);
}

string createEmail(const Message& m) {
    string email = "";

    email += "From: " + m.from + "\r\n";
    email += "To: " + m.to + "\r\n";
    email += "Subject: " + m.subject + "\r\n";
    email += "Date: " + getCurrentDate() + "\r\n";
    email += "Content-Type: text/plain; charset=UTF-8\r\n";
    email += "\r\n"; // Empty line between headers and body
    email += m.body + "\r\n";

    return email;
}


void sendEmail(const string& accessToken, const Message& m) {
    string encoded_message = base64_encode(createEmail(m), true);
    const string post_fields = R"({"raw": ")" + encoded_message + R"("})";

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());

    string url = "https://gmail.googleapis.com/gmail/v1/users/me/messages/send";

    makeRequest(url, headers, post_fields.c_str());
}

string getLatestMessageID(const string& accessToken) {
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());

    string url = "https://gmail.googleapis.com/gmail/v1/users/me/messages?maxResults=1&includeSpamTrash=true";

    string response = makeRequest(url, headers, "");

    json j = json::parse(response);
    return j["messages"][0]["id"];
}

Message getLatestMessage(const string& accessToken) {
    Message m;

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());

    string id = getLatestMessageID(accessToken);
    string url = "https://gmail.googleapis.com/gmail/v1/users/me/messages/" + id + "?format=full";

    string response = makeRequest(url, headers, "");

    json j = json::parse(response);

    auto response_headers = j["payload"]["headers"];
    for (auto header: response_headers) {
        if (header["name"] == "Delivered-To") m.to = header["value"];
        if (header["name"] == "From") {
            if (string(header["value"]).find('\u003c') != string::npos) m.from = getStringBetween(string(header["value"]), "\u003c", "\u003e");
            else m.from = header["value"];
        }
        if (header["name"] == "Subject") m.subject = header["value"];
    }

    m.body = trim(base64_decode(j["payload"]["parts"][0]["body"]["data"]));

    return m;
}