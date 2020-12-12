cmake_minimum_required(VERSION 3.17)
project(GRASPVNS LANGUAGES C)

#set(CMAKE_C_FLAGS "-std=c99")
#set(CMAKE_C_STANDARD 98)
#set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(SOURCE_FILES constructive.c constructive.h grasp.c grasp.h read_instance.c)
add_executable(GRASPVNS ${SOURCE_FILES})