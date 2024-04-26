#include <ncurses.h>
#include "../../prototype/timeanddate.hpp"
#include "../../prototype/calendar.hpp"
#include <string.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cstring>

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

void draw_register_window(WINDOW* win){
    mvwprintw(win, 1, 1, "Create to your account:");
    mvwprintw(win, 2, 1, "\tUsername:");
    mvwprintw(win, 4, 1, "\tPassword:");
    mvwprintw(win, 6, 1, "\tConfirm Password:");

    wrefresh(win);  // Refresh the window to show the text
}

void draw_login_window(WINDOW* win) {
    mvwprintw(win, 1, 1, "Login to your account:");
    mvwprintw(win, 2, 1, "\tUsername:");
    mvwprintw(win, 4, 1, "\tPassword:");
    wrefresh(win);  // Refresh the window to show the text
}
// Draws the login/registration window and handles user selection
bool draw_account_auth_window(WINDOW *win) {
    int current_selection = 0;  // 0 for login, 1 for registration
    int ch;
    const char *choices[] = { "Login", "Register" };
    const int num_choices = sizeof(choices) / sizeof(choices[0]);

    keypad(win, TRUE);  // Enable keyboard input for the window
    noecho();           // Don't echo the pressed keys to the window
    mvwprintw(win, 0, 0, "Choose to either login or register for an account");
    // Draw choices and handle input
    while (true) {
        for (int i = 0; i < num_choices; ++i) {
            if (i == current_selection) {
                wattron(win, A_REVERSE);  // Highlight the selected choice
            }
            mvwprintw(win, i + 1, 1, "%s", choices[i]);
            if (i == current_selection) {
                wattroff(win, A_REVERSE);
            }
        }
        wrefresh(win);

        ch = wgetch(win);  // Get user input

        switch (ch) {
            case KEY_UP:
            case 'k':  // Move selection up
                current_selection--;
                if (current_selection < 0) {
                    current_selection = num_choices - 1;
                }
                break;
            case KEY_DOWN:
            case 'j':  // Move selection down
                current_selection++;
                if (current_selection >= num_choices) {
                    current_selection = 0;
                }
                break;
            case '\n':  // User made a selection
                switch (current_selection) {
                    case 0:
                        draw_login_window(win);
                        wrefresh(win);
                        break;
                    case 1:
                        draw_register_window(win);
                        wrefresh(win);
                        break;
                }
                return current_selection;  // Exit after handling the action
            default:
                break;
        }
    }
}

void prompt_username(WINDOW* window, char* username) {
    wmove(window, 2, 19);
    wgetnstr(window, username, 22);  
}

void prompt_password(WINDOW* window, char* password) {
    
    wmove(window, 4, 19);
    wgetnstr(window, password, 22);  
}

void prompt_confirm_password(WINDOW* window, char* password) {
    
    wmove(window, 6, 29);
    wgetnstr(window, password, 22);  
}

int main() {
    initscr();
    noecho();
    cbreak();
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);
    
    int login_height = LINES / 4;
    int login_width = COLS / 4;
    int start_row = (LINES - login_height) / 2;
    int start_col = (COLS - login_width) / 2;
    WINDOW* login_window = create_window(login_height, login_width, start_row, start_col);
    char username[22];
    char password[22];
    char confirm_password[22];
    int registered = draw_account_auth_window(login_window); // can be 1 (not registered) or 0(registered)
    echo();
    prompt_username(login_window, username);
    prompt_password(login_window, password);
    
    if (registered == 1){
        prompt_confirm_password(login_window, confirm_password);
        if( strcmp(password,confirm_password)) {
            mvwprintw(login_window, 10, 0, "Creating Account...");
        }else{
            mvwprintw(login_window, 10, 0, "Passwords Don't Match!");
        }
    }else{
        mvwprintw(login_window, 10, 0, "Logging into account!");
    }
    noecho();
    wrefresh(login_window);
    // main character controls
    int character;
    while ((character = wgetch(login_window)) != 'q') { // Loop until F1 is pressed
        wrefresh(login_window);
    }

    destroy_window(login_window);
    endwin();
    return 0;
}



