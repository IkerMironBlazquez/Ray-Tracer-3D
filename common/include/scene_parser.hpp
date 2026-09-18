#ifndef RENDER_SCENE_PARSER_HPP
#define RENDER_SCENE_PARSER_HPP

#include "scene.hpp"

#include <string>

namespace render {

  /// Clase para parsear archivos de escena
  class scene_parser {
  public:
    /// @param filename Nombre del archivo de escena
    /// @return Objeto scene con los materiales y objetos leídos
    /// @throws Excepciones
    static scene parse_file(std::string const & filename);

  private:
    /// Procesa una línea del archivo de escena
    /// @param line
    /// @param line_number
    /// @param scn
    static void p_line(std::string const & line, int line_number, scene & scn);

    static std::string trim(std::string const & str);

    static bool is_empty_line(std::string const & line);

    static void parse_matte(std::string const & value, std::string const & line, scene & scn);

    static void parse_metal(std::string const & value, std::string const & line, scene & scn);

    static void parse_refractive(std::string const & value, std::string const & line, scene & scn);

    static void p_sphere(std::string const & value, std::string const & line, scene & scn);

    static void p_cylinder(std::string const & value, std::string const & line, scene & scn);
  };

}  // namespace render

#endif
