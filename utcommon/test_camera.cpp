#include "camera.hpp"
#include "config.hpp"
#include "vector.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <numbers>

namespace render {

  class CameraTest : public ::testing::Test {
  protected:
    void SetUp() override {
      // Configuración estándar para pruebas
      std_config_.set_camera_position(vector(0, 0, -10));
      std_config_.set_camera_target(vector(0, 0, 0));
      std_config_.set_camera_north(vector(0, 1, 0));
      std_config_.set_field_of_view(90.0);
      std_config_.set_aspect_ratio(4, 3);  // 4:3 aspect ratio
      std_config_.set_image_width(800);    // Height will be 600 (800*3/4)
      std_config_.set_ray_rng_seed(12'345);
      rng_.seed(12'345);  // Mismo seed para reproducibilidad
    }

    config std_config_;
    std::mt19937_64 rng_;  // RNG para pruebas
  };

  // ========== PRUEBAS BÁSICAS ==========

  TEST_F(CameraTest, ConstructorBasic) {
    EXPECT_NO_THROW(camera cam(std_config_));
  }

  TEST_F(CameraTest, GettersWork) {
    camera cam(std_config_);

    EXPECT_EQ(cam.get_camera_position(), vector(0, 0, -10));
    EXPECT_EQ(cam.get_camera_target(), vector(0, 0, 0));
    EXPECT_DOUBLE_EQ(cam.get_field_of_view(), 90.0);
    EXPECT_EQ(cam.get_image_width(), 800);
    EXPECT_EQ(cam.get_image_height(), 600);
  }

  TEST_F(CameraTest, ProjectionWindowCalculated) {
    camera cam(std_config_);

    // Verificar que se calcularon valores sensatos
    vector origin  = cam.get_origin();
    vector delta_x = cam.get_delta_x();
    vector delta_y = cam.get_delta_y();

    // Origin no debe ser cero (se desplaza desde la cámara)
    EXPECT_NE(origin.magnitude(), 0.0);

    // Deltas deben tener magnitud razonable
    EXPECT_GT(delta_x.magnitude(), 0.0);
    EXPECT_GT(delta_y.magnitude(), 0.0);
  }

  // ========== PRUEBAS DE GENERACIÓN DE RAYOS ==========

  TEST_F(CameraTest, GetRayBasic) {
    camera cam(std_config_);

    ray r = cam.get_ray(0, 0, rng_);  // Esquina superior izquierda

    // El rayo debe originarse en la posición de la cámara
    EXPECT_EQ(r.origin(), vector(0, 0, -10));

    // La dirección debe apuntar hacia adelante (+Z) aproximadamente
    vector dir = r.direction();
    EXPECT_GT(dir.get_z(), 0.0);  // Hacia +Z
  }

  TEST_F(CameraTest, GetRayCenter) {
    camera cam(std_config_);

    // Rayo hacia el centro de la imagen
    ray r = cam.get_ray(300, 400, rng_);  // Centro aproximado de 600x800

    EXPECT_EQ(r.origin(), vector(0, 0, -10));

    // El rayo del centro debería apuntar aproximadamente hacia el target (0,0,0)
    vector dir          = r.direction().normalize();
    vector expected_dir = (vector(0, 0, 0) - vector(0, 0, -10)).normalize();

    // Debería ser aproximadamente paralelo (permitir algo de error por anti-aliasing)
    double dot_product = dir.dot(expected_dir);
    EXPECT_GT(dot_product, 0.9);  // Casi paralelos
  }

  TEST_F(CameraTest, GetRayDifferentPositions) {
    camera cam(std_config_);

    ray r1 = cam.get_ray(0, 0, rng_);      // Esquina superior izquierda
    ray r2 = cam.get_ray(0, 799, rng_);    // Esquina superior derecha
    ray r3 = cam.get_ray(599, 0, rng_);    // Esquina inferior izquierda
    ray r4 = cam.get_ray(599, 799, rng_);  // Esquina inferior derecha

    // Todos deben tener el mismo origen
    EXPECT_EQ(r1.origin(), r2.origin());
    EXPECT_EQ(r2.origin(), r3.origin());
    EXPECT_EQ(r3.origin(), r4.origin());

    // Pero direcciones diferentes
    EXPECT_NE(r1.direction().get_x(), r2.direction().get_x());
    EXPECT_NE(r1.direction().get_y(), r3.direction().get_y());
  }

  // ========== CASOS EXTREMOS ==========

  TEST_F(CameraTest, NarrowFieldOfView) {
    config narrow_config = std_config_;
    narrow_config.set_field_of_view(1.0);  // FOV muy estrecho

    EXPECT_NO_THROW(camera cam(narrow_config));

    camera cam(narrow_config);
    ray r = cam.get_ray(300, 400, rng_);

    // Con FOV estrecho, los rayos deben ser muy paralelos
    EXPECT_GT(r.direction().magnitude(), 0.0);
  }

  TEST_F(CameraTest, WideFieldOfView) {
    config wide_config = std_config_;
    wide_config.set_field_of_view(179.0);  // FOV muy amplio

    EXPECT_NO_THROW(camera cam(wide_config));

    camera cam(wide_config);
    ray r = cam.get_ray(300, 400, rng_);

    EXPECT_GT(r.direction().magnitude(), 0.0);
  }

  TEST_F(CameraTest, VeryCloseCamera) {
    config close_config = std_config_;
    close_config.set_camera_position(vector(0, 0, -0.1));  // Muy cerca del target

    EXPECT_NO_THROW(camera cam(close_config));
  }

  TEST_F(CameraTest, VeryFarCamera) {
    config far_config = std_config_;
    far_config.set_camera_position(vector(0, 0, -1'000));  // Muy lejos del target

    EXPECT_NO_THROW(camera cam(far_config));
  }

  TEST_F(CameraTest, DiagonalOrientation) {
    config diag_config = std_config_;
    diag_config.set_camera_position(vector(10, 10, 10));
    diag_config.set_camera_target(vector(-5, -5, -5));
    diag_config.set_camera_north(vector(0, 1, 0));

    EXPECT_NO_THROW(camera cam(diag_config));

    camera cam(diag_config);
    ray r = cam.get_ray(300, 400, rng_);

    // El rayo debe apuntar hacia el target aproximadamente
    vector to_target = diag_config.get_camera_target() - diag_config.get_camera_position();
    vector ray_dir   = r.direction();

    // Direcciones deben ser aproximadamente paralelas
    double dot = ray_dir.normalize().dot(to_target.normalize());
    EXPECT_GT(dot, 0.8);
  }

  TEST_F(CameraTest, NonSquareAspectRatio) {
    config rect_config = std_config_;
    rect_config.set_aspect_ratio(16, 9);  // Widescreen aspect ratio
    rect_config.set_image_width(1'920);   // Height will be 1080 (1920*9/16)

    EXPECT_NO_THROW(camera cam(rect_config));

    camera cam(rect_config);

    // Verificar que los deltas mantienen la proporción correcta
    vector delta_x = cam.get_delta_x();
    vector delta_y = cam.get_delta_y();

    EXPECT_GT(delta_x.magnitude(), 0.0);
    EXPECT_GT(delta_y.magnitude(), 0.0);
  }

  TEST_F(CameraTest, DifferentNorthVector) {
    config north_config = std_config_;
    north_config.set_camera_north(vector(1, 0, 0));  // Norte hacia +X

    EXPECT_NO_THROW(camera cam(north_config));

    camera cam(north_config);
    ray r = cam.get_ray(0, 0, rng_);

    EXPECT_GT(r.direction().magnitude(), 0.0);
  }

  // ========== PRUEBAS DE ANTI-ALIASING ==========

  TEST_F(CameraTest, AntiAliasingRandomness) {
    camera cam(std_config_);

    // Generar múltiples rayos para el mismo píxel
    ray r1 = cam.get_ray(100, 100, rng_);
    ray r2 = cam.get_ray(100, 100, rng_);
    ray r3 = cam.get_ray(100, 100, rng_);

    // Los rayos deben tener direcciones ligeramente diferentes (anti-aliasing)
    // Nota: Hay una pequeña probabilidad de que sean iguales, pero es muy baja
    bool different_directions = (r1.direction() != r2.direction()) or
                                (r2.direction() != r3.direction()) or
                                (r1.direction() != r3.direction());

    EXPECT_TRUE(different_directions);
  }

  // ========== PRUEBAS DE CONSISTENCIA MATEMÁTICA ==========

  TEST_F(CameraTest, RayDirectionConsistency) {
    camera cam(std_config_);

    // Los rayos de píxeles adyacentes deben tener direcciones similares
    ray r1 = cam.get_ray(300, 400, rng_);
    ray r2 = cam.get_ray(300, 401, rng_);
    ray r3 = cam.get_ray(301, 400, rng_);

    vector dir1 = r1.direction().normalize();
    vector dir2 = r2.direction().normalize();
    vector dir3 = r3.direction().normalize();

    // Direcciones deben ser muy similares (píxeles adyacentes)
    EXPECT_GT(dir1.dot(dir2), 0.99);
    EXPECT_GT(dir1.dot(dir3), 0.99);
  }

  TEST_F(CameraTest, FieldOfViewCorrectness) {
    camera cam(std_config_);

    // Rayos de esquinas opuestas deben formar aproximadamente el ángulo FOV
    ray corner1 = cam.get_ray(0, 0, rng_);
    ray corner2 = cam.get_ray(0, 799, rng_);

    vector dir1 = corner1.direction().normalize();
    vector dir2 = corner2.direction().normalize();

    double angle          = std::acos(dir1.dot(dir2));
    double expected_angle = std_config_.get_field_of_view() * std::numbers::pi / 180.0;

    // El ángulo debe ser aproximadamente el FOV (considerando aspect ratio)
    // Para FOV horizontal, el ángulo real será menor debido al aspect ratio
    EXPECT_LT(angle, expected_angle + 0.1);  // Algo de tolerancia
    EXPECT_GT(angle, expected_angle * 0.5);  // No demasiado pequeño
  }

  // ========== PRUEBAS DE DATOS INVÁLIDOS ==========

  TEST_F(CameraTest, CameraPositionEqualTarget) {
    config invalid_config = std_config_;
    invalid_config.set_camera_position(vector(5, 5, 5));
    invalid_config.set_camera_target(vector(5, 5, 5));  // Mismo punto

    // Vector focal = (0,0,0), distancia focal = 0
    EXPECT_NO_THROW(camera cam(invalid_config));
  }

  TEST_F(CameraTest, MathematicalCorrectness) {
    camera cam(std_config_);

    // El rayo del centro (300,400) debería apuntar aproximadamente hacia el target
    ray center_ray = cam.get_ray(300, 400, rng_);
    vector ray_dir = center_ray.direction().normalize();
    vector expected_dir =
        (std_config_.get_camera_target() - std_config_.get_camera_position()).normalize();

    // Producto escalar debe ser > 0.9 (muy similar)
    double similarity = ray_dir.dot(expected_dir);
    EXPECT_GT(similarity, 0.9);
  }

  TEST_F(CameraTest, SetupProjectionWindowIdempotent) {
    camera cam1(std_config_);
    camera cam2(std_config_);

    // Dos cámaras con la misma config deben ser idénticas
    EXPECT_EQ(cam1.get_origin(), cam2.get_origin());
    EXPECT_EQ(cam1.get_delta_x(), cam2.get_delta_x());
    EXPECT_EQ(cam1.get_delta_y(), cam2.get_delta_y());
  }

  TEST_F(CameraTest, RayOriginConsistency) {
    camera cam(std_config_);

    // Todos los rayos deben originarse en la posición de la cámara
    for (int i = 0; i < 10; ++i) {
      ray r = cam.get_ray(i * 60, i * 80, rng_);
      EXPECT_EQ(r.origin(), std_config_.get_camera_position());
    }
  }

  TEST_F(CameraTest, ZeroNorthVector) {
    config invalid_config = std_config_;
    invalid_config.set_camera_north(vector(0, 0, 0));  // Vector norte nulo

    // Esto causará problemas en el producto vectorial
    EXPECT_NO_THROW(camera cam(invalid_config));
  }

  TEST_F(CameraTest, NorthParallelToViewDirection) {
    config invalid_config = std_config_;
    invalid_config.set_camera_position(vector(0, 0, -10));
    invalid_config.set_camera_target(vector(0, 0, 0));
    invalid_config.set_camera_north(vector(0, 0, 1));  // Norte paralelo a dirección de vista

    // Producto vectorial n⃗ × vf_hat = 0
    EXPECT_NO_THROW(camera cam(invalid_config));
  }

  TEST_F(CameraTest, GetRayOutOfBounds) {
    camera cam(std_config_);

    // Píxeles fuera de los límites de la imagen - verificamos que no crashea
    ray r1, r2, r3, r4, r5;
    EXPECT_NO_THROW(r1 = cam.get_ray(-1, 0, rng_));       // Fila negativa
    EXPECT_NO_THROW(r2 = cam.get_ray(0, -1, rng_));       // Columna negativa
    EXPECT_NO_THROW(r3 = cam.get_ray(1'000, 0, rng_));    // Fila muy grande
    EXPECT_NO_THROW(r4 = cam.get_ray(0, 1'000, rng_));    // Columna muy grande
    EXPECT_NO_THROW(r5 = cam.get_ray(-100, -100, rng_));  // Ambos negativos

    // Los rayos deben ser válidos (tener magnitud > 0)
    EXPECT_GT(r1.direction().magnitude(), 0.0);
    EXPECT_GT(r2.direction().magnitude(), 0.0);
    EXPECT_GT(r3.direction().magnitude(), 0.0);
    EXPECT_GT(r4.direction().magnitude(), 0.0);
    EXPECT_GT(r5.direction().magnitude(), 0.0);
  }

  TEST_F(CameraTest, ExtremeCoordinates) {
    config extreme_config = std_config_;
    extreme_config.set_camera_position(vector(1e10, 1e10, 1e10));  // Números muy grandes
    extreme_config.set_camera_target(vector(-1e10, -1e10, -1e10));

    EXPECT_NO_THROW(camera cam(extreme_config));
  }

  TEST_F(CameraTest, VerySmallNumbers) {
    config tiny_config = std_config_;
    tiny_config.set_camera_position(vector(1e-10, 1e-10, 1e-10));  // Números muy pequeños
    tiny_config.set_camera_target(vector(-1e-10, -1e-10, -1e-10));

    EXPECT_NO_THROW(camera cam(tiny_config));
  }

  // ========== PRUEBAS DE PARALELIZACIÓN ==========

  TEST_F(CameraTest, MultipleRNGs_ProduceDifferentRays) {
    // Test crítico para paralelización: múltiples RNGs deben producir diferentes secuencias
    camera cam(std_config_);

    std::mt19937_64 rng1(12'345);
    std::mt19937_64 rng2(67'890);
    std::mt19937_64 rng3(11'111);

    // Generar rayos con diferentes RNGs para el mismo píxel
    ray r1 = cam.get_ray(100, 100, rng1);
    ray r2 = cam.get_ray(100, 100, rng2);
    ray r3 = cam.get_ray(100, 100, rng3);

    // Cada RNG debe producir una secuencia diferente (diferentes direcciones debido a
    // anti-aliasing)
    EXPECT_NE(r1.direction(), r2.direction());
    EXPECT_NE(r2.direction(), r3.direction());
    EXPECT_NE(r1.direction(), r3.direction());
  }

  TEST_F(CameraTest, SameRNG_ProducesConsistentSequence) {
    // Verificar que el mismo RNG con la misma semilla produce resultados reproducibles
    camera cam(std_config_);

    std::mt19937_64 rng1(42);
    std::mt19937_64 rng2(42);  // Misma semilla

    ray r1_a = cam.get_ray(50, 50, rng1);
    ray r1_b = cam.get_ray(50, 50, rng1);

    ray r2_a = cam.get_ray(50, 50, rng2);
    ray r2_b = cam.get_ray(50, 50, rng2);

    // Misma semilla, misma secuencia
    EXPECT_EQ(r1_a.direction(), r2_a.direction());
    EXPECT_EQ(r1_b.direction(), r2_b.direction());
    // Pero diferentes llamadas producen diferentes resultados
    EXPECT_NE(r1_a.direction(), r1_b.direction());
  }

}  // namespace render
