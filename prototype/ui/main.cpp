#include <ncurses.h>
#include "../timeanddate.hpp"
#include "../calendar.hpp"
#include <string.h>
#include <vector>
#include <algorithm>
#include <iostream>

// create a new window with defined x, y, width, and height
WINDOW* create_window(int height, int width, int start_y, int start_x) {
    WINDOW* local_window = newwin(height, width, start_y, start_x);
    box(local_window, 0, 0);
    wrefresh(local_window);
    return local_window;
}

void destroy_window(WINDOW* local_win) {
    werase(local_win);
    wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
    wrefresh(local_win);
    delwin(local_win);
}


void draw_calendar_week(WINDOW* window, TimeAndDate start_day, Calendar my_calendar, u32 scroll_offset) {
    wclear(window);
    // const int minutes_intervals = 96; // 15-minute intervals in a 24-hour day
    const u32 max_rows = getmaxy(window) - 2;  // Rows available for time slots
    wattron(window, COLOR_PAIR(1));
    mvwprintw(window, 1, 0, "Time");
    
    // print the days of the week
    for (int i = 0; i < 7; i++) {
        TimeAndDate day = start_day.add_days(i);
        mvwprintw(window, 1, 5 + i * 20, "%s", day.to_string().c_str());
    }
    wattroff(window, COLOR_PAIR(1));
    
    // loop through the time intervals and days of the week
    // prints busy times as # and free times as -
    for (int minutes_interval = 0; minutes_interval < max_rows; minutes_interval++) {
        for (int day_of_week = 0; day_of_week < 7; day_of_week++) {
            int x = 5 + day_of_week * 20; // 20 spaces per column for spacing
            int time_of_day = (scroll_offset + minutes_interval) * 15; // Time in minutes from midnight
            if (day_of_week == 0) {
                mvwprintw(window, 2 + minutes_interval, 0, "%02d:%02d ", time_of_day / 60, time_of_day % 60);
            }
            
            TimeAndDate currentTime = start_day.add_days(day_of_week).add_minutes(time_of_day);
            bool is_busy = my_calendar.is_time_block_busy(currentTime);
            
            wattron(window, COLOR_PAIR(is_busy ? 1 : 2));
            mvwprintw(window, 2 + minutes_interval, x, "%s", is_busy ? "###" : "---");
            wattroff(window, COLOR_PAIR(is_busy ? 1 : 2));
        }
    }
    
    wrefresh(window);
}

// draws the time block removal screen with the selected time block highlighted
void draw_remove_calendar(WINDOW* window, Calendar& my_calendar, int selected_index) {
    wclear(window);

    // print the days of the week
    wattron(window, COLOR_PAIR(1));
    mvwprintw(window, 1, 0, "Time");
    // for (int i = 0; i < days_of_week; ++i) {
    //     TimeAndDate day = start.add_days(i);
    //     mvwprintw(window, 1, 5 + i * 20, "%s", day.to_string().c_str());
    // }
    wattroff(window, COLOR_PAIR(1));

    // print the time blocks
    int line = 2;
    for (int i = 0; i < static_cast<int>(my_calendar.busy_times.size()); ++i) {
        int x = 5;
        TimeBlock& block = my_calendar.busy_times[i];
        std::string display = block.start.to_string() + " - " + block.end.to_string();
        if (i == selected_index) {
            wattron(window, A_REVERSE);
        }
        mvwprintw(window, line++, x, "%s", display.c_str());
        if (i == selected_index) {
            wattroff(window, A_REVERSE);
        }
    }

    wrefresh(window);
}

// draw the controls to the window
void draw_interactions(WINDOW* window) {
    mvwprintw(window, 1, 0, "Press F1 to exit or 'j' and 'k' to scroll the above window.");
    mvwprintw(window, 2, 0, "Press A/D to scroll Left/Right in the calendar.");
    mvwprintw(window, 3, 0, "1) Add Time Block");
    mvwprintw(window, 4, 0, "2) Remove Time Block");
    
    wrefresh(window);
}

void draw_remove_interactions(WINDOW* interact_windowdow, WINDOW* calendar_window, Calendar& my_calendar) {
    int selected_index = 0;
    int character;
    bool running = true;
    while (running) {
        wclear(interact_windowdow);
        mvwprintw(interact_windowdow, 1, 0, "Navigate with UP/DOWN arrows. Press ENTER to delete. Press 'q' to quit.");
        wrefresh(interact_windowdow);
        character = wgetch(interact_windowdow);
        
        switch (character) {
            case KEY_UP:
                if (selected_index > 0) selected_index--;
                break;
            case KEY_DOWN:
                if (selected_index < static_cast<int>(my_calendar.busy_times.size()) - 1) {
                    selected_index++;
                }
                break;
            case '\n':  // User confirms deletion
                my_calendar.busy_times.erase(my_calendar.busy_times.begin() + selected_index);
                if (selected_index > static_cast<int>(my_calendar.busy_times.size())) {
                    selected_index = fmax(0, int(my_calendar.busy_times.size()) - 1);
                }
                break;
            case 'q':  // Exit loop
                running = false;
                break;
        }
        draw_remove_calendar(calendar_window, my_calendar, selected_index);
    }
}


// Convert a string to a TimeAndDate object
TimeAndDate convert_string_to_time(char time_string[22]) {
    int hour, minute, month, day, year;
    sscanf(time_string, "%d:%d %d %d %d", &hour, &minute, &month, &day, &year);
    
    return TimeAndDate::build_from_month(hour * 60 + minute, day, static_cast<Month>(month - 1), year);
}

