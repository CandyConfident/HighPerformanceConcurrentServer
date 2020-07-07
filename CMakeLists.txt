cmake_minimum_required(VERSION 3.16)
project(MyServer CXX)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_DEBUG_FLAG)
    set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -g")
endif()

message(STATUS "CMAKE_CXX_FLAGS = " ${CMAKE_CXX_FLAGS})

add_subdirectory(log/tests)
add_subdirectory(threadpool/tests)
add_subdirectory(timer/tests)
add_subdirectory(memory/tests)
add_subdirectory(net/tests)
