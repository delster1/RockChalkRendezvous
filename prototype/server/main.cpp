#include <stdio.h>
#include <string.h>
#include "httplib.h"

#include "../definitions.hpp"

using namespace httplib;


int main() {
	Server svr;
	
	svr.Get("/", [](const Request & /*req*/, Response &res) {
		res.set_content("Hello World!", "text/plain");
	});
	
	printf("Server started.\nCurrent time:\n");
	auto now = TimeAndDate::now();
	MonthAndDay md = now.get_month_and_day();
	printf("%s %d %d, %s, %d:%d\n", MONTH_NAMES[md.month], md.day, now.get_year(), DAY_NAMES[now.get_day_of_week()], now.get_hour(), now.get_minute());
	
	
	svr.listen("0.0.0.0", 8080);
}

