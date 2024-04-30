ifeq ($(shell echo "check_quotes"),"check_quotes")
    WINDOWS := yes
else
    WINDOWS := no
endif

main: main.cpp
ifeq ($(WINDOWS), yes)
	g++ -std=c++17 -o client.exe -I../../lib/cpp-httplib main.cpp -lWs2_32
else
	g++ -std=c++17 -o client -I../../lib/cpp-httplib main.cpp #-lpthread
endif
