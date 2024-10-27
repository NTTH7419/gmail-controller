#include "OAuth2.h"

string OAuth::getAuthCode() {
	string url;
	cout << "Input the URL Google redirected you to: ";
	cin >> url;
	
	int idx = url.find("code=");
	if (idx == string::npos) {
        return " ";
    }
	
    string auth_code = url.erase(0, idx + 5);
    idx = auth_code.find("&");
    if (idx != string::npos) {
        auth_code.erase(idx);
    };
    return auth_code;
}

void OAuth::openGoogleLogin() {
    // prepare parameter
    const string scope = "https://mail.google.com/";
    const string response_type = "code";
    const string access_type = "offline";
    const string prompt = "consent";

	// construct url
    string auth_url = credential.auth_uri.c_str();
    auth_url += "?client_id=" + credential.client_id;
    auth_url += "&redirect_uri=" + credential.redirect_uri;
    auth_url += "&response_type=" + response_type;
    auth_url += "&scope=" + scope;
    auth_url += "&access_type=" + access_type;
    auth_url += "&prompt=" + prompt;

	// open google login
	string command = "start \"\" \"" + auth_url + '\"';  // start "" "auth_url"
	system(command.c_str());
}

string OAuth::getTokenResponse(const string& auth_code) {
    string post_fields = "code=" + auth_code + 
                             "&client_id=" + credential.client_id +
                             "&client_secret=" + credential.client_secret +
                             "&redirect_uri=" + credential.redirect_uri +
                             "&grant_type=authorization_code";

    string token_response = makeRequest(credential.token_uri, NULL, post_fields);
    
    return token_response;
}

void OAuth::login() {
	openGoogleLogin();

    string auth_code = getAuthCode();
    if (auth_code == "") {
        is_error = true;
        error_message = "Auth code not found.";
    }

	string token_response = getTokenResponse(auth_code);
    if (token_response == "") {
        is_error = true;
        error_message = "Token response not found.";
    }

    json j = json::parse(token_response);
    if (j.contains("access_token")) {
        token.access_token = j["access_token"];
        token.refresh_token = j["refresh_token"];
        token.refresh_time = time(0) + int(int(j["expires_in"]) * 0.9);
        writeTokenToFile();
    }
    else {
        is_error = true;
        error_message = "No access token found";
    }
}

string OAuth::getAccessToken() {
    if (time(0) > token.refresh_time) {
        refreshToken();
    }
    return token.access_token;
}

bool OAuth::good() {
    return !is_error;
}

string OAuth::getErrorMessage() {
    return error_message;
}

void OAuth::refreshToken() {
    string url = credential.token_uri;
    string post_fields = "refresh_token=" + token.refresh_token + 
                         "&client_id=" + credential.client_id +
                         "&client_secret=" + credential.client_secret +
                         "&grant_type=refresh_token";

    string token_response = makeRequest(url, NULL, post_fields);

    json j = json::parse(token_response);
    if (j.contains("access_token")) {
        token.access_token = j["access_token"];
        token.refresh_time = time(0) + int(int(j["expires_in"]) * 0.9);
        writeTokenToFile();
    }
    else {
        is_error = true;
        error_message = "No access token found";
    }
}

void OAuth::writeTokenToFile() {
    ofstream fout (token_file);
    fout << token.refresh_token << endl << token.access_token << endl << token.refresh_time;
    fout.close();
}

OAuth::OAuth() : client_secret_file("client_secret.json"), token_file("token.txt") {

    json j;
    ifstream fin(client_secret_file);
    j = json::parse(fin);
    fin.close();
    credential.client_id = j["installed"]["client_id"];
    credential.client_secret = j["installed"]["client_secret"];
    credential.auth_uri = j["installed"]["auth_uri"];
    credential.token_uri = j["installed"]["token_uri"];
    credential.redirect_uri = j["installed"]["redirect_uris"][0];

    fin.open(token_file);
    if (fin >> token.refresh_token) {
        fin >> token.access_token >> token.refresh_time;
    }
    else {
        login();
    }
    fin.close();
}

OAuth::~OAuth() {}
