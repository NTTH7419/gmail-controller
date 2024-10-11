#include "OAuth2.h"

string ClientSecrect::client_id = "572343860956-2ioat3vvr85nqmjjvtm2p4gvcvcet2ei.apps.googleusercontent.com";
string ClientSecrect::auth_uri = "https://accounts.google.com/o/oauth2/auth";
string ClientSecrect::token_uri = "https://oauth2.googleapis.com/token";
string ClientSecrect::auth_provider_x509_cert_url = "https://www.googleapis.com/oauth2/v1/certs";
string ClientSecrect::client_secret = "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";
string ClientSecrect::redirect_uri = "http://localhost";

string getAuthCode() {
	string url;
	cout << "Input the URL Google redirected you to: ";
	cin >> url;
	
	int idx = url.find("code=");
	if (idx == string::npos) return "";
	string auth_code = url.erase(0, idx + 5);
    idx = auth_code.find("&");
    if (idx != string::npos) {
        auth_code.erase(idx);
    }
	return auth_code;
}

size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    if (userp == nullptr) {
        std::cerr << "Error: userp is null" << std::endl;
        return 0;  // Returning 0 will tell libcurl to abort the request
    }

    try {
        userp->append((char*)contents, totalSize);
    } catch (std::bad_alloc& e) {
        std::cerr << "Memory allocation error: " << e.what() << std::endl;
        return 0;  // Returning 0 will tell libcurl to abort the request
    }

    return totalSize;  // Must return the number of bytes handled
}

string getTokenResponse(const string& auth_code) {
    CURL *curl;
    CURLcode res;
    string tokenResponse = "";

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if(curl) {
        // Set up POST request
        curl_easy_setopt(curl, CURLOPT_URL, ClientSecrect::token_uri.c_str());

        curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");

        // struct curl_slist *headers = NULL;
        // headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        // curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Prepare the POST fields
        string postFields = "code=" + auth_code + 
                                 "&client_id=" + ClientSecrect::client_id +
                                 "&client_secret=" + ClientSecrect::client_secret +
                                 "&redirect_uri=" + ClientSecrect::redirect_uri +
                                 "&grant_type=authorization_code";

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());

        // Response handling
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &tokenResponse);

        // Perform the request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        }

        // Cleanup
        // curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return tokenResponse;
}

void getAccessToken(string token_response, string& access_token, string& refresh_token) {
    string key = "\"access_token\": \"";
    int first = token_response.find(key);
    if (first == string::npos) {
        access_token = "";
        return;
    }
    first += key.length();
    int last = token_response.find('\"', first);
    access_token = token_response.substr(first, last - first);

    key = "\"refresh_token\": \"";
    first = token_response.find(key);
    if (first == string::npos) {
        refresh_token = "";
        return;
    }
    first += key.length();
    last = token_response.find('\"', first);
    refresh_token = token_response.substr(first, last - first);
}

Token login() {
	// prepare parameter
    const string scope = "https://mail.google.com/ https://www.googleapis.com/auth/gmail.send https://www.googleapis.com/auth/gmail.readonly https://www.googleapis.com/auth/gmail.compose https://www.googleapis.com/auth/gmail.modify https://www.googleapis.com/auth/gmail.metadata";
    const string response_type = "code";
    const string access_type = "offline";
    const string prompt = "consent";

	// construct url
    string auth_url = ClientSecrect::auth_uri.c_str();
    auth_url += "?client_id=" + ClientSecrect::client_id;
    auth_url += "&redirect_uri=" + ClientSecrect::redirect_uri;
    auth_url += "&response_type=" + response_type;
    auth_url += "&scope=" + scope;
    auth_url += "&access_type=" + access_type;
    auth_url += "&prompt=" + prompt;

	// open google login
	string command = "start \"\" \"" + auth_url + '\"';  // start "" "auth_url"
	system(command.c_str());

    string auth_code = getAuthCode();
    if (auth_code == "") cerr << "Invalid Auth code";

	string token_response = getTokenResponse(auth_code);
    if (token_response == "") cerr << "Invalid Token response";

    string access_token, refresh_token;
    getAccessToken(token_response, access_token, refresh_token);

    Token t = {access_token, refresh_token};
    return t;
}
