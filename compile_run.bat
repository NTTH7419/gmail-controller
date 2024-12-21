@echo off
set /p "input=Enter 1: Build client, 2: Build server, 3: Run client, 4: run server: "

if %input% == 1 (
    cd .\source\client
    make
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
            powershell -command ".\client"
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