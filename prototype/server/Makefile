ifeq ($(OS),Windows_NT)
    WINDOWS := yes
else
    WINDOWS := no
endif

main: main.cpp
ifeq ($(WINDOWS), yes)
	g++ -std=c++17 -o server.exe -I../../lib/cpp-httplib main.cpp -lWs2_32
else
	g++ -std=c++17 -o server -I../../lib/cpp-httplib main.cpp -lpthread
endif
