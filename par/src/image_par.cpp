#include "image_par.hpp"

#include <cstddef>  // Para size_t
#include <cstdint>  // Para std::uint8_t
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>   // Para std::string
#include <utility>  // Para std::pair
#include <vector>   // Para std::vector

namespace render {

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
  image_par::image_par(int width, int height) : width_(width), height_(height) {
    if (width <= 0 or height <= 0) {
      throw std::invalid_argument("Image dimensions must be positive");
    }

    size_t const total_pixels = static_cast<size_t>(width_) * static_cast<size_t>(height_);

    // PAR: Reservar espacio para los 3 canales separados
    r_channel_.reserve(total_pixels);
    g_channel_.reserve(total_pixels);
    b_channel_.reserve(total_pixels);

    // Inicializar con valores por defecto (negro: 0, 0, 0)
    r_channel_.resize(total_pixels, 0);
    g_channel_.resize(total_pixels, 0);
    b_channel_.resize(total_pixels, 0);
  }

  void image_par::set_pixel(int row, int col, std::uint8_t r, std::uint8_t g, std::uint8_t b) {
    if (row < 0 or row >= height_ or col < 0 or col >= width_) {
      throw std::out_of_range("Pixel coordinates out of bounds");
    }

    size_t const index = get_index(row, col);

    // PAR: Asignar cada componente a su vector
    r_channel_[index] = r;
    g_channel_[index] = g;
    b_channel_[index] = b;
  }

  std::pair<int, int> image_par::get_dimensions() const noexcept {
    return {width_, height_};
  }

  bool image_par::write_ppm(std::string const & filename) const {
    std::ofstream ofs(filename);
    if (!ofs) {
      return false;
    }

    // Escribir cabecera PPM P3
    ofs << "P3\n" << width_ << " " << height_ << "\n255\n";

    // Escribir píxeles: PAR permite canales separados
    for (size_t i = 0; i < r_channel_.size(); ++i) {
      ofs << static_cast<int>(r_channel_[i]) << " " << static_cast<int>(g_channel_[i]) << " "
          << static_cast<int>(b_channel_[i]) << "\n";
    }

    return ofs.good();
  }

  std::vector<std::uint8_t> const & image_par::get_r_channel() const noexcept {
    return r_channel_;
  }

  std::vector<std::uint8_t> const & image_par::get_g_channel() const noexcept {
    return g_channel_;
  }

  std::vector<std::uint8_t> const & image_par::get_b_channel() const noexcept {
    return b_channel_;
  }

  size_t image_par::get_index(int row, int col) const noexcept {
    return static_cast<size_t>(row) * static_cast<size_t>(width_) + static_cast<size_t>(col);
  }

}  // namespace render
