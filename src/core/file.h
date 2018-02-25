/*
 * file.h
 * Declares functions for reading/writing files to/from buffers
 * Copyright (c) 2018 Ethan Martin
 */

#include "buffer.h"

/* opens a file using the given filename, and parses it into a buffer */
buffer *readbuf(const char *filename);

/* attempts to write a buffer to the given file. returns -1 if it fails, otherwise the number of
 * bytes written */
int writebufto(buffer *b, const char *filename);

/* same as above, using filename in the buffer */
int writebuf(buffer *b);
