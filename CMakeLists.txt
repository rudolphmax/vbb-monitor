cmake_minimum_required(VERSION 3.31)
project(vbb_monitor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)
find_package(fmt REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(curl REQUIRED)

add_executable(vbb_monitor
  src/main.cpp
  src/network.cpp
  src/network.hpp
  src/api.cpp
  src/api.hpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENSSL_LIBRARIES} -lm)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl)
