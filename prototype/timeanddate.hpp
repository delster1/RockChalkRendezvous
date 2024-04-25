#ifndef RCR_TIMEANDDATE_DEFINITIONS
#define RCR_TIMEANDDATE_DEFINITIONS

#include <math.h>
#include <time.h>
#include <string.h>
#include "core_utils.hpp"


#define MINUTES_IN_DAY 1440
#define JAN_1_2000_DAY Saturday


enum Month {
	January   =  0,
	February  =  1,
	March     =  2,
	April     =  3,
	May       =  4,
	June      =  5,
	July      =  6,
	August    =  7,
	September =  8,
	October   =  9,
	November  = 10,
	December  = 11,
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

static const char MONTH_NAMES[12][10] = {
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December",
};

static const char DAY_NAMES[7][10] = {
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday",
	"Sunday",
};




// MARK: Functions

// checks if given year is a leap year
bool is_leap_year(i32 year) {
	if (mod(year, 400) == 0) return true;
	if (mod(year, 100) == 0) return false;
	if (mod(year, 4) == 0) return true;
	return false;
}

// returns the number of days in the given year
u16 find_days_in_year(i32 year) {
	if (is_leap_year(year)) return 366;
	else return 365;
}

// returns the number of days in the given month
u16 find_days_in_month(Month month, bool leap) {
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
			return 0;
	}
}

// returns the day of the year for the given month and day
i32 find_day_of_year(Month month, i32 day_of_month, i32 year) {
	bool leap = is_leap_year(year);
	u16 days_this_month;
	
	while (day_of_month > (days_this_month = find_days_in_month(month, leap))) {
		day_of_month -= days_this_month;
		if (month == December) {
			month = January;
			year += 1;
			leap = is_leap_year(year);
		} else month = static_cast<Month>(static_cast<u32>(month) + 1);
	}
	
	while (day_of_month <= 0) {
		day_of_month += find_days_in_month(month, leap);
		if (month == January) {
			month = December;
			year -= 1;
			leap = is_leap_year(year);
		} else month = static_cast<Month>(static_cast<u32>(month) - 1);
	}
	
	return day_of_month - 1;
}






// needed for the function that calculates both at once
struct MonthAndDay {
	Month month;
	u16 day;
};


// stores precise time and date information
struct TimeAndDate { // MARK: TimeAndDate
	private:
	u16 minute;
	u16 day;
	i32 year;
	
	// default constructors for other methods to use
	inline TimeAndDate(u16 minute, u16 day, i32 year) : minute(minute), day(day), year(year) {}
	
	public:
    // default constructor
	inline TimeAndDate() : minute(0), day(0), year(0) {}
	
    // builds a TimeAndDate object from a minute, day, and year
	static TimeAndDate build(i32 minute, i32 day, i32 year) {
		// extraneous minute values roll over to the day number
		i32 extra_days = floor(static_cast<double>(minute) / MINUTES_IN_DAY);
		minute -= extra_days * MINUTES_IN_DAY;
		day += extra_days;
		
		// extraneous day values roll over to the year
		u16 days_in_year;
		while (day >= (days_in_year = find_days_in_year(year))) {
			day -= days_in_year;
			year += 1;
		}
		while (day < 0) {
			day += find_days_in_year(year);
			year -= 1;
		}
		
		return { static_cast<u16>(minute), static_cast<u16>(day), year };
	}
	
	// September 31 -> October 1
	inline static TimeAndDate build_from_month_wrap_day(i32 minute, i32 day, Month month, i32 year) {
		return TimeAndDate::build(minute, find_day_of_year(month, day, year), year);
	}
	
	// September 31 -> September 30
	inline static TimeAndDate build_from_month(i32 minute, i32 day, Month month, i32 year) {
		u16 days_this_month = find_days_in_month(month, is_leap_year(year));
		if (day > days_this_month) day = days_this_month;
		return TimeAndDate::build_from_month_wrap_day(minute, day, month, year);
	}
	
    // gets the current time and date
	static TimeAndDate now() {
		time_t system_time;
		time(&system_time);
		struct tm timeinfo = *gmtime(&system_time);
		
		TimeAndDate output;
		output.minute = timeinfo.tm_hour * 60 + timeinfo.tm_min;
		output.day = timeinfo.tm_yday;
		output.year = timeinfo.tm_year + 1900; // system time year is relative to 1900
		return output;
	}
	
    // getters
	inline u16 get_minute_of_day() const { return this->minute; }
	inline u16 get_day_of_year() const { return this->day; }
	inline u16 get_year() const { return this->year; }
	
	inline u16 get_minute() const { return this->minute % 60; }
	inline u16 get_hour() const { return this->minute / 60; }
	
    // MARK: Serialization methods
	static inline std::string encode_static(const TimeAndDate& td) { return td.encode(); }
	std::string encode() const {
		let s = std::ostringstream();
		s << this->minute << " " << this->day << " " << this->year;
		return s.str();
	}
	
