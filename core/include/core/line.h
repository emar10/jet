/*
 * line.h
 * Contains line struct and declares functions for line manipulation
 * Copyright (c) 2018 Ethan Martin
 */

#ifndef LINE_H
#define LINE_H

#include <stdbool.h>

#include <core/attribute.h>

typedef struct line {
    int len;
    char *s;
    attribute **attrs;
    bool needs_update;
} line;

/* create a new empty line */
line *newline();

/* free a line */
void delline(line *l);

/* grow or shrink a line */
void lresize(line *l, int len);

/* add an attribute to the line */
void laddattr(line *l, attribute *a, int i);

/* remove an attribute from the line */
void lrmattr(line *l, int i);

/* clear the attributes from the line */
void lclrattrs(line *l);

/* add a character to the line */
void laddch(line *l, const char c, int i);

/* add a string to the line */
void laddstr(line *l, const char *s, int len, int i);

/* remove the character at the given index */
void ldelch(line *l, int i);

#endif

