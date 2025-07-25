#include <cstddef>
#include <cstdlib>
#include <fmt/format.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <boost/beast.hpp>

#include "api.hpp"

int main() {
  const char* ACCESS_ID = std::getenv("VBBMON_ACCESS_ID");
  const char* STOP_ID = std::getenv("VBBMON_STOP_ID");

  if (!ACCESS_ID) {
    fmt::print("No Access Id provided, set VBBMON_ACCESS_ID.\n");
    return EXIT_FAILURE;
  }

  if (!STOP_ID) {
    fmt::print("No Stop Id provided, set VBBMON_STOP_ID.\n");
    return EXIT_FAILURE;
  }

  api::api_request request = {
    .path = "/departureBoard",
    .api_params = {
      { "accessId", ACCESS_ID },
      { "id", STOP_ID },
      { "maxJourneys", "10" },
      { "format", "json" }
    }
  };

  api::api_response res = api::get(request);

  if (res.error) {
    fmt::print("{}", res.error);

    return EXIT_FAILURE;
  }

  json departures = res.data["Departure"];

  for (json& departure : departures) {
    std::cout << departure["name"] << " " << departure["direction"] << " ";

    if (departure.contains("rtTime")) {
      std::cout << departure["rtTime"] << "*";
    } else {
      std::cout << departure["time"];
    }

    std::cout << std::endl;
    // fmt::print("{}| {} {}\n", departure["name"], departure["direction"], departure["time"]);
  }

  return EXIT_SUCCESS;
}
