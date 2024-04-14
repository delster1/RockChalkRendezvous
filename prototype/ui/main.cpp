#include <ncurses.h>
#include "../timeanddate.hpp"
#include "../calendar.hpp" // Your existing code
// Include other necessary headers

int main() {
    // Initialize ncurses
    initscr();
    noecho();
    cbreak();
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);

    // Assuming calendar is already populated
    Calendar myCalendar;
    TimeAndDate startTime = TimeAndDate::build(360, 1, 2024); // Assuming 6:00 AM on the first day of the year 2024
    TimeAndDate endTime = TimeAndDate::build(480, 1, 2024); // Assuming 8:00 AM on the same day
    // Rendering logic
    attron(COLOR_PAIR(1));
    mvprintw(0, 0, "Calendar Busy Times:");
    attroff(COLOR_PAIR(1));

    // Dynamically display TimeBlocks
    int y = 2; // Start from the second row
    for (const auto& block : myCalendar.busy_times) {
        mvprintw(y++, 0, "%s", block.to_string().c_str());
    }

    refresh();
    getch(); // Wait for user input to exit
    endwin();

    return 0;
}