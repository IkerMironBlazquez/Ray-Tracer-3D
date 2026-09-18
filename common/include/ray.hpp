#pragma once

#include "vector.hpp"

namespace render {

  /** @brief Representa un rayo en el espacio 3D (Ray-tracing) */
  class ray {
  private:
    vector origin_;
    vector direction_;

  public:
    /** @brief Constructor por defecto - crea un rayo inválido */
    ray() = default;

    /** @brief Constructor parametrizado
     * @param origin Punto de origen
     * @param direction Vector de dirección (puede no estar normalizado) */
    ray(vector const & origin, vector const & direction)
        : origin_(origin), direction_(direction) { }

    /** @brief Obtiene el punto de origen
     * @return Vector con las coordenadas del origen */
    [[nodiscard]] vector origin() const { return origin_; }

    /** @brief Obtiene el vector
     * @return Vector (puede no estar normalizado) */
    [[nodiscard]] vector direction() const { return direction_; }

    /** @brief Calcula un punto a lo largo
     * @param t Parámetro (t >= 0)
     * @return Punto P(t) = origen + t * dirección */
    [[nodiscard]] vector at(double t) const { return origin_ + direction_ * t; }
  };

}  // namespace render
