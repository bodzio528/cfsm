# Licensed under the MIT License. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.2)

project(cfsm)

set(C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include_directories(include)
add_subdirectory(src)

# git submodule update
add_subdirectory(googletest)

enable_testing()
add_subdirectory(test)
