#ifndef RENDER_MATERIAL_HPP
#define RENDER_MATERIAL_HPP

#include "vector.hpp"

#include <string>
#include <variant>

namespace render {

  /// Tipos de materiales
  enum class material_type { matte, metal, refractive };

  /// Material mate
  struct matte_material {
    color reflectance;

    matte_material(double r, double g, double b) : reflectance(r, g, b) { }
  };

  /// Material metálico
  struct metal_material {
    color reflectance;
    double diffusion;

    metal_material(double r, double g, double b, double diff)
        : reflectance(r, g, b), diffusion(diff) { }
  };

  /// Material refractivo
  struct refractive_material {
    double refraction_index;

    explicit refractive_material(double index) : refraction_index(index) { }
  };

  using material_data = std::variant<matte_material, metal_material, refractive_material>;

  /// Clase para un material con su nombre y tipo
  class material {
  public:
    material(std::string name,
             material_data data)  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
        : name_(std::move(name)), data_(data) { }

    [[nodiscard]] std::string const & get_name() const { return name_; }

    [[nodiscard]] material_data const & get_data() const { return data_; }

    [[nodiscard]] material_type get_type() const {
      if (std::holds_alternative<matte_material>(data_)) {
        return material_type::matte;
      }
      if (std::holds_alternative<metal_material>(data_)) {
        return material_type::metal;
      }
      return material_type::refractive;
    }

  private:
    std::string name_;
    material_data data_;
  };

}  // namespace render

#endif
