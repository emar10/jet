cmake_minimum_required(VERSION 3.0)

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)

set(SOURCE_FILES src/term/jet.c
                 src/core/jet.h
                 src/core/buffer.c
                 src/core/buffer.h
                 src/core/file.c
                 src/core/file.h
                 src/core/line.c
                 src/core/line.h)

add_executable(jet ${SOURCE_FILES})

target_link_libraries(jet PRIVATE ncurses)
