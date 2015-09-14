cmake_minimum_required(VERSION 3.0)
project(cfr)

set(gflags_SHARED    FALSE)
set(gflags_NOTHREADS FALSE)
find_package(gflags REQUIRED)

add_library(core cfr.cpp play.cpp )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -g -O3")

add_executable(play play_main.cpp)
target_link_libraries(play gflags core)

add_executable(kuhn kuhn.cpp)
target_link_libraries(kuhn gflags core)

add_executable(dice dice.cpp)
target_link_libraries(dice gflags core)

add_custom_target(format clang-format *.cpp *.h -i)
