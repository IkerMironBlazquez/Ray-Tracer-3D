#include "camera.hpp"
#include "config.hpp"
#include "ray.hpp"
#include "vector.hpp"
#include <cmath>
#include <numbers>
#include <random>

namespace render {

  camera::camera(config const & cfg)
      : camera_position_(cfg.get_camera_position()), camera_target_(cfg.get_camera_target()),
        camera_north_(cfg.get_camera_north()), field_of_view_(cfg.get_field_of_view()),
        image_width_(cfg.get_image_width()), image_height_(cfg.get_image_height()) {
    setup_projection_window();
  }

  void camera::setup_projection_window() {
    // Algoritmo

    // 1. Vector focal: vf⃗ = P - D
    vector const vf = camera_position_ - camera_target_;

    // 2. Distancia focal: df = ||vf⃗||
    double const df = vf.magnitude();

    // 3. Altura ventana: hp = 2×tan(α/2)×df
    double const alpha_rad =
        field_of_view_ * std::numbers::pi / 180.0;  // NOLINT(cppcoreguidelines-init-variables)
    double const hp =
        2.0 * std::tan(alpha_rad / 2.0) * df;  // NOLINT(cppcoreguidelines-init-variables)

    // 4. Anchura ventana: wp = hp×(w/h)
    double const aspect_ratio =
        static_cast<double>(image_width_) / static_cast<double>(image_height_);
    double const wp = hp * aspect_ratio;

    // 5. Vectores directores: u⃗, v⃗
    vector const vf_hat = vf.normalize();
    vector const u      = camera_north_.cro(vf_hat).normalize();
    vector const v      = vf_hat.cro(u);

    // 6. Vectores proyección: ph⃗, pv⃗
    vector const ph = u * wp;
    vector const pv = v * (-hp);  // Negativo para coordenadas imagen

    // Origen y deltas
    delta_x_ = ph / static_cast<double>(image_width_);
    delta_y_ = pv / static_cast<double>(image_height_);
    origin_  = camera_position_ - vf - (ph + pv) * 0.5 + (delta_x_ + delta_y_) * 0.5;
  }

  ray camera::get_ray(int row, int col, std::mt19937_64 & rng) const {
    std::uniform_real_distribution<double> distribution(-0.5, 0.5);

    // Anti-aliasing (usando RNG en el hilo actual)
    double const delta_x = distribution(rng);
    double const delta_y = distribution(rng);

    // Q = O + Δx⃗×(c+δx) + Δy⃗×(f+δy)
    vector const Q = origin_ +
                     delta_x_ * (static_cast<double>(col) + delta_x) +
                     delta_y_ * (static_cast<double>(row) + delta_y);

    return {camera_position_, Q - camera_position_};
  }

}  // namespace render
