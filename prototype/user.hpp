#ifndef RCR_USER_DEFINITIONS
#define RCR_USER_DEFINITIONS

#include <string>
#include <vector>
#include "group.hpp"
//#include "calendar.hpp"

struct Calendar {
	std::string dummy_message;
	
	inline std::string encode() const { return Calendar::encode(*this); }
	static std::string encode(const Calendar& c) {
		return quote_string(c.dummy_message);
	}
	static Status decode(std::istream& stream, Calendar& c) {
		return read_quoted_string(stream, c.dummy_message);
	}
};


struct User {
	std::string username;
	std::string password;
	std::vector<GroupID> group_ids;
	Calendar calendar;
	
	User() {}
	User(std::string username, std::string password) : username(username), password(password) {}
	
	inline std::string encode() const { return User::encode_static(*this); }
	static std::string encode_static(const User& user) {
		let s = std::ostringstream();
		s << quote_string(user.username) << " " << quote_string(user.password) << "\n";
		s << encode_vector<GroupID>(user.group_ids, encode_group_id, true) << "\n" << user.calendar.encode();
		return s.str();
	}
	
	static Status decode(std::istream& stream, User& user) {
		propagate(read_quoted_string(stream, user.username));
		propagate(read_quoted_string(stream, user.password));
		propagate(decode_vector<GroupID>(stream, user.group_ids, decode_group_id));
		propagate(Calendar::decode(stream, user.calendar));
		return Success;
	}
};









#endif