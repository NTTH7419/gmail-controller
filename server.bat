g++ -o ./socket/server/server.exe ./socket/server/*.cpp -lws2_32 -lpsapi -lgdiplus -lgdi32
cd socket\server
server.exe
pause