cmake_minimum_required(VERSION 3.31)
project(vbb_monitor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)
find_package(fmt REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(curl REQUIRED)
find_package(ftxui REQUIRED)
find_package(argparse REQUIRED)

add_executable(vbb_monitor
  src/main.cpp
  src/network.cpp
  src/network.hpp
  src/api.cpp
  src/api.hpp
  src/ui.cpp
  src/ui.hpp
  src/utils.cpp
  src/utils.hpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENSSL_LIBRARIES} -lm)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl)
target_link_libraries(${PROJECT_NAME} PRIVATE ftxui::ftxui)
target_link_libraries(${PROJECT_NAME} PRIVATE argparse::argparse)
