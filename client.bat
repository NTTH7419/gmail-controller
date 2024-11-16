g++ -o .\socket\client\client.exe ./socket/client/*.cpp ./socket/client/gmailapi/*.cpp -lpsapi -lws2_32 -lcurl -I./lib/curl_8.10.1/include -I./lib/json/include -L./lib/curl_8.10.1/lib 
set /p "ip=Input server IPv4 (127.0.0.1 on the same machine): "
.\socket\client\client.exe %ip%
pause