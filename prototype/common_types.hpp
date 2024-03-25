#ifndef RCR_COMMON_TYPES_DEFINITIONS
#define RCR_COMMON_TYPES_DEFINITIONS

#define i32 int
#define i16 short
#define u32 unsigned int
#define u16 unsigned short
#define usize unsigned long long


enum Panic {
	UnwrappedNoneValue,
};


template <typename T>
struct Option {
	private:
	bool is_some_;
	T value;
	
	inline Option(bool is_some, T value) : is_some_(is_some), value(value) {}
	
	public:
	inline static Option<T> some(T value) {
		return Option<T>(true, value);
	}
	inline static Option<T> none() {
		T new_value; // T must have a public default constructor
		return Option<T>(false, new_value);
	}
	
	inline bool is_some() {
		return this->is_some_;
	}
	inline bool is_none() {
		return !this->is_some_;
	}
	
	inline T unwrap() {
		if (this->is_some_) return this->value;
		else throw Panic::UnwrappedNoneValue;
	}
	
	inline T unwrap_or(T default_value) {
		if (this->is_some_) return this->value;
		else return default_value;
	}
};







#endif