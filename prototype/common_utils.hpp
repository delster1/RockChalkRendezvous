#ifndef RCR_COMMON_UTILS_DEFINITIONS
#define RCR_COMMON_UTILS_DEFINITIONS

#include <vector>
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




#endif