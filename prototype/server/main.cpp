#include <stdio.h>
#include <string.h>
#include "httplib.h"

#include "../definitions.hpp"

using namespace httplib;


int main() {
	Server server;
	
	server.Get("/", [](const Request& request, Response& response) {
		response.set_content("Hello from server!", "text/plain");
	});
	
	server.Post("/get-data", [&](const Request& request, Response& response) {
		printf("Received post request: %s\n", request.body.c_str());
		response.set_content("server got your message: '" + request.body + "'", "text/plain");
	});
	
	printf("Server started.\nCurrent time:\n");
	auto now = TimeAndDate::now();
	MonthAndDay md = now.get_month_and_day();
	printf("%s %d %d, %s, %d:%02d\n", MONTH_NAMES[md.month], md.day, now.get_year(), DAY_NAMES[now.get_day_of_week()], now.get_hour(), now.get_minute());
	
	
	server.listen("0.0.0.0", 8080);
}

