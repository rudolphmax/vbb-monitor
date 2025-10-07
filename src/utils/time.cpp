#include <iomanip>
#include <sstream>

#include "time.hpp"

std::tm* parse_time(std::tm* target_tm, std::string date, std::string time, int is_dst) {
  std::stringstream timestring;

  timestring << (std::string) date << " " << time;
  timestring.imbue(std::locale("de_DE.utf-8"));
  timestring >> std::get_time(target_tm, "%Y-%m-%d %H:%M:%S");

  target_tm->tm_isdst = is_dst;

  return target_tm;
}
