#include <stdio.h>
#include <string.h>
#include "httplib.h"

#include "../timeanddate.hpp"

using namespace httplib;


int main() {
	Server server;
	
	auto now = TimeAndDate::now();
	printf("Server started.\nCurrent time: %s\n", now.to_string().c_str());
	
	server.Post("/", [&](const Request& request, Response& response) {
		
		// request.body
		// response.set_content("Hello from server!", "text/plain");
		
		std::string send = now.encode();
		response.set_content(send, "text/plain");
	});
	
	server.listen("0.0.0.0", 8080);
}

