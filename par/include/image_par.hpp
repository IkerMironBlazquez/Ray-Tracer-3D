#ifndef IMAGE_PAR_HPP
#define IMAGE_PAR_HPP

#include <cstdint>
#include <string>
#include <utility>  // Para std::pair
#include <vector>

namespace render {

  /// @brief Estructura PAR para una imagen
  /// Los píxeles se representan como 3 secuencias (SOA)
  /// independientes. Cada una de las secuencias contendrá elementos que deberán estar
  /// en el rango de 0 a 255."
  class image_par {
  private:
    int width_;   ///< Ancho de la imagen en píxeles
    int height_;  ///< Alto de la imagen en píxeles

    // PAR: 3 vectores separados para cada componente de color
    std::vector<std::uint8_t> r_channel_;  ///< Canal rojo [0-255]
    std::vector<std::uint8_t> g_channel_;  ///< Canal verde [0-255]
    std::vector<std::uint8_t> b_channel_;  ///< Canal azul [0-255]

  public:
    /// @brief Constructor que inicializa una imagen PAR con dimensiones específicas
    /// @param width Ancho de la imagen en píxeles
    /// @param height Alto de la imagen en píxeles
    image_par(int width, int height);

    /// @brief Establece el valor RGB de un píxel específico
    /// @param row Fila del píxel (0 a height-1)
    /// @param col Columna del píxel (0 a width-1)
    /// @param red Valor del canal rojo [0-255]
    /// @param green Valor del canal verde [0-255]
    /// @param blue Valor del canal azul [0-255]
    void set_pixel(int row, int col, std::uint8_t red, std::uint8_t green, std::uint8_t blue);

    /// @brief Obtiene las dimensiones de la imagen
    /// @return Par (width, height)
    [[nodiscard]] std::pair<int, int> get_dimensions() const noexcept;

    /// @brief Escribe la imagen en formato PPM (P3) a un archivo
    /// @param filename Nombre del archivo de salida
    /// @return true si se escribió exitosamente, false en caso de error
    [[nodiscard]] bool write_ppm(std::string const & filename) const;

    /// @brief Canales para optimizaciones (solo lectura)
    [[nodiscard]] std::vector<std::uint8_t> const & get_r_channel() const noexcept;
    [[nodiscard]] std::vector<std::uint8_t> const & get_g_channel() const noexcept;
    [[nodiscard]] std::vector<std::uint8_t> const & get_b_channel() const noexcept;

  private:
    /// @brief Convierte (row, col) a índice lineal
    /// @param row Fila del píxel
    /// @param col Columna del píxel
    /// @return Índice en los vectores de canales
    [[nodiscard]] size_t get_index(int row, int col) const noexcept;
  };

}  // namespace render

#endif  // IMAGE_PAR_HPP
