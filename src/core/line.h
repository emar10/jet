/*
 * line.h
 * Contains line struct and declares functions for line manipulation
 * Copyright (c) 2018 Ethan Martin
 */

typedef struct line {
    int len;
    char *s;
} line;

/* create a new empty line */
line *newline();

/* free a line */
void delline(line *l);

/* grow or shrink a line */
void lresize(line *l, int len);

/* add a character to the line */
void laddch(line *l, const char c, int i);

/* add a string to the line */
void laddstr(line *l, const char *s, int len, int i);

/* remove the character at the given index */
void ldelch(line *l, int i);

