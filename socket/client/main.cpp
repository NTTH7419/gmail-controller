#include "commands.h"

int __cdecl main(int argc, char **argv) {    
    // Validate the parameters
    if (argc != 2) {
        cout << "usage: client.exe [server-name]" << endl;
        return 1;
    }

    Client c;
    c.initialize();
    c.connectToServer(argv[1]);
    
	ProcessCommand pc;
	while (!pc.getLatestMessage()) {
		cout << "No new command, retry after 3 secs" << endl;
		Sleep(3000);		
	}
	
	pc.executeCommand(c);
	pc.sendResponse();

    return 0;
}