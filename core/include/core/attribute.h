/*
 * attribute.h
 * Contains attribute struct for marking special formatting in lines
 * Copyright (c) 2018 Ethan Martin
 */

#include <stdbool.h>

/* define possible attributes */
enum attr_type {
    BOLD,
    HIGHLIGHT,
    COLOR1,
    COLOR2,
    COLOR3,
    COLOR4
}

/* contains attribute type, whether it is turned on/off, and location in line */
typedef struct attribute {
    enum attr_type;
    bool enabled;
    int i;
}

