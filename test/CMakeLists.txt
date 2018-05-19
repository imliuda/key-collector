include_directories(${CMAKE_SOURCE_DIR}/include BEFORE)
link_directories(${CMAKE_BUILD_DIRECTORY}/lib)

add_executable(test_map test_map.c)
target_link_libraries(test_map osclt-shared)
