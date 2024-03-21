#include <stdio.h>
#include <string.h>
#include "civetweb.h"

#include "../definitions.hpp"


// This function will be called by civetweb on every new request.
static int handle_request(struct mg_connection* connection) {
	const struct mg_request_info* request_info = mg_get_request_info(connection);
	char content[100];
	
	// Prepare the message we're going to send
	int content_length = snprintf(content, sizeof(content),
		"Rock Chalk Rendezvous first networking test! Remote port: %d",
		request_info->remote_port
	);
	
	// Send HTTP reply to the client
	mg_printf(connection,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: %d\r\n"	// Always set Content-Length
		"\r\n"
		"%s",
		content_length, content
	);

	// Returning non-zero tells civetweb that our function has replied to
	// the client, and civetweb should not send client any more data.
	return 1;
}

int main() {
	
	// Prepare callbacks structure. We have only one callback, the rest are NULL.
	struct mg_callbacks callbacks;
	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.begin_request = handle_request;
	
	// List of options. Last element must be NULL.
	const char* options[] = {"listening_ports", "8080", NULL};
	
	// Start the web server.
	struct mg_context* ctx = mg_start(&callbacks, NULL, options);
	
	// Wait until user enters 'q'. Server is running in separate thread.
	// Navigating to http://localhost:8080 will invoke handle_request().
	while (getchar() != 'q');
	
	auto now = TimeAndDate::now();
	MonthAndDay md = now.get_month_and_day();
	printf("%s %d, %s, %d:%d", MONTH_NAMES[md.month], md.day, DAY_NAMES[now.get_day_of_week()], now.get_hour(), now.get_minute());
	
	// Stop the server.
	mg_stop(ctx);
	
	return 0;
}
