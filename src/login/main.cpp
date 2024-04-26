#include <ncurses.h>
#include "../../prototype/timeanddate.hpp"
#include "../../prototype/calendar.hpp"
#include <string.h>
#include <vector>
#include <algorithm>
#include <iostream>

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

void draw_login_window(WINDOW* win) {
    mvwprintw(win, 1, 1, "Login to your account:");
    mvwprintw(win, 2, 1, "\tUsername:");
    mvwprintw(win, 4, 1, "\tPassword:");
    wrefresh(win);  // Refresh the window to show the text
}

void prompt_username(WINDOW* window, char* username) {
    wmove(window, 3, 3);
    wgetnstr(window, username, 22);  
}

void prompt_password(WINDOW* window, char* password) {
    
    wmove(window, 5, 3);
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
    draw_login_window(login_window);
    echo();
    prompt_username(login_window, username);
    prompt_password(login_window, password);
    noecho();
    // main character controls
    int character;
    while ((character = wgetch(login_window)) != 'q') { // Loop until F1 is pressed
        
        wrefresh(login_window);
    }

    destroy_window(login_window);
    endwin();
    return 0;
}



