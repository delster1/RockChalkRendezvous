#include <stdio.h>
#include <string>
#include "httplib.h"

#include "../timeanddate.hpp"
#include "../networking.hpp"

using namespace httplib;


int main() {
	Client client("localhost", 8080);
	
	std::string input;
	while (true) {
		std::getline(std::cin, input, '\n');
		if (input == "stop" || input == "quit" || input == "exit") break;
		if (input[0] == '/') {
			size_t first_space_position = input.find(' ');
			std::string endpoint, data;
			if (first_space_position != std::string::npos) {
				endpoint = input.substr(0, first_space_position);
				data = input.substr(first_space_position + 1);
			} else {
				endpoint = input;
				data = "";
			}
			
			printf("Sending %s to %s\n", data.c_str(), endpoint.c_str());
			Result result = client.Post(endpoint, data, "text/plain");
			
			if (result) {
				if (result->status == StatusCode::OK_200) {
					std::string message = result->body;
					if (message.length() == 0) {
						printf("Empty response from server.\n");
					} else if (message.length() < 3) {
						printf("%s\n", server_response_to_string(static_cast<ServerResponse>(message[0])).c_str());
					} else {
						printf("%s: %s\n", server_response_to_string(static_cast<ServerResponse>(message[0])).c_str(), message.substr(2).c_str());
					}
				}
			} else {
				Error error = result.error();
				printf("HTTP error: %s\n", to_string(error).c_str());
			}
		}
	}
}

