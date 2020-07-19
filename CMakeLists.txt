cmake_minimum_required(VERSION 3.10)

project(Helpers LANGUAGES CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_VERBOSE_MAKEFILE ON)

include_directories(./cpp)
include_directories(./x86)
include_directories(./cxxopts/include)
add_executable(program src/program.cc)
target_link_libraries(program pfm)

add_executable(read_counters src/pfmlib_test.cc)
target_link_libraries(read_counters pfm)
