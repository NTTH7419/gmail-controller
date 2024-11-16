@echo off
set /p "input=Enter 1: Build client, 2: Build server, 3: Run client, 4: run server: "

if %input% == 1 (
    cd .\socket\client
    g++ -o client.exe *.cpp gmailapi/*.cpp -lws2_32 -lcurl -I./lib/curl_8.10.1/include -I./lib/json/include -L./lib/curl_8.10.1/lib
) else (
    if %input% == 2 (
        cd .\socket\server
        g++ -o server.exe *.cpp -lws2_32
    ) else (
        if %input% == 3 (
            cd .\socket\client
            client.exe 127.0.0.1
        ) else (
            if %input% == 4 (
                cd .\socket\server
                server.exe
            ) else (
                echo Invalid command
            )
        )
    )
)