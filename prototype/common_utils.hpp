#ifndef RCR_COMMON_UTILS_DEFINITIONS
#define RCR_COMMON_UTILS_DEFINITIONS


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







#endif