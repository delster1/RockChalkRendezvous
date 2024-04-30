#include <stdio.h>
#include <string>
#include <fstream>
#include <cstdio>
#include <vector>
#include <sstream>
#include <thread>
#include "httplib.h"

#include "../shared/core_utils.hpp"
#include "../shared/group.hpp"
#include "../shared/calendar.hpp"
#include "../shared/user.hpp"
#include "../shared/networking.hpp"

using namespace httplib;


#define CONFIG_FILE_NAME "config.txt"
#define GROUPS_FILE_NAME "groups.txt"
#define USER_FOLDER_PATH(a) ("users/" + a + ".txt")



#define return_code(a) return response.set_content(std::string(1, static_cast<char>(a)), "text/plain")


// account creation, group management, and calendar sync

struct LoginResult {
	Status status;
	ServerResponse code;
	User user;
};

LoginResult login(std::istream& message) {
	std::string username, password;
	
	if (read_quoted_string(message, username) == Failure) return { Failure, BadData };
	if (read_quoted_string(message, password) == Failure) return { Failure, BadData };
	
	// todo: username and password screening
	
    // getting the user file, failing to login if no account exists
	let user_file = std::ifstream(USER_FOLDER_PATH(username));
	if (!user_file.is_open()) return { Failure, IncorrectLogin };
	
	User user;
    // password format check
	if (user.decode(user_file) == Failure) {
		printf("Couldn't read password for '%s', user data file is improperly formatted.\n", username.c_str());
		return { Failure, IncorrectLogin, user };
	}
    // wrong password
	if (password != user.password) return { Failure, IncorrectLogin, user };
	
	return { Success, AccountOk, user };
}



ServerResponse create_account(std::istream& message) {
	std::string username, password;
	if (read_quoted_string(message, username) == Failure) return BadData;
	if (read_quoted_string(message, password) == Failure) return BadData;
	
    // username invalid
	if (!is_username_valid(username)) return UsernameUnavailable;
	
    // username taken
	let test_user_file = std::ifstream(USER_FOLDER_PATH(username));
	if (test_user_file.is_open()) {
		test_user_file.close();
		return UsernameUnavailable;
	}
	
	// todo: password screening
	// InvalidPassword unused otherwise
	
    // creates a new user file with username/password
	// (std::ofstream(USER_FOLDER_PATH(username)) << User(username, password).encode()).close();
    auto user_file = std::ofstream(USER_FOLDER_PATH(username));
    user_file << User(username, password).encode();
    user_file.close();
	
	return AccountOk;
}

inline void save_user_file(const User& user) {
    // saves entire user data to file
	// (std::ofstream(USER_FOLDER_PATH(user.username)) << user.encode()).close();
    auto user_file = std::ofstream(USER_FOLDER_PATH(user.username));
    user_file << user.encode();
    user_file.close();
}


// Does not save the user's file after removing the group entry
// Fails if the user already has no affiliation with the group
Status leave_group(std::unordered_map<GroupID, Group>& groups, User& user, const GroupID& id) {
	auto group_id_ptr = std::find(user.group_ids.begin(), user.group_ids.end(), id);
	if (group_id_ptr == user.group_ids.end()) return Failure;
	user.group_ids.erase(group_id_ptr);
	
	auto pair_ptr = groups.find(id);
	if (pair_ptr == groups.end()) {
		printf("User '%s' had reference to missing group %s\n", user.username.c_str(), encode_group_id(id).c_str());
		return Success;
	}
	
	auto member_ptr = std::find(pair_ptr->second.members.begin(), pair_ptr->second.members.end(), user.username);
	if (member_ptr == pair_ptr->second.members.end()) {
		printf("Group %s was missing reference to user '%s'\n", encode_group_id(id).c_str(), user.username.c_str());
		return Success;
	}
	pair_ptr->second.members.erase(member_ptr);
	
	if (pair_ptr->second.members.size() == 0) groups.erase(id);
	
	return Success;
}



// MARK: Main

