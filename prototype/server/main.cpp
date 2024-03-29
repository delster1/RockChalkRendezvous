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



struct LoginResult {
	Status status;
	ServerResponse response;
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
	
	let user_file = std::ofstream("users/" + username + ".txt");
	user_file << User(username, password).encode();
	user_file.close();
	
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
	
	
	
	
	
	
	
	
	server.Post(URL_PATTERNS[
		Ping
	], [&](const Request& request, Response& response) {
		response.set_content(std::string(PingResponse, 1), "text/plain");
	});
	
	server.Post(URL_PATTERNS[
		CreateAccount
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		response.set_content(std::string(create_account(message), 1), "text/plain");
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
			response.set_content(std::string(UsernameUnavailable, 1), "text/plain");
		} else response.set_content(std::string(UsernameAvailable, 1), "text/plain");
	});
	
	
	server.Post(URL_PATTERNS[
		ValidateAccount
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		response.set_content(std::string(r.response, 1), "text/plain");
	});
	
	
	server.Post(URL_PATTERNS[
		DeleteAccount
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) { response.set_content(std::string(r.response, 1), "text/plain"); return; }
		
		// todo: leave all groups
		
		std::remove(("users/" + r.user.username + ".txt").c_str());
		response.set_content(std::string(AccountDeleted, 1), "text/plain");
	});
	
	
	server.Post(URL_PATTERNS[
		GetUserCalendar
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) { response.set_content(std::string(r.response, 1), "text/plain"); return; }
		
		
		// todo
		// UserCalendar
	});
	
	
	server.Post(URL_PATTERNS[
		SetUserCalendar
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) { response.set_content(std::string(r.response, 1), "text/plain"); return; }
		
		// todo
		// UserCalendarWritten
	});
	
	
	server.Post(URL_PATTERNS[
		GetGroups
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) { response.set_content(std::string(r.response, 1), "text/plain"); return; }
		
		// todo
		// Groups
	});
	
	
	server.Post(URL_PATTERNS[
		GetGroupCalendar
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) { response.set_content(std::string(r.response, 1), "text/plain"); return; }
		
		// todo
		// InvalidGroup
		// GroupCalendar
	});
	
	
	server.Post(URL_PATTERNS[
		CreateGroup
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) { response.set_content(std::string(r.response, 1), "text/plain"); return; }
		
		
		
		// todo
		// InvalidGroupName
		// GroupCreated
	});
	
	
	server.Post(URL_PATTERNS[
		JoinGroup
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) { response.set_content(std::string(r.response, 1), "text/plain"); return; }
		
		// todo
		// InvalidGroup
		// GroupJoined
	});
	
	
	server.Post(URL_PATTERNS[
		RenameGroup
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) { response.set_content(std::string(r.response, 1), "text/plain"); return; }
		
		// todo
		// InvalidGroup
		// InvalidGroupName
		// GroupRenamed
	});
	
	
	server.Post(URL_PATTERNS[
		LeaveGroup
	], [&](const Request& request, Response& response) {
		let message = std::istringstream(request.body);
		LoginResult r = login(message);
		if (r.status == Failure) { response.set_content(std::string(r.response, 1), "text/plain"); return; }
		
		// todo
		// InvalidGroup
		// GroupLeft
	});
	
	
	
	server.listen("0.0.0.0", 8080);
}

