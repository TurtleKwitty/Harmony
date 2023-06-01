cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS On)

project(Harmony)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED On)

file(GLOB_RECURSE ProjectSourceFiles src/*.cpp)

add_executable(Harmony ${ProjectSourceFiles})
