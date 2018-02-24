/*
 * buffer.h
 * Contains buffer struct and declares functions for manipulation of an internal buffer
 * Copyright (c) 2018 Ethan Martin
 */

#include <stdbool.h>

#include "line.h"

typedef struct buffer {
    int y, x;
    int sy, sx;
    int len;
    line *lines;
    char *name;
    bool dirty;
} buffer;

/* used for movement */
enum direction {
    UP, DOWN, LEFT, RIGHT
};

/* create a new buffer */
buffer *newbuf();

/* clean up the buffer */
void delbuf(buffer *b);

/* insert an empty line into the buffer */
void baddline(buffer *b, int y);

/* remove a line from the buffer */
void bdelline(buffer *b, int y);

/* insert a character at the given location */
void baddchat(buffer *b, const char c, int y, int x);

/* insert a string at the given location */
void baddstrat(buffer *b, const char *s, int len, int y, int x);

/* remove the character at the given location */
void bdelchat(buffer *b, int y, int x);

/* insert a line break */
void baddbreak(buffer *b, int y);

/* remove a line break */
void bdelbreak(buffer *b, int y);

/* move to the nearest valid location to the given coordinates */
void bmoveto(buffer *b, int y, int x);

/* move in the given direction */
void bmove(buffer *b, enum direction dir);
