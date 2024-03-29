#include <stdio.h>
#include <string>
#include <fstream>
#include <cstdio>
#include <vector>
#include "httplib.h"

#include "../timeanddate.hpp"
#include "../calendar.hpp"
#include "../networking.hpp"
#include "../data_codecs.hpp"

using namespace httplib;


#define return_code(a) return response.set_content(std::string(a, 1), "text/plain")



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
	
	let user_file = std::ifstream("users/" + username + ".txt");
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
	
	let test_user_file = std::ifstream("users/" + username + ".txt");
	if (test_user_file.is_open()) {
		test_user_file.close();
		return UsernameUnavailable;
	}
	
	// todo: username and password screening
	// InvalidPassword unused otherwise
	
	(std::ofstream("users/" + username + ".txt") << User(username, password).encode()).close();
	
	return AccountOk;
}




int main() {
	Server server;
	
	let now = TimeAndDate::now();
	printf("Server started.\nCurrent time: %s\n", now.to_string().c_str());
	
	
	let groups = std::unordered_map<GroupId, Group>();
	
	let groups_file = std::ifstream("groups.txt");
	if (!groups_file.is_open()) {
		printf("Couldn't open groups.txt\n");
		return 1;
	}
	
	while (groups_file.good()) {
		Group group;
		if (Group::decode(groups_file, group) == Failure) break;
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
		if (read_quoted_string(message, username) == Failure) return BadData;
		
		// todo: username screening
		
		let test_user_file = std::ifstream("users/" + username + ".txt");
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
		
		for (GroupId id : r.user.groups) {
			auto pair_ptr = groups.find(id);
			if (pair_ptr == groups.end()) {
				printf("User '%s' has reference to missing group '%s'\n", r.user.username.c_str(), encode_group_id(id).c_str());
				continue;
			}
			std::vector<std::string> members = pair_ptr->second.members;
			members.erase(std::find(members.begin(), members.end(), r.user.username));
		}
		
		std::remove(("users/" + r.user.username + ".txt").c_str());
		return_code(AccountDeleted);
	});
	
	
	server.Post(URL_PATTERNS[
		GetUserCalendar
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) return_code(r.code);
		
		response.set_content(UserCalendar + "\n" + r.user.calendar.encode(), "text/plain");
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
		(std::ofstream("users/" + r.user.username + ".txt") << r.user.encode()).close();
		
		return_code(UserCalendarWritten);
	});
	
	
	server.Post(URL_PATTERNS[
		GetGroups
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) return_code(r.code);
		
		let user_group_data = std::vector<Group>();
		for (GroupId id : r.user.groups) {
			auto pair_ptr = groups.find(id);
			if (pair_ptr == groups.end()) {
				printf("User '%s' has missing group reference '%s'\n", r.user.username.c_str(), encode_group_id(id).c_str());
				continue;
			}
			user_group_data.push_back(pair_ptr->second);
		}
		
		response.set_content(Groups + "\n" + encode_vector<Group>(user_group_data, Group::encode_static), "text/plain");
	});
	
	
	server.Post(URL_PATTERNS[
		GetGroupCalendars
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) return_code(r.code);
		
		// read group id
		GroupId id;
		if (decode_group_id(message, id) == Failure) return_code(BadData);
		
		// check that requesting user is in the group
		if (std::find(r.user.groups.begin(), r.user.groups.end(), id) == r.user.groups.end()) return_code(InvalidGroup);
		
		// look up group
		auto pair_ptr = groups.find(id);
		if (pair_ptr == groups.end()) {
			printf("User '%s' has reference to missing group '%s'", r.user.username.c_str(), encode_group_id(id).c_str());
			return_code(InvalidGroup);
		}
		
		// format and send
		Group group = pair_ptr->second;
		std::string calendar_data = encode_vector<std::string>(group.members, [&](std::string username) -> std::string {
			let user_file = std::ifstream("users/" + username + ".txt");
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
		});
		
		response.set_content(GroupCalendars + "\n" + calendar_data, "text/plain");
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
		GroupId id;
		std::hash<time_t> hash_function;
		while (true) {
			time(&now);
			id = hash_function(now);
			if (groups.try_emplace(id, Group(id, name, std::vector<std::string>({ r.user.username }))).second == Success) break;
		}
		
		r.user.groups.push_back(id);
		(std::ofstream("users/" + r.user.username + ".txt") << r.user.encode()).close();
		
		return_code(GroupCreated);
	});
	
	
	server.Post(URL_PATTERNS[
		JoinGroup
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) return_code(r.code);
		
		// read group id
		GroupId id;
		if (decode_group_id(message, id) == Failure) return_code(BadData);
		
		// check that requesting user is not already in the group
		if (std::find(r.user.groups.begin(), r.user.groups.end(), id) != r.user.groups.end()) return_code(InvalidGroup);
		
		// look up group
		auto pair_ptr = groups.find(id);
		if (pair_ptr == groups.end()) return_code(InvalidGroup);
		
		auto member_ptr = std::find(pair_ptr->second.members.begin(), pair_ptr->second.members.end(), r.user.username);
		if (member_ptr == pair_ptr->second.members.end()) {
			pair_ptr->second.members.push_back(r.user.username);
		} else {
			printf("User '%s' was missing group reference '%s'\n", r.user.username.c_str(), pair_ptr->second.name.c_str());
		}
		
		r.user.groups.push_back(id);
		(std::ofstream("users/" + r.user.username + ".txt") << r.user.encode()).close();
		
		return_code(GroupJoined);
	});
	
	
	server.Post(URL_PATTERNS[
		RenameGroup
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) return_code(r.code);
		
		// read group id
		GroupId id;
		if (decode_group_id(message, id) == Failure) return_code(BadData);
		
		// read name
		std::string name;
		if (read_quoted_string(message, name) == Failure) return_code(BadData);
		
		// todo: filter group names
		// InvalidGroupName unused otherwise
		
		// check that requesting user is in the group
		if (std::find(r.user.groups.begin(), r.user.groups.end(), id) == r.user.groups.end()) return_code(InvalidGroup);
		
		// look up group
		auto pair_ptr = groups.find(id);
		if (pair_ptr == groups.end()) {
			printf("User '%s' has reference to missing group '%s'\n", r.user.username.c_str(), encode_group_id(id).c_str());
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
		GroupId id;
		if (decode_group_id(message, id) == Failure) return_code(BadData);
		
		auto group_ptr = std::find(r.user.groups.begin(), r.user.groups.end(), id);
		if (group_ptr == r.user.groups.end()) return_code(InvalidGroup);
		r.user.groups.erase(group_ptr);
		
		(std::ofstream("users/" + r.user.username + ".txt") << r.user.encode()).close();
		
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
	
	
	
	server.listen("0.0.0.0", 8080);
}

