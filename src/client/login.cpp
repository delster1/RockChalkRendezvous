
#include "login.hpp"



int main() {
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
    while (true) { // Loop until 'q' is pressed
        update_login_screen();
    }

    destroy_window(login_window);
    endwin();
    return 0;
}



