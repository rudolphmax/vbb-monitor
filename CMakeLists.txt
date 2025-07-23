cmake_minimum_required(VERSION 3.31)
project(vbb_monitor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(fmt REQUIRED)

add_executable(vbb_monitor
  src/main.cpp
  src/network.cpp
  src/network.hpp
)

target_link_libraries(${PROJECT_NAME} fmt::fmt)
