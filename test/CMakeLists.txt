include_directories(${CMAKE_SOURCE_DIR}/include BEFORE)

add_executable(test_map test_map.c)
target_link_libraries(test_map osclt-shared)
add_test(test_map test_map)
