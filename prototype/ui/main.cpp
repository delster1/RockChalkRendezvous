#include <ncurses.h>
#include "../timeanddate.hpp"
#include "../calendar.hpp"
#include <string.h>
#include <vector>
#include <iostream>
WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);
// Assume DAY_NAMES is defined and accessible
// Example DAY_NAMES could be: const char* DAY_NAMES[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

void draw_calendar(WINDOW *win, TimeAndDate start, Calendar my_cal, int scroll_offset) {
    if (scroll_offset < 0 ){
        scroll_offset = 0;
    }
    wclear(win);  // Clear the window first
    // const int minutes_intervals = 96; // 15-minute intervals in a 24-hour day
    const int days_of_week = 7;  // Only 7 days for the calendar
    const int max_rows = getmaxy(win) - 2;  // Rows available for time slots
    wattron(win, COLOR_PAIR(1));
    mvwprintw(win, 1, 0, "Time");
    for (int i = 0; i < days_of_week; ++i) {
        TimeAndDate day = start.add_days(i);
        mvwprintw(win, 1, 5 + i * 20, "%s", day.to_string().c_str());
    }
    wattroff(win, COLOR_PAIR(1));

    for (int minutes_interval = 0; minutes_interval < max_rows; ++minutes_interval) {
        for (int day_of_week = 0; day_of_week < days_of_week; ++day_of_week) {
            int x = 5 + day_of_week * 20; // 20 spaces per column for spacing
            int time_of_day = (scroll_offset + minutes_interval) * 15; // Time in minutes from midnight
            if(day_of_week == 0){
                mvwprintw(win, 2 + minutes_interval, 0, "%02d:%02d ", time_of_day / 60, time_of_day % 60);
            }

            TimeAndDate currentTime = start.add_days(day_of_week).add_minutes(time_of_day);
            bool is_busy = my_cal.is_time_block_busy(currentTime);
            
            wattron(win, COLOR_PAIR(is_busy ? 1 : 2));
            mvwprintw(win, 2 + minutes_interval, x, "%s", is_busy ? "###" : "---");
            wattroff(win, COLOR_PAIR(is_busy ? 1 : 2));
        }
    }

    wrefresh(win);
}

void draw_interactions(WINDOW* win){
    mvwprintw(win, 1, 0, "Press F1 to exit or 'j' and 'k' to scroll the above window.");
    mvwprintw(win, 2, 0, "Press A/D to scroll Left/Right in the calendar.");
    mvwprintw(win, 3, 0, "1) Add Time Block");
    mvwprintw(win, 4, 0, "2) Remove Time Block");


    wrefresh(win);

}

TimeAndDate convert_string_to_time(char time_string[22]) {  
    int hour, minute, month, day, year;
    sscanf(time_string, "%d:%d %d %d %d", &hour, &minute, &month, &day, &year);

    // Convert to days, minutes, and year
    int days = (month - 1) * 30 + day;
    int minutes = hour * 60 + minute;

    TimeAndDate time = TimeAndDate::build(minutes, days, year);
    return time;

}
std::string prompt_user_for_time(WINDOW* win){
    wclear(win);
    mvwprintw(win, 1, 0, "Enter the time block start Time: 24-hour time:minutes Month(Numerical) Day Year (press Enter when done): ");
    wrefresh(win);

    char time_string[22] = {0};  // Buffer to hold the input string

    wmove(win, 2, 0);
    wgetnstr(win, time_string, 22);  

    // Optional: display the input for confirmation
    mvwprintw(win, 3, 0, "You entered: %s", time_string);
    wrefresh(win);
    return std::string(time_string);
}

