#include <stdio.h>
#include <string>
#include <fstream>
#include <cstdio>
#include <vector>
#include <thread>
#include "httplib.h"

#include "../timeanddate.hpp"
//#include "../calendar.hpp"
#include "../networking.hpp"
#include "../data_codecs.hpp"

using namespace httplib;


#define SERVER_PORT 8080
#define GROUPS_FILE_NAME "groups.txt"
#define USER_FOLDER_PATH(a) ("users/" + a + ".txt")



#define return_code(a) return response.set_content(std::string(1, static_cast<char>(a)), "text/plain")




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
	
	let user_file = std::ifstream(USER_FOLDER_PATH(username));
	if (!user_file.is_open()) return { Failure, IncorrectLogin };
	
	User user;
	if (User::decode(user_file, user) == Failure) {
		printf("Couldn't look up password for '%s', user data is improperly formatted.", username.c_str());
		return { Failure, IncorrectLogin, user };
	}
	if (password != user.password) return { Failure, IncorrectLogin, user };
	
	return { Success, AccountOk, user };
}



ServerResponse create_account(std::istream& message) {
	std::string username, password;
	if (read_quoted_string(message, username) == Failure) return BadData;
	if (read_quoted_string(message, password) == Failure) return BadData;
	
	let test_user_file = std::ifstream(USER_FOLDER_PATH(username));
	if (test_user_file.is_open()) {
		test_user_file.close();
		return UsernameUnavailable;
	}
	
	// todo: username and password screening
	// InvalidPassword unused otherwise
	
	(std::ofstream(USER_FOLDER_PATH(username)) << User(username, password).encode()).close();
	
	return AccountOk;
}

inline void save_user_file(const User& user) {
	(std::ofstream(USER_FOLDER_PATH(user.username)) << user.encode()).close();
}





int main() {
	Server server;
	let static groups = std::unordered_map<GroupID, Group>();
	
	let groups_file_in = std::ifstream(GROUPS_FILE_NAME);
	if (!groups_file_in.is_open()) {
		printf("Couldn't open groups.txt\n");
		return 1;
	}
	
	while (groups_file_in.good()) {
		Group group;
		if (Group::decode(groups_file_in, group) == Failure) break;
		if (groups.try_emplace(group.id, group).second == false) {
			printf("Duplicate group id from '%s'\n", group.name.c_str());
		}
	}
	
	
	
	
	server.Post(URL_PATTERNS[
		Ping
	], [&](const Request& request, Response& response) {
		return_code(PingResponse);
	});
	
	server.Post(URL_PATTERNS[
		CreateAccount
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		return_code(create_account(message));
	});
	
	
	server.Post(URL_PATTERNS[
		CheckUsername
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		std::string username;
		if (read_quoted_string(message, username) == Failure) return_code(BadData);
		
		// todo: username screening
		
		let test_user_file = std::ifstream(USER_FOLDER_PATH(username));
		if (test_user_file.is_open()) {
			test_user_file.close();
			return_code(UsernameUnavailable);
		} else return_code(UsernameAvailable);
	});
	
	
	server.Post(URL_PATTERNS[
		ValidateAccount
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		return_code(r.code);
	});
	
	
	server.Post(URL_PATTERNS[
		DeleteAccount
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) return_code(r.code);
		
		for (GroupID id : r.user.group_ids) {
			auto pair_ptr = groups.find(id);
			if (pair_ptr == groups.end()) {
				printf("User '%s' had reference to missing group '%s'\n", r.user.username.c_str(), encode_group_id(id).c_str());
				continue;
			}
			std::vector<std::string> members = pair_ptr->second.members;
			members.erase(std::find(members.begin(), members.end(), r.user.username));
		}
		
		std::remove(USER_FOLDER_PATH(r.user.username).c_str());
		return_code(AccountDeleted);
	});
	
	
	server.Post(URL_PATTERNS[
		GetUserCalendar
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) return_code(r.code);
		
		response.set_content(std::string(1, static_cast<char>(UserCalendar)) + "\n" + r.user.calendar.encode(), "text/plain");
	});
	
	
	server.Post(URL_PATTERNS[
		SetUserCalendar
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) return_code(r.code);
		
		Calendar calendar;
		if (Calendar::decode(message, calendar) == Failure) return_code(BadData);
		
		r.user.calendar = calendar;
		save_user_file(r.user);
		
		return_code(UserCalendarWritten);
	});
	
	
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
		std::string calendar_data = encode_vector<std::string>(group.members, [&](std::string username) -> std::string {
			let user_file = std::ifstream(USER_FOLDER_PATH(username));
			if (!user_file.is_open()) {
				printf("Reference to missing user '%s' in group '%s'\n", username.c_str(), group.name.c_str());
				return "";
			}
			User user;
			if (User::decode(user_file, user) == Failure) {
				printf("Couldn't read calendar of user '%s', data file is improperly formatted.\n", username.c_str());
				return "\n" + quote_string(username) + " " + Calendar().encode();
			}
			
			return "\n" + quote_string(username) + " " + user.calendar.encode();
		}, true);
		
		response.set_content(std::string(1, static_cast<char>(GroupCalendars)) + "\n" + calendar_data, "text/plain");
	});
	
	
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
	
	
	server.Post(URL_PATTERNS[
		RenameGroup
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) return_code(r.code);
		
		// read group id
		GroupID id;
		if (decode_group_id(message, id) == Failure) return_code(BadData);
		
		// read name
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
	
	
	server.Post(URL_PATTERNS[
		LeaveGroup
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) return_code(r.code);
		
		// read group id
		GroupID id;
		if (decode_group_id(message, id) == Failure) return_code(BadData);
		
		auto group_id_ptr = std::find(r.user.group_ids.begin(), r.user.group_ids.end(), id);
		if (group_id_ptr == r.user.group_ids.end()) return_code(InvalidGroup);
		r.user.group_ids.erase(group_id_ptr);
		
		save_user_file(r.user);
		
		// look up group
		auto pair_ptr = groups.find(id);
		if (pair_ptr == groups.end()) {
			printf("User '%s' had reference to missing group '%s'\n", r.user.username.c_str(), encode_group_id(id).c_str());
			return_code(GroupLeft);
		}
		
		auto member_ptr = std::find(pair_ptr->second.members.begin(), pair_ptr->second.members.end(), r.user.username);
		if (member_ptr == pair_ptr->second.members.end()) {
			printf("User '%s' was missing from group '%s' member list\n", r.user.username.c_str(), encode_group_id(id).c_str());
			return_code(GroupLeft);
		}
		pair_ptr->second.members.erase(member_ptr);
		
		return_code(GroupLeft);
	});
	
	
	
	let http_thread = std::thread([&]() {
		printf("Starting server\n");
		server.listen("0.0.0.0", SERVER_PORT);
		printf("Exiting server\n");
	});
	
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	
	std::string input;
	while (true) {
		std::cin >> input;
		if (input == "stop" || input == "quit" || input == "exit") break;
		else if (input == "groups") {
			for (std::pair<const GroupID, Group> pair : groups) {
				printf("%s\n", pair.second.encode().c_str());
			}
		}
	}
	
	
	let groups_file_out = std::ofstream(GROUPS_FILE_NAME);
	for (const std::pair<const GroupID, Group>& pair : groups) {
		groups_file_out << pair.second.encode() << "\n";
	}
	groups_file_out.close();
	
	server.stop();
	http_thread.join();
	printf("Server exited successfully");
	
}

