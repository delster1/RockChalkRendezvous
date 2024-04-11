#ifndef RCR_CORE_UTILS_DEFINITIONS
#define RCR_CORE_UTILS_DEFINITIONS

#include <string>
#include <vector>
#include <sstream>
#include <functional>


#define let auto

#define i32 int
#define i16 short
#define i8 char
#define isize long long
#define u32 unsigned int
#define u16 unsigned short
#define u8 unsigned char
#define usize unsigned long long


// Find a true modulo since % is actually remainder
inline u32 mod(i32 a, u32 b) {
	return ((a % b) + b) % b;
}


enum Status {
	Success = true,
	Failure = false,
};

#define propagate(a) if (a == Failure) return Failure


template <typename T, typename U>
std::vector<U> vector_map(const std::vector<T>& v, const std::function<U(const T&)> map_function) {
	let out = std::vector<U>();
	out.reserve(v.size());
	for (const T& value : v) out.push_back(map_function(value));
	return out;
}




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