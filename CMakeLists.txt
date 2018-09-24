cmake_minimum_required(VERSION 3.5)

project(infinitearithmetic)

set(CMAKE_CXX_STANDARD 11)

add_executable(
  infinitearithmetic
  src/infinitearithmetic.cpp
)

target_include_directories(infinitearithmetic PUBLIC include)
target_include_directories(infinitearithmetic PRIVATE src)
