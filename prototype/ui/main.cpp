#include <ncurses.h>
#include "../timeanddate.hpp"
#include "../calendar.hpp" // Your existing code
#include "../timeanddate.hpp" // Your existing code

// Include other necessary headers

int main() {
    // Initialize ncurses
    initscr();
    noecho();
    cbreak();
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);

    // Calendar and events setup
    Calendar myCalendar;
    // Assuming the start of the week is Sunday at 0:00 and goes to Saturday 23:45
    // TimeAndDate weekStart = TimeAndDate::build(0, 1, 2024); // Sunday, 0:00 AM
    // TimeAndDate weekEnd = TimeAndDate::build(1435, 7, 2024); // Saturday, 23:45 PM

    // Dummy event for testing
    TimeAndDate startTime = TimeAndDate::build(15, 1, 2024); // 6:00 AM on the first day
    TimeAndDate endTime = TimeAndDate::build(480, 1, 2024); // 8:00 AM on the same day
    TimeBlock my_block = {startTime, endTime, RepeatType::NoRepeat, 0};
    myCalendar.busy_times.push_back(my_block);

    // Create grid
    const int minutes_intervals = 48; // 15-minute intervals in a day
    const int days_of_week = 8;  // Days of the week

    // Print header
    attron(COLOR_PAIR(1));
    mvprintw(0, 0, "TIME %s %s %s %s %s %s %s", DAY_NAMES[0], DAY_NAMES[1], DAY_NAMES[2], DAY_NAMES[3], DAY_NAMES[4], DAY_NAMES[5], DAY_NAMES[6]);
    attroff(COLOR_PAIR(1));

    // Print grid
    int y = 2; // Start from the second row
    for (int minutes_interval = 0; minutes_interval < minutes_intervals; ++minutes_interval) {
        for (int day_of_week = 0; day_of_week < days_of_week; ++day_of_week) {
            
            int x = day_of_week * 20; // 4 spaces per column for spacing
            
            int time_of_day = minutes_interval * 30; // Time in minutes from midnight
            if( day_of_week == 0){
                mvprintw(y + minutes_interval, x, "%4d:%4d",(time_of_day / 60) + 1, time_of_day % 60 );
            }
            else {
                TimeAndDate currentTime = TimeAndDate::build(time_of_day, day_of_week , 2024);
                bool is_busy = myCalendar.is_time_block_busy(currentTime);
                // Check if current time block is busy
                attron(COLOR_PAIR(myCalendar.is_time_block_busy(currentTime) ? 1 : 2));
                mvprintw(y + minutes_interval, x, "%s", is_busy ? "#############" : "-------------");
                attroff(COLOR_PAIR(myCalendar.is_time_block_busy(currentTime) ? 1 : 2));
            }
            
        }
    }

    refresh();
    getch(); // Wait for user input to exit
    endwin();

    return 0;
}