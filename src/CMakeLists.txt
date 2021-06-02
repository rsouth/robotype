cmake_minimum_required(VERSION 3.16)
project(robotyper)

set(CMAKE_CXX_STANDARD 17)

add_executable(robotyper WIN32 robotyper.cpp robotyper.rc resource.h)