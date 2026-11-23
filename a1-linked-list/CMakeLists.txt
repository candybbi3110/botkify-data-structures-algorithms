add_executable(a1_tests main.cpp Playlist.cpp)
target_include_directories(a1_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME a1_linked_list COMMAND a1_tests)
