#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/color.hpp>
#include <thread>
#include <fmt/format.h>

#include "utils.hpp"
#include "ui.hpp"

std::string get_delay(int drt) {
  if (drt == 0) return "";

  return fmt::format("{:+}", drt);
}


Element cell_element(Elements content, Color bg_color = Color::Black, Color fg_color = Color::White, bool is_wide = false, bool is_tall = false) {
  return (
    bgcolor(
      bg_color,
      color(
        fg_color,
        vbox(
          is_tall ? text(" ") : emptyElement(),
          hbox(
            is_wide ? text(" ") : emptyElement(),
            text(" "),
            content,
            text(" "),
            is_wide ? text(" ") : emptyElement()
          ),
          is_tall ? text(" ") : emptyElement()
        )
      )
    )
  );
}

Element text_element(std::string content, Color bg_color = Color::Black, Color fg_color = Color::White, bool is_bold = false, bool is_wide = false, bool is_tall = false) {
  return cell_element({ is_bold ? text(content) | bold : text(content)}, bg_color, fg_color, is_wide, is_tall);
}

Element time_element(std::string time, std::string delay = "", bool is_realtime = false, bool is_soon = false) {
  return cell_element(
    {
      is_realtime ? text("* ") : text("  "),
      (is_soon ? bold(text(time)) : text(time)) | flex,
      text(" "),
      delay.empty() ? text("    ") : color(Color::Red, text(fmt::format("{:4}", delay.substr(0, 4)))),
    },
    Color::Black, Color::White,
    false,
    is_soon
  );
}

void add_separator_line(std::vector<Elements>* lines) {
  (*lines).push_back({separatorLight(), separatorLight(), separatorLight()});
}

void add_line(std::vector<Elements>* lines, Element name, Element direction, Element time, bool is_cancelled = false) {
  (*lines).push_back({
    name,
    separatorEmpty(),
    vbox(
      filler(),
      is_cancelled ? (direction | strikethrough | dim) : direction,
      filler()
    ) | flex,
    separatorEmpty(),
    vbox(
      filler(),
      is_cancelled ? color(Color::Red, text("cancelled ")) : time,
      filler()
    )
  });
}

Element get_document(std::vector<Elements> lines, std::string timestring) {
  return vbox({
    bgcolor(
      Color::White,
      color(
        Color::Black,
        hbox({
          filler(),
          text(timestring),
          filler(),
        })
      )
    ),
    gridbox(lines)
  });
}

void refresh_screen(Screen& screen, std::vector<Elements> lines, tm current_time) {
  screen.Clear();

  std::ostringstream oss;
  oss.imbue(std::locale("de_DE.utf-8"));
  oss << std::put_time(&current_time, "%H:%M");

  Render(screen, get_document(lines, oss.str()));
  screen.Print();
}

void departure_list(Screen screen, const api::api_config api_config, const api::api_request request, int REFRESH_INTERVAL) {
  while (true) {
    std::time_t now_tt = time(nullptr);
    struct tm now_c = *localtime(&now_tt);

    api::api_response res = api::get(api_config, request);

    if (res.error) {
      fmt::print("{}", res.error);
      continue;
    }

    json departures = res.data["Departure"];

    bool has_horizontal_separator = false;
    std::vector<Elements> lines;

    bool is_previous_line_soon = false;

    for (json& departure : departures) {
      json departure_bg_color = departure["ProductAtStop"]["icon"]["backgroundColor"];
      json departure_fg_color = departure["ProductAtStop"]["icon"]["foregroundColor"];

      bool is_realtime = false;

      std::tm tm = {};
      std::tm rt_tm = {};

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

      auto name = text_element(
        departure["name"],
        Color::RGB(departure_bg_color["r"], departure_bg_color["g"], departure_bg_color["b"]),
        Color::RGB(departure_fg_color["r"], departure_fg_color["g"], departure_fg_color["b"]),
        true,
        true,
        dmin <= 10
      );

      auto direction = text_element((std::string) departure["direction"]);

      if (dmin <= 10) {
        is_previous_line_soon = true;
      } else if (is_previous_line_soon == true) {
        is_previous_line_soon = false;

        add_separator_line(&lines);
      }

      add_line(&lines, name, direction, departure_time_element, departure.contains("cancelled") && departure["cancelled"]);
    }

    refresh_screen(screen, lines, now_c);

    std::this_thread::sleep_for(std::chrono::milliseconds(REFRESH_INTERVAL));
  }
}

Screen init_ui() {
  Screen screen = Screen::Create(Dimension::Full(), Dimension::Full());

  return screen;
}

void start_ui(Screen screen, const api::api_config api_config, const api::api_request request, int REFRESH_INTERVAL) {
  std::thread t(departure_list, screen, api_config, request, REFRESH_INTERVAL);
  t.join();
}
