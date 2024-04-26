#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <thread>
#include <ncurses.h>

#include "../timeanddate.hpp"
#include "../calendar.hpp"


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


inline std::string pad_center(std::string s, u32 width) {
    return std::string((width - s.length()) / 2, ' ') + s;
}


void draw_calendar_week(const TimeAndDate& start_day, const Calendar& my_calendar, const u32 scroll_offset) {
    wclear(calendar_window);
    
    wattron(calendar_window, COLOR_PAIR(1));
    mvwprintw(calendar_window, 1, 0, "Date");
    
    // print the days of the week
    for (int day_of_week = 0; day_of_week < 7; day_of_week++) {
        TimeAndDate day = start_day.add_days(day_of_week);
        u32 x = calendar_time_margin + 1 + day_of_week * (calendar_day_width + 1);
        mvwprintw(calendar_window, 0, x, "%s", pad_center(DAY_NAMES[day_of_week], calendar_day_width).c_str());
        let md = day.get_month_and_day();
        mvwprintw(calendar_window, 1, x, "%s", pad_center(MONTH_NAMES[md.month].substr(0, 3) + " " + std::to_string(md.day), calendar_day_width).c_str());
    }
    wattroff(calendar_window, COLOR_PAIR(1));
    
    // loop through the time intervals and days of the week
    // prints busy times as # and free times as -
    for (u32 row = 0; row < calendar_window_height - 2; row++) {
        u32 minute = (row + scroll_offset) * 15;
        mvwprintw(calendar_window, row + 2, 0, "%2d:%02d", minute / 60, minute % 60);
        
        for (int day_of_week = 0; day_of_week < 7; day_of_week++) {
            u32 x = calendar_time_margin + 1 + day_of_week * (calendar_day_width + 1);
            
            TimeAndDate currentTime = start_day.add_days(day_of_week).replace_time(minute);
            
            if (my_calendar.is_time_block_busy(currentTime)) {
                wattron(calendar_window, COLOR_PAIR(1));
                mvwprintw(calendar_window, row + 2, x, "%s", std::string(calendar_day_width, '#').c_str());
                wattroff(calendar_window, COLOR_PAIR(1));
            } else {
                wattron(calendar_window, COLOR_PAIR(2));
                mvwprintw(calendar_window, row + 2, x, "%s", std::string(calendar_day_width, '-').c_str());
                wattroff(calendar_window, COLOR_PAIR(2));
            }
        }
    }
    
    wrefresh(calendar_window);
}

// draws the time block removal screen with the selected time block highlighted
void draw_remove_calendar(Calendar& my_calendar, int selected_index) {
    wclear(calendar_window);
    
    wattron(calendar_window, COLOR_PAIR(1));
    mvwprintw(calendar_window, 1, 0, "Date");
    // for (int i = 0; i < days_of_week; ++i) {
    //     TimeAndDate day = start.add_days(i);
    //     mvwprintw(calendar_window, 1, 5 + i * 20, "%s", day.to_string().c_str());
    // }
    wattroff(calendar_window, COLOR_PAIR(1));
    
    // print the time blocks
    int line = 2;
    for (int i = 0; i < my_calendar.busy_times.size(); i++) {
        int x = 5;
        TimeBlock& block = my_calendar.busy_times[i];
        std::string display = block.start.to_string() + " - " + block.end.to_string();
        if (i == selected_index) {
            wattron(calendar_window, A_REVERSE);
        }
        mvwprintw(calendar_window, line++, x, "%s", display.c_str());
        if (i == selected_index) {
            wattroff(calendar_window, A_REVERSE);
        }
    }
    
    wrefresh(calendar_window);
}

// draw the controls to the window
void draw_interactions() {
    mvwprintw(interact_window, 1, 1, "Press F1 to exit or 'j' and 'k' to scroll the above window.");
    mvwprintw(interact_window, 2, 1, "Press A/D to scroll Left/Right in the calendar.");
    mvwprintw(interact_window, 3, 1, "1) Add Time Block");
    mvwprintw(interact_window, 4, 1, "2) Remove Time Block");
    
    wrefresh(interact_window);
}

