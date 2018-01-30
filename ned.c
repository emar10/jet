/*
 * ned.c
 * Main file. Initializes ncurses window and draws text from a file to it.
 */

#include <ncurses.h>
#include <stdlib.h>

#define VERSION v0.0.1

#define KEY_CTRL(c) ((c)-96)

struct editor_state {
    int y, x;
    int maxy, maxx;
};
struct editor_state es;

void editor_move(int key) {
    switch (key) {
        // arrows
        case KEY_UP:
            if (es.y > 0) {
                es.y--;
            }
            break;
        case KEY_DOWN:
            if (es.y < es.maxy - 1) {
                es.y++;
            }
            break;
        case KEY_RIGHT:
            if (es.x < es.maxx - 1) {
                es.x++;
            }
            break;
        case KEY_LEFT:
            if (es.x > 0) {
                es.x--;
            }
            break;

        // page up/down
        case KEY_PPAGE:  // Page Up
            es.y = 0;
            break;
        case KEY_NPAGE:  // Page Down
            es.y = es.maxy - 1;
            break;

        // home/end
        case KEY_HOME:
            es.x = 0;
            break;
        case KEY_END:
            es.x = es.maxx - 1;

        default:
            ;
    }
}

void screen_shutdown() {
    endwin();
}

void screen_draw_lines() {
    for (int i = 0; i < getmaxy(stdscr); i++) {
        mvprintw(i, 0, "~");
    }
}

void screen_update() {
    // clear the screen and move cursor to top left
    erase();
    move(0, 0);

    // draw text
    screen_draw_lines();

    // move cursor back to current location
    move(es.y, es.x);
    refresh();
}

void screen_input() {
    int c = getch();

    switch (c) {
        case KEY_UP:
        case KEY_DOWN:
        case KEY_RIGHT:
        case KEY_LEFT:
        case KEY_PPAGE:
        case KEY_NPAGE:
        case KEY_HOME:
        case KEY_END:
            editor_move(c);
            break;

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

    // set initial editor state
    es.y = es.x = 0;
    getmaxyx(stdscr, es.maxy, es.maxx);

    while (TRUE) {
        screen_update();
        screen_input();
    }
}
