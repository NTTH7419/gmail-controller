g++ -o .\socket\client\client.exe ./socket/client/*.cpp ./socket/client/gmailapi/*.cpp -lpsapi -lws2_32 -lcurl -I./lib/curl_8.10.1/include -I./lib/json/include -L./lib/curl_8.10.1/lib 
.\socket\client\client.exe
pause