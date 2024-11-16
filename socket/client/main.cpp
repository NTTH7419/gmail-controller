#include "commands.h"

// int __cdecl main(int argc, char **argv) {    
//     // Validate the parameters
//     if (argc != 2) {
//         cout << "usage: client.exe [server-name]" << endl;
//         return 1;
//     }

//     Client c;
//     c.initialize();
//     c.connectToServer(argv[1]);
    
// 	ProcessCommand pc;

//     while (true) {
//         if (!pc.getLatestMessage()) {
//     		cout << "No new command, retry after 3 secs" << endl;
//     		Sleep(3000);
//             continue;	
//     	}
    	
//     	pc.executeCommand(c);
//     }
	

//     // pc.process(c);

//     return 0;
// }

int __cdecl main() {    
    // Validate the parameters

    Client c;
    c.initialize();
    cout << "type in the IP of the computer to connect to" << endl;
    c.receiveAvailableIP();
    string ip;
    getline(cin, ip);
    c.connectToServer(ip.c_str());
    
	ProcessCommand pc;


    while (true) {
        if (!pc.getLatestMessage()) {
    		cout << "No new command, retry after 3 secs" << endl;
    		Sleep(3000);
            continue;	
    	}
    	
    	pc.executeCommand(c);
    }
	

    // pc.process(c);

    return 0;
}