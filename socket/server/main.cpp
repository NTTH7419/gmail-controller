#include "commands.h"

int __cdecl main(void) {
    Server server;
	ReceiveCommand rc;
    server.initialize();

    if (server.listenForConnection() == 0) {
		while (true) {
			rc.getLatestCommand(server);
			rc.executeCommand(server);
		}

		// rc.process(server);

	}
	
    return 0;
}