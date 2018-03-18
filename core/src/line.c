/*
 * line.c
 * Contains functions for manipulating lines
 * Copyright (c) 2018 Ethan Martin
 */

#include <string.h>
#include <stdlib.h>

#include <core/line.h>

/* creates a new, empty line */
line *newline() {
    line *l = malloc(sizeof(line));

    l->s = malloc(1);
    l->s[0] = '\0';

    l->len = 0;

    return l;
}

/* cleans up the line */
void delline(line *l) {
    free(l->s);
    free(l);
}

/* grow or shrink a line */
void lresize(line *l, int len) {
    l->s = realloc(l->s, len + 1);
    l->s[len] = '\0';

    l->len = len;
}

/* add a character to the line at the given index */
void laddch(line *l, const char c, int i) {
    lresize(l, l->len + 1);

    // offset memory if needed
    if (l->s[i] != '\0' && i < l->len) {
        memmove(&l->s[i + 1], &l->s[i], l->len - i);
    }

    // insert the char
    l->s[i] = c;
}

/* add a string to the line at the given index */
void laddstr(line *l, const char *s, int len, int i) {
    int oldlen = l->len;
    lresize(l, l->len + len);

    // offset if needed
    if (i < oldlen) {
        memmove(&l->s[i + len], &l->s[i], oldlen - i);
    }

    // insert the string
    memcpy(&l->s[i], s, len);
}

/* delete the character at the given index */
void ldelch(line *l, int i) {
    if (i < l->len) {
        memmove(&l->s[i], &l->s[i + 1], l->len - i - 1);
    }

    lresize(l, l->len - 1);
}

