cmake_minimum_required(VERSION 3.17)
project(GRASPVNS)

set(SOURCE_FILES constructive.c constructive.h grasp.c grasp.h read_instance.c)
add_executable(GRASPVNS ${SOURCE_FILES})