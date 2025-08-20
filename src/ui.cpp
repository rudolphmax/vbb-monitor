#include <ftxui/dom/node.hpp>
#include <thread>
#include <fmt/format.h>

#include "utils.hpp"
#include "ui.hpp"

std::string get_delay(int drt) {
  if (drt == 0) return "";

  return "+" + std::to_string(drt);
}

Element cell_element(Elements content, Color bg_color = Color::Black, Color fg_color = Color::White) {
  return (
    bgcolor(
      bg_color,
      color(
        fg_color,
        hbox(
          separatorEmpty(),
          content,
          separatorEmpty()
        )
      )
    )
  );
}

Element text_element(std::string content, Color bg_color = Color::Black, Color fg_color = Color::White) {
  return cell_element({ text(content) }, bg_color, fg_color);
}

Element time_element(std::string time, std::string delay = "", bool is_realtime = false, bool is_soon = false) {
  return cell_element({
    is_realtime ? text("* ") : text("  "),
    (is_soon ? bold(text(time)) : text(time)) | flex,
    text(" "),
    delay.empty() ? text("    ") : color(Color::Red, text(fmt::format("{:4}", delay.substr(0, 4)))),
  });
}

void add_line(Elements columns[3], Element name, Element direction, Element time, bool is_cancelled = false) {
  columns[0].push_back(name);
  columns[1].push_back(is_cancelled ? (direction | strikethrough | dim) : direction);
  columns[2].push_back(is_cancelled ? color(Color::Red, text(" cancelled ")) : time);
}

Element get_document(Elements columns[3]) {
  return (
    hbox({
      vbox(columns[0]),
      text(" "),
      vbox(columns[1]) | flex,
      text(" "),
      vbox(columns[2])
    })
  );
}

void refresh_screen(Screen& screen, Elements columns[3]) {
  screen.Clear();

  Render(screen, get_document(columns));
  screen.Print();
}

void departure_list(Screen screen, const api::api_request request, int REFRESH_INTERVAL) {
  while (true) {
    api::api_response res = api::get(request);

    if (res.error) {
      fmt::print("{}", res.error);
      continue;
    }

    json departures = res.data["Departure"];

    bool has_horizontal_separator = false;
    Elements columns[3];

    for (json& departure : departures) {
      json departure_bg_color = departure["ProductAtStop"]["icon"]["backgroundColor"];
      json departure_fg_color = departure["ProductAtStop"]["icon"]["foregroundColor"];

      auto name = text_element(
        departure["name"],
        Color::RGB(departure_bg_color["r"], departure_bg_color["g"], departure_bg_color["b"]),
        Color::RGB(departure_fg_color["r"], departure_fg_color["g"], departure_fg_color["b"])
      );

      auto direction = text_element((std::string) departure["direction"]);

      bool is_realtime = false;

      std::tm tm = {};
      std::tm rt_tm = {};

      std::time_t now_tt = time(nullptr);
      struct tm now_c = *localtime(&now_tt);

      parse_time(&tm, departure["date"], departure["time"], now_c.tm_isdst);

      if (departure.contains("rtTime") && departure.contains("rtDate")) {
        parse_time(&rt_tm, departure["rtDate"], departure["rtTime"], now_c.tm_isdst);
        is_realtime = true;
      }

      int dmin;
      int drt = 0;

      if (is_realtime) {
        dmin = std::floor(std::difftime(std::mktime(&rt_tm), now_tt) / 60);
        drt = std::floor(std::difftime(std::mktime(&rt_tm), std::mktime(&tm)) / 60);
      } else {
        dmin = std::floor(std::difftime(std::mktime(&tm), now_tt) / 60);
      }

      Element departure_time_element;

      if (dmin <= 0) {
        departure_time_element = time_element(
          "now\n",
          "",
          is_realtime,
          true
        );

      } else if (dmin <= 10) {
        departure_time_element = time_element(
          fmt::format("{}\n", dmin),
          get_delay(drt),
          is_realtime,
          true
        );

      } else {
        std::ostringstream oss;
        oss.imbue(std::locale("de_DE.utf-8"));

        if (is_realtime) {
          oss << std::put_time(&rt_tm, "%H:%M");
        } else {
          oss << std::put_time(&tm, "%H:%M");
        }

        departure_time_element = time_element(
          oss.str(),
          get_delay(drt),
          is_realtime
        );
      }

      add_line(columns, name, direction, departure_time_element, departure.contains("cancelled") && departure["cancelled"]);
    }

    refresh_screen(screen, columns);

    std::this_thread::sleep_for(std::chrono::milliseconds(REFRESH_INTERVAL));
  }
}

Screen init_ui() {
  Screen screen = Screen::Create(Dimension::Full(), Dimension::Full());

  return screen;
}

void start_ui(Screen screen, const api::api_request request, int REFRESH_INTERVAL) {
  std::thread t(departure_list, screen, request, REFRESH_INTERVAL);
  t.join();
}
