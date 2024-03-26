#ifndef RCR_USER_AUTHENTICATION_DEFINITIONS
#define RCR_USER_AUTHENTICATION_DEFINITIONS

#include <string.h>


enum ClientRequest {
	Ping             = 'Q',
	CreateAccount    = 'K',
	CheckUsername    = 'U',
	ValidateAccount  = 'V',
	DeleteAccount    = 'D',
	GetUserCalendar  = 'R',
	SetUserCalendar  = 'W',
	GetGroups        = 'G',
	GetGroupCalendar = 'M',
	CreateGroup      = 'C',
	JoinGroup        = 'J',
	LeaveGroup       = 'L',
};

enum ServerResponse {
	Ping                = 'Q',
	AccountOk           = 'K',
	UsernameAvailable   = 'A',
	UsernameUnavailable = 'U',
	InvalidPassword     = 'P',
	IncorrectLogin      = 'V',
	AccountDeleted      = 'D',
	UserCalendar        = 'R',
	UserCalendarWritten = 'W',
	Groups              = 'G',
	GroupCalendar       = 'M',
	GroupCreated        = 'C',
	GroupJoined         = 'J',
	GroupLeft           = 'L',
	BadData             = 'B',
};







#endif