void draw_remove_interactions(Calendar& my_calendar) {
    int selected_index = 0;
    int character;
    bool running = true;
    while (running) {
        wclear(interact_window);
        mvwprintw(interact_window, 1, 1, "Navigate with UP/DOWN arrows. Press ENTER to delete. Press 'q' to quit.");
        wrefresh(interact_window);
        character = wgetch(interact_window);
        
        switch (character) {
            case KEY_UP:
                if (selected_index > 0) selected_index -= 1;
                break;
            case KEY_DOWN:
                if (selected_index < my_calendar.busy_times.size() - 1) {
                    selected_index += 1;
                }
                break;
            case '\n':  // User confirms deletion
                my_calendar.busy_times.erase(my_calendar.busy_times.begin() + selected_index);
                if (selected_index > my_calendar.busy_times.size()) {
                    selected_index = fmax(0, my_calendar.busy_times.size() - 1);
                }
                break;
            case 'q':  // Exit loop
                running = false;
                break;
        }
        draw_remove_calendar(my_calendar, selected_index);
    }
}


// Convert a string to a TimeAndDate object
TimeAndDate convert_string_to_time(char time_string[22]) {
    int hour, minute, month, day, year;
    sscanf(time_string, "%d:%d %d %d %d", &hour, &minute, &month, &day, &year);
    
    return TimeAndDate::build_from_month(hour * 60 + minute, day, static_cast<Month>(month - 1), year);
}

// ask user for a time given as a string
std::string prompt_user_for_time() {
    wclear(interact_window);
    mvwprintw(interact_window, 1, 1, "Enter the time block start Time: 24-hour time:minutes Month(Numerical) Day Year (press Enter when done): ");
    wrefresh(interact_window);
    
    char time_string[22] = {0};  // Buffer to hold the input string
    
    wmove(interact_window, 2, 0);
    wgetnstr(interact_window, time_string, 22);  
    
    // Optional: display the input for confirmation
    mvwprintw(interact_window, 3, 0, "You entered: %s", time_string);
    wrefresh(interact_window);
    return std::string(time_string);
}

// ask user for repeat type of the block
char prompt_user_for_repeat() {
    wclear(interact_window);
    mvwprintw(interact_window, 1, 1, "Please enter the repeat type of this block:\n\t\'N\' - None\n\t\'D\' - Daily\n\t\'W\' - Weekly\n\t\'M\' - Monthly\n\t\'Y\' - Yearly");
    wrefresh(interact_window);
    
    char repeat[1] = {0};  // Buffer to hold the input string
    
    wmove(interact_window, 7, 0);
    wgetnstr(interact_window, repeat, 1);  
    
    return repeat[0];
}

// ask user for time block repeat interval
int prompt_user_for_repeat_interval() {
    wclear(interact_window);
    mvwprintw(interact_window, 1, 1, "Please enter the number of repetitions for this block: ");
    wrefresh(interact_window);
    
    char repeat[3] = {0};  // Buffer to hold the input string
    
    wmove(interact_window, 7, 0);
    wgetnstr(interact_window, repeat, 10);
    
    int repeat_interval = atoi(repeat);
    return repeat_interval;
}

