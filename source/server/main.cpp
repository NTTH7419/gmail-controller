#include "server_cmd.h"

int __cdecl main(void) {
    Server server;
	ReceiveCommand rc;
    server.initialize();

    if (server.listenForConnection() == 0) {
		while (true) {
			if (server.isClientAlive()) {
				rc.getLatestCommand(server);
				rc.executeCommand(server);
			}
			else{
				std::cout << "No client available!" << std::endl;
				server.broadcastDiscovery();
				server.listenForConnection();
			}
		}
	}
	
    return 0;
}