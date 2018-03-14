cmake_minimum_required(VERSION 3.5)
project(core)

# build
add_library(core_lib
            include/buffer.h
            include/file.h
            include/line.h
            include/jet.h
            src/buffer.c
            src/file.c
            src/line.c)
target_include_directories(core_lib PUBLIC include)

