#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <thread>
#include <tuple>
#include <ncurses.h>

#include "../../src/shared/timeanddate.hpp"
#include "../../src/shared/calendar.hpp"
#include "calendar_editor.hpp"

// UI state is modeled as a finite state machine
enum UIState {
    ViewingCalendar,
    ViewingList,
    AddingStart,
    AddingEnd,
    AddingRepeatType,
    AddingRepeatCount,
    RemoveConfirm,
    GroupCalendar,
};


// Static values for use by all functions

static u32 screen_height;
static u32 screen_width;
static u32 calendar_window_height;
static u32 calendar_window_width;
static u32 interact_window_height;
static u32 interact_window_width;

static WINDOW* calendar_window;
static WINDOW* interact_window;

static u32 calendar_time_margin = 5;
static u32 calendar_day_width;

static UIState ui_state;

static u32 calendar_row_size = 15;
static u32 calendar_scroll_height = MINUTES_IN_DAY / calendar_row_size;
static i32 calendar_scroll_offset = 0;
static TimeAndDate calendar_start_day;
static i32 calendar_selected_row, calendar_selected_day_of_week;

static u32 list_scroll_height = 0;
static i32 list_scroll_offset = 0;
static i32 list_selected_index = 0;

static TimeAndDate new_start_time, new_end_time;
static RepeatType new_repeat_type;
static u32 new_repeat_count;


// Pad a string with spaces so that it is centered to the desired width
inline std::string pad_center(std::string s, u32 width) {
    return std::string((width - s.length()) / 2, ' ') + s;
}


// Finds all of the occurrances of the block that happen in a given time range
// Used to determine what will be in view before drawing
std::vector<std::tuple<u32, TimeAndDate, TimeAndDate>> render_block(const TimeBlock& block, const TimeAndDate& start_day, const TimeAndDate& end_day) {
    std::vector<std::tuple<u32, TimeAndDate, TimeAndDate>> visible_occurences;
    
    if (block.start >= end_day) return visible_occurences;
    
    i32 first_occurrence, last_occurrence;
    
    if (block.repeat_period == NoRepeat) {
        first_occurrence = 0;
        last_occurrence = 0;
    } else {
        switch (block.repeat_period) {
            case Daily:
                first_occurrence = -block.end.days_since(start_day);
                last_occurrence = end_day.days_since(block.start);
                break;
            case Weekly:
                first_occurrence = -block.end.days_since(start_day) / 7;
                last_occurrence = end_day.days_since(block.start) / 7;
                break;
            case Monthly:
                first_occurrence = -block.end.months_since(start_day);
                last_occurrence = end_day.months_since(block.start);
                break;
            case Yearly:
                first_occurrence = -block.end.years_since(start_day);
                last_occurrence = end_day.years_since(block.start);
                break;
            default: return visible_occurences;
        }
        
        if (first_occurrence > (int)block.repeat_count) return visible_occurences;
        if (last_occurrence < 0) return visible_occurences;
        if (first_occurrence > last_occurrence) return visible_occurences;
        if (first_occurrence < 0) first_occurrence = 0;
        if (last_occurrence > (int)block.repeat_count) last_occurrence = block.repeat_count;
    }
    
    for (i32 n = first_occurrence; n <= last_occurrence; n++) {
        std::tuple<TimeAndDate, TimeAndDate> occurrence = block.get_occurrence(n);
        TimeAndDate display_start = std::get<0>(occurrence);
        TimeAndDate display_end = std::get<1>(occurrence);
        
        if (display_end <= start_day) continue; // skip the occurrence if it was only permitted due to the extraneous row
        if (display_start < start_day) display_start = start_day;
        if (display_end > end_day) display_end = end_day;
        
        visible_occurences.push_back(std::make_tuple(n, display_start, display_end));
    }
    
    return visible_occurences;
}