// add time block to calendar
TimeBlock run_add_block() {
    echo();  // Enable echoing of characters typed by the user
    keypad(interact_window, TRUE);  // Enable keypad for the window to handle function keys
    
    // Clear the window and prepare for input
    char start_time_string[22];
    strcpy(start_time_string, prompt_user_for_time().c_str());
    wclear(interact_window);
    char end_time_string[22];
    strcpy(end_time_string, prompt_user_for_time().c_str());
    wclear(interact_window);
    
    
    // Optional: display the input for confirmation
    wrefresh(interact_window);
    
    // Wait for a key press before continuing
    TimeAndDate start_time = convert_string_to_time(start_time_string);
    TimeAndDate end_time = convert_string_to_time(end_time_string);
    char repeat = prompt_user_for_repeat();
    unsigned int repeat_interval = 0;
    RepeatType repeat_type;
    
    switch (repeat) {
        case 'N':
            repeat_type = RepeatType::NoRepeat;
            break;
        case 'D':
            repeat_type = RepeatType::Daily;
            break;
        case 'W':
            repeat_type = RepeatType::Weekly;
            break;
        case 'M':
            repeat_type = RepeatType::Monthly;
            break;
        case 'Y':
            repeat_type = RepeatType::Yearly;
            break;
        default:
            repeat_type = RepeatType::NoRepeat;
            break;
    }
    
    if (repeat != 'N') {
        repeat_interval = prompt_user_for_repeat_interval();
    }
    
    let new_block = TimeBlock(start_time, end_time, repeat_type, repeat_interval);
    wclear(interact_window);
    mvwprintw(interact_window, 1, 0, "%s", new_block.encode().c_str());
    
    wgetch(interact_window);
    
    noecho();
    return new_block;
}


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
}



int main() {
    initscr();
    noecho();
    cbreak();
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);
    
    calendar_window = newwin(0, 0, 0, 0);
    interact_window = newwin(0, 0, 0, 0);
    scrollok(calendar_window, TRUE); // Enable scrolling for the calendar window
    keypad(interact_window, TRUE); // Enable keypad input for interaction window
    nodelay(interact_window, TRUE);
    box(interact_window, 0, 0);
    
    on_resize();
    
    u32 scroll_count = 0;
    let my_calendar = Calendar();
    let now = TimeAndDate::now();
    
    my_calendar.busy_times.push_back(TimeBlock(
        now.replace_time(1 * 60),
        now.replace_time(4 * 60),
        RepeatType::NoRepeat,
        0
    ));
    
    my_calendar.busy_times.push_back(TimeBlock(
        now.replace_time(12 * 60),
        now.replace_time(15 * 60),
        RepeatType::Weekly,
        12
    ));
    
    let start_day = now.add_days(-static_cast<int>(now.get_day_of_week()));
    
    draw_calendar_week(start_day, my_calendar, scroll_count);
    draw_interactions();
    u32 displayable_rows = calendar_window_height - 2;

    // main calendar controls
    TimeBlock new_time;
    int character;
    while (true) {
        character = wgetch(interact_window);
        
        if (character == ERR) {
            if (getmaxy(stdscr) != screen_height || getmaxx(stdscr) != screen_width) {
                on_resize();
                
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } else if (character == 'q') {
            break;
        }
        
        switch (character) {
            case 'k':
                if (scroll_count <= 24 * 4 + 1 - getmaxy(calendar_window)) {
                    scroll_count += 1;
                    draw_calendar_week(start_day, my_calendar, scroll_count);
                    wrefresh(calendar_window);
                }
                break;
            case 'j':
                if (scroll_count > 0) {
                    scroll_count -= 1;
                    draw_calendar_week(start_day, my_calendar, scroll_count);
                    wrefresh(calendar_window);
                }
                break;
            case 'a':
                start_day = start_day.add_days(-7);
                draw_calendar_week(start_day, my_calendar, 0);
                wrefresh(calendar_window);
                break;
            case 'd':
                start_day = start_day.add_days(7);
                draw_calendar_week(start_day, my_calendar, 0);
                wrefresh(calendar_window);
                break;
            case '1':
                new_time = run_add_block();
                my_calendar.add_time(new_time);
                wrefresh(calendar_window);
                draw_interactions();
                break;
            case '2':
                draw_remove_calendar(my_calendar, 0);
                draw_remove_interactions(my_calendar);
                break;
        }
    }
    

    delwin(calendar_window);
    delwin(interact_window);
    endwin();
    return 0;
}



