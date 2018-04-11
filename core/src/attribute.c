/*
 * attribute.c
 * Implements attribute functions
 * Copyright (c) 2018 Ethan Martin
 */

#include <stdlib.h>

#include <core/attribute.h>

/* create new attribute with default values */
attribute *newattr() {
    attribute *a = malloc(sizeof(attribute));
    a->type = NORMAL;
    a->enabled = true;

    return a;
}

/* free the attribute */
void delattr(attribute *a) {
    free(a);
}