// Draw an instance of a block that has been rendered to the calendar display
void draw_block(const TimeAndDate& display_start, const TimeAndDate& display_end, const u32 color_choice) {
    bool render_select = ui_state == ViewingCalendar || ui_state == AddingStart || ui_state == AddingEnd;
    
    u32 start_day_of_week = display_start.get_day_of_week();
    u32 end_day_of_week = display_end.add_minutes(-1).get_day_of_week();
    for (u32 day_of_week = display_start.get_day_of_week(); day_of_week <= end_day_of_week; day_of_week++) {
        u32 x = calendar_time_margin + 1 + day_of_week * (calendar_day_width + 1);
        
        i32 start_row = 0;
        i32 end_row = calendar_scroll_height - 1;
        if (day_of_week == start_day_of_week) start_row = display_start.get_minute_of_day() / calendar_row_size;
        if (day_of_week == end_day_of_week) end_row = display_end.add_minutes(-1).get_minute_of_day() / calendar_row_size;
        
        if (start_row > calendar_scroll_offset) start_row -= calendar_scroll_offset;
        else start_row = 0;
        if (end_row < calendar_window_height - 2 + calendar_scroll_offset) end_row -= calendar_scroll_offset;
        else end_row = calendar_window_height - 3;
        
        if (start_row >= calendar_window_height - 2) continue;
        if (end_row < 0) continue;
        
        for (u32 row = start_row + 2; row <= end_row + 2; row++) {
            u32 color_id = color_choice;
            if (render_select && color_id < 8 && (row == calendar_selected_row - calendar_scroll_offset + 2) && (day_of_week == calendar_selected_day_of_week)) color_id += 8;
            wattron(calendar_window, COLOR_PAIR(color_id));
            mvwprintw(calendar_window, row, x, "%s", std::string(calendar_day_width, '#').c_str());
            wattroff(calendar_window, COLOR_PAIR(color_id));
        }
    }
}

// Draw the frame of the calendar display with no blocks added yet
void draw_calendar_frame(const bool render_select) {
    wattron(calendar_window, COLOR_PAIR(2));
    mvwprintw(calendar_window, 1, 0, pad_center(std::to_string(calendar_start_day.get_year()), 6).c_str());
    
    // print the days of the week
    for (int day_of_week = 0; day_of_week < 7; day_of_week++) {
        TimeAndDate day = calendar_start_day.add_days(day_of_week);
        u32 x = calendar_time_margin + 1 + day_of_week * (calendar_day_width + 1);
        mvwprintw(calendar_window, 0, x, "%s", pad_center(DAY_NAMES[day_of_week], calendar_day_width).c_str());
        let md = day.get_month_and_day();
        mvwprintw(calendar_window, 1, x, "%s", pad_center(MONTH_NAMES[md.month] + " " + std::to_string(md.day), calendar_day_width).c_str());
    }
    wattroff(calendar_window, COLOR_PAIR(2));
    
    // loop through the time intervals and days of the week
    // prints busy times as # and free times as -
    for (u32 row = 2; row < calendar_window_height; row++) {
        u32 minute = (row - 2 + calendar_scroll_offset) * calendar_row_size;
        mvwprintw(calendar_window, row, 0, "%2d:%02d", minute / 60, minute % 60);
        
        for (u32 day_of_week = 0; day_of_week < 7; day_of_week++) {
            u32 x = calendar_time_margin + 1 + day_of_week * (calendar_day_width + 1);
            bool selected = render_select && (row == calendar_selected_row - calendar_scroll_offset + 2) && (day_of_week == calendar_selected_day_of_week);
            
            if (selected) wattron(calendar_window, COLOR_PAIR(9));
            else wattron(calendar_window, COLOR_PAIR(1));
            mvwprintw(calendar_window, row, x, "%s", std::string(calendar_day_width, '-').c_str());
            if (selected) wattroff(calendar_window, COLOR_PAIR(9));
            else wattroff(calendar_window, COLOR_PAIR(1));
        }
    }
    
    i32 selected_row_draw = calendar_selected_row - calendar_scroll_offset + 2;
    if (render_select && selected_row_draw >= 2 && selected_row_draw < calendar_window_height) {
        u32 selected_x = calendar_time_margin + 1 + calendar_selected_day_of_week * (calendar_day_width + 1);
        wattron(calendar_window, COLOR_PAIR(10));
        mvwprintw(calendar_window, selected_row_draw, selected_x - 1, ">");
        mvwprintw(calendar_window, selected_row_draw, selected_x + calendar_day_width, "<");
        wattroff(calendar_window, COLOR_PAIR(10));
    }
}

// Draw the default user calendar display
void draw_calendar_week() {
    bool render_select = ui_state == ViewingCalendar || ui_state == AddingStart || ui_state == AddingEnd;
    
    draw_calendar_frame(render_select);
    
    TimeAndDate end_day = calendar_start_day.add_days(7);
    
    u32 color_choice = 3;
    
    for (const TimeBlock& block : calendar.busy_times) {
        std::vector<std::tuple<u32, TimeAndDate, TimeAndDate>> occurrences = render_block(block, calendar_start_day, end_day);
        for (auto occurrence : occurrences) {
            draw_block(std::get<1>(occurrence), std::get<2>(occurrence), color_choice);
        }
        color_choice += 1;
        if (color_choice > 7) color_choice = 3;
    }
}

