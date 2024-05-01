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
void draw_edit_groups_window();


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

void draw_group_interactions_window() {
    
    switch (MenuState) {
        case InMenu:
            draw_menu_choice_window();
            break;
        case ViewingCalendars: // this is invalid, it shouldn't run when the state is ViewingCalendars
            break;
            
        case ViewingGroups: // this will be where a user selects a group's calendar to view
            
            transfer_to_group_calendar_view();
            // set active_group to request for get_groups[0]
            // set set group_calendars to get_groups fn.

            break;
        case EditingGroups:
            draw_edit_groups_window();
            // this is where I'll display editing menu choices to edit a users groups
            break;
    }
}

void draw_edit_groups_window() {
    static const char *editing_menu_choices[] = { "Create Group", "Join Group", "Leave Group", "Rename Group", "View Groups" };
    const int num_choices = sizeof(editing_menu_choices) / sizeof(editing_menu_choices[0]);
    int current_selection = 0;
    int ch;
    keypad(menu_window, TRUE); // Enable keyboard input for the menu_windowdow
    noecho();          // Don't echo the pressed keys to the menu_windowdow
    mvwprintw(menu_window, 0, 0, "Choose one:");
    wrefresh(menu_window);
    // int character;
    bool not_chosen = true;
    while (not_chosen ) {
        // character = wgetch(interact_window);

        for (int i = 0; i < num_choices; ++i) {
            if (i == current_selection) {
                wattron(menu_window, A_REVERSE);  // Highlight the selected choice
            }
            mvwprintw(menu_window, i + 1, 1, "%s", editing_menu_choices[i]);
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
                not_chosen = false;
                break;
        }
    }
    wclear(menu_window);

    switch (current_selection){
        case 0:
            mvwprintw(menu_window, 1, 1, "CREATE GROUP");
            break;
        case 1:
            mvwprintw(menu_window, 1, 1, "JOIN GROUP");
            break;
        case 2:
            mvwprintw(menu_window, 1, 1, "LEAVE GROUP");
            break;
        case 3:
            mvwprintw(menu_window, 1, 1, "RENAME GROUP");
            break;
        case 4:
            mvwprintw(menu_window, 1, 1, "VIEW GROUP");
            break;
    }
    wrefresh(menu_window);

    napms(3000);

}
// TODO - create catchall function that iterates through groups and can return whatever is needed dependent on selection
// CreateGroup(user, group_name)
void draw_groups_create_window() {
    mvwprintw(menu_window, 1, 1, "Enter group name:")
}
// JoinGroup(user, group_id)
void draw_groups_join_window() {}
// LeaveGroup(user, group_id)
void draw_groups_leave_window() {}
// GetGroups(user) -> get encoded groups
void draw_groups_get_window() {}


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
        case EditingGroups:
            wclear(menu_window);
            draw_group_interactions_window();
            MenuState = MenuOption::InMenu;
            break;
    }
}

#endif