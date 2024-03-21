#ifndef RCR_DATETIME_CONSTANTS_DEFINITIONS
#define RCR_DATETIME_CONSTANTS_DEFINITIONS

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

enum Day {
	Monday    = 0,
	Tuesday   = 1,
	Wednesday = 2,
	Thursday  = 3,
	Friday    = 4,
	Saturday  = 5,
	Sunday    = 6,
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



#endif