/*
 * buffer.c
 * Contains functions for manipulating buffers
 * Copyright (c) 2018 Ethan Martin
 */

#include <stdlib.h>

#include "buffer.h"

/* returns a new, empty buffer */
buffer *newbuf() {
    buffer *b = malloc(sizeof(buffer));
    b->y = b->x = 0;
    b->sy = b->sx = 0;
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
        delline(&b->lines[i]);
    }
    free(b->lines);

    // delete the filename
    free(b->name);

    // free the buffer
    free(b);
}

/* insert an empty line into the buffer */
void baddline(buffer *b, int y) {

}

/* remove a line */
void bdelline(buffer *b, int y) {

}

/* insert a character */
void baddchat(buffer *b, const char c, int y, int x) {

}

/* insert a string */
void baddstrat(buffer *b, const char *s, int len, int y, int x) {

}

/* remove a character */
void bdelchat(buffer *b, int y, int x) {

}

/* insert a line break */
void baddbreak(buffer *b, int y) {

}

/* remove a line break */
void bdelbreak(buffer *b, int y) {

}

/* move to the given location */
void bmoveto(buffer *b, int y, int x) {

}

/* move in the given direction */
void bmove(buffer *b, enum direction dir) {

}

