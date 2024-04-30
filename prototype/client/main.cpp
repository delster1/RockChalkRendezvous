#include <fstream>
#include <iostream>
#include <string>
#include "httplib.h"
#include "../networking.hpp"
#include "../core_utils.hpp"

#define CONFIG_FILE_NAME "config.txt"
#define DEFAULT_SERVER_HOSTNAME "localhost"
#define DEFAULT_SERVER_PORT 8080


Status send_login_request(httplib::Client client, const std::string& username, const std::string& password) {
    // Quote the username and password to ensure they are transmitted in a format the server expects.

    // Prepare the body of the POST request
    std::string body = quote_string(username) + "\n" + quote_string(password);

    // Make a POST request
    auto res = client.Post("/validate_account", body, "text/plain");

    // Check the response
    if (res && res->status == 200) { // Assuming 200 is the HTTP OK status
        std::cout << "Status: " << res->status << std::endl;
        std::cout << "Body: " << res->body << std::endl;
        return Success; // Or convert server response to appropriate status
        let response_stream = std::istringstream(res -> body);
        char response_code;
        response_stream >> response_code;
        if (response_stream.fail()) return Failure;
		if (response_code == AccountOk) {
			return Success;
		}
    } else {
        if (res) {
            std::cout << "HTTP Error: " << res->status << std::endl;
        } else {
            std::cout << "Network Error: " << res.error() << std::endl;
        }
        return Failure;
    }
}
// cached info in static variables - at top of file, can be referenced outside 
Status send_create_account_request(httplib::Client client, const std::string& username, const std::string& password) {


    // Prepare the body of the POST request
    std::string body = quote_string(username) + "\n" + quote_string(password);

    
    // Make a POST request
    auto res = client.Post("/create_account", body, "text/plain");
	if (res && res->status == 200) { // Assuming 200 is the HTTP OK status
        std::cout << "Status: " << res->status << std::endl;
        std::cout << "Body: " << res->body << std::endl;
		
		let response_stream = std::istringstream(res-> body);
		char response_code;
		response_stream >> response_code;
		if (response_stream.fail()) return Failure;
		if (response_code == AccountOk) {
            return Success; 
		}
    } else {
        if (res) {
            std::cout << "HTTP Error: " << res->status << std::endl;
        } else {
            std::cout << "Network Error: " << res.error() << std::endl;
        }
        return Failure;
    }
}

int main() {
    std::string hostname;
    int port;

    // Open the config file for reading
    std::ifstream config_file(CONFIG_FILE_NAME);
    if (config_file.is_open()) {
        std::getline(config_file, hostname, '\n');  // Assumes the hostname is the first line
        config_file >> port;
        if (config_file.fail()) {
            std::cout << "Couldn't read port from config file, resetting it to default.\n";
            port = DEFAULT_SERVER_PORT;
        }
        config_file.close();
    } else {
        std::cout << "Config file not found, creating a new one.\n";
        hostname = DEFAULT_SERVER_HOSTNAME;
        port = DEFAULT_SERVER_PORT;
    }

    // Always rewrite the config to ensure it's up to date or create if it does not exist
    std::ofstream output_config(CONFIG_FILE_NAME);
    if (output_config.is_open()) {
        output_config << hostname << "\n" << port;
        output_config.close(); // Properly close the file
    } else {
        std::cerr << "Failed to open config file for writing.\n";
    }

    // Continue with your application logic
    std::cout << "Configured to connect to " << hostname << " on port " << port << std::endl;
    
    httplib::Client my_client(hostname.c_str(), port); // initialize httplib client for sending and making requests
    send_create_account_request(std::move(my_client), "d3", "password");
    return 0;


}
