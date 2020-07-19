cmake_minimum_required(VERSION 3.10)

project(Helpers LANGUAGES CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -O0 -std=c++17")
set(CMAKE_VERBOSE_MAKEFILE ON)

include_directories(./cpp)
include_directories(./x86)

add_executable(program program.cc)
target_link_libraries(program pfm)

add_executable(test_pfm pfmlib_test.cc)
target_link_libraries(test_pfm pfm)