char prompt_user_for_repeat(WINDOW* win){
    wclear(win);
    mvwprintw(win, 1, 0, "Please enter the repeat type of this block:\n\t\'N\' - None\n\t\'D\' - Daily\n\t\'W\' - Weekly\n\t\'M\' - Monthly\n\t\'Y\' - Yearly");
    wrefresh(win);

    char repeat[1] = {0};  // Buffer to hold the input string

    wmove(win, 7, 0);
    wgetnstr(win, repeat, 1);  

    return repeat[0];
}
int prompt_user_for_repeat_interval(WINDOW* win){
    wclear(win);
    mvwprintw(win, 1, 0, "Please enter the number of repetitions for this block:");
    wrefresh(win);

    char repeat[3] = {0};  // Buffer to hold the input string

    wmove(win, 7, 0);
    wgetnstr(win, repeat, 10);  

    int repeat_interval = atoi(repeat);
    return repeat_interval;
}
TimeBlock run_add_block(WINDOW* win) {
    echo();  // Enable echoing of characters typed by the user
    keypad(win, TRUE);  // Enable keypad for the window to handle function keys
    
    // Clear the window and prepare for input
    char start_time_string[22];
    strcpy(start_time_string, prompt_user_for_time(win).c_str());
    wclear(win);
    char end_time_string[22];
    strcpy(end_time_string, prompt_user_for_time(win).c_str());
    wclear(win);

 
    // Optional: display the input for confirmation
    wrefresh(win);
    // Wait for a key press before continuing
    TimeAndDate start_time = convert_string_to_time(start_time_string);
    TimeAndDate end_time = convert_string_to_time(end_time_string);
    char repeat = prompt_user_for_repeat(win);
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
    if (repeat != 'N'){
        repeat_interval = prompt_user_for_repeat_interval(win);
    }
    TimeBlock new_block = {start_time, end_time, repeat_type, repeat_interval};
    std::string my_timeBlock_string = TimeBlock::encode_static(new_block);
    wclear(win);
    mvwprintw(win, 1, 0, "%s", my_timeBlock_string.c_str());

    wgetch(win);

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

    int cal_height = 3 * (LINES / 4); // 3/4 of the screen height
    int cal_width = COLS;
    WINDOW *calendar_win = create_newwin(cal_height, cal_width, 0, 0);

    int interact_height = LINES / 4; // 1/4 of the screen height
    int interact_width = COLS;
    WINDOW *interact_win = create_newwin(interact_height, interact_width, cal_height, 0);
    scrollok(calendar_win, TRUE);  // Enable scrolling for the calendar window
    keypad(interact_win, TRUE);    // Enable keypad input for interaction window
    int scroll_ct = 0;
    // Calendar setup
    Calendar myCalendar;
    TimeAndDate startTime = TimeAndDate::build(0, 7, 2024); // Midnight
    TimeAndDate endTime = TimeAndDate::build(360, 7, 2024); // 6:00 AM
    TimeBlock my_block = {startTime, endTime, RepeatType::NoRepeat, 0};
    myCalendar.busy_times.push_back(my_block);

    TimeAndDate startCalendar = TimeAndDate::build(0, 3, 2024);
    draw_calendar(calendar_win, startCalendar, myCalendar, scroll_ct);

    // Interaction window setup
    draw_interactions(interact_win);
    const int max_rows = getmaxy(calendar_win) - 2;  // Rows available for time slots

    int ch;
    while ((ch = wgetch(interact_win)) != 'q') { // Loop until F1 is pressed
        switch (ch) {
            case 'k':
                if (scroll_ct <= max_rows + 10){
                    scroll_ct++;
                    // Scroll down
                    break;
                }
                break;
                
            case 'j':
                if (scroll_ct >= 0) {
                    scroll_ct--;
                    break;
                }
                break;
            case 'a':
                startCalendar = startCalendar.add_days(-7);
                draw_calendar(calendar_win, startCalendar, myCalendar, 0);
                wrefresh(calendar_win);
                break;
            case 'd':
                startCalendar = startCalendar.add_days(7);
                draw_calendar(calendar_win, startCalendar, myCalendar, 0);
                wrefresh(calendar_win);
                break;
            case '1':
                TimeBlock new_time = run_add_block(interact_win);
                myCalendar.add_time(new_time);
                wrefresh(calendar_win);
                draw_interactions(interact_win);
                break;

        }
        draw_calendar(calendar_win, startCalendar, myCalendar, scroll_ct); // Redraw the calendar
        wrefresh(interact_win);
    }

    destroy_win(calendar_win);
    destroy_win(interact_win);
    endwin();
    return 0;
}


WINDOW *create_newwin(int height, int width, int starty, int startx) {
    WINDOW *local_win = newwin(height, width, starty, startx);
    box(local_win, 0, 0);
    wrefresh(local_win);
    return local_win;
}

void destroy_win(WINDOW *local_win) {
    werase(local_win);
    wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
    wrefresh(local_win);
    delwin(local_win);
}