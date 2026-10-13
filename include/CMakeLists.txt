cmake_minimum_required(VERSION 3.20)
project(LockFreeMemoryPool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(lockfree_pool INTERFACE)
target_include_directories(lockfree_pool INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(main_demo src/main.cpp)
target_link_libraries(main_demo PRIVATE lockfree_pool)

if (MSVC)
    target_compile_options(main_demo PRIVATE /W4 /O2)
else()
    target_compile_options(main_demo PRIVATE -Wall -Wextra -Wpedantic -O3 -pthread)
endif()
