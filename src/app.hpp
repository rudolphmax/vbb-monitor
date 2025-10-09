#include <string>
#include <ftxui/screen/color.hpp>

#include "utils/api.hpp"

using namespace ftxui;

typedef struct Departure {
  std::string name;
  Color bg_color;
  Color fg_color;
  std::string direction;
  std::string time;
  bool is_realtime;
  bool is_cancelled;
  int dmin;
  int drt;
} Departure;

typedef struct Error {
  std::string message;
} Error;

typedef struct Data {
  Departure* departures;
  Error* error;
} Data;

void app(const api::api_config api_config, const api::api_request request, int REFRESH_INTERVAL, int NUM_LINES);
