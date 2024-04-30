#include <ncurses.h>
#include "../../prototype/timeanddate.hpp"
#include "../../prototype/calendar.hpp"
#include <string.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cstring>


enum MenuOption {
    // SUCCESS,
    // FAILURE, MIGHT USE THESE FOR LOGIN/REGISTRATION SUCCESS/FAILURE
    LOGIN,
    REGISTER,
    FAILURE,
    EXIT // Optional, if you have an exit option
};

void prompt_username(WINDOW* window, char* username);
void prompt_password(WINDOW* window, char* password);
void get_username_and_password(WINDOW* window, MenuOption result, char* username, char* passwd);
void draw_login_window(WINDOW* win);
// MenuOption attempt_login(const char* username, const char* password) {
//     if (/* login successful */) {
//         return SUCCESS;
//     } else {
//         return FAILURE;
//     }
// }

// // Attempt registration and return success or failure
// MenuOption attempt_register(const char* username, const char* password) {
//     if (/* registration successful */) {
//         return SUCCESS;
//     } else {
//         return FAILURE;
//     }
// }

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
MenuOption draw_account_choice_window(WINDOW* win) {
    int current_selection = 0;
    int ch;
    const char *choices[] = { "Login", "Register" };
    const int num_choices = sizeof(choices) / sizeof(choices[0]);

    keypad(win, TRUE); // Enable keyboard input for the window
    noecho();          // Don't echo the pressed keys to the window
    mvwprintw(win, 0, 0, "Choose to either login or register for an account");
    
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

        ch = wgetch(win); // Get user input

        switch (ch) {
            case KEY_UP:
            case 'k': // Move selection up
                if (--current_selection < 0) {
                    current_selection = num_choices - 1;
                }
                break;
            case KEY_DOWN:
            case 'j': // Move selection down
                if (++current_selection >= num_choices) {
                    current_selection = 0;
                }
                break;
            case '\n': // User made a selection
                return static_cast<MenuOption>(current_selection); // Return the selected option as an enum value
        }
    }
}

Status draw_account_auth_window(WINDOW* login_window,MenuOption result,char* username,char* password) {
    wclear(login_window);

    mvwprintw(login_window, 1, 0, "Trying to authorize your account");
    switch (result) {
        case LOGIN:
            // THis is where I'll send login requests

            // another_function(iss);
            break;
        case REGISTER:
            // This is where I'll send register requests 
            break;
        case FAILURE:
            wclear(login_window);
    }   
    return Failure;
}
void prompt_username(WINDOW* window, char* username) {
    echo();
    wmove(window, 2, 19);
    wgetnstr(window, username, 22);  
}

void prompt_password(WINDOW* window, char* password) {
    echo();
    wmove(window, 4, 19);
    wgetnstr(window, password, 22);  
}

void prompt_confirm_password(WINDOW* window, char* password) {
    echo();
    wmove(window, 6, 29);
    wgetnstr(window, password, 22);  
}

void get_username_and_password(WINDOW* window,MenuOption result, char* username, char* password){
    char confirm_password[22];
    switch (result) {
        // Handle LOGIN and REGISTER if they're still relevant here
        case LOGIN:
            draw_login_window(window);
            prompt_username(window, username);
            prompt_password(window, password);
            break;
        case REGISTER:
            draw_register_window(window);
            prompt_username(window, username);
            prompt_password(window, password);
            prompt_confirm_password(window, confirm_password);
            if( strcmp(password,confirm_password) == 0) { // also handle account creation here
                mvwprintw(window, 10, 0, "Creating Account...");
            }else{
                mvwprintw(window, 10, 0, "Passwords Don't Match!");
                wclear(window);

                get_username_and_password(window, result, username, password);
            }
            break;
    }
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
    
    echo();

    MenuOption result = draw_account_choice_window(login_window);
    get_username_and_password(login_window, result, username, password);
    // take user input for username and password
    Status authorization = draw_account_auth_window(login_window, result, username, password);
    
 
    noecho();
    wrefresh(login_window);
    // main character controls
    int character;
    while ((character = wgetch(login_window)) != 'q') { // Loop until q is pressed
        wrefresh(login_window);
    }

    destroy_window(login_window);
    endwin();
    return 0;
}