// Find the render color from the availability of a time
u32 get_group_color_map(const u32 count) {
    const u32 max = group_calendars.size();
    
    if (count == max) return 3; // Green
    else if (count >= max * 4/5) return 5; // Yellow
    else if (count >= max * 1/2) return 7; // Red
    else return 1; // White
}

// Group calendar display
void draw_group_calendar_week() {
    draw_calendar_frame(true);
    
    TimeAndDate end_day = calendar_start_day.add_days(7);
    
    u32 counts[7][96] = {{ 0 }};
    u32 max = group_calendars.size();
    
    for (const auto& pair : group_calendars) {
        const std::string& user_name = std::get<0>(pair);
        const Calendar& user_calendar = std::get<1>(pair);
        
        for (const TimeBlock& block : user_calendar.busy_times) {
            std::vector<std::tuple<u32, TimeAndDate, TimeAndDate>> occurrences = render_block(block, calendar_start_day, end_day);
            for (auto occurrence : occurrences) {
                TimeAndDate display_start = std::get<1>(occurrence);
                TimeAndDate display_end = std::get<2>(occurrence);
                
                u32 start_day_of_week = display_start.get_day_of_week();
                u32 end_day_of_week = display_end.add_minutes(-1).get_day_of_week();
                for (u32 day_of_week = display_start.get_day_of_week(); day_of_week <= end_day_of_week; day_of_week++) {
                    i32 start_row = 0;
                    i32 end_row = calendar_scroll_height - 1;
                    if (day_of_week == start_day_of_week) start_row = display_start.get_minute_of_day() / calendar_row_size;
                    if (day_of_week == end_day_of_week) end_row = display_end.add_minutes(-1).get_minute_of_day() / calendar_row_size;
                    
                    for (u32 row = start_row; row <= end_row; row++) {
                        counts[day_of_week][row] += 1;
                    }
                }
            }
        }
    }
    
    for (u32 day_of_week = 0; day_of_week < 7; day_of_week++) {
        u32 x = calendar_time_margin + 1 + day_of_week * (calendar_day_width + 1);
        for (u32 row = 2; row < calendar_window_height; row++) {
            u32 color_id = get_group_color_map(max - counts[day_of_week][row + calendar_scroll_offset - 2]);
            if (color_id < 8 && (row == calendar_selected_row - calendar_scroll_offset + 2) && (day_of_week == calendar_selected_day_of_week)) color_id += 8;
            wattron(calendar_window, COLOR_PAIR(color_id));
            mvwprintw(calendar_window, row, x, "%s", std::string(calendar_day_width, '#').c_str());
            wattroff(calendar_window, COLOR_PAIR(color_id));
        }
    }
    
}

// Draws the list display screen
void draw_event_list() {
    const int margin = 20;
    
    wattron(calendar_window, COLOR_PAIR(2));
    mvwprintw(calendar_window, 1, margin, "Name");
    mvwprintw(calendar_window, 1, calendar_window_width - margin - 88, "12:00 Wednesday, September 29 2003       122 hours, 15 minutes       99999 times Monthly");
    wattroff(calendar_window, COLOR_PAIR(2));
    
    for (i32 row = 2; row - 2 + list_scroll_offset < calendar.busy_times.size(); row++) {
        const TimeBlock& block = calendar.busy_times[row - 2 + list_scroll_offset];
        i32 duration = block.duration();
        std::string duration_string, repeat_string;
        
        if (duration >= 60) {
            if (duration % 60 == 0) duration_string = std::to_string(duration / 60) + " hours";
            else duration_string = std::to_string(duration / 60) + " hours, " + std::to_string(duration % 60) + " minutes";
        } else duration_string = std::to_string(duration) + " minutes";
        
        if (block.repeat_period == NoRepeat) repeat_string = "Once";
        else repeat_string = std::to_string(block.repeat_count + 1) + " times " + repeat_type_to_string(block.repeat_period);
        
        u32 color_id = 1;
        if (row - 2 + list_scroll_offset == list_selected_index) color_id += 8;
        
        wattron(calendar_window, COLOR_PAIR(color_id));
        mvwprintw(calendar_window, row, margin, "%s", block.name.c_str());
        mvwprintw(calendar_window, row, calendar_window_width - margin - 88, "%s", block.start.to_string().c_str());
        mvwprintw(calendar_window, row, calendar_window_width - margin - 47, "%s", duration_string.c_str());
        mvwprintw(calendar_window, row, calendar_window_width - margin - 19, "%s", repeat_string.c_str());
        wattroff(calendar_window, COLOR_PAIR(color_id));
    }
    
    i32 selected_row_draw = list_selected_index - list_scroll_offset + 2;
    if (selected_row_draw >= 2 && selected_row_draw < calendar_window_height) {
        wattron(calendar_window, COLOR_PAIR(10));
        mvwprintw(calendar_window, selected_row_draw, margin - 2, ">");
        mvwprintw(calendar_window, selected_row_draw, calendar_window_width - (margin - 2), "<");
        wattroff(calendar_window, COLOR_PAIR(10));
    }
}


