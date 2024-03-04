cmake_minimum_required(VERSION 3.25)
project(BmpAnalyzer)

set(CMAKE_CXX_STANDARD 17)

add_executable(BmpAnalyzer main.cpp bmp.h bmp.cpp)