int main() {
	Server server;
	let static groups = std::unordered_map<GroupID, Group>();
	
    // open groups file to parse
	let groups_file_in = std::ifstream(GROUPS_FILE_NAME);
	if (!groups_file_in.is_open()) {
		printf("Couldn't open groups.txt\n");
		return 1;
	}
	
    // parsing groups file
	while (groups_file_in.good()) {
		Group group;
		if (group.decode(groups_file_in) == Failure) break;
		if (group.members.size() == 0) {
			printf("Group %s had no members\n", encode_group_id(group.id).c_str());
			continue;
		}
		if (groups.try_emplace(group.id, group).second == false) {
			printf("Duplicate group id %s from '%s'\n", encode_group_id(group.id).c_str(), group.name.c_str());
		}
	}
	
	
	
	// MARK: Handlers
    // users send id alongside message to indicate what they want to do
	
    // Ping -> PingResponse
	server.Post(URL_PATTERNS[
		Ping
	], [&](const Request& request, Response& response) {
		return_code(PingResponse);
	});
	
    // CreateAccount -> create_account(msg)
	server.Post(URL_PATTERNS[
		CreateAccount
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		return_code(create_account(message));
	});
	
	// CheckUsername -> check username validity / availability
	server.Post(URL_PATTERNS[
		CheckUsername
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		std::string username;
		if (read_quoted_string(message, username) == Failure) return_code(BadData);
		
		if (!is_username_valid(username)) return_code(UsernameUnavailable);
		
		let test_user_file = std::ifstream(USER_FOLDER_PATH(username));
		if (test_user_file.is_open()) {
			test_user_file.close();
			return_code(UsernameUnavailable);
		} else return_code(UsernameAvailable);
	});
	
	// ValidateAccount -> login(msg)
	server.Post(URL_PATTERNS[
		ValidateAccount
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		return_code(r.code);
	});
	
	
    // DeleteAccount -> attempt to delete account
	server.Post(URL_PATTERNS[
		DeleteAccount
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) return_code(r.code);
		
		for (GroupID id : r.user.group_ids) leave_group(groups, r.user, id);
		std::remove(USER_FOLDER_PATH(r.user.username).c_str());
		return_code(AccountDeleted);
	});
	
    
    // GetUserCalendar -> get encoded user calendar
	server.Post(URL_PATTERNS[
		GetUserCalendar
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) return_code(r.code);
		
		response.set_content(std::string(1, static_cast<char>(UserCalendar)) + "\n" + r.user.calendar.encode(), "text/plain");
	});
	
	
    // SetUserCalendar -> set user cal after login(msg)
	server.Post(URL_PATTERNS[
		SetUserCalendar
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) return_code(r.code);
		
		Calendar calendar;
		if (calendar.decode(message) == Failure) return_code(BadData);
		
		r.user.calendar = calendar;
		save_user_file(r.user);
		
		return_code(UserCalendarWritten);
	});
	
	
    // GetGroups(user) -> get encoded groups
	server.Post(URL_PATTERNS[
		GetGroups
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) return_code(r.code);
		
		let user_group_data = std::vector<Group>();
		for (int i = r.user.group_ids.size() - 1; i >= 0; i--) {
			auto pair_ptr = groups.find(r.user.group_ids[i]);
			if (pair_ptr == groups.end()) {
				printf("User '%s' had reference to missing group '%s'\n", r.user.username.c_str(), encode_group_id(r.user.group_ids[i]).c_str());
				r.user.group_ids.erase(r.user.group_ids.begin() + i);
				save_user_file(r.user);
				continue;
			}
			user_group_data.push_back(pair_ptr->second);
		}
		
		response.set_content(std::string(1, static_cast<char>(Groups)) + "\n" + encode_vector<Group>(user_group_data, Group::encode_static, true), "text/plain");
	});
	
	
    // GetGroupCalendars(user, group_id)
	server.Post(URL_PATTERNS[
		GetGroupCalendars
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) return_code(r.code);
		
		// read group id
		GroupID id;
		if (decode_group_id(message, id) == Failure) return_code(BadData);
		
		// check that requesting user is in the group
		auto group_id_ptr = std::find(r.user.group_ids.begin(), r.user.group_ids.end(), id);
		if (group_id_ptr == r.user.group_ids.end()) return_code(InvalidGroup);
		
		// look up group
		auto pair_ptr = groups.find(id);
		if (pair_ptr == groups.end()) {
			r.user.group_ids.erase(group_id_ptr);
			save_user_file(r.user);
			printf("User '%s' had reference to missing group '%s'", r.user.username.c_str(), encode_group_id(id).c_str());
			return_code(InvalidGroup);
		}
		
		// format and send
		Group group = pair_ptr->second;
		std::string calendar_data = encode_vector<std::string>(group.members, [&](const std::string& username) -> std::string {
			let user_file = std::ifstream(USER_FOLDER_PATH(username));
			if (!user_file.is_open()) {
				printf("Reference to missing user '%s' in group '%s'\n", username.c_str(), group.name.c_str());
				return "";
			}
			User user;
			if (user.decode(user_file) == Failure) {
				printf("Couldn't read calendar of user '%s', user data file is improperly formatted.\n", username.c_str());
				user.calendar = Calendar();
			}
			
			return "\n" + quote_string(username) + " " + user.calendar.encode();
		}, false);
		
		response.set_content(std::string(1, static_cast<char>(GroupCalendars)) + "\n" + calendar_data, "text/plain");
	});
	
	
    // CreateGroup(user, group_name)
	server.Post(URL_PATTERNS[
		CreateGroup
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) return_code(r.code);
		
		std::string name;
		if (read_quoted_string(message, name) == Failure) return_code(BadData);
		
		// todo: filter group names
		// InvalidGroupName unused otherwise
		
		time_t now;
		GroupID id;
		std::hash<time_t> hash_function;
		while (true) {
			time(&now);
			id = hash_function(now);
			if (groups.try_emplace(id, Group(id, name, std::vector<std::string>({ r.user.username }))).second == Success) break;
		}
		
		r.user.group_ids.push_back(id);
		save_user_file(r.user);
		
		return_code(GroupCreated);
	});
	
	
    // JoinGroup(user, group_id)
	server.Post(URL_PATTERNS[
		JoinGroup
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) return_code(r.code);
		
		// read group id
		GroupID id;
		if (decode_group_id(message, id) == Failure) return_code(BadData);
		
		// check that requesting user is not already in the group
		if (std::find(r.user.group_ids.begin(), r.user.group_ids.end(), id) != r.user.group_ids.end()) return_code(InvalidGroup);
		
		// look up group
		auto pair_ptr = groups.find(id);
		if (pair_ptr == groups.end()) return_code(InvalidGroup);
		
        // look up user in group
		auto member_ptr = std::find(pair_ptr->second.members.begin(), pair_ptr->second.members.end(), r.user.username);
		if (member_ptr == pair_ptr->second.members.end()) {
			pair_ptr->second.members.push_back(r.user.username);
		} else {
			printf("User '%s' was missing group reference '%s'\n", r.user.username.c_str(), pair_ptr->second.name.c_str());
		}
		
		r.user.group_ids.push_back(id);
		save_user_file(r.user);
		
		return_code(GroupJoined);
	});
	
	
    // RenameGroup(user, group_id, new_name)
	server.Post(URL_PATTERNS[
		RenameGroup
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) return_code(r.code);
		
		GroupID id;
		if (decode_group_id(message, id) == Failure) return_code(BadData);
		
		std::string name;
		if (read_quoted_string(message, name) == Failure) return_code(BadData);
		
		// todo: filter group names
		// InvalidGroupName unused otherwise
		
		// check that requesting user is in the group
		auto group_id_ptr = std::find(r.user.group_ids.begin(), r.user.group_ids.end(), id);
		if (group_id_ptr == r.user.group_ids.end()) return_code(InvalidGroup);
		
		// look up group
		auto pair_ptr = groups.find(id);
		if (pair_ptr == groups.end()) {
			r.user.group_ids.erase(group_id_ptr);
			save_user_file(r.user);
			printf("User '%s' had reference to missing group '%s'\n", r.user.username.c_str(), encode_group_id(id).c_str());
			return_code(InvalidGroup);
		}
		
		pair_ptr->second.name = name;
		
		return_code(GroupRenamed);
	});
	
	
    // LeaveGroup(user, group_id)
	server.Post(URL_PATTERNS[
		LeaveGroup
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) return_code(r.code);
		
		GroupID id;
		if (decode_group_id(message, id) == Failure) return_code(BadData);
		
		if (leave_group(groups, r.user, id) == Failure) return_code(InvalidGroup);
		
		save_user_file(r.user);
		return_code(GroupLeft);
	});
	
	
	// MARK: Start server
	
    // init variables with optional config file
	int port;
	let config_file = std::ifstream(CONFIG_FILE_NAME);
	if (config_file.is_open()) {
		config_file >> port;
		if (config_file.fail()) {
			printf("Couldn't read port from config file, resetting it to default.\n");
			port = DEFAULT_SERVER_PORT;
			// (std::ofstream(CONFIG_FILE_NAME) << port).close();
            auto config_file = std::ofstream(CONFIG_FILE_NAME);
            config_file << port;
            config_file.close();
		}
	} else {
		printf("Config file not found, creating a new one.\n");
		port = DEFAULT_SERVER_PORT;
		// (std::ofstream(CONFIG_FILE_NAME) << port).close();
        auto config_file = std::ofstream(CONFIG_FILE_NAME);
        config_file << port;
        config_file.close();
	}
	
	
    // create server thread
	let http_thread = std::thread([&]() {
		printf("Starting server\n");
		server.listen("0.0.0.0", port);
		printf("Exiting server\n");
	});
	
    // wait 1s for server to start
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	
	std::string input;
    // core server loop
	while (true) {
		std::cin >> input;
		if (input == "stop" || input == "quit" || input == "exit") break;
		else if (input == "groups") {
			for (std::pair<const GroupID, Group> pair : groups) {
				printf("%s\n", pair.second.encode().c_str());
			}
		} else if (input == "cool") {
			printf("👍\n");
		}
	}
	
	// save groups to file
	    
	let groups_file_out = std::ofstream(GROUPS_FILE_NAME);
	for (const std::pair<const GroupID, Group>& pair : groups) {
		groups_file_out << pair.second.encode() << "\n";
	}
	groups_file_out.close();
	
	server.stop();
	http_thread.join();
	printf("Server exited successfully");
	
}

