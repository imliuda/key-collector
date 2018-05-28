cmake_minimum_required(VERSION 2.4)

project(osclt)

option(BUILD_STATIC "build osclt static link" ON)
option(BUILD_TEST "build test programs" OFF)

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

if (${BUILD_STATIC} EQUAL ON)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static")
    target_link_libraries(osclt-bin ${LIBEV} osclt-static)
else()
    target_link_libraries(osclt-bin ${LIBEV} osclt-shared)
endif()

add_subdirectory(bin)

if (${BUILD_TEST} EQUAL ON)
    enable_testing()
    add_subdirectory(test)
endif()

