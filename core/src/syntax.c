/**
 * syntax.c
 * Implements functions for matching syntax
 */

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <core/syntax.h>

/* set up rules for c */
void syntax_init() {
    rules = malloc(sizeof(rule) * 2);
    rules_len = 2;

    // keywords
    rules[0].s = malloc(sizeof(char*) * 16);
    rules[0].len = 16;
    const char *keywords[] = {
        "break ",
        "case ",
        "char ",
        "double ",
        "else ",
        "enum ",
        "float ",
        "for ",
        "if ",
        "int ",
        "return ",
        "short ",
        "struct ",
        "switch ",
        "void ",
        "while "
    };
    for (int i = 0; i < 16; i++) {
        rules[0].s[i] = malloc(strlen(keywords[i]) + 1);
        strcpy(rules[0].s[i], keywords[i]);
    }

    // single line comment
    rules[1].s = malloc(sizeof(char*));
    rules[1].s[0] = malloc(3);
    rules[1].len = 1;
    strcpy(rules[1].s[0], "//");

    syntax_enabled = true;
}

/* clean up rules */
void syntax_end() {
    for (int i = 0; i < rules_len; i++) {
       free(rules[i].s);
    }
    free(rules);

    syntax_enabled = false;
}

/* check a string for matches to the given rule. returns the length of the
 * match if found, -1 otherwise. */
int match_rule(const char *s, rule r) {
    int match;

    for (int i = 0; i < r.len; i++) {
        for (match = 0; r.s[i][match] != '\0' && s[match] != '\0'; match++) {
            if (r.s[i][match] != s[match]) {
                break;
            }
        }

        if (r.s[i][match] == '\0') {
            return match;
        }
    }
    
    return -1;
}

