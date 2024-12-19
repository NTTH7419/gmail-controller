#include "OAuth2.h"

std::string OAuth::getAuthCode() {
	std::string url;
	std::cout << "Input the URL Google redirected you to: ";
	std::cin >> url;
	
	int idx = url.find("code=");
	if (idx == std::string::npos) {
        return "";
    }
	
    std::string auth_code = url.erase(0, idx + 5);
    idx = auth_code.find("&");
    if (idx != std::string::npos) {
        auth_code.erase(idx);
    };
    return auth_code;
}

void OAuth::openGoogleLogin() {
    // prepare parameter
    const std::string scope = "https://mail.google.com/";
    const std::string response_type = "code";
    const std::string access_type = "offline";
    const std::string prompt = "consent";

	// construct url
    std::string auth_url = credential.auth_uri;
    auth_url += "?client_id=" + credential.client_id;
    auth_url += "&redirect_uri=" + credential.redirect_uri;
    auth_url += "&response_type=" + response_type;
    auth_url += "&scope=" + scope;
    auth_url += "&access_type=" + access_type;
    auth_url += "&prompt=" + prompt;

	// open google login
	std::string command = "start \"\" \"" + auth_url + '\"';  // start "" "auth_url"
	system(command.c_str());
}

std::string OAuth::getTokenResponse(const std::string& auth_code) {
    std::string post_fields = "code=" + auth_code + 
                             "&client_id=" + credential.client_id +
                             "&client_secret=" + credential.client_secret +
                             "&redirect_uri=" + credential.redirect_uri +
                             "&grant_type=authorization_code";

    std::string token_response = makeRequest(credential.token_uri, NULL, post_fields, "POST").body;
    
    return token_response;
}

void OAuth::login() {
	openGoogleLogin();

    std::string auth_code = getAuthCode();
    if (auth_code.empty()) {
        is_error = true;
        error_message = "Auth code not found.";
        return;
    }

	std::string token_response = getTokenResponse(auth_code);
    if (token_response.empty()) {
        is_error = true;
        error_message = "Token response not found.";
        return;
    }


    json j = json::parse(token_response);
    if (j.contains("access_token") && j.contains("refresh_token") && j.contains("expires_in")) {
        token.access_token = j["access_token"];
        token.refresh_token = j["refresh_token"];
        token.refresh_time = time(0) + int(int(j["expires_in"]) * 0.9);
        writeTokenToFile();
    }
    else {
        is_error = true;
        error_message = "No access token found";
        return;
    }
    is_error = false;
    error_message.clear();
}

std::string OAuth::getAccessToken() {
    if (time(0) > token.refresh_time) {
        refreshToken();
    }
    return token.access_token;
}

bool OAuth::good() {
    return !is_error;
}

std::string OAuth::getErrorMessage() {
    return error_message;
}

void OAuth::refreshToken() {
    std::string url = credential.token_uri;
    std::string post_fields = "refresh_token=" + token.refresh_token + 
                         "&client_id=" + credential.client_id +
                         "&client_secret=" + credential.client_secret +
                         "&grant_type=refresh_token";

    std::string token_response = makeRequest(url, NULL, post_fields, "POST").body;

    json j = json::parse(token_response);
    if (j.contains("access_token") && j.contains("expires_in")) {
        token.access_token = j["access_token"];
        token.refresh_time = time(0) + int(int(j["expires_in"]) * 0.9);
        writeTokenToFile();
    }
    // Token has been expired or revoked, or something else
    else {
        login();
    }

    is_error = false;
    error_message.clear();
}

void OAuth::writeTokenToFile() {
    std::ofstream fout (token_file);
    fout << token.refresh_token << std::endl << token.access_token << std::endl << token.refresh_time;
    fout.close();
}

void OAuth::init() {
    json j;
    try {
        std::ifstream fin(client_secret_file);
        j = json::parse(fin);
        fin.close();
        credential.client_id = j["installed"]["client_id"];
        credential.client_secret = j["installed"]["client_secret"];
        credential.auth_uri = j["installed"]["auth_uri"];
        credential.token_uri = j["installed"]["token_uri"];
        credential.redirect_uri = j["installed"]["redirect_uris"][0];
    }
    catch(...) {
        error_message = "\"client_secret.json\" content is not found or has wrong format.";
        is_error = true;
    }

    std::ifstream fin(token_file);
    if (!(fin >> token.refresh_token) ||
        !(fin >> token.access_token) ||
        !(fin >> token.refresh_time)) {
            login();
    }
    fin.close();

    refreshToken();

    is_error = false;
    error_message.clear();
}

OAuth::OAuth() : client_secret_file("client_secret.json"), token_file("token.txt"),
                    is_error(true), error_message("OAuth hasn't been initialized") {}

OAuth::~OAuth() {}
