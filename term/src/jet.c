/*
 * jet.c
 * Ncurses Jet. For help, see README.md in the root directory.
 * Copyright (c) 2018 Ethan Martin
 */

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_PATCH 1

#include <ncurses.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <core/jet.h>

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

bool screen_confirmquit() {
    char response[80];
    int len;
    bool invalid;

    do {
        screen_read_message(response, "File has not been saved! Really quit? (y/N): ");
        len = strlen(response);

        invalid = false;
        switch (len) {
            case 0:
                return false;

            case 1:
                if (response[0] == 'y' || response[0] == 'Y') {
                    return true;
                }
                if (response[0] == 'n' || response[0] == 'N') {
                    return false;
                }
                invalid = true;
                break;

            default:
                invalid = true;
                break;
        }
    } while (invalid);
    return false;
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
                if (l->attrs[x + s.x].type != NONE) {
                    attribute a = l->attrs[x + s.x];
                    short curses_attr;

                    switch (a.type) {
                        case COLOR1:
                            curses_attr = COLOR_PAIR(1);
                            break;

                        case COLOR2:
                            curses_attr = COLOR_PAIR(2);
                            break;

                        case COLOR3:
                            curses_attr = COLOR_PAIR(3);
                            break;

                        case COLOR4:
                            curses_attr = COLOR_PAIR(4);
                            break;

                        default:
                            curses_attr = A_NORMAL;
                            break;
                    }

                    if (a.enabled) {
                        wattron(s.bufferwin, curses_attr);
                    } else {
                        wattroff(s.bufferwin, curses_attr);
                    }
                }

                waddch(s.bufferwin, (chtype)l->s[x + s.x]);
                x++;
            }
        }
        wattrset(s.bufferwin, A_NORMAL);
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

    // generate syntax
    gen_syntax(s.b);

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

void screen_resize() {
    getmaxyx(stdscr, s.maxy, s.maxx);

    // move and resize windows
    wresize(s.bufferwin, s.maxy - 1, s.maxx - 4);
    wresize(s.linenumbers, s.maxy - 1, 4);
    wresize(s.statusbar, 1, s.maxx);
    mvwin(s.statusbar, s.maxy - 1, 0);
    if (s.messagebox != NULL) {
        wresize(s.messagebox, 1, s.maxx);
        mvwin(s.messagebox, s.maxy - 1, 0);
    }
    erase();
}

void screen_input() {
    int c = getch();

    switch (c) {
        case KEY_RESIZE:
            screen_resize();
            break;
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
            bmoveto(s.b, s.b->y - (s.maxy - 1) - 1, s.b->x);
            break;
        case KEY_NPAGE:
            bmoveto(s.b, s.b->y + (s.maxy - 1) - 1, s.b->x);
            if (s.b->y < s.b->len - 1) {
                s.y = s.b->y;
            }
            break;
        case KEY_HOME:
            bmoveto(s.b, s.b->y, 0);
            break;
        case KEY_END:
            bmoveto(s.b, s.b->y, s.b->lines[s.b->y]->len);
            break;

        case KEY_CTRL('q'):
            if (!s.b->dirty || screen_confirmquit()) {
                screen_shutdown();
                delbuf(s.b);
                exit(0);
            }
            break;

        case KEY_CTRL('s'):
            screen_getfilename();
            if (s.b->name != NULL) {
                if (writebuf(s.b) != -1) {
                    screen_message("File successfully written.");
                } else {
                    screen_message("Failed to write file.");
                }
            } else {
                screen_message("Save aborted.");
            }
            break;

        case KEY_CTRL('o'):
            screen_open();
            break;

        case KEY_CTRL('h'):
            screen_message("Ctrl-S to save buffer, Ctrl-O to open file, Ctrl-Q to quit");
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

    // setup colors
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);
    init_pair(3, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(4, COLOR_CYAN, COLOR_BLACK);

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
    set_tabsize(TABSTOP);
    getmaxyx(stdscr, s.maxy, s.maxx);
    s.bufferwin = newwin(s.maxy - 1, s.maxx - 4, 0, 4);
    s.linenumbers = newwin(s.maxy - 1, 4, 0, 0);
    s.statusbar = newwin(1, s.maxx, s.maxy - 1, 0);
    s.messagebox = NULL;
    s.y = s.x = 0;

    // start syntax
    syntax_init(s.b);

    // add a friendly welcome message
    char message[80];
    sprintf(message, "Welcome to Jet v%d.%d.%d! Use Ctrl-H to display help.", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    screen_message(message);

    while (TRUE) {
        screen_update();
        screen_input();
    }
}

