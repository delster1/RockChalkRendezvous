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
			return 0;
	}
}


struct MonthAndDay {
	Month month;
	u16 day;
};


struct TimeAndDate {
	private:
	u16 minute;
	u16 day;
	i32 year;
	
	public:
	static TimeAndDate build(i32 minute, i32 day, i32 year) {
		i32 extra_days = floor((f64) minute / MINUTES_IN_DAY);
		minute -= extra_days * MINUTES_IN_DAY;
		day += extra_days;
		
		u16 days_in_year;
		while (day >= (days_in_year = get_days_in_year(year))) {
			day -= days_in_year;
			year += 1;
		}
		
		while (day < 0) {
			day += get_days_in_year(year);
			year -= 1;
		}
		
		TimeAndDate output;
		output.minute = minute;
		output.day = day;
		output.year = year;
		return output;
	}
	
	static TimeAndDate now() {
		time_t system_time;
		time(&system_time);
		struct tm timeinfo = *gmtime(&system_time);
		
		TimeAndDate output;
		output.minute = timeinfo.tm_hour * 60 + timeinfo.tm_min;
		output.day = timeinfo.tm_yday;
		output.year = timeinfo.tm_year;
		return output;
	}
	
	inline u16 get_minute_in_day() { return this->minute; }
	inline u16 get_day_in_year() { return this->day; }
	inline u16 get_year() { return this->year; }
	
	inline u16 get_minute() { return this->minute % 60; }
	inline u16 get_hour() { return this->minute / 60; }
	
	MonthAndDay get_month_and_day() {
		Month month = January;
		bool leap = is_leap_year(this->year);
		u16 days_remaining = this->day;
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
					  + this->day
					  + year_diff
					  + (year_diff - optional_one) / 4
					  - (year_diff - optional_one) / 100
					  + (year_diff - optional_one) / 400;
		
		// Find a true modulo since % is actually remainder
		return (Day) ((day_count % 7 + 7) % 7);
	}
	
	
};



#endif