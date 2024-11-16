g++ -o client.exe client.cpp -lws2_32 -lpsapi
set /p "ip=Input server IPv4 (127.0.0.1 on the same machine): "
client.exe %ip%
pause