
LIB_FOLDER = ../../lib
LIBHTTP_INCLUDE = $(LIB_FOLDER)/libhttp/include
LIBHTTP_SOURCE = $(LIB_FOLDER)/libhttp/src/civetweb.c

main:
	gcc -o server.exe -I$(LIBHTTP_INCLUDE) main.cpp $(LIBHTTP_SOURCE) -lws2_32