#include "vector.hpp"

#include <algorithm>
#include <cmath>
#include <ostream>

namespace render {

  // Implementación de operaciones básicas
  double vector::magnitude() const {
    return std::sqrt(x * x + y * y + z * z);
  }

  // magnitude_squared ahora está inline en el header

  vector vector::normalize() const {
    double const mag_sq = magnitude_squared();
    if (mag_sq < 1e-12) {  // Evitar división por cero con epsilon más eficiente
      return vector{0.0, 0.0, 0.0};
    }
    // Usar rsqrt aproximado para mejor rendimiento
    double const inv_mag = 1.0 / std::sqrt(mag_sq);  // NOLINT(cppcoreguidelines-init-variables)
    return vector{x * inv_mag, y * inv_mag, z * inv_mag};
  }

  // Todas las operaciones básicas ahora están inline en el header para mejor rendimiento

  // NOLINTNEXTLINE(misc-use-internal-linkage)
  std::ostream & operator<<(std::ostream & os, vector const & vec) {
    os << "(" << vec.get_x() << ", " << vec.get_y() << ", " << vec.get_z() << ")";
    return os;
  }

  // Funciones útiles para colores (RGB)
  vector color_to_rgb(color const & col) {
    return vector{std::clamp(col.r() * 255.0, 0.0, 255.0), std::clamp(col.g() * 255.0, 0.0, 255.0),
                  std::clamp(col.b() * 255.0, 0.0, 255.0)};
  }

  color a_gamma(color const & col, double gamma) {
    double const gamma_inv = 1.0 / gamma;
    return color{std::pow(std::clamp(col.r(), 0.0, 1.0), gamma_inv),
                 std::pow(std::clamp(col.g(), 0.0, 1.0), gamma_inv),
                 std::pow(std::clamp(col.b(), 0.0, 1.0), gamma_inv)};
  }

  color clamp_color(color const & col) {
    return color{std::clamp(col.r(), 0.0, 1.0), std::clamp(col.g(), 0.0, 1.0),
                 std::clamp(col.b(), 0.0, 1.0)};
  }

}  // namespace render
