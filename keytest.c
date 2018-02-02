/*
 * keytest.c
 * Runs a curses screen and echoes the keycode for any pressed key
 */
#include <ncurses.h>

int main() {
    initscr();
    raw();
    noecho();
    nonl();
    keypad(stdscr, TRUE);
    define_key("\b", 8);

    while (TRUE) {
        printw("Key Test -- 'q' to quit.\n");
        int c = getch();

        if (c == 'q') {
            endwin();
            exit(0);
        }

        printw("%d", c);
    }
