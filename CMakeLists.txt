cmake_minimum_required(VERSION 3.2)

project(cfsm LANGUAGES C)

set(C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

include_directories(include)
add_subdirectory(src)

enable_testing()
add_subdirectory(test)
