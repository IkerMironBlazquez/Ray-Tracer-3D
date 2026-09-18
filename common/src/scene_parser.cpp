#include "scene_parser.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <print>
#include <ranges>
#include <sstream>
#include <stdexcept>

namespace render {

  scene scene_parser::parse_file(std::string const & filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      throw std::runtime_error("Cannot open scene file: " + filename);
    }

    scene scn;
    std::string line;
    int line_number = 0;

    while (std::getline(file, line)) {
      ++line_number;
      try {
        p_line(line, line_number, scn);
      } catch (std::exception const & exception) {
        std::println(std::cerr, "{}", exception.what());
        std::exit(1);
      }
    }

    return scn;
  }

  // Advertencia por función extensa, parseo, por lo que la refactorización no es necesaria
  void scene_parser::p_line(std::string const & line, int, scene & scn) {
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
        std::println(std::cerr, "Error: Unknown scene entity: {}", word);
        std::exit(1);
      }
      return;
    }  // Extraer la etiqueta y el valor
    std::string key   = trim(line.substr(0, colon_pos + 1));
    std::string value = trim(line.substr(colon_pos + 1));
    if (key.empty() or key == ":") {
      std::println(std::cerr, "Error: Unknown scene entity: :");
      std::exit(1);
    }
    if (key == "matte:") {  // Procesamiento según la etiqueta
      parse_matte(value, line, scn);
    } else if (key == "metal:") {
      parse_metal(value, line, scn);
    } else if (key == "refractive:") {
      parse_refractive(value, line, scn);
    } else if (key == "sphere:") {
      p_sphere(value, line, scn);
    } else if (key == "cylinder:") {
      p_cylinder(value, line, scn);
    } else {
      std::string entity = key.substr(0, key.length() - 1);
      std::println(std::cerr, "Error: Unknown scene entity: {}", entity);
      std::exit(1);
    }
  }

  std::string scene_parser::trim(std::string const & str) {
    auto const not_space = [](unsigned char chr) { return !std::isspace(chr); };

    auto start = std::ranges::find_if(str, not_space);
    if (start == str.end()) {
      return {};
    }
    auto end = std::ranges::find_if(std::views::reverse(str), not_space).base();

    return {start, end};
  }

  bool scene_parser::is_empty_line(std::string const & line) {
    return std::ranges::all_of(line, [](unsigned char chr) { return std::isspace(chr); });
  }

  void scene_parser::parse_matte(std::string const & value, std::string const & line, scene & scn) {
    std::istringstream iss(value);
    std::string name;
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;
    std::string extra;
    if (!(iss >> name)) {
      std::println(std::cerr, "Error: Invalid matte material parameters");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
    if (!(iss >> r >> g >> b)) {
      std::println(std::cerr, "Error: Invalid matte material parameters");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
    if (iss >> extra) {  // Verificar si hay datos extra
      std::println(std::cerr, "Error: Extra data after configuration value for key: [matte:]");
      std::println(std::cerr, "Extra: \"{}\"", extra);
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
    material mat(name, matte_material(r, g, b));
    try {
      scn.add_material(mat);
    } catch (std::invalid_argument const & exception) {
      std::println(std::cerr, "Error: Material with name [{}] already exists", name);
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
  }

  void scene_parser::parse_metal(std::string const & value, std::string const & line, scene & scn) {
    std::istringstream iss(value);
    std::string name;
    double r         = 0.0;
    double g         = 0.0;
    double b         = 0.0;
    double diffusion = 0.0;
    std::string extra;
    if (!(iss >> name)) {
      std::println(std::cerr, "Error: Invalid metal material parameters");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
    if (!(iss >> r >> g >> b >> diffusion)) {
      std::println(std::cerr, "Error: Invalid metal material parameters");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
    if (iss >> extra) {  // Verificar si hay datos extra
      std::println(std::cerr, "Error: Extra data after configuration value for key: [metal:]");
      std::println(std::cerr, "Extra: \"{}\"", extra);
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
    material mat(name, metal_material(r, g, b, diffusion));
    try {
      scn.add_material(mat);
    } catch (std::invalid_argument const & exception) {
      std::println(std::cerr, "Error: Material with name [{}] already exists", name);
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
  }

  void scene_parser::parse_refractive(std::string const & value, std::string const & line,
                                      scene & scn) {
    std::istringstream iss(value);
    std::string name;
    double refraction_index = 0.0;
    std::string extra;
    if (!(iss >> name)) {
      std::println(std::cerr, "Error: Invalid refractive material parameters");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
    if (!(iss >> refraction_index)) {
      std::println(std::cerr, "Error: Invalid refractive material parameters");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
    if (iss >> extra) {  // Verificar si hay datos extra
      std::println(std::cerr, "Error: Extra data after configuration value for key: [refractive:]");
      std::println(std::cerr, "Extra: \"{}\"", extra);
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
    material mat(name, refractive_material(refraction_index));
    try {
      scn.add_material(mat);
    } catch (std::invalid_argument const & exception) {
      std::println(std::cerr, "Error: Material with name [{}] already exists", name);
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
  }

  // Advertencia por función extensa, parseo, por lo que la refactorización no es necesaria
  void scene_parser::p_sphere(std::string const & value, std::string const & line, scene & scn) {
    std::istringstream iss(value);
    double cx     = 0.0;
    double cy     = 0.0;
    double cz     = 0.0;
    double radius = 0.0;
    std::string material_name;
    std::string extra;
    if (!(iss >> cx >> cy >> cz >> radius)) {
      std::println(std::cerr, "Error: Invalid sphere parameters");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
    if (!(iss >> material_name)) {
      std::println(std::cerr, "Error: Invalid sphere parameters");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
    if (iss >> extra) {  // Verificar si hay datos extra
      std::println(std::cerr, "Error: Extra data after configuration value for key: [sphere:]");
      std::println(std::cerr, "Extra: \"{}\"", extra);
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
    if (radius <= 0.0) {
      std::println(std::cerr, "Error: Invalid sphere parameters");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
    if (!scn.has_material(material_name)) {
      std::println(std::cerr, "Error: Material not found: [{}]", material_name);
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
    sphere const sph(point3d(cx, cy, cz), radius);
    object const obj(sph, material_name);
    scn.add_object(obj);
  }

  // Advertencia por función extensa, parseo, por lo que la refactorización no es necesaria
  void scene_parser::p_cylinder(std::string const & value, std::string const & line, scene & scn) {
    std::istringstream iss(value);
    double cx     = 0.0;
    double cy     = 0.0;
    double cz     = 0.0;
    double radius = 0.0;
    double ax     = 0.0;
    double ay     = 0.0;
    double az     = 0.0;
    std::string material_name;
    std::string extra;
    if (!(iss >> cx >> cy >> cz >> radius >> ax >> ay >> az)) {
      std::println(std::cerr, "Error: Invalid cylinder parameters");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
    if (!(iss >> material_name)) {
      std::println(std::cerr, "Error: Invalid cylinder parameters");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
    if (iss >> extra) {  // Verificar si hay datos extra
      std::println(std::cerr, "Error: Extra data after configuration value for key: [cylinder:]");
      std::println(std::cerr, "Extra: \"{}\"", extra);
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
    if (radius <= 0.0) {
      std::println(std::cerr, "Error: Invalid cylinder parameters");
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
    if (!scn.has_material(material_name)) {
      std::println(std::cerr, "Error: Material not found: [{}]", material_name);
      std::println(std::cerr, "Line: \"{}\"", line);
      std::exit(1);
    }
    cylinder const cyl(point3d(cx, cy, cz), radius, vector(ax, ay, az));
    object const obj(cyl, material_name);
    scn.add_object(obj);
  }

}  // namespace render
