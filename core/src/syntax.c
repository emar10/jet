/**
 * syntax.c
 * Implements functions for matching syntax
 * Copyright (c) 2018 Ethan Martin
 */

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#include <core/attribute.h>
#include <core/syntax.h>
#include <core/regex.h>
#include <core/line.h>

typedef struct _rule {
    char *s;
    int len;
    enum attr_type type;
} rule;

static bool syntax_enabled = false;

/*
 * rule arrays:
 *  - key is for keywords, those are simply literal matches
 *  - reg is for a regular expression to match
 *  - enc is encapsulations. enc_b stores the beginning rules while enc_e stores
 *      the corresponding end rules
 */
static rule *key, *reg, *enc_b, *enc_e;
static int key_len, reg_len, enc_len;

/* private functions */
static bool test_keyword(const line *l, int i, rule r);
static int test_regex(const line *l, int i, rule r);

/* sets up syntax  */
void syntax_init(buffer *b) {
    // if we don't have a name, no syntax for you
    if (b->name == NULL) {
        return;
    }

    // grab the extension
    const char *ext = strchr(b->name, '.');
    if (!ext || ext == b->name || *(ext + 1) == '\0') { // did we get it?
        return;
    }
    ext++;

    // if we don't support the extension, no syntax for you
    if (!(strcmp(ext, "c") == 0 || strcmp(ext, "h") == 0)) {
        return;
    }

    // hoo boy we're in the clear, let's go
    // keywords first
    const char* keywords[] = {
        "break", "case", "char", "const", "continue", "default", "do",
        "double", "else", "enum", "extern", "float", "for", "goto", "if",
        "int", "long", "return", "short", "signed", "sizeof", "static",
        "struct", "switch", "typedef", "union", "unsigned", "void", "while"
    };
    key_len = 29;
    key = malloc(sizeof(rule) * 29);
    for (int i = 0; i < key_len; i++) {
        key[i].len = strlen(keywords[i]);
        key[i].s = malloc(key[i].len + 1);
        strcpy(key[i].s, keywords[i]);
        key[i].type = COLOR1;
    }

    // regular expressions
    const char* comment = "//.*$";
    const char* preprocessor = "#.+$";
    reg_len = 2;
    reg = malloc(sizeof(rule) * reg_len);

    // single line comments
    reg[0].len = strlen(comment);
    reg[0].s = malloc(strlen(comment + 1));
    strcpy(reg[0].s, comment);
    reg[0].type = COLOR2;

    // preprocessor commands
    reg[1].len = strlen(preprocessor);
    reg[1].s = malloc(strlen(preprocessor) + 1);
    strcpy(reg[1].s, preprocessor);
    reg[1].type = COLOR3;

    // encapsulations
    enc_len = 0;

    syntax_enabled = true;
}

/* clears all syntax rules */
void syntax_end() {
    for (int i = 0; i < key_len; i++) {
        free(key[i].s);
    }
    free(key);
    key_len = 0;
}

/* generate syntax attributes for the given buffer */
void gen_syntax(buffer *b) {
    // only proceed if syntax is enabled
    if (!syntax_enabled) {
        return;
    }

    int y, x;
    line *prev, *curr, *next;

    // iterate over each line
    for (y = 0; y < b->len; y++) {
        x = 0;
        curr = b->lines[y];

        // only regenerate if needed
        if (!curr->needs_update) {
            continue;
        }

        lclrattrs(curr);

        // check each x for matches to any rule
        do {
            bool matched = false;
            // regexes
            if (!matched) {
                for (int i = 0; i < reg_len; i++) {
                    rule r = reg[i];
                    int len;

                    if ((len = test_regex(curr, x, r)) != -1) {
                        // create attrs
                        attribute beg = { r.type, true };
                        attribute end = { r.type, false };

                        // add attributes and update x
                        laddattr(curr, beg, x);
                        x += len;
                        if (x < curr->len) {
                            laddattr(curr, end, x);
                        }

                        matched = true;
                        break;
                    }
                }
            }

            // keywords
            if (!matched) {
                for (int i = 0; i < key_len; i++) {
                    rule r = key[i];

                    if (test_keyword(curr, x, r)) {
                        // create attrs
                        attribute beg = { r.type, true };
                        attribute end = { r.type, false };

                        // add attributes and update x
                        laddattr(curr, beg, x);
                        x += r.len;
                        if (x < curr->len) {
                            laddattr(curr, end, x);
                        }

                        break;
                    }
                }
            }

            x++;
        } while (x < curr->len);

        curr->needs_update = false;
    }
}

/* checks for matches to rule r with a keyword in l at index i */
static bool test_keyword(const line *l, int i, rule r) {
    // do we have the word?
    if (re_match(r.s, l->s + i) == -1) {
        return false;
    }

    // is this an actual word?
    if ((i != 0 && isalpha(l->s[i - 1]))
            || (i + r.len != l->len && isalpha(l->s[i + r.len]))) {
        return false;
    }

    return true;
}

/* checks for matches to rule r with provided index and regex. returns -1 if no
 * match is found, length of the match otherwise */
static int test_regex(const line *l, int i, rule r) {
    int len = re_match(r.s, l->s + i);

    return len;
}