// Calendar view controls instructions
void draw_view_interaction() {
    mvwprintw(interact_window, 1, 3, "L: List View");
    mvwprintw(interact_window, 2, 3, "W/S: Scroll Up / Down");
    mvwprintw(interact_window, 3, 3, "A/D: Scroll Left / Right");
    mvwprintw(interact_window, 4, 3, "Arrow Keys: Move Selection Cursor");
    mvwprintw(interact_window, 5, 3, "N: Add a New Event");
    mvwprintw(interact_window, 6, 3, "Q: Save and Exit");
}
// List view controls instructions
void draw_list_interaction() {
    mvwprintw(interact_window, 1, 3, "C: Calendar View");
    mvwprintw(interact_window, 2, 3, "W/S: Scroll Up / Down");
    mvwprintw(interact_window, 3, 3, "Arrow Keys: Select Event");
    mvwprintw(interact_window, 4, 3, "Backspace: Delete Selected Event");
    mvwprintw(interact_window, 5, 3, "Q: Save and Exit");
}
// Confirming remove block dialogue
void draw_remove_confirm_interaction() {
    mvwprintw(interact_window, 1, 3, "Are you sure you want to delete this event?");
    mvwprintw(interact_window, 3, 3, "Backspace: Cancel");
    mvwprintw(interact_window, 4, 3, "Enter: Confirm");
    mvwprintw(interact_window, 5, 3, "Q: Save and Exit");
}
// Adding start time instructions
void draw_adding_start_interaction() {
    mvwprintw(interact_window, 1, 3, "Select the start time of the new event. Start time: %s", calendar_start_day.add_days(calendar_selected_day_of_week).add_minutes(calendar_selected_row * calendar_row_size).to_string().c_str());
    mvwprintw(interact_window, 3, 3, "Backspace: Cancel");
    mvwprintw(interact_window, 4, 3, "Enter: Confirm");
    mvwprintw(interact_window, 5, 3, "W/S: Scroll Up / Down");
    mvwprintw(interact_window, 6, 3, "A/D: Scroll Left / Right");
    mvwprintw(interact_window, 7, 3, "Arrow Keys: Move Selection Cursor");
    mvwprintw(interact_window, 8, 3, "Q: Save and Exit");
}
// Adding end time instructions
void draw_adding_end_interaction() {
    mvwprintw(interact_window, 1, 3, "Select the end time of the new event. Start time: %s", new_start_time.to_string().c_str());
    mvwprintw(interact_window, 2, 3, "                                        End time: %s", calendar_start_day.add_days(calendar_selected_day_of_week).add_minutes((calendar_selected_row + 1) * calendar_row_size).to_string().c_str());
    mvwprintw(interact_window, 3, 3, "Backspace: Go Back");
    mvwprintw(interact_window, 4, 3, "Enter: Confirm");
    mvwprintw(interact_window, 5, 3, "W/S: Scroll Up / Down");
    mvwprintw(interact_window, 6, 3, "A/D: Scroll Left / Right");
    mvwprintw(interact_window, 7, 3, "Arrow Keys: Move Selection Cursor");
    mvwprintw(interact_window, 8, 3, "Q: Save and Exit");
}
// Selecting repeat type instructions
void draw_adding_repeat_type_interaction() {
    mvwprintw(interact_window, 1, 3, "Choose a repeat frequency.");
    mvwprintw(interact_window, 3, 3, "O/N: One Time Event");
    mvwprintw(interact_window, 4, 3, "D: Daily");
    mvwprintw(interact_window, 5, 3, "W: Weekly");
    mvwprintw(interact_window, 6, 3, "M: Monthly");
    mvwprintw(interact_window, 7, 3, "Y: Yearly");
    mvwprintw(interact_window, 8, 3, "Backspace: Go Back");
    mvwprintw(interact_window, 9, 3, "Q: Save and Exit");
}
// Adding repeat count instructions
void draw_adding_repeat_count_interaction() {
    mvwprintw(interact_window, 1, 3, "Choose the number of repetitions. Repeat count: %d    Last occurrance: %s",
        new_repeat_count + 1,
        std::get<0>(TimeBlock("", new_start_time, new_end_time, new_repeat_type, new_repeat_count).get_occurrence(new_repeat_count)).to_string().c_str()
    );
    mvwprintw(interact_window, 3, 3, "Arrow Keys: Select Repeat Count");
    mvwprintw(interact_window, 4, 3, "Backspace: Go Back");
    mvwprintw(interact_window, 5, 3, "Enter: Create Event");
    mvwprintw(interact_window, 6, 3, "W/S: Scroll Up / Down");
    mvwprintw(interact_window, 7, 3, "A/D: Scroll Left / Right");
    mvwprintw(interact_window, 8, 3, "Q: Save and Exit");
}
// Group calendar view controls instructions and color key
void draw_group_calendar_interaction() {
    mvwprintw(interact_window, 1, 3, "Group calendar for %s", active_group_name.c_str());
    
    u32 max = group_calendars.size();
    u32 space = 8;
    wattron(interact_window, COLOR_PAIR(3));
    mvwprintw(interact_window, 3, 8, "All %d Available", max);
    wattroff(interact_window, COLOR_PAIR(3));
    wattron(interact_window, COLOR_PAIR(5));
    mvwprintw(interact_window, 3, 24 + space, "%d to %d Available", max - 1, max * 4/5);
    wattroff(interact_window, COLOR_PAIR(5));
    wattron(interact_window, COLOR_PAIR(7));
    mvwprintw(interact_window, 3, 42 + 2*space, "%d to %d Available", max * 4/5 - 1, max * 1/2);
    wattroff(interact_window, COLOR_PAIR(7));
    wattron(interact_window, COLOR_PAIR(1));
    mvwprintw(interact_window, 3, 60 + 3*space, "Fewer than %d Available", max * 1/2);
    wattroff(interact_window, COLOR_PAIR(1));
    
    mvwprintw(interact_window, 5, 3, "W/S: Scroll Up / Down");
    mvwprintw(interact_window, 6, 3, "A/D: Scroll Left / Right");
    mvwprintw(interact_window, 7, 3, "Arrow Keys: Move Selection Cursor");
    mvwprintw(interact_window, 8, 3, "Q: Exit");
}




