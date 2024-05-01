#include "login.hpp"
#include "menu.hpp"
#include "calendar_editor.hpp"

int main() {
    initscr();
    noecho();
    cbreak();
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);

    // Create and handle the login window
    login_window = create_window(LINES, COLS, 0, 0);
    LoginState = LoginOption::Unauthorized; // Assuming LoginState and LoginOption are defined
    box(login_window, 0,0);
    while (LoginState != LoginOption::Authorized) { // Loop until authorized
        update_login_screen(); // Function needs to update loginState
    }
    destroy_window(login_window); // Destroy the login window after successful login

    // Transition to the menu window after successful login
    menu_window = create_window(LINES, COLS, 0, 0);
    box(menu_window, 0, 0); // Optional: Draw a box around the window
    mvwprintw(menu_window, 1, 1, ""); // Example content
    wrefresh(menu_window); // Refresh to show the menu window

    MenuState= MenuOption::InMenu; // Assuming MenuState and MenuOption are defined
    int ch;
    update_menu_screen();
    while (true) { // Update to exit on 'q' key press

        update_menu_screen(); // Needs implementation to handle menu interactions
    }

    // Clean up
    destroy_window(menu_window);
    endwin();
    return 0;
}
