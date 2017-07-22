cmake_minimum_required(VERSION 3.7)


set(CMAKE_CONFIG_DIR ${CMAKE_CURRENT_LIST_DIR}/cmake)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CONFIG_DIR}/ArduinoToolchain.cmake)
set(PROJECT_NAME DueVGA)


project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        src/crntsc.h
        src/crpal.h
        src/font8x8.cpp
        src/graphics.cpp
        src/text.cpp
        src/VGA.cpp
        src/VGA.h)

add_executable(DueVGA ${SOURCE_FILES})
