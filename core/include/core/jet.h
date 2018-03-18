/*
 * jet.h
 * Declares platform-dependent functions required of a Jet implementation
 * Copyright (c) 2018 Ethan Martin
 */

#include <core/buffer.h>
#include <core/file.h>

/* fatal error, print a message and terminate the program */
void die(const char *error, int code);
