cmake_minimum_required(VERSION 3.5)
project(shared_memory)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILE lib/common/common.c lib/common/common.h)
set(SOURCE_FILE main.cpp init.c sender.c receiver.c test.h)

add_executable(shared_memory ${SOURCE_FILES} main.cpp init1.c test.h)

