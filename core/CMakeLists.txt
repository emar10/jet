cmake_minimum_required(VERSION 3.5)
project(core)

# build
add_library(core_lib
            include/core/buffer.h
            include/core/file.h
            include/core/line.h
            include/core/attribute.h
            include/core/syntax.h
            include/core/jet.h
            src/buffer.c
            src/file.c
            src/line.c
            src/attribute.c
            src/syntax.c
            )
target_include_directories(core_lib PUBLIC include)

