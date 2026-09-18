#ifndef RENDER_OBJECT_HPP
#define RENDER_OBJECT_HPP

#include "intersection.hpp"
#include "ray.hpp"
#include "vector.hpp"

#include <string>
#include <variant>

namespace render {

  /// Tipos de objetos
  enum class object_type { sphere, cylinder };

  /// Esfera
  struct sphere {
    point3d center;
    double radius;

    sphere(point3d const & c, double r) : center(c), radius(r) { }

    /// @param r Rayo a intersectar
    /// @return Información de la intersección (hit=false si no hay intersección)
    [[nodiscard]] Intersection intersect(ray const & r) const;
  };

  /// Cilindro
  struct cylinder {
    point3d center;
    double radius;
    vector axis;

    cylinder(point3d const & c, double r, vector const & a) : center(c), radius(r), axis(a) { }

    /// Altura del cilindro
    [[nodiscard]] double height() const { return axis.magnitude(); }

    /// @param r Rayo a intersectar
    /// @return Información de la intersección (hit=false si no hay intersección)
    [[nodiscard]] Intersection intersect(ray const & r) const;
  };

  using object_data = std::variant<sphere, cylinder>;

  /// Clase para un objeto con su material
  class object {
  public:
    object(object_data data,
           std::string material_name)  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
        : data_(data), material_name_(std::move(material_name)) { }

    [[nodiscard]] object_data const & get_data() const { return data_; }

    [[nodiscard]] std::string const & get_material_name() const { return material_name_; }

    [[nodiscard]] object_type get_type() const {
      if (std::holds_alternative<sphere>(data_)) {
        return object_type::sphere;
      }
      return object_type::cylinder;
    }

  private:
    object_data data_;
    std::string material_name_;
  };

}  // namespace render

#endif
