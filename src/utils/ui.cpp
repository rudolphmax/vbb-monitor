#include <cstddef>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/color.hpp>
#include <iomanip>
#include <fmt/format.h>

#include "time.hpp"
#include "ui.hpp"

Screen* screen = NULL;

std::string get_delay(int drt) {
  if (drt == 0) return "";

  return fmt::format("{:+}", drt);
}

Element cell_element(Elements content, Color bg_color, Color fg_color, bool is_wide, bool is_tall) {
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

Element text_element(std::string content, Color bg_color, Color fg_color, bool is_bold, bool is_wide, bool is_tall) {
  return cell_element({ is_bold ? text(content) | bold : text(content)}, bg_color, fg_color, is_wide, is_tall);
}

Element time_element(std::string time, std::string delay, bool is_realtime, bool is_soon) {
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
  (*lines).push_back({
    separatorLight() | dim,
    separatorCharacter("─") | dim,
    separatorLight() | dim | flex,
    separatorCharacter("─") | dim,
    separatorLight() | dim
  });
}

void add_line(std::vector<Elements>* lines, Element name, Element direction, Element time, bool is_cancelled) {
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

void draw_departure_screen(std::vector<Elements> lines, tm current_time) {
  (*screen).Clear();

  std::ostringstream oss;
  oss.imbue(std::locale("de_DE.utf-8"));
  oss << std::put_time(&current_time, "%H:%M:%S");

  Render((*screen), get_document(lines, oss.str()));
  (*screen).Print();
}

void draw_error_screen(std::string message) {
  (*screen).Clear();

  Render(
    (*screen),
    vcenter(
      hcenter(
        color(
          Color::Red,
          borderRounded(
            hbox(
              separatorEmpty(),
              underlined(text("Error")),
              text(": "),
              text(message),
              separatorEmpty()
            )
          )
        )
      )
    )
  );

  (*screen).Print();
}

Screen* init_ui() {
  static Screen s = Screen::Create(Dimension::Full(), Dimension::Full());

  screen = &s;

  return screen;
}
