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

    l->attrs = NULL;
    l->needs_update = true;

    return l;
}

/* cleans up the line */
void delline(line *l) {
    free(l->s);
    if (l->attrs != NULL) {
        for (int i = 0; i < l->len; i++) {
            delattr(l->attrs[i]);
        }
        free(l->attrs);
    }
    free(l);
}

/* grow or shrink a line */
void lresize(line *l, int len) {
    l->s = realloc(l->s, len + 1);
    l->s[len] = '\0';

    // if we're shrinking, free any attributes that will be out of bounds 
    if (len < l->len) {
        for (int i = len; i < l->len; i++) {
            if (l->attrs[i] != NULL) {
                delattr(l->attrs[i]);
            }
        }
    }
    l->attrs = realloc(l->attrs, sizeof(attribute*) * len);
    // if we're growing, fill the new space with null pointers
    if (len > l->len) {
        for (int i = l->len; i < len; i++) {
            l->attrs[i] = NULL;
        }
    }

    l->len = len;
    l->needs_update = true;
}

/* add an attribute to the line */
void laddattr(line *l, attribute *a, int i) {
    lrmattr(l, i);
    l->attrs[i] = a;
}

/* remove an attribute from the line */
void lrmattr(line *l, int i) {
    if (l->attrs[i] != NULL) {
        delattr(l->attrs[i]);
        l->attrs[i] = NULL;
    }
}

/* clear the attributes from the line */
void lclrattrs(line *l) {
    for (int i = 0; i < l->len; i++) {
        lrmattr(l, i);
    }
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
    l->needs_update = true;
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
    l->needs_update = true;
}

/* delete the character at the given index */
void ldelch(line *l, int i) {
    if (i < l->len) {
        memmove(&l->s[i], &l->s[i + 1], l->len - i - 1);
    }

    lresize(l, l->len - 1);
    l->needs_update = true;
}

