#pragma once

#include "config.hpp"
#include "ray.hpp"
#include "scene.hpp"
#include "vector.hpp"

#include <random>

namespace render {

  // Color de fondo como gradiente vertical entre `background_dark_color` y
  // `background_light_color`.
  color background_gradient(config const & cfg, ray const & r);

  // Color de un rayo simple.
  // Recibe RNG para eliminar thread_local interno
  color trace_ray(ray const & r, scene const & scene, config const & cfg, int depth,
                  std::mt19937_64 & rng);

  // Funciones de reflexión para cada tipo de material
  vector reflect_matte(vector const & normal, std::mt19937_64 & rng);
  vector reflect_metal(vector const & incident, vector const & normal, double diffusion,
                       std::mt19937_64 & rng);
  vector reflect_refractive(vector const & incident, vector const & normal, double refraction_index,
                            bool outward) noexcept;

}  // namespace render
