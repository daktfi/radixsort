cmake_minimum_required(VERSION 3.5)

project(radixsort LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(example example.cpp)
add_executable(test_speed test_speed.cpp)
