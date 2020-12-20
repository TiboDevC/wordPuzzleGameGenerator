cmake_minimum_required(VERSION 3.17)
project(puzzleWord C CXX)

add_executable(${PROJECT_NAME}_c main.c)
add_executable(${PROJECT_NAME}_cpp main.cpp)

target_compile_options(${PROJECT_NAME}_c PRIVATE -Wall -Wextra -pedantic -Werror)
target_compile_options(${PROJECT_NAME}_cpp PRIVATE -Wall -Wextra -pedantic -Werror)
target_compile_features(${PROJECT_NAME}_cpp PRIVATE cxx_std_20)
