#include "server_cmd.h"

int __cdecl main(void) {
    Server server;
	ReceiveCommand rc;
    server.initialize();

    if (server.listenForConnection() == 0) {
		while (true) {
			rc.getLatestCommand(server);
			rc.executeCommand(server);
		}
	}
	
    return 0;
}