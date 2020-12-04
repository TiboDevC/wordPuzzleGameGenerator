cmake_minimum_required(VERSION 3.17)
project(motsMeles)

set(CMAKE_CXX_STANDARD 20)

add_executable(motsMeles main.cpp)

target_compile_options(motsMeles PRIVATE -Wall -Wextra -pedantic -Werror)
