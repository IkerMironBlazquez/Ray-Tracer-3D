#include "scene.hpp"
#include "scene_parser.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <thread>

namespace {

  // Clase para crear archivos temporales
  class temp_file {
  public:
    explicit temp_file(std::string const & content) {
      auto tmpdir = std::filesystem::temp_directory_path();
      path_       = (tmpdir / ("test_temp_" + random_name() + ".txt")).string();
      std::ofstream out(path_);
      out << content;
      out.close();
    }

    ~temp_file() {
      std::error_code ec;
      std::filesystem::remove(path_, ec);
    }

    [[nodiscard]] std::string get_path() const { return path_; }

    temp_file(temp_file const &)             = delete;
    temp_file & operator=(temp_file const &) = delete;
    temp_file(temp_file &&)                  = delete;
    temp_file & operator=(temp_file &&)      = delete;

  private:
    std::string path_;

    static std::string random_name() {
      using namespace std::chrono;
      auto now      = high_resolution_clock::now().time_since_epoch().count();
      auto tid_hash = std::hash<std::thread::id>{}(std::this_thread::get_id());
      static std::atomic<int> counter{0};
      return std::to_string(now) +
             "_" +
             std::to_string(tid_hash) +
             "_" +
             std::to_string(counter.fetch_add(1));
    }
  };

}  // namespace

// Test para material mate
TEST(scene_test, matte_material) {
  temp_file file("matte: mat1 0.5 0.6 0.7\n");
  auto scn = render::scene_parser::parse_file(file.get_path());

  EXPECT_TRUE(scn.has_material("mat1"));
  auto const & mat = scn.get_material("mat1");
  EXPECT_EQ(mat.get_name(), "mat1");
  EXPECT_EQ(mat.get_type(), render::material_type::matte);

  auto const & matte_data = std::get<render::matte_material>(mat.get_data());
  EXPECT_DOUBLE_EQ(matte_data.reflectance.get_x(), 0.5);
  EXPECT_DOUBLE_EQ(matte_data.reflectance.get_y(), 0.6);
  EXPECT_DOUBLE_EQ(matte_data.reflectance.get_z(), 0.7);
}

// Test para material metal
TEST(scene_test, metal_material) {
  temp_file file("metal: metal1 0.8 0.9 1.0 2.5\n");
  auto scn = render::scene_parser::parse_file(file.get_path());

  EXPECT_TRUE(scn.has_material("metal1"));
  auto const & mat = scn.get_material("metal1");
  EXPECT_EQ(mat.get_name(), "metal1");
  EXPECT_EQ(mat.get_type(), render::material_type::metal);

  auto const & metal_data = std::get<render::metal_material>(mat.get_data());
  EXPECT_DOUBLE_EQ(metal_data.reflectance.get_x(), 0.8);
  EXPECT_DOUBLE_EQ(metal_data.reflectance.get_y(), 0.9);
  EXPECT_DOUBLE_EQ(metal_data.reflectance.get_z(), 1.0);
  EXPECT_DOUBLE_EQ(metal_data.diffusion, 2.5);
}

// Test para material refractivo
TEST(scene_test, refractive_material) {
  temp_file file("refractive: ref99 1.5\n");
  auto scn = render::scene_parser::parse_file(file.get_path());

  EXPECT_TRUE(scn.has_material("ref99"));
  auto const & mat = scn.get_material("ref99");
  EXPECT_EQ(mat.get_name(), "ref99");
  EXPECT_EQ(mat.get_type(), render::material_type::refractive);

  auto const & ref_data = std::get<render::refractive_material>(mat.get_data());
  EXPECT_DOUBLE_EQ(ref_data.refraction_index, 1.5);
}

// Test para esfera
TEST(scene_test, sphere_object) {
  temp_file file("matte: mat1 0.5 0.6 0.7\n"
                 "sphere: 1.0 2.0 3.0 0.5 mat1\n");
  auto scn = render::scene_parser::parse_file(file.get_path());

  EXPECT_EQ(scn.get_objects().size(), 1);
  auto const & obj = scn.get_objects()[0];
  EXPECT_EQ(obj.get_type(), render::object_type::sphere);
  EXPECT_EQ(obj.get_material_name(), "mat1");

  auto const & sphere_data = std::get<render::sphere>(obj.get_data());
  EXPECT_DOUBLE_EQ(sphere_data.center.get_x(), 1.0);
  EXPECT_DOUBLE_EQ(sphere_data.center.get_y(), 2.0);
  EXPECT_DOUBLE_EQ(sphere_data.center.get_z(), 3.0);
  EXPECT_DOUBLE_EQ(sphere_data.radius, 0.5);
}

