cmake_minimum_required(VERSION 3.16)
project(botkify_data_structures CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

# Optional: cmake -B build -DSANITIZE=ON  → AddressSanitizer + UndefinedBehaviorSanitizer
option(SANITIZE "Build with ASan + UBSan" OFF)
if(SANITIZE)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
  add_link_options(-fsanitize=address,undefined)
endif()

enable_testing()
add_subdirectory(a1-linked-list)
add_subdirectory(a2-threaded-avl)
add_subdirectory(a3-music-graph)
