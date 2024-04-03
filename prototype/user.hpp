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
	
	static inline Status decode_static(std::istream& stream, User& user) { return user.decode(stream); }
	Status decode(std::istream& stream) {
		propagate(read_quoted_string(stream, this->username));
		propagate(read_quoted_string(stream, this->password));
		propagate(decode_vector<GroupID>(stream, this->group_ids, decode_group_id));
		propagate(this->calendar.decode(stream));
		return Success;
	}
};



static const char USERNAME_DISALLOWED_CHARACTERS[] = "<>:\"/\\|?*";

bool is_username_valid(const std::string& username) {
	for (char c : username) {
		if (c & 0b11100000 == 0) return false;
		for (const char* p = USERNAME_DISALLOWED_CHARACTERS; *p != 0; p++) {
			if (c == *p) return false;
		}
	}
	return true;
}






#endif