// Test para cilindro
TEST(scene_test, cylinder_object) {
  temp_file file("metal: metal1 0.8 0.9 1.0 2.5\n"
                 "cylinder: 1.0 2.0 3.0 0.75 4.0 5.0 6.0 metal1\n");
  auto scn = render::scene_parser::parse_file(file.get_path());

  EXPECT_EQ(scn.get_objects().size(), 1);
  auto const & obj = scn.get_objects()[0];
  EXPECT_EQ(obj.get_type(), render::object_type::cylinder);
  EXPECT_EQ(obj.get_material_name(), "metal1");

  auto const & cyl_data = std::get<render::cylinder>(obj.get_data());
  EXPECT_DOUBLE_EQ(cyl_data.center.get_x(), 1.0);
  EXPECT_DOUBLE_EQ(cyl_data.center.get_y(), 2.0);
  EXPECT_DOUBLE_EQ(cyl_data.center.get_z(), 3.0);
  EXPECT_DOUBLE_EQ(cyl_data.radius, 0.75);
  EXPECT_DOUBLE_EQ(cyl_data.axis.get_x(), 4.0);
  EXPECT_DOUBLE_EQ(cyl_data.axis.get_y(), 5.0);
  EXPECT_DOUBLE_EQ(cyl_data.axis.get_z(), 6.0);
}

// Test para escena completa (ejemplo del enunciado)
TEST(scene_test, complete_scene) {
  temp_file file("matte: mat1 0 0.8 0.8\n"
                 "metal: metal1 0 0.8 0 2.0\n"
                 "refractive: ref99 1.3\n"
                 "sphere: 0 0 0 0.65 mat1\n"
                 "cylinder: 0 0 0 0.5 20 10 -5 metal1\n");
  auto scn = render::scene_parser::parse_file(file.get_path());

  // Verificar materiales
  EXPECT_EQ(scn.get_materials().size(), 3);
  EXPECT_TRUE(scn.has_material("mat1"));
  EXPECT_TRUE(scn.has_material("metal1"));
  EXPECT_TRUE(scn.has_material("ref99"));

  // Verificar objetos
  EXPECT_EQ(scn.get_objects().size(), 2);
  EXPECT_EQ(scn.get_objects()[0].get_type(), render::object_type::sphere);
  EXPECT_EQ(scn.get_objects()[1].get_type(), render::object_type::cylinder);
}

// Test para líneas vacías y comentarios
TEST(scene_test, empty_lines_and_comments) {
  temp_file file("\n"
                 "   \n"
                 "# Comentario\n"
                 "matte: mat1 0.5 0.6 0.7\n"
                 "\n"
                 "   # Otro comentario\n"
                 "sphere: 0 0 0 1.0 mat1\n");
  auto scn = render::scene_parser::parse_file(file.get_path());

  EXPECT_EQ(scn.get_materials().size(), 1);
  EXPECT_EQ(scn.get_objects().size(), 1);
}

// Test para múltiples objetos
TEST(scene_test, multiple_objects) {
  temp_file file("matte: mat1 0.5 0.6 0.7\n"
                 "metal: metal1 0.8 0.9 1.0 2.5\n"
                 "sphere: 0 0 0 1.0 mat1\n"
                 "sphere: 1 1 1 0.5 metal1\n"
                 "cylinder: 2 2 2 0.3 1 0 0 mat1\n");
  auto scn = render::scene_parser::parse_file(file.get_path());

  EXPECT_EQ(scn.get_materials().size(), 2);
  EXPECT_EQ(scn.get_objects().size(), 3);
}

// Tests de errores

// Test para entidad desconocida
TEST(scene_test, unknown_entity) {
  temp_file file("triangle: 0 0 0 1 1 1 2 2 2 mat1\n");
  EXPECT_EXIT(render::scene_parser::parse_file(file.get_path()), ::testing::ExitedWithCode(1),
              "Error: Unknown scene entity: triangle");
}

// Test para parámetros insuficientes en material mate
TEST(scene_test, matte_insufficient_params) {
  temp_file file("matte: mat1 0.5 0.6\n");
  EXPECT_EXIT(render::scene_parser::parse_file(file.get_path()), ::testing::ExitedWithCode(1),
              "Error: Invalid matte material parameters");
}

// Test para parámetros insuficientes en material metal
TEST(scene_test, metal_insufficient_params) {
  temp_file file("metal: metal1 0.5 0.6 0.7\n");
  EXPECT_EXIT(render::scene_parser::parse_file(file.get_path()), ::testing::ExitedWithCode(1),
              "Error: Invalid metal material parameters");
}

