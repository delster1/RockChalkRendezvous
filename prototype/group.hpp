#ifndef RCR_GROUP_DEFINITIONS
#define RCR_GROUP_DEFINITIONS

#include <string>
#include <vector>
#include <sstream>
#include "common_utils.hpp"
#include "data_codecs.hpp"



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