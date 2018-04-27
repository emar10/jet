/**
 * syntax.c
 * Implements functions for matching syntax
 * Copyright (c) 2018 Ethan Martin
 */

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <dirent.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#ifdef __linux__
#include <unistd.h>
#endif

#include <core/attribute.h>
#include <core/syntax.h>
#include <core/regex.h>
#include <core/line.h>
#include <core/file.h>

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

static char **filetypes;
static char **jsrfiles;
static int fileslen;

/* private functions */
static bool test_keyword(const line *l, int i, rule r);
static int test_regex(const line *l, int i, rule r);

/* populates list of supported filetypes */
void syntax_readfiles() {
    fileslen = 0;

    // get the syntax directory using <exedir>/../share/jet/syntax
    const char *syntax_loc = "/../share/jet/syntax";
    char dirpath[1024];

#ifdef __APPLE__
    uint32_t len = 1024;
    _NSGetExecutablePath(dirpath, &len);
#endif

#ifdef __linux__
    size_t len;
    len = readlink("/proc/self/exe", dirpath, sizeof(dirpath) - 1);
    if (len != -1) {
        dirpath[len] = '\0';
    }
#endif
    if (dirpath[0] == '\0') {
        return;
    }
    dirpath[strlen(dirpath) - 4] = '\0';
    strcat(dirpath, syntax_loc);

    // grab all the syntax files
    DIR *d = opendir(dirpath);
    struct dirent *curr;
    if (!d) {
        return;
    }
    while ((curr = readdir(d)) != NULL) {
        // ignore ./ and ../
        if (strcmp(curr->d_name, ".") == 0 || strcmp(curr->d_name, "..") == 0) {
            continue;
        }

        char fpath[512];
        strcpy(fpath, dirpath);
        strcat(fpath, "/");
        strcat(fpath, curr->d_name);

        // read the first line of the file and add entries to available filetypes
        buffer *buf = readbuf(fpath);
        if (buf->len == 0) {
            delbuf(buf);
            closedir(d);
            return;
        }
        char *ext = strtok(buf->lines[0]->s, " ");
        while (ext != NULL) {
            filetypes = realloc(filetypes, sizeof(char*) * fileslen + 1);
            filetypes[fileslen] = malloc(strlen(ext) + 1);
            strcpy(filetypes[fileslen], ext);

            jsrfiles = realloc(jsrfiles, sizeof(char*) * fileslen + 1);
            jsrfiles[fileslen] = malloc(strlen(fpath) + 1);
            strcpy(jsrfiles[fileslen], fpath);

            fileslen++;
            ext = strtok(NULL, " ");
        }
    }

    closedir(d);
}

/* clears the list of supported filetypes */
void syntax_clearfiles() {

}

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
    char *fname = NULL;
    for (int i = 0; i < fileslen; i++) {
        if (strcmp(ext, filetypes[i]) == 0) {
            fname = jsrfiles[i];
            break;
        }
    }
    if (fname == NULL) {
        return;
    }

    // hoo boy we're in the clear, let's go
    buffer *syn = readbuf(fname);

    // for each line (except the first)
    for (int y = 1; y < syn->len; y++) {
        line *l = syn->lines[y];
        enum attr_type a;

        char type[512];
        char attr[512];
        char val[2048];

        // parse our data
        sscanf(l->s, "%s %s %[^\n]", attr, type, val);

        // set the attribute
        if (strcmp(attr, "HIGHLIGHT") == 0) {
            a = HIGHLIGHT;
        }
        else if (strcmp(attr, "BOLD") == 0) {
            a = BOLD;
        }
        else if (strcmp(attr, "COLOR1") == 0) {
            a = COLOR1;
        }
        else if (strcmp(attr, "COLOR2") == 0) {
            a = COLOR2;
        }
        else if (strcmp(attr, "COLOR3") == 0) {
            a = COLOR3;
        }
        else if (strcmp(attr, "COLOR4") == 0) {
            a = COLOR4;
        }
        else a = NONE;

        // keywords
        if (strcmp(type, "KEY") == 0) {
            char *k = strtok(val, " ");
            while (k != NULL) {
                rule r;
                r.len = strlen(k);
                r.s = malloc(r.len + 1);
                strcpy(r.s, k);
                r.type = a;

                key_len++;
                key = realloc(key, sizeof(rule) * key_len);
                key[key_len - 1] = r;

                k = strtok(NULL, " ");
            }
        }

        // regex
        else if (strcmp(type, "REG") == 0) {
            rule r;
            r.len = strlen(val);
            r.s = malloc(r.len + 1);
            strcpy(r.s, val);
            r.type = a;

            reg_len++;
            reg = realloc(reg, sizeof(rule) * reg_len);
            reg[reg_len - 1] = r;
        }

        // encapsulation
        else if (strcmp(type, "ENC") == 0) {
            rule b, e;
            char bs[128], es[128];            
            sscanf(val, "%s %s", bs, es);

            b.len = strlen(bs);
            b.s = malloc(b.len + 1);
            strcpy(b.s, bs);
            b.type = a;

            e.len = strlen(es);
            e.s = malloc(e.len + 1);
            strcpy(e.s, es);
            e.type = a;

            enc_len++;
            enc_b = realloc(enc_b, sizeof(rule) * enc_len);
            enc_b[enc_len - 1] = b;

            enc_e = realloc(enc_e, sizeof(rule) * enc_len);
            enc_e[enc_len - 1] = e;
        }
    }

    delbuf(syn);

    syntax_enabled = true;
}

