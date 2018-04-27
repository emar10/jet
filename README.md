# Jet v1.0
"Snappy" terminal-based \*nix editor written in C using ncurses.

## Prerequisites
Jet requires Ncurses, and CMake for building.

## Building
Clone, `cmake ./`, `make`. Binary is written as `jet`.

## Usage
Jet can open existing files or start with an empty buffer. Changes can be saved with Ctrl-S, and
new files can be opened with Ctrl-O.

Run using `jet <filename>`. Arrow keys navigate through the file, Page Up and Page Down to scroll,
Home and End snap to beginning/end of line. Ctrl-Q to quit.

Jet will automatically highlight syntax for supported filetypes. See below for more info.

## Syntax Highlighting
Syntax highlighting is done through all `.jsr` files found at `../share/jet/syntax/` relative to
Jet's executable. The package currently includes highlighting for C and Java. If you are interested
in creating your own syntax files, refer to the existing rule files and `syntax.c`.

## A note on trustworthiness
Jet is now at the point where it can theoretically be used as a general-purpose editor.
That said, it may still behave strangely under certain circumstances, and I do not suggest using it
for anything mission critical, *especially* for files not under version control or some other backup
mechanism. By using it, you accept any risk of data loss, equipment damage, thermonuclear war, etc.

As always, please do report any bugs that you run across, so that I can continue to improve
reliability and stability.
