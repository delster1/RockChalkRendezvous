#ifndef RCR_DATA_CODECS_DEFINITIONS
#define RCR_DATA_CODECS_DEFINITIONS

#include <string>
#include <vector>
#include <sstream>
#include "common_utils.hpp"



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




#endif