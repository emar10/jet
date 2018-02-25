/*
 * file.c
 * Functions for reading/writing files to/from buffers
 * Copyright (c) 2018 Ethan Martin
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "file.h"
#include "jet.h"

/* opens a file using the given filename, and parses it into a buffer */
buffer *readbuf(const char *filename) {
    buffer *b;

    // if the file does not exist, return NULL
    if (access(filename, F_OK)) {
        return NULL;
    }

    // otherwise, attempt to open
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        die("Failed to open file for reading", 1);
    }

    // create the buffer
    b = newbuf();

    // read lines to the buffer
    char *curr = NULL;
    size_t len;
    ssize_t read;
    while ((read = getline(&curr, &len, f)) != -1) {
        while (curr[read - 1] == '\n' || curr[read - 1] == '\r') {
            read--;
            curr[read] = '\0';
        }
        line *l = newline();
        laddstr(l, curr, read, 0);

        bappendline(b, l);
    }
    free(curr);
    fclose(f);

    // name the buffer
    bname(b, filename);
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
