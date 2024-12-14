@echo off
set /p "input=Enter 1: Build client, 2: Build server, 3: Run client, 4: run server: "

if %input% == 1 (
    g++ -o release/client/client.exe source/client/*.cpp source/client/gmailapi/*.cpp -lpsapi -lws2_32 -lcurl -I./source/client/lib/curl_8.11/include -I./source/client/lib/json/include -L./source/client/lib/curl_8.11/lib
    echo completed
    pause
) else (
    if %input% == 2 (
        g++ -o release/server/server.exe source/server/*.cpp -lws2_32 -lpsapi -lgdiplus -lgdi32 -lole32 -loleaut32 -lstrmiids
	echo completed
	pause
    ) else (
        if %input% == 3 (
            cd .\release\client
            powershell -command "Start-Process client.exe -Verb runas"
	    pause
        ) else (
            if %input% == 4 (
                cd .\release\server
                powershell -command "Start-Process server.exe -Verb runas"
		pause
            ) else (
                echo Invalid command
            )
        )
    )
)