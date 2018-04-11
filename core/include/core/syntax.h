/**
 * syntax.h
 * Functions for matching text with provided rules for syntax highlighting.
 */

#ifndef SYNTAX_H
#define SYNTAX_H

#include <stdio.h>
#include <stdbool.h>

/* contains information required for a rule */
typedef struct rule {
    char **s;
    int len;
} rule;
rule *rules;
int rules_len;
bool syntax_enabled;

/* initializes rules for c */
void syntax_init();

/* cleans up rules */
void syntax_end();

/* check a provided string for matches to the given rule. returns the length of
 * the match if found, or -1 if not found. */
int match_rule(const char *s, rule r);

#endif

