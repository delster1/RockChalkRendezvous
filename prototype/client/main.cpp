#include <stdio.h>
#include <string>
#include "httplib.h"

#include "../common_utils.hpp"
#include "../timeanddate.hpp"
#include "../data_codecs.hpp"
#include "../networking.hpp"

using namespace httplib;


#define CONFIG_FILE_NAME "config.txt"



int main() {
	
	std::string hostname;
	int port;
	let config_file = std::ifstream(CONFIG_FILE_NAME);
	if (config_file.is_open()) {
		if (read_quoted_string(config_file, hostname) == Failure) {
			printf("Couldn't read hostname from config file, resetting it to default.\n");
			hostname = DEFAULT_SERVER_HOSTNAME;
			port = DEFAULT_SERVER_PORT;
			(std::ofstream(CONFIG_FILE_NAME) << quote_string(hostname) << "\n" << port).close();
		} else {
			config_file >> port;
			if (config_file.fail()) {
				printf("Couldn't read port from config file, resetting it to default.\n");
				port = DEFAULT_SERVER_PORT;
				(std::ofstream(CONFIG_FILE_NAME) << quote_string(hostname) << "\n" << port).close();
			}
		}
	} else {
		printf("Config file not found, creating a new one.\n");
		hostname = DEFAULT_SERVER_HOSTNAME;
		port = DEFAULT_SERVER_PORT;
		(std::ofstream(CONFIG_FILE_NAME) << quote_string(hostname) << "\n" << port).close();
	}
	
	
	Client client(hostname, port);
	
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

