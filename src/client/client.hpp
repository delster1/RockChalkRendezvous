#ifndef RCR_CLIENT_FILE_CLIENT
#define RCR_CLIENT_FILE_CLIENT
#include <fstream>
#include <iostream>
#include <string>
#include "httplib.h"
#include <iostream>
#include "../shared/networking.hpp"
#include "../shared/core_utils.hpp"
#include "../shared/group.hpp"
#define CONFIG_FILE_NAME "config.txt"
#define DEFAULT_SERVER_HOSTNAME "localhost"
#define DEFAULT_SERVER_PORT 7777
// cached info in static variables - at top of file, can be referenced outside 

static std::string hostname;
static int port;
static httplib::Client* my_client = nullptr;
static std::string username_string;
static std::string password_string; // need to figure out how to initialize these so i don't have to add them everytime i all a fn

Status send_ping_request() {
    // prep body of the POST request
    std::string body = "ping\n";

    auto res = my_client->Post("/ping", body, "text/plain");

    // Check the response
    if (res && res->status == 200) { // Assuming 200 is the HTTP OK status
        // std::cout << "Status: " << res->status << std::endl;
        // std::cout << "Body: " << res->body << std::endl;
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
    }
    return Failure;
}

Status send_login_request() {
    // Quote the username_string and password_string to ensure they are transmitted in a format the server expects.

    // Prepare the body of the POST request
    std::string body = quote_string(username_string) + "\n" + quote_string(password_string);

    // Make a POST request
    auto res = my_client->Post("/validate_account", body, "text/plain");

    // Check the response
    if (res && res->status == 200) { // Assuming 200 is the HTTP OK status
        // std::cout << "Status: " << res->status << std::endl;
        // std::cout << "Body: " << res->body << std::endl;
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
    }
    return Failure;

}
Status send_create_account_request() {

    // Prepare the body of the POST request
    std::string body = quote_string(username_string) + "\n" + quote_string(password_string);

    
    // Make a POST request
    auto res = my_client->Post("/create_account", body, "text/plain");
	if (res && res->status == 200) { // Assuming 200 is the HTTP OK status
        // std::cout << "Status: " << res->status << std::endl;
        // std::cout << "Body: " << res->body << std::endl;
		
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
    }
    return Failure;

}

Status send_check_username_request() {

    // Prepare the body of the POST request
    std::string body = quote_string(username_string);

    
    // Make a POST request
    auto res = my_client->Post("/check_username", body, "text/plain");
	if (res && res->status == 200) { // Assuming 200 is the HTTP OK status
        // std::cout << "Status: " << res->status << std::endl;
        // std::cout << "Body: " << res->body << std::endl;
		
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
    }
    return Failure;

}
Status send_delete_account_request() {

    // Prepare the body of the POST request
    std::string body = quote_string(username_string) + "\n" + quote_string(password_string);

    
    // Make a POST request
    auto res = my_client->Post("/delete_account", body, "text/plain");
	if (res && res->status == 200) { // Assuming 200 is the HTTP OK status
        // std::cout << "Status: " << res->status << std::endl;
        // std::cout << "Body: " << res->body << std::endl;
		
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
    }
    return Failure;

}

std::string send_get_user_calendar_request() {
    // Prepare the body of the POST request
    std::string body = quote_string(username_string) + "\n" + quote_string(password_string);

    // Make a POST request
    auto res = my_client->Post("/get_user_calendar", body, "text/plain");

    // Check the response
    if (res && res->status == 200) { // Assuming 200 is the HTTP OK status
        std::istringstream response_stream(res->body);
        char response_code;
        response_stream >> response_code;
        if (response_stream.fail() || response_code != 'R') {
            return "Failure";
        } else {
            return res->body.substr(1);
        }
    } else {
        if (res) {
            std::cout << "HTTP Error: " << res->status << std::endl;
        } else {
            std::cout << "Network Error: " << res.error() << std::endl;
        }
    }

    return "Failure";
}

Status send_set_user_calendar_request(const std::string encoded_calendar) {
    // Quote the username_string and password_string to ensure they are transmitted in a format the server expects.
    // Prepare the body of the POST request
    std::string body = quote_string(username_string) + "\n" + quote_string(password_string) + "\n" + encoded_calendar;

    // Make a POST request
    auto res = my_client->Post("/set_user_calendar", body, "text/plain");

    // Check the response
    if (res && res->status == 200) { // Assuming 200 is the HTTP OK status
        // std::cout << "Status: " << res->status << std::endl;
        // std::cout << "Body: " << res->body << std::endl;
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
    }
    return Failure;

}

std::string send_get_groups_request() {
    std::string body = quote_string(username_string) + "\n" + quote_string(password_string);
    auto res = my_client->Post("/get_groups", body, "text/plain");

    // Check the response
    if (res && res->status == 200) { // Assuming 200 is the HTTP OK status
        std::istringstream response_stream(res->body);
        char response_code;
        response_stream >> response_code;
        if (response_stream.fail() || response_code != 'G') {
            return "Failure";
        } else {
            return res->body.substr(1);
        }
    } else {
        if (res) {
            std::cout << "HTTP Error: " << res->status << std::endl;
        } else {
            std::cout << "Network Error: " << res.error() << std::endl;
        }
    }

    return "Failure";
}

std::string send_get_group_calendars_request(const usize group_id) { 
    std::string group_id_string = encode_group_id(group_id);
    std::string body = quote_string(group_id_string);
    auto res = my_client->Post("/get_group_calendars", body, "text/plain");

    // Check the response
    if (res && res->status == 200) { // Assuming 200 is the HTTP OK status
        std::istringstream response_stream(res->body);
        char response_code;
        response_stream >> response_code;
        if (response_stream.fail() || response_code != 'M') {
            return "Failure";
        } else {
            return res->body.substr(1);
        }
    } else {
        if (res) {
            std::cout << "HTTP Error: " << res->status << std::endl;
        } else {
            std::cout << "Network Error: " << res.error() << std::endl;
        }
    }

    return "Failure";
}