// Scroll the view to the current repeat count selection
void focus_repeat_count_select() {
    TimeAndDate focus = std::get<0>(TimeBlock("", new_start_time, new_end_time, new_repeat_type, new_repeat_count).get_occurrence(new_repeat_count));
    calendar_start_day = focus.replace_time(0).add_days(-static_cast<int>(focus.get_day_of_week()));
    calendar_selected_row = focus.get_minute_of_day() / calendar_row_size;
    calendar_selected_day_of_week = focus.get_day_of_week();
    if (calendar_scroll_offset > calendar_selected_row) {
        calendar_scroll_offset = calendar_selected_row;
    } else if (calendar_scroll_offset <= (int)(calendar_selected_row - (calendar_window_height - 2))) {
        calendar_scroll_offset = calendar_selected_row - (calendar_window_height - 2) + 1;
    }
}

// Adjust the display parameters to match a new screen size
void on_resize() {
    screen_height = getmaxy(stdscr);
    screen_width = getmaxx(stdscr);
    calendar_window_height = screen_height * 3/4;
    calendar_window_width = screen_width;
    interact_window_height = screen_height - calendar_window_height;
    interact_window_width = screen_width;
    
    wresize(calendar_window, calendar_window_height, calendar_window_width);
    wresize(interact_window, interact_window_height, interact_window_width);
    mvwin(calendar_window, 0, 0);
    mvwin(interact_window, calendar_window_height, 0);
    
    calendar_day_width = (calendar_window_width - calendar_time_margin - 8) / 7;
    
    if (calendar_scroll_offset > calendar_scroll_height - (calendar_window_height - 2)) {
        calendar_scroll_offset = calendar_scroll_height - (calendar_window_height - 2);
    }
}

