/*
 * ned.c
 * Main file. Initializes ncurses window and draws text from a file to it.
 */

#include <ncurses.h>
#include <stdlib.h>

#define KEY_CTRL(c) ((c)-96)

void screen_shutdown() {
    endwin();
}

void screen_draw_lines() {
    for (int i = 0; i < getmaxy(stdscr); i++) {
        mvaddch(i, 0, '~');
    }
}

void screen_update() {
    // clear the screen and move cursor to top left
    erase();
    move(0, 0);

    // draw text
    screen_draw_lines();

    // move cursor back to top
    move(0, 0);
}

void screen_input() {
    int c = getch();

    switch (c) {
        case KEY_CTRL('q'):
            exit(0);

        default:
            ;
    }
}

int main() {
    // set up ncurses and enable raw mode so we can get those sweet, sweet keycodes
    initscr();
    raw();
    noecho();
    nonl();
    keypad(stdscr, TRUE);
    define_key("\b", 8);  // there's some weirdness with Ctrl-H sending some funky backspace char instead of 8, band-aid while investigating this
    atexit(screen_shutdown);

    while (TRUE) {
        screen_update();
        screen_input();
    }
}
