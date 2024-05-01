#ifndef RCR_CALENDAR_EDITOR_EXPOSED_FUNCTIONS
#define RCR_CALENDAR_EDITOR_EXPOSED_FUNCTIONS

#include "../shared/calendar.hpp"

// Set this before calling transfer_to_calendar_editor
static Calendar calendar;
void transfer_to_calendar_editor();

// Set these before calling transfer_to_group_calendar_view
static std::string active_group_name;
static std::vector<std::tuple<std::string, Calendar>> group_calendars; // vec of group_name and calendar?
void transfer_to_group_calendar_view();

#endif