#include "tool.h"

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

string makeRequest(const string& request_url, struct curl_slist *headers, const string& post_fields) {
    CURL *curl;
    CURLcode res;
    string response;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, request_url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
        curl_easy_setopt(curl, CURLOPT_CAINFO, "socket/client/cacert.pem");
        if (headers) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        if (post_fields.length()) curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

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