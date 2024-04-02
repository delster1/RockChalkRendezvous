#ifndef RCR_DATA_CODECS_DEFINITIONS
#define RCR_DATA_CODECS_DEFINITIONS

#include <string>
#include <vector>
//#include "calendar.hpp"



template <typename T>
std::string encode_vector(const std::vector<T>& vector, const std::function<std::string(const T&)> encode_function, bool use_newlines) {
	char delimiter = ' ';
	if (use_newlines) delimiter = '\n';
	
	let s = std::ostringstream();
	s << vector.size();
	for (T item : vector) {
		s << delimiter << encode_function(item);
	}
	return s.str();
}

template <typename T>
Status decode_vector(std::istream& stream, std::vector<T>& vector, const std::function<Status(std::istream&, T&)> decode_function) {
	usize count;
	stream >> count;
	if (stream.fail()) return Failure;
	vector.reserve(count);
	
	T value;
	for (int i = 0; i < count; i++) {
		propagate(decode_function(stream, value));
		vector.push_back(value);
	}
	return Success;
}



std::string quote_string(const std::string& string) {
	let s = std::ostringstream();
	s << '"' << string << '"';
	return s.str();
}

Status read_quoted_string(std::istream& stream, std::string& s) {
	char next;
	stream >> next;
	if (stream.fail()) return Failure;
	if (next != '"') return Failure;
	std::getline(stream, s, '"');
	if (stream.fail()) return Failure;
	return Success;
}


typedef usize GroupID;

std::string encode_group_id(const GroupID& id) {
	let s = std::ostringstream();
	let n = id;
	for (int i = 8 * sizeof(GroupID) - 4; i >= 0; i -= 4) {
		char c = (n >> i) & 0b1111;
		if (c <= 9) c += '0';
		else c += 'A' - 10;
		s << c;
	}
	return s.str();
}

Status decode_group_id(std::istream& stream, GroupID& id) {
	char c;
	GroupID n = 0;
	for (int i = 0; i < 2*sizeof(GroupID); i++) {
		stream >> c;
		if (stream.fail()) return Failure;
		
		n <<= 4;
		     if (c >= '0' && c <= '9') n |= c - '0';
		else if (c >= 'A' && c <= 'F') n |= c - 'A' + 10;
		else if (c >= 'a' && c <= 'f') n |= c - 'a' + 10;
		else return Failure;
	}
	id = n;
	return Success;
}






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


struct Group {
	GroupID id;
	std::string name;
	std::vector<std::string> members;
	
	Group() {}
	Group(GroupID id, std::string name, std::vector<std::string> members) : id(id), name(name), members(members) {}
	
	inline std::string encode() const { return Group::encode_static(*this); }
	static std::string encode_static(const Group& group) {
		let s = std::ostringstream();
		s << encode_group_id(group.id) << " " << quote_string(group.name) << " " << encode_vector<std::string>(group.members, quote_string, false);
		return s.str();
	}
	
	static Status decode(std::istream& stream, Group& group) {
		propagate(decode_group_id(stream, group.id));
		propagate(read_quoted_string(stream, group.name));
		propagate(decode_vector<std::string>(stream, group.members, read_quoted_string));
		return Success;
	} 
};






#endif