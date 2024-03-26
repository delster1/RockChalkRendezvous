#include <stdio.h>
#include <string.h>
#include "httplib.h"

#include "../timeanddate.hpp"

using namespace httplib;


int main() {
	Client client("localhost", 8080);
	
	Result result = client.Post("/", "message", "text/plain");
	if (result) {
		if (result->status == StatusCode::OK_200) {
			printf("HTTP Response: \n%s\n", result->body.c_str());
			
		}
	} else {
		Error error = result.error();
		printf("HTTP error: %s\n", to_string(error).c_str());
	}
}

