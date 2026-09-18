#ifndef RENDER_VECTOR_HPP
#define RENDER_VECTOR_HPP

#include <cmath>
#include <ostream>

namespace render {

  /// Clase (Vector3D) optimizada para rendimiento
  class vector {
  public:
    // Constructores optimizados con noexcept y constexpr
    constexpr vector() noexcept : x{0.0}, y{0.0}, z{0.0} { }

    constexpr vector(double cx, double cy, double cz) noexcept : x{cx}, y{cy}, z{cz} { }

    // Componentes inline optimizadas
    [[nodiscard]] constexpr double get_x() const noexcept { return x; }

    [[nodiscard]] constexpr double get_y() const noexcept { return y; }

    [[nodiscard]] constexpr double get_z() const noexcept { return z; }

    constexpr void set_x(double value) noexcept { x = value; }

    constexpr void set_y(double value) noexcept { y = value; }

    constexpr void set_z(double value) noexcept { z = value; }

    // Operaciones básicas optimizadas
    [[nodiscard]] double magnitude() const;

    [[nodiscard]] constexpr double magnitude_squared() const noexcept {
      return x * x + y * y + z * z;
    }

    [[nodiscard]] vector normalize() const;

    // Operadores optimizados inline en header para mejor rendimiento
    [[nodiscard]] constexpr vector operator+(vector const & other) const noexcept {
      return vector{x + other.x, y + other.y, z + other.z};
    }

    [[nodiscard]] constexpr vector operator-(vector const & other) const noexcept {
      return vector{x - other.x, y - other.y, z - other.z};
    }

    [[nodiscard]] constexpr vector operator*(double scalar) const noexcept {
      return vector{x * scalar, y * scalar, z * scalar};
    }

    [[nodiscard]] constexpr vector operator/(double scalar) const noexcept {
      return vector{x / scalar, y / scalar, z / scalar};
    }

    constexpr vector & operator+=(vector const & other) noexcept {
      x += other.x;
      y += other.y;
      z += other.z;
      return *this;
    }

    constexpr vector & operator-=(vector const & other) noexcept {
      x -= other.x;
      y -= other.y;
      z -= other.z;
      return *this;
    }

    constexpr vector & operator*=(double scalar) noexcept {
      x *= scalar;
      y *= scalar;
      z *= scalar;
      return *this;
    }

    constexpr vector & operator/=(double scalar) noexcept {
      x /= scalar;
      y /= scalar;
      z /= scalar;
      return *this;
    }

    // -> Comparación optimizada inline
    [[nodiscard]] bool operator==(vector const & other) const noexcept {
      constexpr double epsilon = 1e-9;
      return std::abs(x - other.x) < epsilon and
             std::abs(y - other.y) < epsilon and
             std::abs(z - other.z) < epsilon;
    }

    [[nodiscard]] bool operator!=(vector const & other) const noexcept { return !(*this == other); }

    // Producto escalar optimizado inline (operación crítica para intersecciones)
    [[nodiscard]] constexpr double dot(vector const & other) const noexcept {
      return x * other.x + y * other.y + z * other.z;
    }

    // Producto vectorial optimizado inline
    [[nodiscard]] constexpr vector cro(vector const & other) const noexcept {
      return vector{y * other.z - z * other.y, z * other.x - x * other.z,
                    x * other.y - y * other.x};
    }

    // Negativo optimizado inline
    [[nodiscard]] constexpr vector operator-() const noexcept { return vector{-x, -y, -z}; }

    // Compatibilidad con RGB optimizada
    [[nodiscard]] constexpr double r() const noexcept { return x; }

    [[nodiscard]] constexpr double g() const noexcept { return y; }

    [[nodiscard]] constexpr double b() const noexcept { return z; }

  private:
    double x, y, z;
  };

  // Operadores externos optimizados inline
  [[nodiscard]] constexpr vector operator*(double scalar, vector const & vec) noexcept {
    return vec * scalar;
  }

  std::ostream & operator<<(std::ostream & os, vector const & vec);

  // Alias
  using point3d = vector;
  using color   = vector;
  using rgb     = vector;

  // Funciones útiles para colores (RGB)
  /// Convierte un color con valores [0,1] a RGB entero [0,255]
  [[nodiscard]] vector color_to_rgb(color const & col);

  /// Aplica gamma a un color
  [[nodiscard]] color a_gamma(color const & col, double gamma);

  /// Clamp de valores de color al rango [0,1]
  [[nodiscard]] color clamp_color(color const & col);

}  // namespace render

#endif
