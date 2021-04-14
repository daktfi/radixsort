cmake_minimum_required(VERSION 3.5)

project(radixsort LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set( CMAKE_CXX_FLAGS_RELEASE "-O3 -s -std=c++17" )
set( CMAKE_CXX_FLAGS_DEBUG "-g3 -Wall -std=c++17" )

add_executable(example example.cpp)
add_executable(test_speed test_speed.cpp)