	static inline Status decode_static(std::istream& stream, TimeAndDate& td) { return td.decode(stream); }
	Status decode(std::istream& stream) {
		i32 minute, day, year;
		if (stream >> minute && stream >> day && stream >> year) {
			*this = TimeAndDate::build(minute, day, year);
			return Success;
		} else return Failure;
	}
	
	std::string to_string() const {
		MonthAndDay md = this->get_month_and_day();
		char s[64] = {0};
		snprintf(s, 64, "%d:%02d %s, %s %d %d", this->get_hour(), this->get_minute(), DAY_NAMES[this->get_day_of_week()], MONTH_NAMES[md.month], md.day, this->year);
		return std::string(s);
	}
	
	
	// MARK: Helper methods
	
	MonthAndDay get_month_and_day() const { // these are together since it's basically the same computation
		Month month = January;
		bool leap = is_leap_year(this->year);
		u16 days_remaining = this->day;
		u16 days_this_month;
		while (days_remaining >= (days_this_month = find_days_in_month(month, leap))) {
			days_remaining -= days_this_month;
			month = static_cast<Month>(static_cast<u32>(month) + 1);
		}
		return MonthAndDay { month, (u16) (days_remaining + 1) }; // this day number is 1 based because that's how month dates work
	}
	
	Day get_day_of_week() const {
		// base all week days off of january 1st 2000
		i32 year_diff = this->year - 2000;
		i32 optional_one = static_cast<i32>(year_diff > 0);
		
		i32 day_count = static_cast<i32>(JAN_1_2000_DAY)
					  + this->day
					  + year_diff
					  + (year_diff - optional_one) / 4
					  - (year_diff - optional_one) / 100
					  + (year_diff - optional_one) / 400
					  + optional_one;
		
		return static_cast<Day>(mod(day_count, 7));
	}
	
	// don't use this on times more than 4085 years apart or else
	i32 minutes_since(const TimeAndDate& t) const {
		i32 minute_diff = this->minute - t.minute + (this->day - t.day) * MINUTES_IN_DAY;
		
		for (i32 year = t.year; year <= this->year; year++) {
			minute_diff += find_days_in_year(year) * MINUTES_IN_DAY;
		}
		for (i32 year = this->year; year <= t.year; year++) {
			minute_diff -= find_days_in_year(year) * MINUTES_IN_DAY;
		}
		
		return minute_diff;
	}
	
	// functions for adding time correctly
	// see comments for explanations of similar functions
	
	TimeAndDate add_minutes(const i32 minutes) const {
		return TimeAndDate::build(this->minute + minutes, this->day, this->year);
	}
	TimeAndDate add_days(const i32 days) const {
		return TimeAndDate::build(this->minute, this->day + days, this->year);
	}
	
	// will move the day down to stay in the correct month
	// i.e. August 31 + 1 month = September 30
	TimeAndDate add_months(const i32 months) const {
		MonthAndDay md = this->get_month_and_day();
		return TimeAndDate::build_from_month(this->minute, md.day,
			static_cast<Month>(mod(static_cast<i32>(md.month) + months, 12)),
		this->year);
	}
	
	// will wrap around to the next month
	// i.e. August 31 + 1 month = October 1
	TimeAndDate add_months_wrap_day(const i32 months) const {
		MonthAndDay md = this->get_month_and_day();
		return TimeAndDate::build_from_month_wrap_day(this->minute, md.day,
			static_cast<Month>((static_cast<i32>(md.month) + months + 12) % 12),
		this->year);
	}
	
	// leap day won't shift everything off by a day
	// February 29 2024 + 1 year = February 28 2025
	TimeAndDate add_years(const i32 years) const {
		MonthAndDay md = this->get_month_and_day();
		return TimeAndDate::build_from_month(this->minute, md.day, md.month, this->year + years);
	}
	
	// February 29 2024 + 1 year = March 1 2025
	TimeAndDate add_years_wrap_day(const i32 years) const {
		MonthAndDay md = this->get_month_and_day();
		return TimeAndDate::build_from_month_wrap_day(this->minute, md.day, md.month, this->year + years);
	}
	
    // operator overloads
	inline bool operator==(const TimeAndDate& other) const {
		return this->year == other.year && this->day == other.day && this->minute == other.minute;
	}
	inline bool operator!=(const TimeAndDate& other) const {
		return !(*this == other);
	}
	inline bool operator>(const TimeAndDate& other) const {
		if (this->year != other.year) return this->year > other.year;
		if (this->day != other.day) return this->day > other.day;
		if (this->minute != other.minute) return this->minute > other.minute;
		return false;
	}
	inline bool operator<(const TimeAndDate& other) const {
		if (this->year != other.year) return this->year < other.year;
		if (this->day != other.day) return this->day < other.day;
		if (this->minute != other.minute) return this->minute < other.minute;
		return false;
	}
	inline bool operator>=(const TimeAndDate& other) const {
		return !(*this < other);
	}
	inline bool operator<=(const TimeAndDate& other) const {
		return !(*this > other);
	}
	
};



#endif
