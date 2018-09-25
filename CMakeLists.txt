cmake_minimum_required(VERSION 3.5)

project(mypython)

set(CMAKE_CXX_STANDARD 11)

add_executable(
  mypython
  src/mypython.cpp
)

target_include_directories(mypython PUBLIC include)
target_include_directories(mypython PRIVATE src)
