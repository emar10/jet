/**
 * syntax.c
 * Implements functions for matching syntax
 */

#include <string.h>
#include <stdlib.h>

#include <core/syntax.h>

/* set up rules for c */
void syntax_init() {
    rules = malloc(sizeof(rule) * 2);
    rules_len = 1;

    // keywords
    rules[0].s = malloc(sizeof(char*) * 16);
    rules[0].len = 16;
    const char *keywords[] = {
        "break",
        "case",
        "char",
        "double",
        "else",
        "enum",
        "float",
        "for",
        "if",
        "int",
        "return",
        "short",
        "struct",
        "switch",
        "void",
        "while"
    };
    for (int i = 0; i < 16; i++) {
        strcpy(rules[0].s[i], keywords[i]);
    }
}

/* clean up rules */
void syntax_end() {
    for (int i = 0; i < rules_len; i++) {
       free(rules[i].s); 
    }
    free(rules);
}

/* check a string for matches to the given rule. returns the length of the
 * match if found, -1 otherwise. */
int match_rule(const char *s, int len, rule *r) {
    int i = 0;

    for (i = 0; i < len; i++) {

    }
    
    return -1;
}

