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

    Message m = getLatestMessage(t.access_token);
    cout << "From: " << m.from << endl;
    cout << "To: " << m.to << endl;
    cout << "Subject: " << m.subject << endl;
    cout << "Body: " << m.body << endl;

    return 0;
}
