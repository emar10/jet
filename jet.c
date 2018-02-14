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

// save the current state and return to regular mode
void screen_pause() {
    def_prog_mode();
    endwin();
}

void screen_resume() {
    reset_prog_mode();
    refresh();
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
    char *filename;
};
struct editor_state es;

struct screen_state {
    WINDOW *buffer;
    WINDOW *statusbar;
    WINDOW *messagebox;
};
struct screen_state screen;

void editor_trim_line(line *l, int len) {
    if (es.x < l->len) {
        l->len -= len;
        l->s = realloc(l->s, l->len + 1);
        l->s[l->len] = '\0';
    }
}

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
        screen_die("could not open file", 1);
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

    // set the filename
    es.filename = malloc(strlen(filename) + 1);
    strcpy(es.filename, filename);

    free(curr);
    fclose(f);
}

void editor_write(char *filename) {
    FILE *f = fopen(filename, "w");

    if (!f) {
        screen_die("could not open file for writing", 1);
    }

    // write the lines
    for (int i = 0; i < es.len; i++) {
        fputs(es.lines[i].s, f);
        fputc('\n', f);
    }

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

void screen_getfilename() {
    mvwprintw(screen.statusbar, 0, 0, "Filename to write ");
    if (es.filename != NULL) {
        wprintw(screen.statusbar, "(%s) ", es.filename);
    }
    wprintw(screen.statusbar, ": ");
    wrefresh(screen.statusbar);

    echo();
    char str[80];
    getstr(str);

    if (strlen(str) > 0) {
        es.filename = realloc(es.filename, strlen(str) + 1);
        strcpy(es.filename, str);
    }
    noecho();
}

void screen_draw_lines() {
    for (int y = 0; y < es.maxy; y++) {
        wmove(screen.buffer, y, 0);
        if (y + es.sy < es.len) {
            line l = es.lines[y + es.sy];
            int x = 0;
            while (x < es.maxx && x < l.len - es.sx) {
                waddch(screen.buffer, (chtype)l.s[x + es.sx]);
                x++;
            }
        } else {
            waddch(screen.buffer, '~');
        }
    }
}

void screen_update() {
    int y, x;

    // clear the screen and move cursor to top left
    erase();
    getyx(screen.buffer, y, x);
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

    // draw status bar
    mvwaddch(screen.statusbar, 0, 0, (chtype)'a');

    // move cursor back to current location
    wmove(screen.buffer, es.y - es.sy, es.x - es.sx);

    // refresh windows
    refresh();
    wrefresh(screen.statusbar);
    wrefresh(screen.buffer);
}

int screen_is_printable(int c) {
    // for now only print between 32 and 255
    if (c >= 32 && c <= 255) {
        return TRUE;
    } else {
        return FALSE;
    }
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

        case KEY_CTRL('s'):
            screen_getfilename();
            editor_write(es.filename);
            break;

        case KEY_BACKSPACE:
        case 127:
            editor_del_char();
            break;

        case KEY_ENTER:
        case 13:
            editor_insert_line(&es.lines[es.y].s[es.x], es.lines[es.y].len - es.x, es.y + 1);
            editor_trim_line(&es.lines[es.y - 1], es.lines[es.y - 1].len - es.x);
            es.x = 0;
            break;

        default:
            if (screen_is_printable(c)) {
                editor_insert_char(c);
            }
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
    es.maxy--;
    es.len = 0;
    es.lines = NULL;
    set_tabsize(TABSTOP);

    // set up screen state
    screen.buffer = newwin(es.maxy, es.maxx, 0, 0);
    screen.statusbar = newwin(1, es.maxx, es.maxy, 0);

    // open file
    if (argc > 1) {
        editor_open(argv[1]);
    }

    // add blank line if no args/file is empty
    if (es.len == 0) {
        editor_insert_line("", 0, 0);
    }

    // move cursor back to start
    es.y = es.x = 0;

    while (TRUE) {
        screen_update();
        screen_input();
    }
}
