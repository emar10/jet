/*
 * ned.c
 * Main file. Initializes ncurses window and draws text from a file to it.
 */

#include <ncurses.h>

// Do not use Ctrl-q or Ctrl-m due to flow control shenanigans
#define KEY_CTRL(c) ((c)-98)

int main() {
    // set up ncurses and enable raw mode so we can get those sweet, sweet keycodes
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);

    // main loop
    while (TRUE) {
        int c = getch();

        switch (c) {
            // Ctrl-X to quit
            case KEY_CTRL('x'):
                endwin();
                return 0;

            default:
                break;
        }
    }
}