cmake_minimum_required(VERSION 3.5)
project(myshell)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILE main.cpp)
set(SOURCE_FILE  myshell.c myshell cmd1.c cmd1 cmd2.c cmd2 cmd3.c cmd3)

add_executable(myshell ${SOURCE_FILES} main.cpp cmd1.c cmd1 cmd2.c cmd2 cmd3.c cmd3 )

