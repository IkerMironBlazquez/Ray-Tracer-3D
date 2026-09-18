#include "config_parser.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <print>
#include <ranges>
#include <sstream>
#include <stdexcept>

namespace render {

  config config_parser::parse_file(std::string const & filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      throw std::runtime_error("Cannot open configuration file: " + filename);
    }

    config cfg;
    std::string line;
    int line_number = 0;

    while (std::getline(file, line)) {
      ++line_number;
      try {
        p_line(line, line_number, cfg);
      } catch (std::exception const & exception) {
        std::println(std::cerr, "{}", exception.what());
        std::exit(1);
      }
    }

    return cfg;
  }

  // Advertencia por función extensa, parseo, por lo que la refactorización no es necesaria
  void config_parser::p_line(std::string const & line, int, config & cfg) {
    if (is_empty_line(line)) {
      return;
    }
    std::string trim_line = trim(line);
    if (trim_line.empty() or trim_line[0] == '#') {
      return;
    }
    size_t colon_pos = line.find(':');
    if (colon_pos == std::string::npos) {
      std::istringstream iss(line);
      std::string word;
      iss >> word;
      if (!word.empty()) {
        std::println(std::cerr, "Error: Unknown configuration key: [{}:]", word);
        std::exit(1);
      }
      return;
    }  // Extraer la etiqueta y el valor
    std::string key   = trim(line.substr(0, colon_pos + 1));
    std::string value = trim(line.substr(colon_pos + 1));

    if (key.empty() or key == ":") {
      std::println(std::cerr, "Error: Unknown configuration key: [:]");
      std::exit(1);
    }
    pro_config_key(key, value, line, cfg);
  }

  void config_parser::pro_config_key(std::string const & key, std::string const & value,
                                     std::string const & line, config & cfg) {
    if (key == "aspect_ratio:") {
      parse_aspect_ratio(value, line, cfg);
    } else if (key == "image_width:") {
      parse_image_width(value, line, cfg);
    } else if (key == "gamma:") {
      parse_gamma(value, line, cfg);
    } else if (key == "camera_position:") {
      parse_camera_position(value, line, cfg);
    } else if (key == "camera_target:") {
      parse_camera_target(value, line, cfg);
    } else if (key == "camera_north:") {
      parse_camera_north(value, line, cfg);
    } else if (key == "field_of_view:") {
      parse_field_of_view(value, line, cfg);
    } else if (key == "samples_per_pixel:") {
      parse_samples_per_pixel(value, line, cfg);
    } else if (key == "max_depth:") {
      parse_max_depth(value, line, cfg);
    } else if (key == "material_rng_seed:") {
      parse_material_rng_s(value, line, cfg);
    } else if (key == "ray_rng_seed:") {
      parse_ray_rng_s(value, line, cfg);
    } else if (key == "background_dark_color:") {
      parse_background_dark_color(value, line, cfg);
    } else if (key == "background_light_color:") {
      parse_background_light_color(value, line, cfg);
    } else {
      std::println(std::cerr, "Error: Unknown configuration key: [{}]", key);
      std::exit(1);
    }
  }

  std::string config_parser::trim(std::string const & str) {
    auto const not_space = [](unsigned char chr) { return !std::isspace(chr); };

    auto start = std::ranges::find_if(str, not_space);
    if (start == str.end()) {
      return {};
    }

    auto end = std::ranges::find_if(std::views::reverse(str), not_space).base();

    return {start, end};
  }

  bool config_parser::is_empty_line(std::string const & line) {
    return std::ranges::all_of(line, [](unsigned char chr) { return std::isspace(chr); });
  }

  void config_parser::parse_aspect_ratio(std::string const & value, std::string const & line,
                                         config & cfg) {
    std::istringstream iss(value);
    int width  = 0;
    int height = 0;
    std::string extra;

    if (!(iss >> width >> height)) {
      std::println(std::cerr, "Error: Invalid value for key: [aspect_ratio:]");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    // Verificar si hay datos extra
    if (iss >> extra) {
      std::println(std::cerr,
                   "Error: Extra data after configuration value for key: [aspect_ratio:]");
      std::println(std::cerr, "Extra: \"{}\"", extra);
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    try {
      cfg.set_aspect_ratio(width, height);
    } catch (std::invalid_argument const & exception) {
      std::println(std::cerr, "Error: Invalid value for key: [aspect_ratio:]");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
  }

  void config_parser::parse_image_width(std::string const & value, std::string const & line,
                                        config & cfg) {
    std::istringstream iss(value);
    int width = 0;
    std::string extra;

    if (!(iss >> width)) {
      std::println(std::cerr, "Error: Invalid value for key: [image_width:]");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    // Verificar si hay datos extra
    if (iss >> extra) {
      std::println(std::cerr,
                   "Error: Extra data after configuration value for key: [image_width:]");
      std::println(std::cerr, "Extra: \"{}\"", extra);
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    try {
      cfg.set_image_width(width);
    } catch (std::invalid_argument const & exception) {
      std::println(std::cerr, "Error: Invalid value for key: [image_width:]");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
  }

  void config_parser::parse_gamma(std::string const & value, std::string const & line,
                                  config & cfg) {
    std::istringstream iss(value);
    double gamma = 0.0;
    std::string extra;

    if (!(iss >> gamma)) {
      std::println(std::cerr, "Error: Invalid value for key: [gamma:]");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    // Verificar si hay datos extra
    if (iss >> extra) {
      std::println(std::cerr, "Error: Extra data after configuration value for key: [gamma:]");
      std::println(std::cerr, "Extra: \"{}\"", extra);
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    cfg.set_gamma(gamma);
  }

  void config_parser::parse_camera_position(std::string const & value, std::string const & line,
                                            config & cfg) {
    std::istringstream iss(value);
    double x_val = 0.0;
    double y_val = 0.0;
    double z_val = 0.0;
    std::string extra;

    if (!(iss >> x_val >> y_val >> z_val)) {
      std::println(std::cerr, "Error: Invalid value for key: [camera_position:]");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    // Verificar si hay datos extra
    if (iss >> extra) {
      std::println(std::cerr,
                   "Error: Extra data after configuration value for key: [camera_position:]");
      std::println(std::cerr, "Extra: \"{}\"", extra);
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    cfg.set_camera_position(point3d(x_val, y_val, z_val));
  }

  void config_parser::parse_camera_target(std::string const & value, std::string const & line,
                                          config & cfg) {
    std::istringstream iss(value);
    double x_val = 0.0;
    double y_val = 0.0;
    double z_val = 0.0;
    std::string extra;

    if (!(iss >> x_val >> y_val >> z_val)) {
      std::println(std::cerr, "Error: Invalid value for key: [camera_target:]");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    // Verificar si hay datos extra
    if (iss >> extra) {
      std::println(std::cerr,
                   "Error: Extra data after configuration value for key: [camera_target:]");
      std::println(std::cerr, "Extra: \"{}\"", extra);
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    cfg.set_camera_target(point3d(x_val, y_val, z_val));
  }

  void config_parser::parse_camera_north(std::string const & value, std::string const & line,
                                         config & cfg) {
    std::istringstream iss(value);
    double x_val = 0.0;
    double y_val = 0.0;
    double z_val = 0.0;
    std::string extra;

    if (!(iss >> x_val >> y_val >> z_val)) {
      std::println(std::cerr, "Error: Invalid value for key: [camera_north:]");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    // Verificar si hay datos extra
    if (iss >> extra) {
      std::println(std::cerr,
                   "Error: Extra data after configuration value for key: [camera_north:]");
      std::println(std::cerr, "Extra: \"{}\"", extra);
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    cfg.set_camera_north(vector(x_val, y_val, z_val));
  }

  void config_parser::parse_field_of_view(std::string const & value, std::string const & line,
                                          config & cfg) {
    std::istringstream iss(value);
    double fov = 0.0;
    std::string extra;

    if (!(iss >> fov)) {
      std::println(std::cerr, "Error: Invalid value for key: [field_of_view:]");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    // Verificar si hay datos extra
    if (iss >> extra) {
      std::println(std::cerr,
                   "Error: Extra data after configuration value for key: [field_of_view:]");
      std::println(std::cerr, "Extra: \"{}\"", extra);
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    try {
      cfg.set_field_of_view(fov);
    } catch (std::invalid_argument const & exception) {
      std::println(std::cerr, "Error: Invalid value for key: [field_of_view:]");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
  }

  void config_parser::parse_samples_per_pixel(std::string const & value, std::string const & line,
                                              config & cfg) {
    std::istringstream iss(value);
    int samples = 0;
    std::string extra;

    if (!(iss >> samples)) {
      std::println(std::cerr, "Error: Invalid value for key: [samples_per_pixel:]");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    // Verificar si hay datos extra
    if (iss >> extra) {
      std::println(std::cerr,
                   "Error: Extra data after configuration value for key: [samples_per_pixel:]");
      std::println(std::cerr, "Extra: \"{}\"", extra);
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    try {
      cfg.set_samples_per_pixel(samples);
    } catch (std::invalid_argument const & exception) {
      std::println(std::cerr, "Error: Invalid value for key: [samples_per_pixel:]");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
  }

  void config_parser::parse_max_depth(std::string const & value, std::string const & line,
                                      config & cfg) {
    std::istringstream iss(value);
    int depth = 0;
    std::string extra;

    if (!(iss >> depth)) {
      std::println(std::cerr, "Error: Invalid value for key: [max_depth:]");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    // Verificar si hay datos extra
    if (iss >> extra) {
      std::println(std::cerr, "Error: Extra data after configuration value for key: [max_depth:]");
      std::println(std::cerr, "Extra: \"{}\"", extra);
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    try {
      cfg.set_max_depth(depth);
    } catch (std::invalid_argument const & exception) {
      std::println(std::cerr, "Error: Invalid value for key: [max_depth:]");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
  }

  void config_parser::parse_material_rng_s(std::string const & value, std::string const & line,
                                           config & cfg) {
    std::istringstream iss(value);
    int seed = 0;
    std::string extra;

    if (!(iss >> seed)) {
      std::println(std::cerr, "Error: Invalid value for key: [material_rng_seed:]");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    // Verificar si hay datos extra
    if (iss >> extra) {
      std::println(std::cerr,
                   "Error: Extra data after configuration value for key: [material_rng_seed:]");
      std::println(std::cerr, "Extra: \"{}\"", extra);
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    try {
      cfg.set_material_rng_seed(seed);
    } catch (std::invalid_argument const & exception) {
      std::println(std::cerr, "Error: Invalid value for key: [material_rng_seed:]");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
  }

  void config_parser::parse_ray_rng_s(std::string const & value, std::string const & line,
                                      config & cfg) {
    std::istringstream iss(value);
    int seed = 0;
    std::string extra;

    if (!(iss >> seed)) {
      std::println(std::cerr, "Error: Invalid value for key: [ray_rng_seed:]");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    // Verificar si hay datos extra
    if (iss >> extra) {
      std::println(std::cerr,
                   "Error: Extra data after configuration value for key: [ray_rng_seed:]");
      std::println(std::cerr, "Extra: \"{}\"", extra);
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    try {
      cfg.set_ray_rng_seed(seed);
    } catch (std::invalid_argument const & exception) {
      std::println(std::cerr, "Error: Invalid value for key: [ray_rng_seed:]");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
  }

  void config_parser::parse_background_dark_color(std::string const & value,
                                                  std::string const & line, config & cfg) {
    std::istringstream iss(value);
    double r_val = 0.0;
    double g_val = 0.0;
    double b_val = 0.0;
    std::string extra;

    if (!(iss >> r_val >> g_val >> b_val)) {
      std::println(std::cerr, "Error: Invalid value for key: [background_dark_color:]");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    // Verificar si hay datos extra
    if (iss >> extra) {
      std::println(std::cerr,
                   "Error: Extra data after configuration value for key: [background_dark_color:]");
      std::println(std::cerr, "Extra: \"{}\"", extra);
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    try {
      cfg.set_background_dark_color(color(r_val, g_val, b_val));
    } catch (std::invalid_argument const & exception) {
      std::println(std::cerr, "Error: Invalid value for key: [background_dark_color:]");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
  }

  void config_parser::parse_background_light_color(std::string const & value,
                                                   std::string const & line, config & cfg) {
    std::istringstream iss(value);
    double r_val = 0.0;
    double g_val = 0.0;
    double b_val = 0.0;
    std::string extra;

    if (!(iss >> r_val >> g_val >> b_val)) {
      std::println(std::cerr, "Error: Invalid value for key: [background_light_color:]");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    // Verificar si hay datos extra
    if (iss >> extra) {
      std::println(
          std::cerr,
          "Error: Extra data after configuration value for key: [background_light_color:]");
      std::println(std::cerr, "Extra: \"{}\"", extra);
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }

    try {
      cfg.set_background_light_color(color(r_val, g_val, b_val));
    } catch (std::invalid_argument const & exception) {
      std::println(std::cerr, "Error: Invalid value for key: [background_light_color:]");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
  }

}  // namespace render
