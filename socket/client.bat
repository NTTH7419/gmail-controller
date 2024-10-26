cd client
g++ -o client.exe client.cpp -lws2_32
set /p "ip=Input server IPv4 (127.0.0.1 on the same machine): "
client.exe %ip%
pause