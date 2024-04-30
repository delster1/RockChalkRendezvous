#ifndef RCR_NETWORKING_DEFINITIONS
#define RCR_NETWORKING_DEFINITIONS

#include <string>


#define DEFAULT_SERVER_PORT 7777
#define DEFAULT_SERVER_HOSTNAME "localhost"


// must match URL_PATTERNS
enum ClientRequest {
	Ping              = 0,
	CreateAccount     = 1,
	CheckUsername     = 2,
	ValidateAccount   = 3,
	DeleteAccount     = 4,
	GetUserCalendar   = 5,
	SetUserCalendar   = 6,
	GetGroups         = 7,
	GetGroupCalendars = 8,
	CreateGroup       = 9,
	JoinGroup         = 10,
	RenameGroup       = 11,
	LeaveGroup        = 12,
};

// must match ClientRequest - what to look for in URL
static const std::string URL_PATTERNS[] = {
	"/ping",
	"/create_account",
	"/check_username",
	"/validate_account",
	"/delete_account",
	"/get_user_calendar",
	"/set_user_calendar",
	"/get_groups",
	"/get_group_calendars",
	"/create_group",
	"/join_group",
	"/rename_group",
	"/leave_group",
};

enum ServerResponse {
	PingResponse        = 'Q',
	AccountOk           = 'K',
	UsernameAvailable   = 'A',
	UsernameUnavailable = 'U',
	InvalidPassword     = 'P',
	IncorrectLogin      = 'V',
	AccountDeleted      = 'D',
	UserCalendar        = 'R',
	UserCalendarWritten = 'W',
	Groups              = 'G',
	GroupCalendars      = 'M',
	InvalidGroup        = 'F',
	InvalidGroupName    = 'H',
	GroupCreated        = 'C',
	GroupJoined         = 'J',
	GroupRenamed        = 'N',
	GroupLeft           = 'L',
	BadData             = '?',
};

std::string server_response_to_string(ServerResponse sr) {
	switch (sr) {
		case PingResponse:        return "Ping Response";
		case AccountOk:           return "Account Ok";
		case UsernameAvailable:   return "Username Available";
		case UsernameUnavailable: return "Username Unavailable";
		case InvalidPassword:     return "Invalid Password";
		case IncorrectLogin:      return "Incorrect Login";
		case AccountDeleted:      return "Account Deleted";
		case UserCalendar:        return "User Calendar";
		case UserCalendarWritten: return "User Calendar Written";
		case Groups:              return "Groups";
		case GroupCalendars:      return "Group Calendars";
		case InvalidGroup:        return "Invalid Group";
		case InvalidGroupName:    return "Invalid Group Name";
		case GroupCreated:        return "Group Created";
		case GroupJoined:         return "Group Joined";
		case GroupRenamed:        return "Group Renamed";
		case GroupLeft:           return "Group Left";
		case BadData:             return "Bad Data";
		default:                  return "Unknown";
	};
}







#endif