/* clears all syntax rules */
void syntax_end() {
    for (int i = 0; i < key_len; i++) {
        free(key[i].s);
    }
    free(key);
    key_len = 0;

    for (int i = 0; i < reg_len; i++) {
        free(reg[i].s);
    }
    free(reg);
    reg_len = 0;

    for (int i = 0; i < enc_len; i++) {
        free(enc_b[i].s);
        free(enc_e[i].s);
    }
    free(enc_b);
    free(enc_e);
    enc_len = 0;

    syntax_enabled = false;
}

/* generate syntax attributes for the given buffer */
void gen_syntax(buffer *b) {
    // only proceed if syntax is enabled
    if (!syntax_enabled) {
        return;
    }

    int y, x;
    line *prev, *curr, *next;
    int curr_enc = -1;

    // iterate over each line
    for (y = 0; y < b->len; y++) {
        bool enc_ended = false;
        x = 0;
        if (y > 0) {
            prev = b->lines[y - 1];
        } else {
            prev = NULL;
        }
        curr = b->lines[y];
        if (y < b->len - 1) {
            next = b->lines[y + 1];
        } else {
            next = NULL;
        }

        // only regenerate if needed
        if (!curr->needs_update) {
            continue;
        }

        lclrattrs(curr);

        // if we're in an encapsulation, add begin attribute
        if (curr_enc != -1 && curr->len > 0) {
            attribute a = { enc_b[curr_enc].type, true };
            laddattr(curr, a, 0);
        }

        // check each x for matches to any rule
        do {
            bool matched = false;
            // closing encapsulations
            if (curr_enc != -1) {
                rule r = enc_e[curr_enc];
                int len;

                if ((len = re_match(r.s, curr->s + x)) != -1) {
                    // create the attribute
                    attribute a = { r.type, false };

                    x += len;
                    if (x < curr->len) {
                        laddattr(curr, a, x);
                    }

                    // following lines need to be updated
                    for (int j = y + 1; j < b->len; j++) {
                        b->lines[j]->needs_update = true;
                    }

                    curr_enc = -1;
                }

                enc_ended = true;
                matched = true;
            }

            // opening encapsulations
            if (!matched) {
                for (int i = 0; i < enc_len; i++) {
                    rule r = enc_b[i];
                    int len;

                    if ((len = re_match(r.s, curr->s + x)) != -1) {
                        // create attr
                        attribute a = { r.type, true };

                        laddattr(curr, a, x);
                        x += len - 1;

                        curr_enc = i;
                        matched = true;
                        break;
                    }
                }
            }

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

        // if we are in an encapsulation, we need to update the next line
        if (next != NULL && curr_enc != -1) {
            next->needs_update = true;
        }

        // if we're in an encapsulation, we always want this line to update
        if (curr_enc == -1 && !enc_ended) {
            curr->needs_update = false;
        }
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

