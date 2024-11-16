#include "tool.h"

string HTTPResponse::getHeader(const string& name) {
    string value;
    int pos = headers.find(name);
    if (pos == string::npos) return value;
    int pos2 = headers.find('\n', pos);
    if (pos2 == string::npos) return value;
    value = trim(headers.substr(pos + name.size() + 2, pos2 - (pos + name.size() + 2)));
    return value;
}

size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t content_size = size * nmemb;
    ((string*)userp)->append((char*)contents, content_size);
    return content_size;
}

size_t headerCallback(char* buffer, size_t size, size_t nitems, void* headers) {
    size_t headerSize = size * nitems;
    ((string*)headers)->append(buffer, headerSize);
    return headerSize;
}

HTTPResponse makeRequest(const string& request_url, curl_slist *headers, const string& post_fields, const string& type) {
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
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;

        if (headers) curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    return response;
}

string getStringBetween(const string& str, const string& begin, const string& end) {
    string result;
    int idx;
    idx = str.find(begin) + begin.size();
    result = str.substr(idx, str.find(end) - idx);
    return result;
}

string trim(string s) {
    string unwanted_char = " \n\r";
    s.erase(0, s.find_first_not_of(unwanted_char));
    s.erase(s.find_last_not_of(unwanted_char) + 1);
    return s;
}

string urlEncode(const string& value) {
    ostringstream encoded;
    for (unsigned char c : value) {
        // Check if character is alphanumeric or safe
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded << c;
        } else {
            // Percent-encode the character
            encoded << '%' << setw(2) << setfill('0') << hex << uppercase << int(c);
        }
    }
    return encoded.str();
}