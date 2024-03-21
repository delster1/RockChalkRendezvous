#ifndef RCR_COMMON_TYPES_DEFINITIONS
#define RCR_COMMON_TYPES_DEFINITIONS


#define i32 int
#define i16 short
#define u32 unsigned int
#define u16 unsigned short
#define f32 float
#define f64 double


template <typename T>
struct Option {
	bool is_some;
	T value;
	
	inline Option<T> some(T value) {
		return Option { true, value };
	}
	inline Option<T> none() {
		return Option { false };
	}
};



#endif