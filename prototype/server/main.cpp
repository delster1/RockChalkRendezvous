#include <stdio.h>
#include <string.h>
#include "httplib.h"

#include "../definitions.hpp"

using namespace httplib;


int main() {
	Server server;
	
	server.Get("/", [](const Request& request, Response& response) {
		response.set_content("Hello from server!", "text/plain");
	});
	
	auto now = TimeAndDate::now();
	printf("Server started.\nCurrent time: %s\n", now.to_string().c_str());
	
	server.Post("/get-data", [&](const Request& request, Response& response) {
		printf("Received post request: %s\n", request.body.c_str());
		
		std::string send = now.encode();
		response.set_content(send, "text/plain");
	});
	
	
	
	
	server.listen("0.0.0.0", 8080);
}

