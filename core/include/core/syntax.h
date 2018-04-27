/**
 * syntax.h
 * Functions for matching text with provided rules for syntax highlighting.
 */

#pragma once

#include <core/buffer.h>

#include <stdio.h>
#include <stdbool.h>

/* populates the list of supported file types */
void syntax_readfiles();

/* initializes syntax rules based on filetype */
void syntax_init(buffer *b);

/* cleans up rules */
void syntax_end();

/* generate attributes for the given buffer */
void gen_syntax(buffer *b);