// Set up the windows and display parameters
void initialize_window() {
    //initscr();
    noecho();
    cbreak();
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(5, COLOR_YELLOW, COLOR_BLACK);
    init_pair(6, COLOR_BLUE, COLOR_BLACK);
    init_pair(7, COLOR_RED, COLOR_BLACK);
    
    init_pair(9, COLOR_BLACK, COLOR_WHITE);
    init_pair(10, COLOR_BLACK, COLOR_CYAN);
    init_pair(11, COLOR_WHITE, COLOR_GREEN);
    init_pair(12, COLOR_WHITE, COLOR_MAGENTA);
    init_pair(13, COLOR_WHITE, COLOR_YELLOW);
    init_pair(14, COLOR_WHITE, COLOR_BLUE);
    init_pair(15, COLOR_WHITE, COLOR_RED);
    
    calendar_window = newwin(0, 0, 0, 0);
    interact_window = newwin(0, 0, 0, 0);
    keypad(interact_window, TRUE); // Enable keypad input for interaction window
    nodelay(interact_window, TRUE); // Make character reads non-blocking
    //box(interact_window, 0, 0);
    
    on_resize();
    
    let now = TimeAndDate::now();
    
    calendar_selected_row = now.get_minute_of_day() / calendar_row_size;
    calendar_selected_day_of_week = now.get_day_of_week();
    
    calendar_scroll_offset = calendar_selected_row - 4;
    
    calendar_start_day = now.replace_time(0).add_days(-static_cast<int>(now.get_day_of_week()));
}

