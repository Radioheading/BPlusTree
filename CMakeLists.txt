cmake_minimum_required(VERSION 3.16)
project(BPT)

set(CMAKE_CXX_STANDARD 17)

add_executable(BPT exceptions.hpp vector.hpp bpt.hpp main.cpp recycle.hpp)
