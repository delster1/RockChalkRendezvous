#ifndef RCR_MENU_FILE_CLIENT
#define RCR_MENU_FILE_CLIENT
#include <ncurses.h>
#include "../shared/timeanddate.hpp"
#include "../shared/calendar.hpp"
#include "calendar_editor.cpp"
#include <string.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cstring>
#include "httplib.h"
#include <iostream>

enum MenuOption {
    InMenu,     // This includes being in the menu, viewing options to change to the state below
    ViewingCalendars,    // This includes viewing/editing a user's calendar
    ViewingGroups,      // This includes viewing group calendars and selecting a group calendar to view
    EditingGroups,      // This includes editing a group - join group, create group, leave group
};

static MenuOption MenuState;
static WINDOW* menu_window;
static const char *menu_choices[] = { "View Calendar", "View Group Calendars", "Edit Groups" };


MenuOption draw_menu_choice_window() {
    const int num_choices = sizeof(menu_choices) / sizeof(menu_choices[0]);
    int current_selection = 0;
    int ch;
    keypad(menu_window, TRUE); // Enable keyboard input for the menu_windowdow
    noecho();          // Don't echo the pressed keys to the menu_windowdow
    mvwprintw(menu_window, 0, 0, "Choose one:");
    wrefresh(menu_window);
    while (true) {
        for (int i = 0; i < num_choices; ++i) {
            if (i == current_selection) {
                wattron(menu_window, A_REVERSE);  // Highlight the selected choice
            }
            mvwprintw(menu_window, i + 1, 1, "%s", menu_choices[i]);
            if (i == current_selection) {
                wattroff(menu_window, A_REVERSE);
            }
        }
        wrefresh(menu_window);

        ch = wgetch(menu_window); // Get user input

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
                return static_cast<MenuOption>(current_selection + 1);
        }
    }
}
void update_menu_screen() {
    switch (MenuState) {
        case InMenu:
            MenuState = draw_menu_choice_window();
            break;
        case ViewingCalendars:
            wclear(menu_window);
            transfer_to_calendar_editor();
            MenuState = MenuOption::InMenu;
            break;
        case ViewingGroups: 
            wclear(menu_window);
            transfer_to_group_calendar_view();
            MenuState = MenuOption::InMenu;
            break;
        case EditingGroups:
            break;
    }
}

#endif