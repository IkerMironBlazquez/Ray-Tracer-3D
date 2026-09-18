#ifndef RENDER_CONFIG_HPP
#define RENDER_CONFIG_HPP

#include "vector.hpp"

namespace render {

  /// Clase para el motor
  class config {
  public:
    config();

    [[nodiscard]] int get_aspect_ratio_width() const { return aspect_ratio_width_; }

    [[nodiscard]] int get_aspect_ratio_height() const { return aspect_ratio_height_; }

    [[nodiscard]] int get_image_width() const { return image_width_; }

    [[nodiscard]] int get_image_height() const;

    [[nodiscard]] double get_gamma() const { return gamma_; }

    [[nodiscard]] point3d get_camera_position() const { return camera_position_; }

    [[nodiscard]] point3d get_camera_target() const { return camera_target_; }

    [[nodiscard]] vector get_camera_north() const { return camera_north_; }

    [[nodiscard]] double get_field_of_view() const { return field_of_view_; }

    [[nodiscard]] int get_samples_per_pixel() const { return samples_per_pixel_; }

    [[nodiscard]] int get_max_depth() const { return max_depth_; }

    [[nodiscard]] int get_material_rng_seed() const { return material_rng_seed_; }

    [[nodiscard]] int get_ray_rng_seed() const { return ray_rng_seed_; }

    // Compatibilidad
    [[nodiscard]] int get_material_rng_s() const { return material_rng_seed_; }

    [[nodiscard]] int get_ray_rng_s() const { return ray_rng_seed_; }

    [[nodiscard]] color get_background_dark_color() const { return background_dark_color_; }

    [[nodiscard]] color get_background_light_color() const { return background_light_color_; }

    // Setters
    void set_aspect_ratio(int width, int height);
    void set_image_width(int width);
    void set_gamma(double gamma);
    void set_camera_position(point3d const & pos);
    void set_camera_target(point3d const & target);
    void set_camera_north(vector const & north);
    void set_field_of_view(double fov);
    void set_samples_per_pixel(int samples);
    void set_max_depth(int depth);
    void set_material_rng_seed(int seed);
    void set_ray_rng_seed(int seed);
    void set_background_dark_color(color const & col);
    void set_background_light_color(color const & col);

  private:
    // Parámetros de configuración
    int aspect_ratio_width_{16};
    int aspect_ratio_height_{9};
    int image_width_{1'920};
    double gamma_{2.2};
    point3d camera_position_;
    point3d camera_target_;
    vector camera_north_;
    double field_of_view_{90.0};
    int samples_per_pixel_{20};
    int max_depth_{5};
    int material_rng_seed_{13};
    int ray_rng_seed_{19};
    color background_dark_color_;
    color background_light_color_;
  };

}  // namespace render

#endif
