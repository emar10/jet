/*
 * attribute.c
 * Implements attribute functions
 * Copyright (c) 2018 Ethan Martin
 */

#include <stdlib.h>

#include <core/attribute.h>

/* creates and returns an empty attribute */
attribute nullattr() {
    attribute a = { NONE, false };
    return a;
}

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

