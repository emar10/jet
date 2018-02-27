/*
 * jet.c
 * Ncurses Jet. For help, see README.md in the root directory.
 * Copyright (c) Ethan Martin
 */

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#include "../core/jet.h"

#define TABSTOP 4

#define KEY_CTRL(c) ((c)-96)

struct screen_state {
    WINDOW *bufferwin;
    WINDOW *statusbar;
    WINDOW *messagebox;
    WINDOW *linenumbers;
    buffer *b;
    int maxy, maxx;
    int y, x;
};
struct screen_state s;

void screen_shutdown() {
    delwin(s.bufferwin);
    delwin(s.statusbar);
    delwin(s.messagebox);
    delwin(s.linenumbers);
    endwin();
}

void die(const char *error, int code) {
    screen_shutdown();
    printf("Error: %s\n", error);
    exit(code);
}

void screen_pause() {
    def_prog_mode();
    endwin();
}

void screen_resume() {
    reset_prog_mode();
    refresh();
}

/* display a message */
void screen_message(const char *message) {
    if (s.messagebox != NULL) {
        delwin(s.messagebox);
    }
    s.messagebox = newwin(1, s.maxx, s.maxy - 1, 0);
    wbkgd(s.messagebox, A_STANDOUT);

    mvwprintw(s.messagebox, 0, 0, "%s%*s", message, s.maxx - strlen(message), "Ctrl-X to dismiss");
    wrefresh(s.messagebox);    wbkgd(s.messagebox, A_STANDOUT);
}

/* ask for a line of text from the user with the given prompt string */
void screen_read_message(char *readto, const char *prompt) {
    if (s.messagebox != NULL) {
        delwin(s.messagebox);

    }
    s.messagebox = newwin(1, s.maxx, s.maxy - 1, 0);
    wbkgd(s.messagebox, A_STANDOUT);

    mvwprintw(s.messagebox, 0, 0, "%s", prompt);
    wrefresh(s.messagebox);

    echo();
    wgetstr(s.messagebox, readto);

    noecho();
    //wrefresh(s.bufferwin);
    delwin(s.messagebox);
    s.messagebox = NULL;
}

void screen_getfilename() {
    char prompt[80];
    char newname[80];
    int isnull = s.b->name == NULL;
    sprintf(prompt, "Filename to write%s%s%s: ", isnull ? "" : "(", isnull ? "" : s.b->name, isnull ? "" : ")");

    screen_read_message(newname, prompt);

    if (strlen(newname) > 0) {
        bname(s.b, newname);
    }
}

void screen_open() {
    char filename[80];

    screen_read_message(filename, "Filename to open: ");

    if (strlen(filename) > 0) {
        buffer *b = readbuf(filename);
        delbuf(s.b);
        s.b = b;
        s.y = s.x = 0;
    }
}

void screen_draw_lines() {
    for (int y = 0; y < s.maxy - 1; y++) {
        wmove(s.bufferwin, y, 0);
        if (y + s.y < s.b->len) {
            line *l = s.b->lines[y + s.y];
            int x = 0;
            while (x < s.maxx - 4 && x < l->len - s.x) {
                waddch(s.bufferwin, (chtype)l->s[x + s.x]);
                x++;
            }
        }
    }
}

