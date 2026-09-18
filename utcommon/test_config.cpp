#include "config.hpp"
#include "config_parser.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

namespace {

  // Clase para crear archivos temporales
  class temp_file {
  public:
    explicit temp_file(std::string const & content) : path_("test_temp_" + random_name() + ".txt") {
      std::ofstream out(path_);
      out << content;
      out.close();
    }

    ~temp_file() { std::filesystem::remove(path_); }

    [[nodiscard]] std::string get_path() const { return path_; }

    temp_file(temp_file const &)             = delete;
    temp_file & operator=(temp_file const &) = delete;
    temp_file(temp_file &&)                  = delete;
    temp_file & operator=(temp_file &&)      = delete;

  private:
    std::string path_;

    static std::string random_name() {
      static int counter = 0;
      return std::to_string(counter++);
    }
  };

}  // namespace

// Test para valores por defecto
TEST(config_test, default_values) {
  render::config cfg;

  EXPECT_EQ(cfg.get_aspect_ratio_width(), 16);
  EXPECT_EQ(cfg.get_aspect_ratio_height(), 9);
  EXPECT_EQ(cfg.get_image_width(), 1'920);
  EXPECT_EQ(cfg.get_image_height(), 1'080);
  EXPECT_DOUBLE_EQ(cfg.get_gamma(), 2.2);
  EXPECT_DOUBLE_EQ(cfg.get_field_of_view(), 90.0);
  EXPECT_EQ(cfg.get_samples_per_pixel(), 20);
  EXPECT_EQ(cfg.get_max_depth(), 5);
  EXPECT_EQ(cfg.get_material_rng_s(), 13);
  EXPECT_EQ(cfg.get_ray_rng_s(), 19);

  // Verificar posiciones de cámara
  EXPECT_DOUBLE_EQ(cfg.get_camera_position().get_x(), 0.0);
  EXPECT_DOUBLE_EQ(cfg.get_camera_position().get_y(), 0.0);
  EXPECT_DOUBLE_EQ(cfg.get_camera_position().get_z(), -10.0);

  EXPECT_DOUBLE_EQ(cfg.get_camera_target().get_x(), 0.0);
  EXPECT_DOUBLE_EQ(cfg.get_camera_target().get_y(), 0.0);
  EXPECT_DOUBLE_EQ(cfg.get_camera_target().get_z(), 0.0);

  EXPECT_DOUBLE_EQ(cfg.get_camera_north().get_x(), 0.0);
  EXPECT_DOUBLE_EQ(cfg.get_camera_north().get_y(), 1.0);
  EXPECT_DOUBLE_EQ(cfg.get_camera_north().get_z(), 0.0);

  // Verificar colores de fondo
  EXPECT_DOUBLE_EQ(cfg.get_background_dark_color().r(), 0.25);
  EXPECT_DOUBLE_EQ(cfg.get_background_dark_color().g(), 0.5);
  EXPECT_DOUBLE_EQ(cfg.get_background_dark_color().b(), 1.0);

  EXPECT_DOUBLE_EQ(cfg.get_background_light_color().r(), 1.0);
  EXPECT_DOUBLE_EQ(cfg.get_background_light_color().g(), 1.0);
  EXPECT_DOUBLE_EQ(cfg.get_background_light_color().b(), 1.0);
}

TEST(config_test, setters) {
  render::config cfg;

  cfg.set_image_width(1'200);
  EXPECT_EQ(cfg.get_image_width(), 1'200);

  cfg.set_gamma(2.5);
  EXPECT_DOUBLE_EQ(cfg.get_gamma(), 2.5);

  cfg.set_field_of_view(45.0);
  EXPECT_DOUBLE_EQ(cfg.get_field_of_view(), 45.0);

  cfg.set_samples_per_pixel(100);
  EXPECT_EQ(cfg.get_samples_per_pixel(), 100);

  cfg.set_max_depth(10);
  EXPECT_EQ(cfg.get_max_depth(), 10);

  cfg.set_camera_position(render::point3d(1.0, 2.0, 3.0));
  EXPECT_DOUBLE_EQ(cfg.get_camera_position().get_x(), 1.0);
  EXPECT_DOUBLE_EQ(cfg.get_camera_position().get_y(), 2.0);
  EXPECT_DOUBLE_EQ(cfg.get_camera_position().get_z(), 3.0);
}

// Test para validación de aspect_ratio
TEST(config_test, invalid_aspect_ratio) {
  render::config cfg;

  EXPECT_THROW(cfg.set_aspect_ratio(0, 9), std::invalid_argument);
  EXPECT_THROW(cfg.set_aspect_ratio(16, 0), std::invalid_argument);
  EXPECT_THROW(cfg.set_aspect_ratio(-16, 9), std::invalid_argument);
}

// Test para validación de image_width
TEST(config_test, invalid_image_width) {
  render::config cfg;

  EXPECT_THROW(cfg.set_image_width(0), std::invalid_argument);
  EXPECT_THROW(cfg.set_image_width(-100), std::invalid_argument);
}

// Test para validación de field_of_view
TEST(config_test, invalid_field_of_view) {
  render::config cfg;

  EXPECT_THROW(cfg.set_field_of_view(0.0), std::invalid_argument);
  EXPECT_THROW(cfg.set_field_of_view(180.0), std::invalid_argument);
  EXPECT_THROW(cfg.set_field_of_view(200.0), std::invalid_argument);
  EXPECT_THROW(cfg.set_field_of_view(-10.0), std::invalid_argument);
}

// Test para validación de samples_per_pixel
TEST(config_test, invalid_samples_per_pixel) {
  render::config cfg;

  EXPECT_THROW(cfg.set_samples_per_pixel(0), std::invalid_argument);
  EXPECT_THROW(cfg.set_samples_per_pixel(-5), std::invalid_argument);
}

// Test para validación de max_depth
TEST(config_test, invalid_max_depth) {
  render::config cfg;

  EXPECT_THROW(cfg.set_max_depth(0), std::invalid_argument);
  EXPECT_THROW(cfg.set_max_depth(-1), std::invalid_argument);
}

// Test para validación de seeds
TEST(config_test, invalid_seeds) {
  render::config cfg;

  EXPECT_THROW(cfg.set_material_rng_seed(0), std::invalid_argument);
  EXPECT_THROW(cfg.set_material_rng_seed(-1), std::invalid_argument);

  EXPECT_THROW(cfg.set_ray_rng_seed(0), std::invalid_argument);
  EXPECT_THROW(cfg.set_ray_rng_seed(-1), std::invalid_argument);
}

// Test para validación de colores
TEST(config_test, invalid_colors) {
  render::config cfg;

  // Componentes fuera de rango [0, 1]
  EXPECT_THROW(cfg.set_background_dark_color(render::color(-0.1, 0.5, 0.5)), std::invalid_argument);
  EXPECT_THROW(cfg.set_background_dark_color(render::color(0.5, 1.5, 0.5)), std::invalid_argument);
  EXPECT_THROW(cfg.set_background_dark_color(render::color(0.5, 0.5, -0.1)), std::invalid_argument);

  EXPECT_THROW(cfg.set_background_light_color(render::color(1.5, 0.5, 0.5)), std::invalid_argument);
}

// Test para archivo de configuración válido completo
TEST(config_parser_test, valid_complete_config) {
  temp_file file(R"(image_width: 1200
gamma: 2.2
camera_position: 13 2 3
camera_target: 0 0 0
camera_north: 0 1 0
field_of_view: 20
samples_per_pixel: 10
max_depth: 5
material_rng_seed: 45
ray_rng_seed: 133
background_dark_color: .25 .5 1
background_light_color: 1 1 1
)");

  render::config cfg = render::config_parser::parse_file(file.get_path());

  EXPECT_EQ(cfg.get_image_width(), 1'200);
  EXPECT_DOUBLE_EQ(cfg.get_gamma(), 2.2);
  EXPECT_DOUBLE_EQ(cfg.get_camera_position().get_x(), 13.0);
  EXPECT_DOUBLE_EQ(cfg.get_camera_position().get_y(), 2.0);
  EXPECT_DOUBLE_EQ(cfg.get_camera_position().get_z(), 3.0);
  EXPECT_DOUBLE_EQ(cfg.get_field_of_view(), 20.0);
  EXPECT_EQ(cfg.get_samples_per_pixel(), 10);
  EXPECT_EQ(cfg.get_max_depth(), 5);
  EXPECT_EQ(cfg.get_material_rng_seed(), 45);
  EXPECT_EQ(cfg.get_ray_rng_seed(), 133);
}

// Test para archivo vacío usa valores por defecto
TEST(config_parser_test, empty_file_uses_defaults) {
  temp_file file("");

  render::config cfg = render::config_parser::parse_file(file.get_path());

  EXPECT_EQ(cfg.get_image_width(), 1'920);
  EXPECT_DOUBLE_EQ(cfg.get_gamma(), 2.2);
  EXPECT_EQ(cfg.get_samples_per_pixel(), 20);
}

// Test para líneas vacías y comentarios
TEST(config_parser_test, blank_lines_and_comments) {
  temp_file file(R"(
# Este es un comentario
image_width: 800

gamma: 1.8
   
# Otro comentario
samples_per_pixel: 5
)");

  render::config cfg = render::config_parser::parse_file(file.get_path());

  EXPECT_EQ(cfg.get_image_width(), 800);
  EXPECT_DOUBLE_EQ(cfg.get_gamma(), 1.8);
  EXPECT_EQ(cfg.get_samples_per_pixel(), 5);
}

// Test para trim
TEST(config_parser_test, whitespace_handling) {
  temp_file file(R"(   image_width:    1000   
   gamma:   2.0   
camera_position:    10   5   -8   
)");

  render::config cfg = render::config_parser::parse_file(file.get_path());

  EXPECT_EQ(cfg.get_image_width(), 1'000);
  EXPECT_DOUBLE_EQ(cfg.get_gamma(), 2.0);
  EXPECT_DOUBLE_EQ(cfg.get_camera_position().get_x(), 10.0);
  EXPECT_DOUBLE_EQ(cfg.get_camera_position().get_y(), 5.0);
  EXPECT_DOUBLE_EQ(cfg.get_camera_position().get_z(), -8.0);
}

// Test para parámetros repetidos
TEST(config_parser_test, repeated_parameters_last_wins) {
  temp_file file(R"(gamma: 1.5
image_width: 900
gamma: 2.5
)");

  render::config cfg = render::config_parser::parse_file(file.get_path());

  EXPECT_DOUBLE_EQ(cfg.get_gamma(), 2.5);
  EXPECT_EQ(cfg.get_image_width(), 900);
}

// Test para aspect_ratio
TEST(config_parser_test, aspect_ratio) {
  temp_file file("aspect_ratio: 4 3");

  render::config cfg = render::config_parser::parse_file(file.get_path());

  EXPECT_EQ(cfg.get_aspect_ratio_width(), 4);
  EXPECT_EQ(cfg.get_aspect_ratio_height(), 3);
}

// Test para todos los parámetros de cámara
TEST(config_parser_test, camera_parameters) {
  temp_file file(R"(camera_position: 10 20 30
camera_target: 1 2 3
camera_north: 0 1 0
)");

  render::config cfg = render::config_parser::parse_file(file.get_path());

  EXPECT_DOUBLE_EQ(cfg.get_camera_position().get_x(), 10.0);
  EXPECT_DOUBLE_EQ(cfg.get_camera_position().get_y(), 20.0);
  EXPECT_DOUBLE_EQ(cfg.get_camera_position().get_z(), 30.0);

  EXPECT_DOUBLE_EQ(cfg.get_camera_target().get_x(), 1.0);
  EXPECT_DOUBLE_EQ(cfg.get_camera_target().get_y(), 2.0);
  EXPECT_DOUBLE_EQ(cfg.get_camera_target().get_z(), 3.0);

  EXPECT_DOUBLE_EQ(cfg.get_camera_north().get_x(), 0.0);
  EXPECT_DOUBLE_EQ(cfg.get_camera_north().get_y(), 1.0);
  EXPECT_DOUBLE_EQ(cfg.get_camera_north().get_z(), 0.0);
}

// Test para fondo
TEST(config_parser_test, background_colors) {
  temp_file file(R"(background_dark_color: 0.1 0.2 0.3
background_light_color: 0.9 0.8 0.7
)");

  render::config cfg = render::config_parser::parse_file(file.get_path());

  EXPECT_DOUBLE_EQ(cfg.get_background_dark_color().r(), 0.1);
  EXPECT_DOUBLE_EQ(cfg.get_background_dark_color().g(), 0.2);
  EXPECT_DOUBLE_EQ(cfg.get_background_dark_color().b(), 0.3);

  EXPECT_DOUBLE_EQ(cfg.get_background_light_color().r(), 0.9);
  EXPECT_DOUBLE_EQ(cfg.get_background_light_color().g(), 0.8);
  EXPECT_DOUBLE_EQ(cfg.get_background_light_color().b(), 0.7);
}
