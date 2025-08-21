cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(pico_sdk_import.cmake)

project(pico_pcf8523 C CXX ASM)

pico_sdk_init()

add_subdirectory("src")
add_subdirectory("examples")
