/*
 * ned.c
 * Main file. Initializes ncurses window and draws text from a file to it.
 */
#define _GNU_SOURCE

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#define VERSION v0.0.1
#define TABSTOP 4

#define KEY_CTRL(c) ((c)-96)

void screen_shutdown() {
    endwin();
}

void screen_die(const char *error, int code) {
    screen_shutdown();
    printf("Error: %s\n", error);
    exit(code);
}

typedef struct line {
    int len;
    char *s;
} line;

struct editor_state {
    int y, x;
    int sy, sx;
    int maxy, maxx;
    int len;
    line *lines;
};
struct editor_state es;

void editor_insert_line(char *s, size_t len, int y)  {
    es.lines = realloc(es.lines, sizeof(line) * (es.len + 1));

    if (y < es.len) {
        memmove(&es.lines[y + 1], &es.lines[y], sizeof(line) * (es.len - y));
    } else {
        y = es.len;
    }

    es.lines[y].len = (int)len;
    es.lines[y].s = malloc(len + 1);
    memcpy(es.lines[y].s, s, len);
    es.lines[y].s[len] = '\0';
    es.len++;
    es.y++;
    es.x = 0;
}

void editor_insert_char(int c) {
    es.lines[es.y].s = realloc(es.lines[es.y].s, es.lines[es.y].len + 1);
    memmove(&es.lines[es.y].s[es.x + 1], &es.lines[es.y].s[es.x], es.lines[es.y].len - es.x);
    es.lines[es.y].len++;
    es.lines[es.y].s[es.x] = c;
    es.x++;
}

void editor_del_char() {
    line *l = &es.lines[es.y];

    if (es.x > 0) {
        es.x--;
        memmove(&l->s[es.x], &l->s[es.x + 1], l->len - es.x);
        l->len--;
    } else if (es.y > 0) {  // don't delete lines[0] pls
        line *prev = &es.lines[es.y - 1];
        prev->s = realloc(prev->s, prev->len + l->len);
        strcat(prev->s, l->s);
        es.x = prev->len;
        prev->len += l->len;
        memmove(l, &es.lines[es.y + 1], sizeof(line) * (es.len - (es.y + 1)));
        es.len--;
        es.y--;
    }
}

void editor_open(char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        screen_die("could not open file", 1);;
    }

    char *curr = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&curr, &len, f)) != -1) {
        if (curr[read - 1] == '\n') {
            read--;
        }
        editor_insert_line(curr, (size_t)read, es.len);
    }

    free(curr);
    fclose(f);
}

void editor_move(int key) {
    switch (key) {
        // arrows
        case KEY_UP:
            if (es.y > 0) {
                es.y--;
            }
            break;
        case KEY_DOWN:
            if (es.y < es.len - 1) {
                es.y++;
            }
            break;
        case KEY_RIGHT:
            if (es.x < es.lines[es.y].len) {
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
            es.y = es.sy -= es.maxy;
            if (es.y < 0) {
                es.y = es.sy = 0;
            }
            break;
        case KEY_NPAGE:  // Page Down
            es.y = es.sy += es.maxy;
            break;

        // home/end
        case KEY_HOME:
            es.x -= es.maxx - 1;
            if (es.x < 0) {
                es.x = 0;
            }
            break;
        case KEY_END:
            es.x = es.maxx - 1;

        default:
            ;
    }

    // snap the cursor to bounds of editor if needed
    if (es.y >= es.len) {
        es.y = es.len - 1;
    }
    line l = es.lines[es.y];
    if (es.x > l.len) {
        es.x = l.len;
    }
}

void screen_draw_lines() {
    for (int y = 0; y < getmaxy(stdscr); y++) {
        move(y, 0);
        if (y + es.sy < es.len) {
            line l = es.lines[y + es.sy];
            int x = 0;
            while (x < es.maxx && x < l.len - es.sx) {
                addch((chtype)l.s[x + es.sx]);
                x++;
            }
        } else {
            addch('~');
        }
    }
}

void screen_update() {
    int y, x;

    // clear the screen and move cursor to top left
    erase();
    getyx(stdscr, y, x);
    move(0, 0);

    // scroll if needed
    if (es.y < es.sy) {
        es.sy = es.y;
    } else if (es.y >= es.sy + es.maxy) {
        es.sy = es.y - es.maxy + 1;
    }

    if (es.x < es.sx) {
        es.sx = es.x;
    } else if (es.x >= es.sx + es.maxx) {
        es.sx = es.x - es.maxx + 1;
    }

    // draw text
    screen_draw_lines();

    // move cursor back to current location
    move(es.y - es.sy, es.x - es.sx);
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
            screen_shutdown();
            exit(0);
            break;

        case KEY_BACKSPACE:
        case 127:
            editor_del_char();
            break;

        case KEY_ENTER:
        case 13:
            editor_insert_line(NULL, 0, es.y + 1);
            break;

        default:
            editor_insert_char(c);
    }
}

int main(int argc, char *argv[]) {
    // set up ncurses and enable raw mode so we can get those sweet, sweet keycodes
    initscr();
    raw();
    noecho();
    nonl();
    keypad(stdscr, TRUE);
    define_key("\b", 8);

    // set initial editor state
    es.y = es.x = 0;
    es.sy = es.sx = 0;
    getmaxyx(stdscr, es.maxy, es.maxx);
    es.len = 0;
    es.lines = NULL;
    set_tabsize(TABSTOP);

    // open file
    if (argc > 1) {
        editor_open(argv[1]);
    }

    // add blank line if no args/file is empty
    if (es.len == 0) {
        editor_insert_line("", 0, 0);
    }

    while (TRUE) {
        screen_update();
        screen_input();
    }
}
