
#include "login.hpp"
#include "menu.hpp"
#include "calendar_editor.hpp"
int main() {
    const char *menu_choices[] = { "View Calendar", "View Group Calendars", "Edit Groups" };
    WINDOW* menu_window;
    initscr();
    noecho();
    cbreak();
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);

    login_window = create_window(LINES / 4, COLS / 2, LINES / 4, COLS / 4);
    
    // Ensure the window is clear at start
    wclear(login_window);
    wrefresh(login_window); // Refresh to show initial state

    LoginState = LoginOption::Unauthorized; // Start state

    int character;
    while (LoginState != LoginOption::Authorized) { // Loop until 'q' is pressed
        update_login_screen();
    }
    destroy_window(login_window);

    menu_window = create_window(LINES /4, COLS /2, LINES / 4, COLS);

    endwin();
    return 0;
}



