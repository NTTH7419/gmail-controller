#include "tool.h"

std::string HTTPResponse::getHeader(const std::string& name) {
    std::string value = "";
    int pos = headers.find(name);
    if (pos == std::string::npos) return value;
    int pos2 = headers.find('\n', pos);
    if (pos2 == std::string::npos) return value;
    value = trim(headers.substr(pos + name.size() + 2, pos2 - (pos + name.size() + 2)));
    return value;
}

int HTTPResponse::getStatus() {
    if (headers.empty()) return 0;
    int idx = headers.find(' ');
    return stoi(headers.substr(idx + 1, 3));
}

size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t content_size = size * nmemb;
    ((std::string*)userp)->append((char*)contents, content_size);
    return content_size;
}

size_t headerCallback(char* buffer, size_t size, size_t nitems, void* headers) {
    size_t header_size = size * nitems;
    ((std::string*)headers)->append(buffer, header_size);
    return header_size;
}

HTTPResponse makeRequest(const std::string& request_url, curl_slist *headers, const std::string& post_fields, const std::string& type) {
    CURL *curl;
    CURLcode res;
    HTTPResponse response;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, request_url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
        curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, type.c_str());
        if (headers) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        if (!post_fields.empty()) curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &(response.body));
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &(response.headers));

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            throw std::runtime_error(std::string("curl_easy_perform() failed: ") + curl_easy_strerror(res));

        if (headers) curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    return response;
}

std::string getStringBetween(const std::string& str, const std::string& begin, const std::string& end) {
    std::string result;
    int idx;
    idx = str.find(begin) + begin.size();
    result = str.substr(idx, str.find(end) - idx);
    return result;
}

std::string trim(std::string s) {
    std::string unwanted_char = " \n\r";
    s.erase(0, s.find_first_not_of(unwanted_char));
    s.erase(s.find_last_not_of(unwanted_char) + 1);
    return s;
}

std::string urlEncode(const std::string& value) {
    std::ostringstream encoded;
    for (unsigned char c : value) {
        // Check if character is alphanumeric or safe
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded << c;
        } else {
            // Percent-encode the character
            encoded << '%' << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << int(c);
        }
    }
    return encoded.str();
}