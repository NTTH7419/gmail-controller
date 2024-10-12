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

string getTokenResponse(const string& auth_code) {
    string post_fields = "code=" + auth_code + 
                             "&client_id=" + ClientSecrect::client_id +
                             "&client_secret=" + ClientSecrect::client_secret +
                             "&redirect_uri=" + ClientSecrect::redirect_uri +
                             "&grant_type=authorization_code";

    string token_response = makeRequest(ClientSecrect::token_uri.c_str(), NULL, post_fields);
    
    return token_response;
}

void getAccessToken(const string& token_response, string& access_token, string& refresh_token) {
    json j = json::parse(token_response);
    access_token = j["access_token"];
    refresh_token = j["refresh_token"];
}

Token login() {
	// prepare parameter
    const string scope = "https://mail.google.com/ https://www.googleapis.com/auth/gmail.send https://www.googleapis.com/auth/gmail.readonly https://www.googleapis.com/auth/gmail.compose https://www.googleapis.com/auth/gmail.modify";
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
