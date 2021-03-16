# CMakeList.txt : CMake project for coredesat, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("coredesat")

set(CMAKE_CXX_STANDARD 20)
add_definitions(-DUNICODE -D_UNICODE)

# Add source to this project's executable.
add_executable (coredesat "coredesat.cpp")