// ask user for a time given as a string
std::string prompt_user_for_time(WINDOW* window) {
    wclear(window);
    mvwprintw(window, 1, 0, "Enter the time block start Time: 24-hour time:minutes Month(Numerical) Day Year (press Enter when done): ");
    wrefresh(window);
    
    char time_string[22] = {0};  // Buffer to hold the input string
    
    wmove(window, 2, 0);
    wgetnstr(window, time_string, 22);  
    
    // Optional: display the input for confirmation
    mvwprintw(window, 3, 0, "You entered: %s", time_string);
    wrefresh(window);
    return std::string(time_string);
}

// ask user for repeat type of the block
char prompt_user_for_repeat(WINDOW* window) {
    wclear(window);
    mvwprintw(window, 1, 0, "Please enter the repeat type of this block:\n\t\'N\' - None\n\t\'D\' - Daily\n\t\'W\' - Weekly\n\t\'M\' - Monthly\n\t\'Y\' - Yearly");
    wrefresh(window);
    
    char repeat[1] = {0};  // Buffer to hold the input string
    
    wmove(window, 7, 0);
    wgetnstr(window, repeat, 1);  
    
    return repeat[0];
}

// ask user for time block repeat interval
int prompt_user_for_repeat_interval(WINDOW* window){
    wclear(window);
    mvwprintw(window, 1, 0, "Please enter the number of repetitions for this block:");
    wrefresh(window);
    
    char repeat[3] = {0};  // Buffer to hold the input string
    
    wmove(window, 7, 0);
    wgetnstr(window, repeat, 10);
    
    int repeat_interval = atoi(repeat);
    return repeat_interval;
}

// add time block to calendar
TimeBlock run_add_block(WINDOW* window) {
    echo();  // Enable echoing of characters typed by the user
    keypad(window, TRUE);  // Enable keypad for the window to handle function keys
    
    // Clear the window and prepare for input
    char start_time_string[22];
    strcpy(start_time_string, prompt_user_for_time(window).c_str());
    wclear(window);
    char end_time_string[22];
    strcpy(end_time_string, prompt_user_for_time(window).c_str());
    wclear(window);
    
    
    // Optional: display the input for confirmation
    wrefresh(window);
    
    // Wait for a key press before continuing
    TimeAndDate start_time = convert_string_to_time(start_time_string);
    TimeAndDate end_time = convert_string_to_time(end_time_string);
    char repeat = prompt_user_for_repeat(window);
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
        repeat_interval = prompt_user_for_repeat_interval(window);
    }
    
    let new_block = TimeBlock(start_time, end_time, repeat_type, repeat_interval);
    wclear(window);
    mvwprintw(window, 1, 0, "%s", new_block.encode().c_str());
    
    wgetch(window);
    
    noecho();  // Turn off echoing of characters typed
    return new_block;
}


int main() {
    initscr();
    noecho();
    cbreak();
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);
    
    u32 calendar_window_height = LINES * 3/4;
    u32 calendar_window_width = COLS;
    WINDOW* calendar_window = create_window(calendar_window_height, calendar_window_width, 0, 0);
    
    int interact_height = LINES * 1/4;
    int interact_width = COLS;
    WINDOW* interact_window = create_window(interact_height, interact_width, calendar_window_height, 0);
    scrollok(calendar_window, TRUE); // Enable scrolling for the calendar window
    keypad(interact_window, TRUE); // Enable keypad input for interaction window
    int scroll_count = 0;

    // Calendar setup
    let my_calendar = Calendar();
    
    my_calendar.busy_times.push_back(TimeBlock(
        TimeAndDate::build(0, 7, 2024),
        TimeAndDate::build(6 * 60, 7, 2024),
        RepeatType::NoRepeat,
        0
    ));
    
    my_calendar.busy_times.push_back(TimeBlock(
        TimeAndDate::build(12 * 60, 7, 2024),
        TimeAndDate::build(15 * 60, 7, 2024),
        RepeatType::NoRepeat,
        0
    ));
    
    let new_time = TimeBlock();
    let start_day = TimeAndDate::build(0, 3, 2024);
    
    draw_calendar_week(calendar_window, start_day, my_calendar, scroll_count);

    // Interaction window setup
    draw_interactions(interact_window);
    const int displayable_rows = getmaxy(calendar_window) - 2;

    // main calendar controls
    int character;
    while ((character = wgetch(interact_window)) != 'q') { // Loop until F1 is pressed
        switch (character) {
            case 'k':
                if (scroll_count <= 24 * 4 + 1 - getmaxy(calendar_window)) {
                    scroll_count += 1;
                    break;
                }
                break;
                
            case 'j':
                if (scroll_count > 0) {
                    scroll_count -= 1;
                    break;
                }
                break;
            case 'a':
                start_day = start_day.add_days(-7);
                draw_calendar_week(calendar_window, start_day, my_calendar, 0);
                wrefresh(calendar_window);
                break;
            case 'd':
                start_day = start_day.add_days(7);
                draw_calendar_week(calendar_window, start_day, my_calendar, 0);
                wrefresh(calendar_window);
                break;
            case '1':
                new_time = run_add_block(interact_window);
                my_calendar.add_time(new_time);
                wrefresh(calendar_window);
                draw_interactions(interact_window);
                break;
            case '2':
                draw_remove_calendar(calendar_window, my_calendar, 0);
                draw_remove_interactions(interact_window, calendar_window, my_calendar);
                break;
        }
        
        draw_calendar_week(calendar_window, start_day, my_calendar, scroll_count);
        wrefresh(calendar_window);
        wrefresh(interact_window);
    }

    destroy_window(calendar_window);
    destroy_window(interact_window);
    endwin();
    return 0;
}



