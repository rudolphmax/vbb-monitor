#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <boost/beast.hpp>
#include <argparse/argparse.hpp>
#include <string>

#include "api.hpp"
#include "ui.hpp"

int main(int argc, char *argv[]) {
  argparse::ArgumentParser program("program_name");

  program.add_argument("-h", "--host")
    .help("The hostname of the HAFAS API, e.g.: hafas.example.com");

  program.add_argument("-p", "--port")
    .help("The port of the HAFAS API, e.g.: 443 for SSL or 80 for plain http.")
    .default_value("80");

  program.add_argument("-b", "--base")
    .help("The base url of API, e.g.: /api/info/v2")
    .default_value("");

  program.add_argument("--api-params")
    .help("Additional parameters passed to the API. Provided as a JSON string.")
    .default_value("");

  program.add_argument("-i", "--access-id")
    .help("The HAFAS Access-ID to use with the API.");

  program.add_argument("-s", "--stop")
    .help("The ID of the stop to monitor.");

  program.add_argument("-r", "--refresh-interval")
    .help("The interval in which the data is to be refetched (in ms).")
    .default_value(25000)
    .scan<'d', int>();

  program.add_argument("-l", "--lines")
    .help("The amount of lines (departures) to display.")
    .default_value(10)
    .scan<'d', int>();

  try {
    program.parse_args(argc, argv);
  }
  catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }

  std::string API_HOST;
  std::string API_PORT;
  std::string API_BASE;
  std::string ACCESS_ID;
  std::string STOP_ID;
  std::string raw_api_params;
  int REFRESH_INTERVAL;
  int NUM_LINES;

  if (program.is_used("-h")) {
    API_HOST = program.get<std::string>("-h");

  } else if (std::getenv("VBBMON_API_HOST")) {
    API_HOST = std::getenv("VBBMON_API_HOST");
  }

  if (!program.is_used("-p") && std::getenv("VBBMON_API_PORT")) {
    API_PORT = std::getenv("VBBMON_API_PORT");

  } else {
    API_PORT = program.get<std::string>("-p");
  }

  if (!program.is_used("-b") && std::getenv("VBBMON_API_BASE")) {
    API_BASE = std::getenv("VBBMON_API_BASE");

  } else {
    API_BASE = program.get<std::string>("-b");
  }

  if (!program.is_used("-b") && std::getenv("VBBMON_API_BASE")) {
    raw_api_params = std::getenv("VBBMON_API_PARAMS");

  } else {
    raw_api_params = program.get<std::string>("--api-params");
  }

  if (program.is_used("-i")) {
      ACCESS_ID = program.get<std::string>("-i");

  } else if (std::getenv("VBBMON_ACCESS_ID")) {
    ACCESS_ID = std::getenv("VBBMON_ACCESS_ID");
  }

  if (program.is_used("-s")) {
    STOP_ID = program.get<std::string>("-s");

  } else if (std::getenv("VBBMON_STOP_ID")) {
    STOP_ID = std::getenv("VBBMON_STOP_ID");
  }

  if (!program.is_used("-r") && std::getenv("VBBMON_REFRESH_INTERVAL")) {
    REFRESH_INTERVAL = std::stoi(std::getenv("VBBMON_REFRESH_INTERVAL"));
  } else {
    REFRESH_INTERVAL = REFRESH_INTERVAL = program.get<int>("-r");
  }

  if (!program.is_used("-l") && std::getenv("VBBMON_NUM_LINES")) {
    NUM_LINES = std::stoi(std::getenv("VBBMON_NUM_LINES"));
  } else {
    NUM_LINES = NUM_LINES = program.get<int>("-l");
  }

  if (API_HOST.empty()) {
    fmt::print("No API hostname provided, set VBBMON_API_HOST or use the command-line argument.\n");
    return EXIT_FAILURE;
  }

  if (ACCESS_ID.empty()) {
    fmt::print("No Access Id provided, set VBBMON_ACCESS_ID or use the command-line argument.\n");
    return EXIT_FAILURE;
  }

  if (STOP_ID.empty()) {
    fmt::print("No Stop Id provided, set VBBMON_STOP_ID or use the command-line argument.\n");
    return EXIT_FAILURE;
  }

  api::api_config api_config = {
    .host = API_HOST,
    .port = API_PORT,
    .base = API_BASE
  };

  json base_api_params = {
    { "accessId", ACCESS_ID },
    { "id", STOP_ID },
    { "maxJourneys", std::to_string(NUM_LINES) },
    { "format", "json" }
  };

  json api_params = json::object();

  // TODO: add error handling
  if (!raw_api_params.empty()) {
    api_params = json::parse(raw_api_params);
  }

  api_params.merge_patch(base_api_params);

  api::api_request request = {
    .path = "/departureBoard",
    .api_params = api_params
  };

  auto screen = init_ui();
  start_ui(screen, api_config, request, REFRESH_INTERVAL, NUM_LINES);

  return EXIT_SUCCESS;
}
