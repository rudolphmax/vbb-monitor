#include <cstddef>
#include <iostream>
#include <fmt/format.h>

#include "network.hpp"

int main() {
  fmt::print("Hello World!\n");

  const std::string host = "example.com";
  const std::string port = "80";
  const std::string target = "/";
  const std::string version = "1.1";

  http::response<http::dynamic_body> response;

  const char* error = network::get("example.com", "80", "/", &response);

  if (error != NULL) {
    std::cout << error << std::endl;
  } else {
    std::cout << response << std::endl;
  }

  return EXIT_SUCCESS;
}
