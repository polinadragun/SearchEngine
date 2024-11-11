cmake_minimum_required(VERSION 3.12)

project(
    SearchEngine
    VERSION 1.0
    LANGUAGES CXX
)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER "C:/Program Files/LLVM/bin/clang++.exe")

add_subdirectory(SearchingProgram)
add_subdirectory(IndexProgram)

enable_testing()
add_subdirectory(tests)
