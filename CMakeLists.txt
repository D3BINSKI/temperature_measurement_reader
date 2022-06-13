cmake_minimum_required(VERSION 3.21)
project(04_binary_reader C)

set(CMAKE_C_STANDARD 99)

add_executable(04_binary_reader main.c bitops.c bitops.h com.c com.h)
