#ifndef RCR_NETWORKING_DEFINITIONS
#define RCR_NETWORKING_DEFINITIONS

#include <string>


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









#endif