#include <fmt/format.h>

#include "api.hpp"
#include "network.hpp"

const char* API_HOST = "vbb-demo.demo2.hafas.cloud";
const char* API_PORT = "443";
const char* API_BASE = "/api/fahrinfo/latest";

const api::api_response api::get(const api::api_request request) {
  std::string target = API_BASE;

  // Ensuring leading slash before path
  if (!(request.path.substr(0, 1) == "/")) {
    target += std::string("/");
  }

  target += request.path;

  // Adding trailing slash to path before args
  if (!(request.path.substr(request.path.length()-1, 1) == "/")) {
    target += "/";
  }

  for (int i = 0; i < MAX_API_PARAMS; i++) {
    if (request.api_params[i].key.empty()) break;

    if (i == 0) {
      target += "?";
    } else {
      target += "&";
    }

    target += request.api_params[i].key + "=" + network::url_encode(request.api_params[i].value);
  }

  boost::beast::http::response<boost::beast::http::dynamic_body> beast_res;
  const char* error = network::get(API_HOST, API_PORT, target.c_str(), &beast_res);

  if (error) return api::api_response({ error, nullptr });

  return api::api_response({ nullptr, json::parse(boost::beast::buffers_to_string(beast_res.body().data())) });
}
