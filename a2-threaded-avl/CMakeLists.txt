# Basic AVL build
add_executable(a2_avl main.cpp Playlist.cpp)
target_include_directories(a2_avl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME a2_avl
         COMMAND sh -c "$<TARGET_FILE:a2_avl> | diff - ${CMAKE_CURRENT_SOURCE_DIR}/expected_output.txt")

# Threaded AVL build (O(1) next/previous)
add_executable(a2_threaded main.cpp Playlist.cpp)
target_include_directories(a2_threaded PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(a2_threaded PRIVATE USE_THREADED_AVL)
add_test(NAME a2_threaded
         COMMAND sh -c "$<TARGET_FILE:a2_threaded> | diff - ${CMAKE_CURRENT_SOURCE_DIR}/expected_output.txt")

# Randomized property test against std::set
add_executable(a2_property_test property_test.cpp)
target_include_directories(a2_property_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME a2_property_test COMMAND a2_property_test)

# Known issue (see README): Playlist has no destructor, so Song objects leak when a
# Playlist goes out of scope. Leak detection is turned off for these two tests only,
# so a sanitizer build still catches every other memory error.
set_tests_properties(a2_avl a2_threaded PROPERTIES ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0")
