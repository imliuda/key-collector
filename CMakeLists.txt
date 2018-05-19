cmake_minimum_required(VERSION 2.4)

project(osclt)

option(BUILD_STATIC "build osclt static link" ON)

include_directories(include BEFORE)

file(GLOB LIBOC_SRC
    lib/*.c
)
add_library(osclt-shared SHARED ${LIBOC_SRC})
set_target_properties(osclt-shared PROPERTIES LIBRARY_OUTPUT_DIRECTORY lib)
set_target_properties(osclt-shared PROPERTIES OUTPUT_NAME osclt)
add_library(osclt-static STATIC ${LIBOC_SRC})
set_target_properties(osclt-static PROPERTIES ARCHIVE_OUTPUT_DIRECTORY lib)
set_target_properties(osclt-static PROPERTIES OUTPUT_NAME osclt)


file(GLOB OSCLT_SRC
    src/*.h
    src/*.c
    src/collect/*.c
    src/output/*.c
    src/process/*.c
)
add_executable(osclt-bin ${OSCLT_SRC})
set_target_properties(osclt-bin PROPERTIES RUNTIME_OUTPUT_DIRECTORY src)
set_target_properties(osclt-bin PROPERTIES OUTPUT_NAME osclt)

add_subdirectory(bin)
add_subdirectory(test)