// Main loop handles input, invariants, and drawing
void editor_main_loop() {
    
    bool redraw = true;
    
    int character;
    while (true) {
        character = wgetch(interact_window);
        
        if (character == 'q') {
            break;
        }
        
        
        // Inputs handled as transitions between states in a FSM in this switch statement
        switch (character) {
            case ERR:
                if (getmaxy(stdscr) != screen_height || getmaxx(stdscr) != screen_width) {
                    on_resize();
                    redraw = true;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                break;
            case 'l':
                switch (ui_state) {
                    case ViewingCalendar:
                        ui_state = ViewingList;
                        redraw = true; break;
                } break;
            case 'c':
                switch (ui_state) {
                    case ViewingList:
                        ui_state = ViewingCalendar;
                        redraw = true; break;
                } break;
            case 'n':
                switch (ui_state) {
                    case ViewingCalendar:
                    case ViewingList:
                        ui_state = AddingStart;
                        redraw = true; break;
                    case AddingRepeatType:
                        new_repeat_type = NoRepeat;
                        new_repeat_count = 0;
                        // todo read new block name
                        calendar.busy_times.push_back(TimeBlock("New Block", new_start_time, new_end_time, new_repeat_type, new_repeat_count));
                        ui_state = ViewingCalendar;
                        redraw = true; break;
                } break;
            case KEY_BACKSPACE:
                switch (ui_state) {
                    case ViewingList:
                        if (calendar.busy_times.size() > 0) ui_state = RemoveConfirm;
                        redraw = true; break;
                    case RemoveConfirm:
                        ui_state = ViewingList;
                        redraw = true; break;
                    case AddingRepeatCount:
                        ui_state = AddingRepeatType;
                        redraw = true; break;
                    case AddingRepeatType:
                        ui_state = AddingEnd;
                        redraw = true; break;
                    case AddingEnd:
                        ui_state = AddingStart;
                        redraw = true; break;
                    case AddingStart:
                        ui_state = ViewingCalendar;
                        redraw = true; break;
                } break;
            case '\n':
                switch (ui_state) {
                    case AddingStart:
                        ui_state = AddingEnd;
                        new_start_time = calendar_start_day.add_days(calendar_selected_day_of_week).add_minutes(calendar_selected_row * calendar_row_size);
                        calendar_selected_row += 3;
                        redraw = true; break;
                    case AddingEnd:
                        new_end_time = calendar_start_day.add_days(calendar_selected_day_of_week).add_minutes((calendar_selected_row + 1) * calendar_row_size);
                        ui_state = AddingRepeatType;
                        redraw = true; break;
                    case AddingRepeatCount:
                        // todo read new block name
                        calendar.busy_times.push_back(TimeBlock("New Block", new_start_time, new_end_time, new_repeat_type, new_repeat_count));
                        ui_state = ViewingCalendar;
                        redraw = true; break;
                    case RemoveConfirm:
                        calendar.busy_times.erase(calendar.busy_times.begin() + list_selected_index);
                        ui_state = ViewingList;
                        redraw = true; break;
                } break;
            case 'y':
                switch (ui_state) {
                    case AddingRepeatType:
                        new_repeat_type = Yearly;
                        new_repeat_count = 1;
                        ui_state = AddingRepeatCount;
                        redraw = true; break;
                } break;
            case 'm':
                switch (ui_state) {
                    case AddingRepeatType:
                        new_repeat_type = Monthly;
                        new_repeat_count = 1;
                        ui_state = AddingRepeatCount;
                        redraw = true; break;
                } break;
            case 'o':
                switch (ui_state) {
                    case AddingRepeatType:
                        new_repeat_type = NoRepeat;
                        new_repeat_count = 0;
                        // todo read new block name
                        calendar.busy_times.push_back(TimeBlock("New Block", new_start_time, new_end_time, new_repeat_type, new_repeat_count));
                        ui_state = ViewingCalendar;
                        redraw = true; break;
                } break;
            
            case 'w':
                switch (ui_state) {
                    case ViewingCalendar:
                    case AddingStart:
                    case AddingEnd:
                    case AddingRepeatCount:
                    case GroupCalendar:
                        calendar_scroll_offset -= 1;
                        redraw = true; break;
                    case ViewingList:
                        list_scroll_offset -= 1;
                        redraw = true; break;
                    case AddingRepeatType:
                        new_repeat_type = Weekly;
                        new_repeat_count = 1;
                        focus_repeat_count_select();
                        ui_state = AddingRepeatCount;
                        redraw = true; break;
                } break;
            case 's':
                switch (ui_state) {
                    case ViewingCalendar:
                    case AddingStart:
                    case AddingEnd:
                    case AddingRepeatCount:
                    case GroupCalendar:
                        calendar_scroll_offset += 1;
                        redraw = true; break;
                    case ViewingList:
                        list_scroll_offset += 1;
                        redraw = true; break;
                } break;
            case 'a':
                switch (ui_state) {
                    case ViewingCalendar:
                    case AddingStart:
                    case AddingEnd:
                    case AddingRepeatCount:
                    case GroupCalendar:
                        calendar_start_day = calendar_start_day.add_days(-7);
                        redraw = true; break;
                } break;
            case 'd':
                switch (ui_state) {
                    case ViewingCalendar:
                    case AddingStart:
                    case AddingEnd:
                    case AddingRepeatCount:
                    case GroupCalendar:
                        calendar_start_day = calendar_start_day.add_days(7);
                        redraw = true; break;
                    case AddingRepeatType:
                        new_repeat_type = Daily;
                        new_repeat_count = 1;
                        ui_state = AddingRepeatCount;
                        redraw = true; break;
                } break;
            case KEY_UP:
                switch (ui_state) {
                    case ViewingCalendar:
                    case AddingStart:
                    case AddingEnd:
                    case GroupCalendar:
                        calendar_selected_row -= 1;
                        if (calendar_scroll_offset > calendar_selected_row) {
                            calendar_scroll_offset = calendar_selected_row;
                        } else if (calendar_scroll_offset <= (int)(calendar_selected_row - (calendar_window_height - 2))) {
                            calendar_scroll_offset = calendar_selected_row - (calendar_window_height - 2) + 1;
                        }
                        redraw = true; break;
                    case ViewingList:
                        list_selected_index -= 1;
                        if (list_scroll_offset > list_selected_index) {
                            list_scroll_offset = list_selected_index;
                        } else if (list_scroll_offset <= (int)(list_selected_index - (calendar_window_height - 2))) {
                            list_scroll_offset = list_selected_index - (calendar_window_height - 2) + 1;
                        }
                        redraw = true; break;
                } break;
            case KEY_DOWN:
                switch (ui_state) {
                    case ViewingCalendar:
                    case AddingStart:
                    case AddingEnd:
                    case GroupCalendar:
                        calendar_selected_row += 1;
                        if (calendar_scroll_offset > calendar_selected_row) {
                            calendar_scroll_offset = calendar_selected_row;
                        } else if (calendar_scroll_offset <= (int)(calendar_selected_row - (calendar_window_height - 2))) {
                            calendar_scroll_offset = calendar_selected_row - (calendar_window_height - 2) + 1;
                        }
                        redraw = true; break;
                    case ViewingList:
                        list_selected_index += 1;
                        if (list_scroll_offset > list_selected_index) {
                            list_scroll_offset = list_selected_index;
                        } else if (list_scroll_offset <= (int)(list_selected_index - (calendar_window_height - 2))) {
                            list_scroll_offset = list_selected_index - (calendar_window_height - 2) + 1;
                        }
                        redraw = true; break;
                } break;
            case KEY_LEFT:
                switch (ui_state) {
                    case ViewingCalendar:
                    case AddingStart:
                    case AddingEnd:
                    case GroupCalendar:
                        calendar_selected_day_of_week -= 1;
                        redraw = true; break;
                    case AddingRepeatCount:
                        new_repeat_count -= 1;
                        if (new_repeat_count < 1) new_repeat_count = 1;
                        focus_repeat_count_select();
                        redraw = true; break;
                } break;
            case KEY_RIGHT:
                switch (ui_state) {
                    case ViewingCalendar:
                    case AddingStart:
                    case AddingEnd:
                    case GroupCalendar:
                        calendar_selected_day_of_week += 1;
                        redraw = true; break;
                    case AddingRepeatCount:
                        new_repeat_count += 1;
                        focus_repeat_count_select();
                        redraw = true; break;
                } break;
        }
        
        
        if (!redraw) continue;
        redraw = false;
        
        
        // Enforce invariants, or make sure all drawing parameters have valid values
        
        if (calendar_selected_row < 0) {
            calendar_selected_row += calendar_scroll_height;
            calendar_selected_day_of_week -= 1;
            calendar_scroll_offset = calendar_scroll_height - (calendar_window_height - 2);
        } else if (calendar_selected_row >= calendar_scroll_height) {
            calendar_selected_row = 0;
            calendar_selected_day_of_week += 1;
            calendar_scroll_offset = 0;
        }
        
        if (calendar_selected_day_of_week < 0) {
            calendar_start_day = calendar_start_day.add_days(-7);
            calendar_selected_day_of_week += 7;
        } else if (calendar_selected_day_of_week >= 7) {
            calendar_start_day = calendar_start_day.add_days(7);
            calendar_selected_day_of_week -= 7;
        }
        
        list_scroll_height = calendar.busy_times.size();
        
        if (list_selected_index < 0) list_selected_index = 0;
        else if (list_selected_index >= list_scroll_height) list_selected_index = list_scroll_height - 1;
        
        if ((calendar_window_height - 2) >= calendar_scroll_height) calendar_scroll_offset = 0;
        else if (calendar_scroll_offset < 0) calendar_scroll_offset = 0;
        else if (calendar_scroll_offset > calendar_scroll_height - (calendar_window_height - 2)) calendar_scroll_offset = calendar_scroll_height - (calendar_window_height - 2);
        
        if ((calendar_window_height - 2) >= list_scroll_height) list_scroll_offset = 0;
        else if (list_scroll_offset < 0) list_scroll_offset = 0;
        else if (list_scroll_offset > list_scroll_height - (calendar_window_height - 2)) list_scroll_offset = list_scroll_height - (calendar_window_height - 2);
        
        
        
        // Draw based on current menu state
        
        wclear(calendar_window);
        wclear(interact_window);
        
        std::vector<std::tuple<u32, TimeAndDate, TimeAndDate>> occurrences;
        switch (ui_state) {
            case ViewingCalendar:
                draw_calendar_week();
                draw_view_interaction();
                break;
            case ViewingList:
                draw_event_list();
                draw_list_interaction();
                break;
            case AddingStart:
                draw_calendar_week();
                draw_adding_start_interaction();
                break;
            case AddingEnd:
                draw_calendar_week();
                draw_block(new_start_time, calendar_start_day.add_days(calendar_selected_day_of_week).add_minutes((calendar_selected_row + 1) * calendar_row_size), 14);
                draw_adding_end_interaction();
                break;
            case AddingRepeatType:
                draw_calendar_week();
                draw_block(new_start_time, calendar_start_day.add_days(calendar_selected_day_of_week).add_minutes((calendar_selected_row + 1) * calendar_row_size), 14);
                draw_adding_repeat_type_interaction();
                break;
            case AddingRepeatCount:
                draw_calendar_week();
                occurrences = render_block(
                    TimeBlock("", new_start_time, new_end_time, new_repeat_type, new_repeat_count),
                    calendar_start_day,
                    calendar_start_day.add_days(7)
                );
                for (auto occurrence : occurrences) {
                    draw_block(std::get<1>(occurrence), std::get<2>(occurrence), 14);
                }
                draw_adding_repeat_count_interaction();
                break;
            case RemoveConfirm:
                draw_event_list();
                draw_remove_confirm_interaction();
                break;
            case GroupCalendar:
                draw_group_calendar_week();
                draw_group_calendar_interaction();
        }
        
        wrefresh(calendar_window);
        wrefresh(interact_window);
        
    }
    

    delwin(calendar_window);
    delwin(interact_window);
    //endwin();
}


// Entry point functions to be called by the parent program

void transfer_to_calendar_editor() {
    initialize_window();
    ui_state = ViewingCalendar;
    editor_main_loop();
}


void transfer_to_group_calendar_view() {
    initialize_window();
    ui_state = GroupCalendar;
    editor_main_loop();
}



