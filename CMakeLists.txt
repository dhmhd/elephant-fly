cmake_minimum_required(VERSION 3.0)
project(elephant_fly)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(elephant_fly ${SOURCE_FILES})
