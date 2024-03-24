#include <stdio.h>
#include <string.h>
#include "httplib.h"

#include "../definitions.hpp"

using namespace httplib;


int main() {
	Client client("localhost", 8080);
	
	Result result = client.Get("/");
	if (result) {
		if (result->status == StatusCode::OK_200) {
			printf("HTTP Response: %s\n", result->body.c_str());
			
			result = client.Post("/get-data", "client says hi", "text/plain");
			
			std::istringstream parser(result->body);
			
			std::cout << "Post response (" << result->body.length() << "): " << TimeAndDate::decode(parser).unwrap().to_string().c_str() << "\n";
			
		}
	} else {
		Error error = result.error();
		printf("HTTP error: %s\n", to_string(error).c_str());
	}
}

