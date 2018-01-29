/*
 * ned.c
 * Main file. Initializes ncurses window and draws text from a file to it.
 */

#include <ncurses.h>

#define KEY_CTRL(c) ((c)-96)

int main() {
    // set up ncurses and enable raw mode so we can get those sweet, sweet keycodes
    initscr();
    raw();
    noecho();
    nonl();
    keypad(stdscr, TRUE);
    define_key("\b", 8);  // there's some weirdness with Ctrl-H sending some funky backspace char instead of 8, band-aid while investigating this

    while (TRUE) {
        int c = getch();

        if (c == KEY_CTRL('q')) {
            break;
        }
    }

    endwin();
    return 0;
}
