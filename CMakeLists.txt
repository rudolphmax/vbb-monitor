cmake_minimum_required(VERSION 3.31)
project(vbb_monitor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED)
find_package(fmt REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)
find_package(ftxui REQUIRED)
find_package(argparse REQUIRED)

add_executable(vbb_monitor
  src/main.cpp
  src/app.cpp
  src/app.hpp
  src/utils/ui.cpp
  src/utils/ui.hpp
  src/utils/network.cpp
  src/utils/network.hpp
  src/utils/api.cpp
  src/utils/api.hpp
  src/utils/time.cpp
  src/utils/time.hpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENSSL_LIBRARIES} -lm)
target_link_libraries(${PROJECT_NAME} PRIVATE boost::boost)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl)
target_link_libraries(${PROJECT_NAME} PRIVATE ftxui::ftxui)
target_link_libraries(${PROJECT_NAME} PRIVATE argparse::argparse)
