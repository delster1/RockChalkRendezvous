#ifndef RCR_USER_DEFINITIONS
#define RCR_USER_DEFINITIONS

#include <string>
#include <vector>
#include "group.hpp"
#include "calendar.hpp"


struct User {
	std::string username;
	std::string password;
	std::vector<GroupID> group_ids;
	Calendar calendar;
	
	inline User() {}
	inline User(std::string username, std::string password) : username(username), password(password) {}
	
	static inline std::string encode_static(const User& user) { return user.encode(); }
	std::string encode() const {
		let s = std::ostringstream();
		s << quote_string(this->username) << " " << quote_string(this->password) << "\n";
		s << encode_vector<GroupID>(this->group_ids, encode_group_id, true) << "\n" << this->calendar.encode();
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