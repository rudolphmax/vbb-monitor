#ifndef API_H
#define API_H

#include <string>
#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace api {
  typedef struct api_config {
    std::string host;
    std::string port;
    std::string base;
  } api_config;

  typedef struct api_response {
    const char* error;
    const json data;
  } api_response;

  typedef struct api_param {
    const std::string key;
    const std::string value;
  } api_param;

  struct api_request {
    const std::string path; // TODO: make enum
    const json api_params;
  };

  const api_response get(const api::api_config config, const api::api_request request);
}

#endif
