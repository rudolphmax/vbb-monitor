#ifndef API_H
#define API_H

#include <string>
#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#define MAX_API_PARAMS 50

namespace api {
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
    const api_param api_params[MAX_API_PARAMS];
  };

  const api_response get(const api::api_request request);
}

#endif
