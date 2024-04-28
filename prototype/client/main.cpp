#include <fstream>
#include <iostream>
#include <string>
#include "httplib.h"
#include "../core_utils.hpp"

#define CONFIG_FILE_NAME "config.txt"
#define DEFAULT_SERVER_HOSTNAME "localhost"
#define DEFAULT_SERVER_PORT 8080

std::string quote_string(const std::string& input) {
    return "\"" + input + "\"";
}

Status send_login_request(const std::string& hostname, int port, const std::string& username, const std::string& password) {
    // Quote the username and password to ensure they are transmitted in a format the server expects.
    std::string quoted_username = quote_string(username);
    std::string quoted_password = quote_string(password);

    // Prepare the body of the POST request
    std::string body = quoted_username + "\n" + quoted_password;

    httplib::Client cli(hostname.c_str(), port);
    // Make a POST request
    auto res = cli.Post("/validate_account", body, "text/plain");

    // Check the response
    if (res && res->status == 'K') { // Assuming 200 is the HTTP OK status
        std::cout << "Status: " << res->status << std::endl;
        std::cout << "Body: " << res->body << std::endl;
        return Success; // Or convert server response to appropriate status
    } else {
        if (res) {
            std::cout << "HTTP Error: " << res->status << std::endl;
        } else {
            std::cout << "Network Error: " << res.error() << std::endl;
        }
        return Failure;
    }
}

Status send_create_account_request(const std::string& hostname, int port, const std::string& username, const std::string& password) {
	std::string quoted_username = quote_string(username);
    std::string quoted_password = quote_string(password);

    // Prepare the body of the POST request
    std::string body = quoted_username + "\n" + quoted_password;

    httplib::Client cli(hostname.c_str(), port);
    // Make a POST request
    auto res = cli.Post("/create_account", body, "text/plain");
	if (res && res->status == 'K') { // Assuming 200 is the HTTP OK status
        std::cout << "Status: " << res->status << std::endl;
        std::cout << "Body: " << res->body << std::endl;
        return Success; // Or convert server response to appropriate status
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
        output_config << quote_string(hostname) << "\n" << port;
        output_config.close(); // Properly close the file
    } else {
        std::cerr << "Failed to open config file for writing.\n";
    }

    // Continue with your application logic
    std::cout << "Configured to connect to " << hostname << " on port " << port << std::endl;
    return 0;
}
