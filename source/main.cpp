#include "OAuth2.h"
#include "gmail-api.h"
#include <fstream>

int main() {
    Token t;
    ifstream fin("token.txt");
    if (fin >> t.access_token) {
        fin >> t.refresh_token;
        fin.close();
    }
    else {
        fin.close();
        ofstream fout("token.txt");
        t = login();
        fout << t.access_token << endl << t.refresh_token;
        fout.close();
    }

    Message m;
    m.from = "pntuan23@clc.fitus.edu.vn";
    m.to = "quangthangngo181@gmail.com";
    m.subject = "Test Gmail API";
    m.body = "Hello World";

    sendEmail(t.access_token, m);
    
    return 0;
}
