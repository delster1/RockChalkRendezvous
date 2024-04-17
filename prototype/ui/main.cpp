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
    mvwprintw(interact_win, 1, 0, "Press F1 to exit or 'j' and 'k' to scroll.");
    wrefresh(interact_win);
    const int max_rows = getmaxy(calendar_win) - 2;  // Rows available for time slots

    int ch;
    while ((ch = wgetch(interact_win)) != KEY_F(1)) { // Loop until F1 is pressed
        switch (ch) {
            case 'j':
                if (scroll_ct <= max_rows + 10){
                    scroll_ct++;
                    // Scroll down
                    break;
                }
                break;
                
            case 'k':
            if (scroll_ct >= 0) {
                scroll_ct--;
                break;
            }
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