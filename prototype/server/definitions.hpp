#ifndef RCR_DEFINITIONS

#define i32 int
#define i16 short
#define u32 unsigned int
#define u16 unsigned short
#define f32 float
#define f64 double

#define SECONDS_IN_DAY 86400
#define JAN_1_2000_DAY Saturday

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


enum Month {
	NullMonth =  0,
	January   =  1,
	February  =  2,
	March     =  3,
	April     =  4,
	May       =  5,
	June      =  6,
	July      =  7,
	August    =  8,
	September =  9,
	October   = 10,
	November  = 11,
	December  = 12,
};

enum Day {
	Monday    = 0,
	Tuesday   = 1,
	Wednesday = 2,
	Thursday  = 3,
	Friday    = 4,
	Saturday  = 5,
	Sunday    = 6,
};


bool is_leap_year(u16 year) {
	if (year % 400 == 0) return true;
	if (year % 100 == 0) return false;
	if (year % 4 == 0) return true;
	return false;
}

u16 get_days_in_year(u16 year) {
	if (is_leap_year(year)) return 366;
	else return 365;
}

u16 get_days_in_month(Month month, bool leap) {
	switch (month) {
		case January:
		case March:
		case May:
		case July:
		case August:
		case October:
		case December:
			return 31;
			break;
		case April:
		case June:
		case September:
		case November:
			return 30;
			break;
		case February:
			if (leap) return 29;
			else return 28;
			break;
		default:
			return NullMonth;
	}
}

struct MonthAndDay {
	Month month;
	u16 day;
};


struct TimeAndDate {
	private:
	u32 seconds;
	u16 days;
	u16 year;
	
	public:
	static TimeAndDate build(u32 seconds, u16 days, u16 year) {
		days += seconds / SECONDS_IN_DAY;
		seconds %= SECONDS_IN_DAY;
		
		u16 days_in_year;
		while (days >= (days_in_year = get_days_in_year(year))) {
			days -= days_in_year;
			year += 1;
		}
		
		TimeAndDate output;
		output.seconds = seconds;
		output.days = days;
		output.year = year;
		return output;
	}
	
	u32 get_second_in_day() { return this->seconds; }
	u16 get_day_in_year() { return this->days; }
	u16 get_year() { return this->year; }
	
	MonthAndDay get_month_and_day() {
		Month month = January;
		bool leap = is_leap_year(this->year);
		u16 days_remaining = this->days;
		u16 days_this_month;
		while (days_remaining >= (days_this_month = get_days_in_month(month, leap))) {
			days_remaining -= days_this_month;
			month = (Month)((u32) month + 1);
		}
		return MonthAndDay { month, (u16)(days_remaining + 1) };
	}
	
	Day get_day_of_week() {
		i32 year_diff = this->year - 2000;
		i32 optional_one = (i32) (year_diff > 0);
		
		i32 day_count = (i32) JAN_1_2000_DAY
					  + this->days
					  + year_diff
					  + (year_diff - optional_one) / 4
					  - (year_diff - optional_one) / 100
					  + (year_diff - optional_one) / 400;
		
		// Find a true modulo since % is actually remainder
		return (Day) ((day_count % 7 + 7) % 7);
	}
	
};


#define RCR_DEFINITIONS
#endif

