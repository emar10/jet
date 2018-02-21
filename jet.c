/** ned.c
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
    int dirty;
};
struct editor_state es;

struct screen_state {
    WINDOW *buffer;
    WINDOW *statusbar;
    WINDOW *messagebox;
    WINDOW *linenumbers;
    int screeny, screenx;
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
    es.dirty = TRUE;
}

void editor_insert_char(int c) {
    es.lines[es.y].s = realloc(es.lines[es.y].s, es.lines[es.y].len + 1);
    memmove(&es.lines[es.y].s[es.x + 1], &es.lines[es.y].s[es.x], es.lines[es.y].len - es.x);
    es.lines[es.y].len++;
    es.lines[es.y].s[es.x] = c;
    es.x++;
    es.dirty = TRUE;
}

void editor_del_char() {
    line *l = &es.lines[es.y];

    if (es.x > 0) {
        es.x--;
        memmove(&l->s[es.x], &l->s[es.x + 1], l->len - es.x);
        l->len--;
    } else if (es.y > 0) {  // don't delete lines[0] pls
        line *prev = &es.lines[es.y - 1];
        prev->s = realloc(prev->s, prev->len + l->len + 1);
        strcat(prev->s, l->s);
        es.x = prev->len;
        prev->len += l->len;
        memmove(l, &es.lines[es.y + 1], sizeof(line) * (es.len - (es.y + 1)));
        es.len--;
        es.y--;
    }

    es.dirty = TRUE;
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
    es.dirty = FALSE;
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
    es.dirty = FALSE;
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

/* display a message */
void screen_message(const char *message) {
    if (screen.messagebox != NULL) {
        delwin(screen.messagebox);
    }
    screen.messagebox = newwin(1, screen.screenx, screen.screeny - 1, 0);
    wbkgd(screen.messagebox, A_STANDOUT);

    mvwprintw(screen.messagebox, 0, 0, "%s%*s", message, screen.screenx - strlen(message), "Ctrl-X to dismiss");
    wrefresh(screen.messagebox);    wbkgd(screen.messagebox, A_STANDOUT);
}

/* ask for a line of text from the user with the given prompt string */
void screen_read_message(char *readto, const char *prompt) {
    if (screen.messagebox != NULL) {
        delwin(screen.messagebox);

    }
    screen.messagebox = newwin(1, screen.screenx, screen.screeny - 1, 0);
    wbkgd(screen.messagebox, A_STANDOUT);

    mvwprintw(screen.messagebox, 0, 0, "%s", prompt);
    wrefresh(screen.messagebox);

    echo();
    wgetstr(screen.messagebox, readto);

    noecho();
    //wrefresh(screen.buffer);
    delwin(screen.messagebox);
    screen.messagebox = NULL;
}

void screen_getfilename() {
    char prompt[80];
    char newname[80];
    int isnull = es.filename == NULL;
    sprintf(prompt, "Filename to write%s%s%s: ", isnull ? "" : "(", isnull ? "" : es.filename, isnull ? "" : ")");

    screen_read_message(newname, prompt);

    if (strlen(newname) > 0) {
        es.filename = realloc(es.filename, strlen(newname) + 1);
        strcpy(es.filename, newname);
    }
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
        }
    }
}

void screen_update() {
    // clear the screen and move cursor to top left
    werase(screen.buffer);
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

    // draw line numbers
    werase(screen.linenumbers);
    for (int y = 0; y < es.maxy; y++) {
        if (y + es.sy < es.len) {
            mvwprintw(screen.linenumbers, y, 0, "%3d ", y + es.sy + 1);
        } else {
            mvwaddch(screen.linenumbers, y, 0, '~');
        }
    }

    // draw text
    screen_draw_lines();

    // draw status bar
    werase(screen.statusbar);

    char left[screen.screenx];
    char right[screen.screenx];

    sprintf(left, " %s%s", es.filename != NULL ? es.filename : "<No File>", es.dirty ? " [!] " : "");
    sprintf(right, " %d/%d ", es.y + 1, es.len);

    mvwprintw(screen.statusbar, 0, 0, "%.*s%*s", screen.screenx - strlen(left), left, screen.screenx - strlen(left), right);

    // move cursor back to current location
    wmove(screen.buffer, es.y - es.sy, es.x - es.sx);

    // refresh windows
    refresh();
    wrefresh(screen.statusbar);
    if (screen.messagebox != NULL) {
        touchwin(screen.messagebox);
        wrefresh(screen.messagebox);
    }
    wrefresh(screen.linenumbers);
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
            if (es.filename != NULL) {
                editor_write(es.filename);
            }
            break;

        case KEY_CTRL('h'):
            screen_message("Ctrl-S to save buffer, Ctrl-Q to quit");
            break;

        case KEY_CTRL('x'):
            if (screen.messagebox != NULL) {
                delwin(screen.messagebox);
                screen.messagebox = NULL;
            }
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

        case '\t':
            for (int i = 0; i < 4; i++) {
                editor_insert_char(' ');
            }
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
    es.maxx -= 4;
    es.len = 0;
    es.lines = NULL;
    es.dirty = FALSE;
    set_tabsize(TABSTOP);

    // set up screen state
    screen.buffer = newwin(es.maxy, es.maxx, 0, 4);
    screen.linenumbers = newwin(es.maxy, 4, 0, 0);
    screen.statusbar = newwin(1, screen.screeny, es.maxy, 0);
    screen.messagebox = NULL;
    getmaxyx(stdscr, screen.screeny, screen.screenx);

    // open file
    if (argc > 1) {
        editor_open(argv[1]);
    }

    // add blank line if no args/file is empty
    if (es.len == 0) {
        editor_insert_line("", 0, 0);
        es.dirty = FALSE;
    }

    // move cursor back to start
    es.y = es.x = 0;

    while (TRUE) {
        screen_update();
        screen_input();
    }
}
