set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)

set(SOURCE_FILES jet.c
                 src/core/line.c
                 src/core/line.h)

add_executable(jet ${SOURCE_FILES})

target_link_libraries(jet PRIVATE Curses)
