#pragma once

#include "config.hpp"
#include "ray.hpp"
#include "vector.hpp"
#include <random>

namespace render {

  /**
   * @brief Sistema de cámara para ray tracing
   *
   * Implementa el cálculo de la ventana.
   * Transforma los parámetros de configuración de la cámara en una ventana 3D donde
   * proyectar los rayos hacia la escena.
   *
   * Algoritmo
   * 1. Vector focal: vf = P - D
   * 2. Distancia focal: df = ||vf||
   * 3. Altura ventana: hp = 2 * tan(α/2) * df
   * 4. Anchura ventana: wp = hp * (width/height)
   * 5. Vectores directores: u, v
   * 6. Vectores proyectados: ph, pv
   */
  class camera {
  private:
    // PARÁMETROS DE GEOMETRÍA DE LA CÁMARA
    vector camera_position_;  ///< Posición P de la cámara (defecto: 0,0,-10)
    vector camera_target_;    ///< Punto destino D hacia donde mira (defecto: 0,0,0)
    vector camera_north_;     ///< Vector norte n⃗ (defecto: 0,1,0)
    double field_of_view_;    ///< Ángulo de campo de visión α en grados (defecto: 90°)

    // DIMENSIONES DE LA IMAGEN
    int image_width_;   ///< Anchura en píxeles
    int image_height_;  ///< Altura en píxeles (calculada desde aspect_ratio)

    // VENTANA
    vector origin_;   ///< Origen O de la ventana (esquina sup-izq del píxel 0,0)
    vector delta_x_;  ///< Vector Δx⃗ = ph⃗/width (desplazamiento horizontal por píxel)
    vector delta_y_;  ///< Vector Δy⃗ = pv⃗/height (desplazamiento vertical por píxel)

  public:
    /**
     * @brief Constructor que inicializa la cámara desde configuración
     * @param cfg Configuración con parámetros de cámara/imagen
     *
     * Automáticamente calcula la ventana usando setup_projection_window()
     */
    explicit camera(config const & cfg);

    /**
     * @brief Calcula la ventana usando el algoritmo de 6 pasos
     *
     * Implementa exactamente 3.1 (Fórmulas):
     * 1. vf⃗ = P - D (vector focal)
     * 2. df = ||vf⃗|| (distancia focal)
     * 3. hp = 2×tan(α/2)×df (altura ventana)
     * 4. wp = hp×(w/h) (anchura ventana)
     * 5. u⃗, v⃗ (vectores directores)
     * 6. ph⃗, pv⃗ (vectores proyectados)
     * 7. O, Δx⃗, Δy⃗ (origen y deltas)
     */
    void setup_projection_window();

    /**
     * @brief Genera un rayo desde la cámara hacia un píxel con muestreo aleatorio
     * @param row Fila del píxel (0 = arriba)
     * @param col Columna del píxel (0 = izquierda)
     * @param rng Generador de números aleatorios (uno por hilo para paralelización)
     * @return Rayo desde la posición de cámara hacia una posición aleatoria en el píxel
     *
     * Implementa la fórmula: Q = O + Δx⃗×(c+δx) + Δy⃗×(f+δy)
     * donde δx, δy ∈ [-0.5, 0.5] son valores aleatorios para anti-aliasing
     */
    [[nodiscard]] ray get_ray(int row, int col, std::mt19937_64 & rng) const;

    [[nodiscard]] vector get_origin() const { return origin_; }

    [[nodiscard]] vector get_delta_x() const { return delta_x_; }

    [[nodiscard]] vector get_delta_y() const { return delta_y_; }

    [[nodiscard]] vector get_camera_position() const { return camera_position_; }

    [[nodiscard]] vector get_camera_target() const { return camera_target_; }

    [[nodiscard]] double get_field_of_view() const { return field_of_view_; }

    [[nodiscard]] int get_image_width() const { return image_width_; }

    [[nodiscard]] int get_image_height() const { return image_height_; }
  };

}  // namespace render
