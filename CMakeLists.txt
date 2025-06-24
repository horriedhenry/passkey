cmake_minimum_required(VERSION 3.10)

project(passkey)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wall)

set(SOURCES
    src/main.cpp
)

add_executable(passkey ${SOURCES})
