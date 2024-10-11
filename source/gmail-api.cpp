#include "gmail-api.h"


// Function to get the current date/time in RFC 2822 format
std::string getCurrentDateRFC2822() {
    time_t now = time(nullptr);
    struct tm tstruct;
    char buf[80];
    tstruct = *gmtime(&now);
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %z", &tstruct);
    return std::string(buf);
}

std::string createEmail(const Message& m) {
    std::ostringstream email;

    email << "From: " << m.from << "\r\n";
    email << "To: " << m.to << "\r\n";
    email << "Subject: " << m.subject << "\r\n";
    email << "Date: " << getCurrentDateRFC2822() << "\r\n";
    email << "Content-Type: text/plain; charset=UTF-8\r\n";
    email << "\r\n"; // Empty line between headers and body
    email << m.body << "\r\n";

    std::string emailMessage = email.str();
    
    return emailMessage;
}


void sendEmail(const string& accessToken, const Message& m) {
    CURL *curl;
    CURLcode res;

    string encoded_message = base64_encode(createEmail(m), true);

    const std::string json = R"({"raw": ")" + encoded_message + R"("})";
    cout << json.c_str() << endl;

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://gmail.googleapis.com/gmail/v1/users/me/messages/send");
        curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;

        curl_easy_cleanup(curl);
    }

    curl_slist_free_all(headers);
}