Status send_create_group_request(const std::string& group_name) { 
    std::string body = quote_string(username_string) + "\n" + quote_string(password_string) + "\n" + quote_string(group_name);
    auto res = my_client->Post("/create_group", body, "text/plain");

    // Check the response
    if (res && res->status == 200) { // Assuming 200 is the HTTP OK status
        let response_stream = std::istringstream(res -> body);
        char response_code;
        response_stream >> response_code;
        if (response_stream.fail()) return Failure;
		if (response_code == GroupCreated) {
			return Success;
		}
    } else {
        if (res) {
            std::cout << "HTTP Error: " << res->status << std::endl;
        } else {
            std::cout << "Network Error: " << res.error() << std::endl;
        }
    }

    return Failure;
}

Status send_join_group_request(const usize group_id) { 
    std::string group_id_string = encode_group_id(group_id);
    std::string body = quote_string(username_string) + "\n" + quote_string(password_string) + "\n" + quote_string(group_id_string);
    auto res = my_client->Post("/join_group", body, "text/plain");

    // Check the response
    if (res && res->status == 200) { // Assuming 200 is the HTTP OK status
        let response_stream = std::istringstream(res -> body);
        char response_code;
        response_stream >> response_code;
        if (response_stream.fail()) return Failure;
		if (response_code == GroupJoined) {
			return Success;
		}
    } else {
        if (res) {
            std::cout << "HTTP Error: " << res->status << std::endl;
        } else {
            std::cout << "Network Error: " << res.error() << std::endl;
        }
    }

    return Failure;
}

Status send_rename_group_request(const usize group_id, const std::string& group_name) { 
    std::string group_id_string = encode_group_id(group_id);
    std::string body = quote_string(username_string) + "\n" + quote_string(password_string) + "\n" + quote_string(group_id_string);
    body += "\n" + quote_string(group_name);
    auto res = my_client->Post("/rename_group", body, "text/plain");

    // Check the response
    if (res && res->status == 200) { // Assuming 200 is the HTTP OK status
        let response_stream = std::istringstream(res -> body);
        char response_code;
        response_stream >> response_code;
        if (response_stream.fail()) return Failure;
		if (response_code == GroupRenamed) {
			return Success;
		}
    } else {
        if (res) {
            std::cout << "HTTP Error: " << res->status << std::endl;
        } else {
            std::cout << "Network Error: " << res.error() << std::endl;
        }
    }

    return Failure;
}

Status send_leave_group_request(const usize group_id) { 
    std::string group_id_string = encode_group_id(group_id);
    std::string body = quote_string(username_string) + "\n" + quote_string(password_string) + "\n" + quote_string(group_id_string);
    auto res = my_client->Post("/leave_group", body, "text/plain");

    // Check the response
    if (res && res->status == 200) { // Assuming 200 is the HTTP OK status
        let response_stream = std::istringstream(res -> body);
        char response_code;
        response_stream >> response_code;
        if (response_stream.fail()) return Failure;
		if (response_code == GroupLeft) {
			return Success;
		}
    } else {
        if (res) {
            std::cout << "HTTP Error: " << res->status << std::endl;
        } else {
            std::cout << "Network Error: " << res.error() << std::endl;
        }
    }

    return Failure;
}


	// For decoding groups to vector, use decoxde_vector<Group>

// Status send_set_calendar_request(httplib::Client* client,std::string std::string decoded_user){
//     std::string body = quote_string(username_string) + "\n" + quote_string(password_string) + quote_string(decoded_user);

//     auto res = client->Post("/set_user_calendar", body, "text/plain");
//     if (res && res->status == 200) { // Assuming 200 is the HTTP OK status
//         std::cout << "Status: " << res->status << std::endl;
//         std::cout << "Body: " << res->body << std::endl;
		
// 		let response_stream = std::istringstream(res-> body);
// 		char response_code;
// 		response_stream >> response_code;
// 		if (response_stream.fail()) return Failure;
// 		if (response_code == UserCalendarWritten) {
//             return Success; 
// 		}
//     } else {
//         if (res) {
//             std::cout << "HTTP Error: " << res->status << std::endl;
//         } else {
//             std::cout << "Network Error: " << res.error() << std::endl;
//         }
//         return Failure;
//     }
    
//     return Failure;
// }

httplib::Client* build_client() {
    std::ifstream config_file(CONFIG_FILE_NAME);
    if (config_file.is_open()) {
        std::getline(config_file, hostname);
        config_file >> port;
        if (config_file.fail()) {
            std::cerr << "Error reading port from config file, resetting to default.\n";
            port = DEFAULT_SERVER_PORT;
        }
        config_file.close();
    } else {
        std::cerr << "Config file not found, creating a new one with default settings.\n";
        hostname = DEFAULT_SERVER_HOSTNAME;
        port = DEFAULT_SERVER_PORT;
    }

    std::ofstream output_config(CONFIG_FILE_NAME);
    if (output_config.is_open()) {
        output_config << hostname << "\n" << port;
        output_config.close();
    } else {
        std::cerr << "Failed to open config file for writing.\n";
    }

    // std::cout << "Configured to connect to " << hostname << " on port " << port << std::endl;

    if (my_client != nullptr) delete my_client;
    my_client = new httplib::Client(hostname.c_str(), port);
    return my_client;
}
// int main() {
//     std::string hostname;
//     int port;

//     // Open the config file for reading
    
    
//     send_create_account_request(std::move(my_client), "d3", "password");
//     return 0;


// }
#endif
