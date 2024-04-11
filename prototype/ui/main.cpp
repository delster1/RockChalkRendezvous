#include <ncurses.h>
#include <unistd.h> // for usleep()
#include <string.h> // for strlen()

int main() {
    // Initialize ncurses screen
    initscr();

    // Turn off echoing of keys pressed
    noecho();

    // Enable keypad mode
    keypad(stdscr, TRUE);

    // Loop until 'q' or ETX character ('\x03') is pressed
    while (true) {
        // Clear screen
        clear();

        // Print message
        mvprintw(LINES / 2, COLS / 2 - strlen("Hello world") / 2, "Hello world");

        // Refresh screen
        refresh();

        // Sleep for short duration to reduce CPU usage
        usleep(100 * 1000);

        // Check for user input
        int ch = getch();
        if (ch == '\x03' || ch == 'q') {
            break;
        }
    }

    // Deinitialize ncurses screen
    endwin();

    return 0;
}