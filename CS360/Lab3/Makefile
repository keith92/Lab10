server: server.cpp
	g++ -o server server.cpp -pthread

run: server.cpp
	./server 3010 1000 ~/htdocs/CS360/Lab3/srvfiles

debug: server.cpp
	g++ -g server.cpp -pthread

rundebug: server.cpp
	gdb a.out
