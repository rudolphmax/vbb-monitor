#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <boost/beast.hpp>
#include <argparse/argparse.hpp>

#include "api.hpp"
#include "ui.hpp"

int main(int argc, char *argv[]) {
  argparse::ArgumentParser program("program_name");

  program.add_argument("-i", "--access-id")
    .help("The HAFAS Access-ID to use with the API.");

  program.add_argument("-s", "--stop")
    .help("The ID of the stop to monitor.");

  program.add_argument("-r", "--refresh-interval")
    .help("The interval in which the data is to be refetched (in ms).")
    .default_value(25000)
    .scan<'d', int>();

  try {
    program.parse_args(argc, argv);
  }
  catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }

  std::string ACCESS_ID;
  std::string STOP_ID;
  int REFRESH_INTERVAL;

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

  if (ACCESS_ID.empty()) {
    fmt::print("No Access Id provided, set VBBMON_ACCESS_ID.\n");
    return EXIT_FAILURE;
  }

  if (STOP_ID.empty()) {
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

  auto screen = init_ui();
  start_ui(screen, request, REFRESH_INTERVAL);

  return EXIT_SUCCESS;
}
