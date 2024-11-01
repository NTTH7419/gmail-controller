#include "OAuth2.h"
#include "gmail-api.h"

int main() {
    GmailAPI gmail;
    Message send("pntuan23@clc.fitus.edu.vn", "nqthang23@clc.fitus.edu.vn", "Hello", "Gay");
    gmail.sendMessage(send);

    return 0;
}
