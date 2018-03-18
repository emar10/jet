/*
 * file.c
 * Functions for reading/writing files to/from buffers
 * Copyright (c) 2018 Ethan Martin
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <core/file.h>
#include <core/jet.h>

/* opens a file using the given filename, and parses it into a buffer */
buffer *readbuf(const char *filename) {
    buffer *b;

    // create the buffer
    b = newbuf();

    // name the buffer
    bname(b, filename);

    // if the file does not exist, return empty
    if (access(filename, F_OK)) {
        return b;
    }

    // otherwise, attempt to open
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        die("Failed to open file for reading", 1);
    }

    // read lines to the buffer
    char c;
    int i = 0;
    line *l = newline();
    do {
        c = getc(f);
        if (c == '\n') {
            bappendline(b, l);
            l = newline();
            i = 0;
        } else {
            laddch(l, c, i);
            i++;
        }
    } while (c != EOF);
    fclose(f);

    b->dirty = false;

    return b;
}

/* attempts to write a buffer to the given file. returns -1 if it fails */
/* TODO return bytes written */
int writebufto(buffer *b, const char *filename) {
    // attempt to open file for writing
    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        return -1;
    }

    // write the file
    for (int i = 0; i < b->len; i++) {
        fputs(b->lines[i]->s, f);
        fputc('\n', f);
    }

    // close
    fclose(f);
    b->dirty = false;
    return 0;
}

/* same as above, uses filename from buffer */
int writebuf(buffer *b) {
    return writebufto(b, b->name);
}