void screen_update() {
    // clear the screen and move cursor to top left
    werase(s.bufferwin);
    move(0, 0);

    // scroll if needed
    if (s.b->y < s.y) {
        s.y = s.b->y;
    } else if (s.b->y >= s.y + s.maxy - 1) {
        s.y = s.b->y - (s.maxy - 1) + 1;
    }

    if (s.b->x < s.x) {
        s.x = s.b->x;
    } else if (s.b->x >= s.x + s.maxx - 4) {
        s.x = s.b->x - (s.maxx - 4) + 1;
    }

    // draw line numbers
    werase(s.linenumbers);
    for (int y = 0; y < s.maxy - 1; y++) {
        if (y + s.y < s.b->len) {
            mvwprintw(s.linenumbers, y, 0, "%3d ", y + s.y + 1);
        } else {
            mvwaddch(s.linenumbers, y, 0, '~');
        }
    }

    // draw text
    screen_draw_lines();

    // draw status bar
    werase(s.statusbar);

    char left[s.maxx];
    char right[s.maxx];

    sprintf(left, " %s%s", s.b->name != NULL ? s.b->name : "<No File>", s.b->dirty ? " [!] " : "");
    sprintf(right, " %d/%d ", s.b->y + 1, s.b->len);

    mvwprintw(s.statusbar, 0, 0, "%.*s%*s", s.maxx - strlen(left), left, s.maxx - strlen(left), right);

    // move cursor back to current location
    wmove(s.bufferwin, s.b->y - s.y, s.b->x - s.x);

    // refresh windows
    refresh();
    wrefresh(s.statusbar);
    if (s.messagebox != NULL) {
        touchwin(s.messagebox);
        wrefresh(s.messagebox);
    }
    wrefresh(s.linenumbers);
    wrefresh(s.bufferwin);
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
            bmove(s.b, UP);
            break;
        case KEY_DOWN:
            bmove(s.b, DOWN);
            break;
        case KEY_RIGHT:
            bmove(s.b, RIGHT);
            break;
        case KEY_LEFT:
            bmove(s.b, LEFT);
            break;
        case KEY_PPAGE:
            bmoveto(s.b, s.b->y - s.y, s.b->x);
            break;
        case KEY_NPAGE:
            bmoveto(s.b, s.b->y + s.y, s.b->x);
            break;
        case KEY_HOME:
            bmoveto(s.b, s.b->y, 0);
            break;
        case KEY_END:
            bmoveto(s.b, s.b->y, s.b->lines[s.b->y]->len);
            break;

        case KEY_CTRL('q'):
            screen_shutdown();
            delbuf(s.b);
            exit(0);
            break;

        case KEY_CTRL('s'):
            screen_getfilename();
            if (s.b->name != NULL) {
                writebuf(s.b);
            }
            break;

        case KEY_CTRL('o'):
            screen_open();
            break;

        case KEY_CTRL('h'):
            screen_message("Ctrl-S to save buffer, Ctrl-Q to quit");
            break;

        case KEY_CTRL('x'):
            if (s.messagebox != NULL) {
                delwin(s.messagebox);
                s.messagebox = NULL;
            }
            break;

        case KEY_BACKSPACE:
        case 127:
            if (s.b->x > 0) {
                bmove(s.b, LEFT);
                bdelch(s.b, s.b->y, s.b->x);
            } else if (s.b->y > 0) {
                bmoveto(s.b, s.b->y - 1, s.b->lines[s.b->y - 1]->len);
                bdelbreak(s.b, s.b->y + 1);
            }
            break;

        case KEY_ENTER:
        case 13:
            baddbreak(s.b, s.b->y, s.b->x);
            bmoveto(s.b, s.b->y + 1, 0);
            break;

        case '\t':
            for (int i = 0; i < 4; i++) {
                baddch(s.b, ' ', s.b->y, s.b->x);
                bmove(s.b, RIGHT);
            }
            break;

        default:
            if (screen_is_printable(c)) {
                baddch(s.b, c, s.b->y, s.b->x);
                bmove(s.b, RIGHT);
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

    // create buffer
    if (argc > 1) {
        s.b = readbuf(argv[1]);
    } else {
        s.b = newbuf();
    }

    // make sure the buffer has at least one line
    if (s.b->len == 0) {
        baddline(s.b, 0);
        s.b->dirty = false;
    }

    // set initial screen state
    getmaxyx(stdscr, s.maxy, s.maxx);
    set_tabsize(TABSTOP);
    s.bufferwin = newwin(s.maxy - 1, s.maxx - 4, 0, 4);
    s.linenumbers = newwin(s.maxy - 1, 4, 0, 0);
    s.statusbar = newwin(1, s.maxx, s.maxy - 1, 0);
    s.messagebox = NULL;
    s.y = s.x = 0;

    while (TRUE) {
        screen_update();
        screen_input();
    }
}
