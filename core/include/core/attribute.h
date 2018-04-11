/*
 * attribute.h
 * Contains attribute struct for marking special formatting in lines
 * Copyright (c) 2018 Ethan Martin
 */

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <stdbool.h>

/* define possible attributes */
enum attr_type {
    NORMAL,
    BOLD,
    HIGHLIGHT,
    COLOR1,
    COLOR2,
    COLOR3,
    COLOR4
};

/* contains attribute type, whether it is turned on/off, and location in line */
typedef struct attribute {
    enum attr_type type;
    bool enabled;
} attribute;

/* create new attribute pointer */
attribute *newattr();

/* free the attribute */
void delattr(attribute *a);

#endif

