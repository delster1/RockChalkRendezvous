#include <ncurses.h>
#include "../timeanddate.hpp"
#include "../calendar.hpp"
#include <string.h>
#include <vector>
#include <iostream>

// Assume DAY_NAMES is defined and accessible
// Example DAY_NAMES could be: const char* DAY_NAMES[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

void draw_calendar(TimeAndDate start, Calendar my_cal) {
    const int minutes_intervals = 96; // 15-minute intervals in a 24-hour day
    const int days_of_week = 7;  // Only 7 days for the calendar

    attron(COLOR_PAIR(1));
    mvprintw(0, 0, "Time");
    for (int i = 0; i < days_of_week; ++i) {
        TimeAndDate day = start.add_days(i); // Assuming add_days adds 'i' days to 'start'
        mvprintw(0, 5 + i * 20, "%s", day.to_string().c_str()); // Assuming to_string() provides a formatted date string
    }
    attroff(COLOR_PAIR(1));

    // Print grid
    for (int minutes_interval = 0; minutes_interval < minutes_intervals; ++minutes_interval) {
        for (int day_of_week = 0; day_of_week < days_of_week; ++day_of_week) {
            int x = 5 + day_of_week * 20; // 20 spaces per column for spacing
            int time_of_day = minutes_interval * 15; // Time in minutes from midnight

            if(day_of_week == 0){
                mvprintw(2 + minutes_interval, 0, "%02d:%02d", time_of_day / 60, time_of_day % 60);
            }

            TimeAndDate currentTime = start.add_days(day_of_week).add_minutes(time_of_day);
            bool is_busy = my_cal.is_time_block_busy(currentTime);
            attron(COLOR_PAIR(is_busy ? 1 : 2));
            mvprintw(2 + minutes_interval, x, "%s", is_busy ? "###" : "---");
            attroff(COLOR_PAIR(is_busy ? 1 : 2));
        }
    }

    refresh();
    getch(); // Wait for user input to exit
    endwin();
}

int main() {
    initscr();
    noecho();
    cbreak();
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);

    Calendar myCalendar;
    TimeAndDate startTime = TimeAndDate::build(0, 7, 2024); // Assume this initializes at midnight
    TimeAndDate endTime = TimeAndDate::build(360, 7, 2024);  // 15 minutes into the day
    TimeBlock my_block = {startTime, endTime, RepeatType::NoRepeat, 0};
    myCalendar.busy_times.push_back(my_block);

    TimeAndDate startCalendar = TimeAndDate::build(0, 3, 2024);
    draw_calendar(startCalendar, myCalendar);

    return 0;
}
