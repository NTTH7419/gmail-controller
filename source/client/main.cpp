#include "process_cmd.h"

int __cdecl main() {    
    // Validate the parameters

    Client c;
    c.initialize();
    // cout << "type in the IP of the computer to connect to" << endl;
    // c.receiveAvailableIP();
    c.sendDiscovery();
    // string ip;
    // getline(cin, ip);
    // c.connectToServer(ip.c_str());
    
	ProcessCommand pc;

    while (true) {
        if (!pc.getLatestMessage()) {
    		std::cout << "No new command, retry after 3 secs" << std::endl;
    		Sleep(3000);
            continue;	
    	}
    	
    	pc.executeCommand(c);
    }

    return 0;
}