#include "config.hpp"

#include <stdexcept>

namespace render {

  // Constructor con valores por defecto
  config::config()
      : camera_position_{0.0, 0.0, -10.0}, camera_target_{0.0, 0.0, 0.0},
        camera_north_{0.0, 1.0, 0.0}, background_dark_color_{0.25, 0.5, 1.0},
        background_light_color_{1.0, 1.0, 1.0} { }

  int config::get_image_height() const {
    return static_cast<int>(static_cast<double>(image_width_) /
                            static_cast<double>(aspect_ratio_width_) *
                            static_cast<double>(aspect_ratio_height_));
  }

  void config::set_aspect_ratio(int const width, int const height) {
    if (width <= 0 or height <= 0) {
      throw std::invalid_argument("Aspect ratio values must be positive");
    }
    aspect_ratio_width_  = width;
    aspect_ratio_height_ = height;
  }

  void config::set_image_width(int const width) {
    if (width <= 0) {
      throw std::invalid_argument("Image width must be positive");
    }
    image_width_ = width;
  }

  void config::set_gamma(double const gamma) {
    gamma_ = gamma;
  }

  void config::set_camera_position(point3d const & pos) {
    camera_position_ = pos;
  }

  void config::set_camera_target(point3d const & target) {
    camera_target_ = target;
  }

  void config::set_camera_north(vector const & north) {
    camera_north_ = north;
  }

  void config::set_field_of_view(double const fov) {
    if (fov <= 0.0 or fov >= 180.0) {
      throw std::invalid_argument("Field of view must be between 0 and 180 degrees");
    }
    field_of_view_ = fov;
  }

  void config::set_samples_per_pixel(int const samples) {
    if (samples <= 0) {
      throw std::invalid_argument("Samples per pixel must be positive");
    }
    samples_per_pixel_ = samples;
  }

  void config::set_max_depth(int const depth) {
    if (depth <= 0) {
      throw std::invalid_argument("Max depth must be positive");
    }
    max_depth_ = depth;
  }

  void config::set_material_rng_seed(int const seed) {
    if (seed <= 0) {
      throw std::invalid_argument("Material RNG seed must be positive");
    }
    material_rng_seed_ = seed;
  }

  void config::set_ray_rng_seed(int const seed) {
    if (seed <= 0) {
      throw std::invalid_argument("Ray RNG seed must be positive");
    }
    ray_rng_seed_ = seed;
  }

  void config::set_background_dark_color(color const & col) {
    if (col.r() < 0.0 or
        col.r() > 1.0 or
        col.g() < 0.0 or
        col.g() > 1.0 or
        col.b() < 0.0 or
        col.b() > 1.0)
    {
      throw std::invalid_argument("Color components must be in range [0, 1]");
    }
    background_dark_color_ = col;
  }

  void config::set_background_light_color(color const & col) {
    if (col.r() < 0.0 or
        col.r() > 1.0 or
        col.g() < 0.0 or
        col.g() > 1.0 or
        col.b() < 0.0 or
        col.b() > 1.0)
    {
      throw std::invalid_argument("Color components must be in range [0, 1]");
    }
    background_light_color_ = col;
  }

}  // namespace render
