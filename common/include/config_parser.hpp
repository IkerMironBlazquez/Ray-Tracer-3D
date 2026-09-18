#ifndef RENDER_CONFIG_PARSER_HPP
#define RENDER_CONFIG_PARSER_HPP

#include "config.hpp"

#include <string>

namespace render {

  /// Clase para parsear archivos de configuración
  class config_parser {
  public:
    /// @param filename Nombre del archivo de configuración
    /// @return Objeto config con los parámetros leídos
    /// @throws Excepciones
    static config parse_file(std::string const & filename);

  private:
    /// Procesa una línea del archivo de configuración
    /// @param line
    /// @param line_number
    /// @param cfg
    static void p_line(std::string const & line, int line_number, config & cfg);

    static std::string trim(std::string const & str);

    static bool is_empty_line(std::string const & line);

    static void parse_aspect_ratio(std::string const & value, std::string const & line,
                                   config & cfg);

    static void parse_image_width(std::string const & value, std::string const & line,
                                  config & cfg);

    static void parse_gamma(std::string const & value, std::string const & line, config & cfg);

    static void parse_camera_position(std::string const & value, std::string const & line,
                                      config & cfg);

    static void parse_camera_target(std::string const & value, std::string const & line,
                                    config & cfg);

    static void parse_camera_north(std::string const & value, std::string const & line,
                                   config & cfg);

    static void parse_field_of_view(std::string const & value, std::string const & line,
                                    config & cfg);

    static void parse_samples_per_pixel(std::string const & value, std::string const & line,
                                        config & cfg);

    static void parse_max_depth(std::string const & value, std::string const & line, config & cfg);

    static void parse_material_rng_s(std::string const & value, std::string const & line,
                                     config & cfg);

    static void parse_ray_rng_s(std::string const & value, std::string const & line, config & cfg);

    static void parse_background_dark_color(std::string const & value, std::string const & line,
                                            config & cfg);

    static void parse_background_light_color(std::string const & value, std::string const & line,
                                             config & cfg);

    static void pro_config_key(std::string const & key, std::string const & value,
                               std::string const & line, config & cfg);
  };

}  // namespace render

#endif
