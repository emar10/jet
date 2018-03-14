cmake_minimum_required(VERSION 3.5)
project(jet VERSION 0.1.0 LANGUAGES C)

# enable testing through cmake
enable_testing()

# dependencies
set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)

# core module
add_subdirectory(core)

add_executable(jet term/src/jet.c)
target_link_libraries(jet PRIVATE core_lib ncurses)
