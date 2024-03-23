#ifndef RCR_DATETIME_FUNCTIONS_DEFINITIONS
#define RCR_DATETIME_FUNCTIONS_DEFINITIONS

#include <math.h>
#include <time.h>
#include "common_types.hpp"
#include "datetime_constants.hpp"

bool is_leap_year(u16 year) {
	if (year % 400 == 0) return true;
	if (year % 100 == 0) return false;
	if (year % 4 == 0) return true;
	return false;
}

u16 find_days_in_year(u16 year) {
	if (is_leap_year(year)) return 366;
	else return 365;
}

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

i32 find_day_of_year(Month month, i32 day_of_month, i32 year) {
	bool leap = is_leap_year(year);
	u16 days_this_month;
	
	while (day_of_month > (days_this_month = find_days_in_month(month, leap))) {
		day_of_month -= days_this_month;
		if (month == December) {
			month = January;
			year += 1;
			leap = is_leap_year(year);
		} else month = (Month)((u32) month + 1);
	}
	
	while (day_of_month <= 0) {
		day_of_month += find_days_in_month(month, leap);
		if (month == January) {
			month = December;
			year -= 1;
			leap = is_leap_year(year);
		} else month = (Month)((u32) month - 1);
	}
	
	return day_of_month - 1;
}


// needed for the function that calculates both at once
struct MonthAndDay {
	Month month;
	u16 day;
};


struct TimeAndDate {
	private:
	u16 minute;
	u16 day;
	i32 year;
	
	// default constructors for other methods to use
	TimeAndDate() : minute(0), day(0), year(0) {}
	TimeAndDate(u16 minute, u16 day, i32 year) : minute(minute), day(day), year(year) {}
	
	public:
	static TimeAndDate build(i32 minute, i32 day, i32 year) {
		// extraneous minute values roll over to the day number
		i32 extra_days = floor((double) minute / MINUTES_IN_DAY);
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
		
		return { minute, day, year };
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
	
	inline static TimeAndDate now() {
		time_t system_time;
		time(&system_time);
		struct tm timeinfo = *gmtime(&system_time);
		
		TimeAndDate output;
		output.minute = timeinfo.tm_hour * 60 + timeinfo.tm_min;
		output.day = timeinfo.tm_yday;
		output.year = timeinfo.tm_year + 1900; // system time year is relative to 1900
		return output;
	}
	
	inline u16 get_minute_of_day() { return this->minute; }
	inline u16 get_day_of_year() { return this->day; }
	inline u16 get_year() { return this->year; }
	
	inline u16 get_minute() { return this->minute % 60; }
	inline u16 get_hour() { return this->minute / 60; }
	
	MonthAndDay get_month_and_day() { // these are together since it's basically the same computation
		Month month = January;
		bool leap = is_leap_year(this->year);
		u16 days_remaining = this->day;
		u16 days_this_month;
		while (days_remaining >= (days_this_month = find_days_in_month(month, leap))) {
			days_remaining -= days_this_month;
			month = (Month)((u32) month + 1);
		}
		return MonthAndDay { month, (u16) (days_remaining + 1) }; // this day number is 1 based because that's how month dates work
	}
	
	Day get_day_of_week() {
		// base all week days off of january 1st 2000
		i32 year_diff = this->year - 2000;
		i32 optional_one = (i32) (year_diff > 0);
		
		i32 day_count = (i32) JAN_1_2000_DAY
					  + this->day
					  + year_diff
					  + (year_diff - optional_one) / 4
					  - (year_diff - optional_one) / 100
					  + (year_diff - optional_one) / 400
					  + optional_one;
		
		// Find a true modulo since % is actually remainder
		return (Day) ((day_count % 7 + 7) % 7);
	}
	
	// don't use this on times more than 4085 years apart or else
	i32 minutes_since(const TimeAndDate& t) {
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
	// see comments for explainations of similar functions
	
	TimeAndDate add_minutes(const i32 minutes) {
		return TimeAndDate::build(this->minute + minutes, this->day, this->year);
	}
	TimeAndDate add_days(const i32 days) {
		return TimeAndDate::build(this->minute, this->day + days, this->year);
	}
	// will move the day down to stay in the correct month
	// i.e. August 31 + 1 month = September 30
	TimeAndDate add_months(const i32 months) {
		MonthAndDay md = this->get_month_and_day();
		return TimeAndDate::build_from_month(this->minute, md.day,
			(Month) ((((i32) md.month + months) % 12 + 12) % 12),
		this->year);
	}
	// will wrap around to the next month
	// i.e. August 31 + 1 month = September 31 -> October 1
	TimeAndDate add_months_wrap_day(const i32 months) {
		MonthAndDay md = this->get_month_and_day();
		return TimeAndDate::build_from_month_wrap_day(this->minute, md.day,
			(Month) ((((i32) md.month + months) % 12 + 12) % 12),
		this->year);
	}
	// leap day won't shift everything off by a day
	// February 29 2024 + 1 year = February 28 2025
	TimeAndDate add_years(const i32 years) {
		MonthAndDay md = this->get_month_and_day();
		return TimeAndDate::build_from_month(this->minute, md.day, md.month, this->year + years);
	}
	// February 29 2024 + 1 year = March 1 2025
	TimeAndDate add_years_wrap_day(const i32 years) {
		MonthAndDay md = this->get_month_and_day();
		return TimeAndDate::build_from_month_wrap_day(this->minute, md.day, md.month, this->year + years);
	}
	
	inline bool operator==(const TimeAndDate& other) {
		return this->year == other.year && this->day == other.day && this->minute == other.minute;
	}
	inline bool operator!=(const TimeAndDate& other) {
		return !(*this == other);
	}
	inline bool operator>(const TimeAndDate& other) {
		if (this->year != other.year) return this->year > other.year;
		if (this->day != other.day) return this->day > other.day;
		if (this->minute != other.minute) return this->minute > other.minute;
		return false;
	}
	inline bool operator<(const TimeAndDate& other) {
		if (this->year != other.year) return this->year < other.year;
		if (this->day != other.day) return this->day < other.day;
		if (this->minute != other.minute) return this->minute < other.minute;
		return false;
	}
	inline bool operator>=(const TimeAndDate& other) {
		return !(*this < other);
	}
	inline bool operator<=(const TimeAndDate& other) {
		return !(*this > other);
	}
	
};



#endif