cmake_minimum_required(VERSION 3.17)
project(ANALYSEPC)

set(CMAKE_CXX_STANDARD 20)

set(SOURCE_FILES main.cpp fns.cpp fns.h consts.cpp consts.h)
add_executable(ANALYSEPC ${SOURCE_FILES})