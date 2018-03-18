/*
 * buffer.c
 * Contains functions for manipulating buffers
 * Copyright (c) 2018 Ethan Martin
 */

#include <stdlib.h>
#include <string.h>

#include <core/buffer.h>

/* returns a new, empty buffer */
buffer *newbuf() {
    buffer *b = malloc(sizeof(buffer));
    b->y = b->x = 0;
    b->len = 0;
    b->lines = NULL;
    b->name = NULL;
    b->dirty = false;

    return b;
}

/* clean up and free the buffer */
void delbuf(buffer *b) {
    // delete the lines
    for (int i = 0; i < b->len; i++) {
        delline(b->lines[i]);
    }
    free(b->lines);

    // delete the filename
    free(b->name);

    // free the buffer
    free(b);
}

/* insert an empty line into the buffer */
void baddline(buffer *b, int y) {
    // make room for the line
    b->lines = realloc(b->lines, sizeof(line*) * (b->len + 1));

    // offset if needed
    if (y < b->len) {
        memmove(&b->lines[y + 1], &b->lines[y], sizeof(line*) * (b->len - y));
    }

    // insert the line
    b->lines[y] = newline();
    b->len++;
    b->dirty = true;
}

/* remove a line */
void bdelline(buffer *b, int y) {
    // remove the line
    delline(b->lines[y]);
    b->len--;

    // move memory forward
    if (y < b->len) {
        memmove(&b->lines[y], &b->lines[y + 1], sizeof(line*) * (b->len - y));
    }
    b->dirty = true;

    // realloc
    b->lines = realloc(b->lines, sizeof(line*) * (b->len));
}

/* insert a character */
void baddch(buffer *b, const char c, int y, int x) {
    laddch(b->lines[y], c, x);
    b->dirty = true;
}

/* insert a string */
void baddstr(buffer *b, const char *s, int len, int y, int x) {
    laddstr(b->lines[y], s, len, x);
    b->dirty = true;
}

/* insert an existing line at the end of the buffer */
void bappendline(buffer *b, line *l) {
    b->lines = realloc(b->lines, sizeof(line**) * b->len + 1);
    b->lines[b->len] = l;
    b->len++;
    b->dirty = true;
}

/* remove a character */
void bdelch(buffer *b, int y, int x) {
    ldelch(b->lines[y], x);
    b->dirty = true;
}

/* insert a line break */
void baddbreak(buffer *b, int y, int x) {
    // insert blank line
    baddline(b, y + 1);

    // if needed, append string to new line and shorten previous
    if (x < b->lines[y]->len) {
        line *next = b->lines[y + 1];
        line *prev = b->lines[y];

        laddstr(next, &prev->s[x], prev->len - x, 0);
        lresize(prev, x);
    }
    b->dirty = true;
}

/* remove a line break */
void bdelbreak(buffer *b, int y) {
    // do nothing to top line
    if (y == 0) {
        return;
    }

    // if needed, append current to previous
    if (b->lines[y]->len > 0) {
        line *prev = b->lines[y - 1];
        line *curr = b->lines[y];

        laddstr(prev, curr->s, curr->len, prev->len);
    }

    // remove the current line
    bdelline(b, y);
    b->dirty = true;
}

/* move to the given location */
void bmoveto(buffer *b, int y, int x) {
    // first choose y
    if (y < 0) {
        b->y = 0;
    } else if (y > b->len - 1) {
        b->y = b->len - 1;
    } else {
        b->y = y;
    }

    // then choose x
    line *l = b->lines[b->y];
    if (x < 0) {
        b->x = 0;
    } else if (x > l->len) {
        b->x = l->len;
    } else {
        b->x = x;
    }
}

/* move in the given direction */
void bmove(buffer *b, enum direction dir) {
    switch (dir) {
        case UP:
            bmoveto(b, b->y - 1, b->x);
            break;

        case DOWN:
            bmoveto(b, b->y + 1, b->x);
            break;

        case LEFT:
            bmoveto(b, b->y, b->x - 1);
            break;

        case RIGHT:
            bmoveto(b, b->y, b->x + 1);
    }
}

/* name the buffer */
void bname(buffer *b, const char *name) {
    b->name = realloc(b->name, strlen(name) + 1);
    strcpy(b->name, name);
}

