#ifndef RCR_LOGIN_FILE_CLIENT
#define RCR_LOGIN_FILE_CLIENT
#include <ncurses.h>
#include "../shared/timeanddate.hpp"
#include "../shared/calendar.hpp"
#include <string.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cstring>
#include "client.hpp"
#include "httplib.h"
#include <iostream>
enum LoginOption {
    LoggingIn,
    Registering,
    Unauthorized,
    Authorized, // this will eventually be an "in menu" state, meaning the user is viewing possible menus
    FailureToAuthorize, // Optional, if you have an exit option
    
};
static char username[50];
static char password[50];
static WINDOW* login_window;
static LoginOption LoginState;
void prompt_username();
void prompt_password();
void get_username_and_password();
void draw_login_window();

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

void draw_register_window(){
    wrefresh(login_window);
    mvwprintw(login_window, 1, 1, "Create to your account:");
    mvwprintw(login_window, 2, 1, "\tUsername:");
    mvwprintw(login_window, 4, 1, "\tPassword:");
    mvwprintw(login_window, 6, 1, "\tConfirm Password:");

    wrefresh(login_window);  // Refresh the window to show the text
}

void draw_login_window() {
    wrefresh(login_window);
    mvwprintw(login_window, 1, 1, "Login to your account:");
    mvwprintw(login_window, 2, 1, "\tUsername:");
    mvwprintw(login_window, 4, 1, "\tPassword:");
    wrefresh(login_window);  // Refresh the window to show the text
}
// Draws the login/registration window and handles user selection
LoginOption draw_account_choice_window() {
    wrefresh(login_window);
    int current_selection = 0;
    int ch;
    const char *choices[] = { "Login", "Register" };
    const int num_choices = sizeof(choices) / sizeof(choices[0]);

    keypad(login_window, TRUE); // Enable keyboard input for the login_windowdow
    noecho();          // Don't echo the pressed keys to the login_windowdow
    mvwprintw(login_window, 0, 0, "Choose to either login or register for an account");
    
    while (true) {
        for (int i = 0; i < num_choices; ++i) {
            if (i == current_selection) {
                wattron(login_window, A_REVERSE);  // Highlight the selected choice
            }
            mvwprintw(login_window, i + 1, 1, "%s", choices[i]);
            if (i == current_selection) {
                wattroff(login_window, A_REVERSE);
            }
        }
        wrefresh(login_window);

        ch = wgetch(login_window); // Get user input

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
                return static_cast<LoginOption>(current_selection); // Return the selected option as an enum value
        }
    }
}

Status draw_account_auth_window() {
    wclear(login_window);
    mvwprintw(login_window, 7, 0, "Trying to authorize your account");
    wrefresh(login_window);
    httplib::Client* my_client = build_client();
    username_string = username;
    password_string = password;
    Status authorization_result = Failure;
    switch (LoginState) {
        case LoggingIn:
            // THis is where I'll send login requests
            // another_function(iss);
            authorization_result = send_login_request();
            break;
        case Registering:
            // This is where I'll send register requests 
            authorization_result = send_create_account_request();
            break;
    }
    switch (authorization_result){
        case Success:
            mvwprintw(login_window, 9, 1, "SUCCESSFULLY AUTHORIZED!");
            LoginState = LoginOption::Authorized;
            break;
        case Failure:
            mvwprintw(login_window, 9, 1,  "FAILED TO AUTHORIZE!");
            LoginState = LoginOption::FailureToAuthorize;
            break;
    }
    wrefresh(login_window);
    napms(2000);
    return authorization_result;
}

void prompt_username() {
    echo();
    wmove(login_window, 2, 19);
    wgetnstr(login_window, username, 22);  
}

void prompt_password() {
    echo();
    wmove(login_window, 4, 19);
    wgetnstr(login_window, password, 22);  
}

void prompt_confirm_password(char* confirm_password) {
    echo();
    wmove(login_window, 6, 29);
    wgetnstr(login_window, confirm_password, 22);  
}

void get_username_and_password(){
    wrefresh(login_window);
    char confirm_password[50];
    switch (LoginState) {
        // Handle Login and Register if they're still relevant here
        case LoggingIn:
            draw_login_window();
            prompt_username();
            prompt_password();
            break;
        case Registering:
            draw_register_window();
            prompt_username();
            prompt_password();
            prompt_confirm_password(confirm_password);
            if( strcmp(password,confirm_password) == 0) { // also handle account creation here
                mvwprintw(login_window, 9, 0, "Creating Account...");
            }else{
                mvwprintw(login_window, 9, 0, "Passwords Don't Match!");
                wrefresh(login_window);
                napms(2000);
                wclear(login_window);
                LoginState = LoginOption::Unauthorized; // calls back to function when passwords dont match
            }
            napms(1000);
            break;
    }
}

void update_login_screen() {
    switch (LoginState) {
        case Unauthorized:
            LoginState = draw_account_choice_window();
            wrefresh(login_window);
            break;
        case LoggingIn:
        case Registering:
            wclear(login_window);
            get_username_and_password();
            draw_account_auth_window();
            wrefresh(login_window);
            break;
        case Authorized:
            wrefresh(login_window);
            break;
        case FailureToAuthorize:
            LoginState = LoginOption::Unauthorized;
            wclear(login_window);
            break;
    }
    wrefresh(login_window); // Make sure to refresh after updates
}
#endif