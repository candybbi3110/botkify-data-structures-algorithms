add_executable(a3_demo main.cpp MusicGraph.cpp)
target_include_directories(a3_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# The demo output must match the expected output given in the assignment examples
add_test(NAME a3_demo
         COMMAND sh -c "$<TARGET_FILE:a3_demo> | diff - ${CMAKE_CURRENT_SOURCE_DIR}/expected_output.txt")

add_executable(a3_property_test property_test.cpp MusicGraph.cpp)
target_include_directories(a3_property_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME a3_property_test COMMAND a3_property_test)