// Test para parámetros insuficientes en material refractivo
TEST(scene_test, refractive_insufficient_params) {
  temp_file file("refractive: ref1\n");
  EXPECT_EXIT(render::scene_parser::parse_file(file.get_path()), ::testing::ExitedWithCode(1),
              "Error: Invalid refractive material parameters");
}

// Test para parámetros extra en material mate
TEST(scene_test, matte_extra_params) {
  temp_file file("matte: mat1 0.5 0.6 0.7 extra\n");
  EXPECT_EXIT(render::scene_parser::parse_file(file.get_path()), ::testing::ExitedWithCode(1),
              "Error: Extra data after configuration value for key: \\[matte:\\]");
}

// Test para parámetros extra en esfera
TEST(scene_test, sphere_extra_params) {
  temp_file file("matte: mat1 0.5 0.6 0.7\n"
                 "sphere: 0 0 0 1.0 mat1 extra\n");
  EXPECT_EXIT(render::scene_parser::parse_file(file.get_path()), ::testing::ExitedWithCode(1),
              "Error: Extra data after configuration value for key: \\[sphere:\\]");
}

// Test para parámetros inválidos en esfera
TEST(scene_test, sphere_invalid_params) {
  temp_file file("matte: mat1 0.5 0.6 0.7\n"
                 "sphere: 0 0 0 invalid mat1\n");
  EXPECT_EXIT(render::scene_parser::parse_file(file.get_path()), ::testing::ExitedWithCode(1),
              "Error: Invalid sphere parameters");
}

// Test para radio inválido en esfera (radio <= 0)
TEST(scene_test, sphere_invalid_radius) {
  temp_file file("matte: mat1 0.5 0.6 0.7\n"
                 "sphere: 0 0 0 0.0 mat1\n");
  EXPECT_EXIT(render::scene_parser::parse_file(file.get_path()), ::testing::ExitedWithCode(1),
              "Error: Invalid sphere parameters");
}

// Test para radio negativo en esfera
TEST(scene_test, sphere_negative_radius) {
  temp_file file("matte: mat1 0.5 0.6 0.7\n"
                 "sphere: 0 0 0 -1.0 mat1\n");
  EXPECT_EXIT(render::scene_parser::parse_file(file.get_path()), ::testing::ExitedWithCode(1),
              "Error: Invalid sphere parameters");
}

// Test para radio inválido en cilindro (radio <= 0)
TEST(scene_test, cylinder_invalid_radius) {
  temp_file file("matte: mat1 0.5 0.6 0.7\n"
                 "cylinder: 0 0 0 0.0 1 0 0 mat1\n");
  EXPECT_EXIT(render::scene_parser::parse_file(file.get_path()), ::testing::ExitedWithCode(1),
              "Error: Invalid cylinder parameters");
}

// Test para material no definido en esfera
TEST(scene_test, sphere_undefined_material) {
  temp_file file("sphere: 0 0 0 1.0 undefined_mat\n");
  EXPECT_EXIT(render::scene_parser::parse_file(file.get_path()), ::testing::ExitedWithCode(1),
              "Error: Material not found: \\[undefined_mat\\]");
}

// Test para material no definido en cilindro
TEST(scene_test, cylinder_undefined_material) {
  temp_file file("cylinder: 0 0 0 0.5 1 0 0 undefined_mat\n");
  EXPECT_EXIT(render::scene_parser::parse_file(file.get_path()), ::testing::ExitedWithCode(1),
              "Error: Material not found: \\[undefined_mat\\]");
}

// Test para material repetido
TEST(scene_test, repeated_material) {
  temp_file file("matte: mat1 0.5 0.6 0.7\n"
                 "matte: mat1 0.8 0.9 1.0\n");
  EXPECT_EXIT(render::scene_parser::parse_file(file.get_path()), ::testing::ExitedWithCode(1),
              "Error: Material with name \\[mat1\\] already exists");
}

// Test para parámetros insuficientes en esfera
TEST(scene_test, sphere_insufficient_params) {
  temp_file file("matte: mat1 0.5 0.6 0.7\n"
                 "sphere: 0 0 0 1.0\n");
  EXPECT_EXIT(render::scene_parser::parse_file(file.get_path()), ::testing::ExitedWithCode(1),
              "Error: Invalid sphere parameters");
}

// Test para parámetros insuficientes en cilindro
TEST(scene_test, cylinder_insufficient_params) {
  temp_file file("matte: mat1 0.5 0.6 0.7\n"
                 "cylinder: 0 0 0 0.5 1 0 0\n");
  EXPECT_EXIT(render::scene_parser::parse_file(file.get_path()), ::testing::ExitedWithCode(1),
              "Error: Invalid cylinder parameters");
}
