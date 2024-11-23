@echo off
set /p "input=Enter 1: Build client, 2: Build server, 3: Run client, 4: run server: "

if %input% == 1 (
    cd .\socket\client
    g++ -o client.exe *.cpp gmailapi/*.cpp -lpsapi -lws2_32 -lcurl -I./lib/curl_8.11/include -I./lib/json/include -L./lib/curl_8.11/lib
    echo completed
    pause
) else (
    if %input% == 2 (
        cd .\socket\server
        g++ -o server.exe *.cpp -lws2_32 -lpsapi -lgdiplus -lgdi32
	echo completed
	pause
    ) else (
        if %input% == 3 (
            cd .\socket\client
            client.exe
	    pause
        ) else (
            if %input% == 4 (
                cd .\socket\server
                server.exe
		pause
            ) else (
                echo Invalid command
            )
        )
    )
)