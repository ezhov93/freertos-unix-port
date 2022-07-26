cmake_minimum_required(VERSION 3.3)

project(
    FreeRtosUnixPort
    VERSION 0.1
    DESCRIPTION "FreeRTOS unix port with CMake"
    LANGUAGES C CXX)

add_subdirectory(libs/FreeRTOS)
add_subdirectory(app)