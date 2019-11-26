cmake_minimum_required(VERSION 3.14)
project(simplekvsvr)

set(CMAKE_CXX_STANDARD 11)

add_executable(simplekvsvr processpool.cpp taskpool/shm_cqueue.cpp)