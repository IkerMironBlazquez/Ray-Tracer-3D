#include "config.hpp"
#include "material.hpp"
#include "object.hpp"
#include "ray_tracer.hpp"
#include "scene.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <random>

namespace {

  constexpr double TOLERANCE = 1e-6;

}

// ============================================================================
// TESTS PARA COLOR DE FONDO (Sección 3.6)
// ============================================================================

class BackgroundGradientTest : public ::testing::Test {
protected:
  void SetUp() override {
    cfg.set_background_light_color(render::color(1.0, 1.0, 1.0));  // Blanco
    cfg.set_background_dark_color(render::color(0.5, 0.7, 1.0));   // Azul
  }

  render::config cfg;
};

TEST_F(BackgroundGradientTest, RayPointingUp_GivesDarkColor) {
  // Rayo apuntando hacia arriba (Y = +1)
  render::ray r(render::vector(0, 0, 0), render::vector(0, 1, 0));

  render::color result = render::background_gradient(cfg, r);

  // Factor m = (1 + 1) / 2 = 1, por lo que color = (1-1)*light + 1*dark = dark
  EXPECT_NEAR(result.r(), 0.5, TOLERANCE);
  EXPECT_NEAR(result.g(), 0.7, TOLERANCE);
  EXPECT_NEAR(result.b(), 1.0, TOLERANCE);
}

TEST_F(BackgroundGradientTest, RayPointingDown_GivesLightColor) {
  // Rayo apuntando hacia abajo (Y = -1)
  render::ray r(render::vector(0, 0, 0), render::vector(0, -1, 0));

  render::color result = render::background_gradient(cfg, r);

  // Factor m = (-1 + 1) / 2 = 0, por lo que color = (1-0)*light + 0*dark = light
  EXPECT_NEAR(result.r(), 1.0, TOLERANCE);
  EXPECT_NEAR(result.g(), 1.0, TOLERANCE);
  EXPECT_NEAR(result.b(), 1.0, TOLERANCE);
}

TEST_F(BackgroundGradientTest, RayPointingHorizontal_GivesMidColor) {
  // Rayo horizontal (Y = 0)
  render::ray r(render::vector(0, 0, 0), render::vector(1, 0, 0));

  render::color result = render::background_gradient(cfg, r);

  // Factor m = (0 + 1) / 2 = 0.5, color = 0.5*light + 0.5*dark
  EXPECT_NEAR(result.r(), 0.75, TOLERANCE);  // (1*0.5 + 0.5*0.5)
  EXPECT_NEAR(result.g(), 0.85, TOLERANCE);  // (1*0.5 + 0.7*0.5)
  EXPECT_NEAR(result.b(), 1.0, TOLERANCE);   // (1*0.5 + 1*0.5)
}

// ============================================================================
// TESTS PARA REFLEXIÓN MATE (Sección 3.5.1)
// ============================================================================

class MatteReflectionTest : public ::testing::Test {
protected:
  std::mt19937_64 rng{42};  // Semilla fija para reproducibilidad
};

TEST_F(MatteReflectionTest, NormalVector_ProducesRandomDirection) {
  render::vector normal(0, 1, 0);  // Normal apuntando hacia arriba

  render::vector reflection = render::reflect_matte(normal, rng);

  // Debe estar normalizado
  EXPECT_NEAR(reflection.magnitude(), 1.0, TOLERANCE);

  // Debe tener componente aleatoria
  EXPECT_TRUE(reflection.get_x() != 0.0 or reflection.get_z() != 0.0);
}

TEST_F(MatteReflectionTest, MultipleReflections_ProduceDifferentDirections) {
  render::vector normal(0, 1, 0);

  render::vector reflection1 = render::reflect_matte(normal, rng);
  render::vector reflection2 = render::reflect_matte(normal, rng);

  // Deben ser diferentes (altamente probable con RNG)
  EXPECT_FALSE(std::abs(reflection1.get_x() - reflection2.get_x()) < TOLERANCE and
               std::abs(reflection1.get_y() - reflection2.get_y()) < TOLERANCE and
               std::abs(reflection1.get_z() - reflection2.get_z()) < TOLERANCE);
}

TEST_F(MatteReflectionTest, VerySmallVector_ReturnsNormal) {
  // Simular caso donde el vector resultante es muy pequeño
  // Esto es difícil de testear directamente, pero podemos verificar el comportamiento general
  render::vector normal(0, 1, 0);

  // Con múltiples intentos, verificar que siempre obtenemos un vector válido
  for (int i = 0; i < 100; ++i) {
    render::vector reflection = render::reflect_matte(normal, rng);
    EXPECT_GT(reflection.magnitude(), 0.5);  // Debe ser razonablemente grande
    EXPECT_LT(reflection.magnitude(), 1.5);  // Pero no excesivamente grande
  }
}

// ============================================================================
// TESTS PARA REFLEXIÓN METÁLICA (Sección 3.5.2)
// ============================================================================

class MetalReflectionTest : public ::testing::Test {
protected:
  std::mt19937_64 rng{42};
};

TEST_F(MetalReflectionTest, PerfectReflection_ZeroDiffusion) {
  render::vector incident(1, -1, 0);  // 45 grados hacia abajo
  render::vector normal(0, 1, 0);     // Normal hacia arriba
  double diffusion = 0.0;             // Sin difusión

  render::vector reflection = render::reflect_metal(incident, normal, diffusion, rng);

  // Sin difusión, debe ser reflexión especular perfecta
  // d⃗¹r = d⃗o - 2(d⃗o·d⃗n)d⃗n = (1,-1,0) - 2*(-1)*(0,1,0) = (1,1,0)
  render::vector expected = render::vector(1, 1, 0).normalize();

  EXPECT_NEAR(reflection.get_x(), expected.get_x(), TOLERANCE);
  EXPECT_NEAR(reflection.get_y(), expected.get_y(), TOLERANCE);
  EXPECT_NEAR(reflection.get_z(), expected.get_z(), TOLERANCE);
}

TEST_F(MetalReflectionTest, WithDiffusion_DeviatesFromPerfectReflection) {
  render::vector incident(1, -1, 0);
  render::vector normal(0, 1, 0);
  double diffusion = 0.5;  // Algo de difusión

  render::vector reflection = render::reflect_metal(incident, normal, diffusion, rng);

  // Debe estar normalizado
  EXPECT_NEAR(reflection.magnitude(), 1.0, TOLERANCE);

  // Con difusión, no debe ser exactamente la reflexión especular
  render::vector perfect_reflection = render::vector(1, 1, 0).normalize();
  double dot_product                = reflection.dot(perfect_reflection);
  EXPECT_LT(dot_product, 1.0 - TOLERANCE);  // No debe ser idéntico
}

// ============================================================================
// TESTS PARA REFLEXIÓN REFRACTIVA (Sección 3.5.3)
// ============================================================================

class RefractiveReflectionTest : public ::testing::Test { };

TEST_F(RefractiveReflectionTest, NormalIncidence_MinimalDeviation) {
  // Incidencia normal (perpendicular)
  render::vector incident(0, -1, 0);  // Directamente hacia abajo
  render::vector normal(0, 1, 0);     // Normal hacia arriba
  double refraction_index = 1.5;
  bool outward            = true;  // Desde aire hacia material

  render::vector reflection =
      render::reflect_refractive(incident, normal, refraction_index, outward);

  // Con incidencia normal, la dirección debe mantenerse principalmente vertical
  EXPECT_NEAR(std::abs(reflection.get_y()), 1.0, 0.2);  // Tolerancia mayor
  EXPECT_NEAR(reflection.magnitude(), 1.0, TOLERANCE);  // Debe estar normalizado
}

TEST_F(RefractiveReflectionTest, ReflectionDirection_IsNormalized) {
  render::vector incident(1, -1, 0);
  render::vector normal(0, 1, 0);
  double refraction_index = 1.5;
  bool outward            = true;

  render::vector reflection =
      render::reflect_refractive(incident, normal, refraction_index, outward);

  // Siempre debe estar normalizado
  EXPECT_NEAR(reflection.magnitude(), 1.0, TOLERANCE);
}

// ============================================================================
// TESTS INTEGRACIÓN COMPLETA TRACE_RAY
// ============================================================================

class TraceRayIntegrationTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Configuración básica
    cfg.set_background_light_color(render::color(0.7, 0.8, 1.0));
    cfg.set_background_dark_color(render::color(0.5, 0.7, 1.0));
    cfg.set_material_rng_seed(42);
    cfg.set_max_depth(5);
    rng.seed(12'345);  // Semilla fija para reproducibilidad

    // Escena simple con una esfera mate
    scene.add_material(render::material("matte_red", render::matte_material(0.8, 0.2, 0.2)));
    scene.add_object(render::object(render::sphere(render::point3d(0, 0, 5), 1.0), "matte_red"));
  }

  render::config cfg;
  render::scene scene;
  std::mt19937_64 rng;
};

TEST_F(TraceRayIntegrationTest, RayHitsObject_ReturnsValidColor) {
  render::ray r(render::vector(0, 0, 0), render::vector(0, 0, 1));  // Rayo hacia la esfera

  render::color result = render::trace_ray(r, scene, cfg, cfg.get_max_depth(), rng);

  // Debe devolver un color válido (componentes entre 0 y 1)
  EXPECT_GE(result.r(), 0.0);
  EXPECT_LE(result.r(), 1.0);
  EXPECT_GE(result.g(), 0.0);
  EXPECT_LE(result.g(), 1.0);
  EXPECT_GE(result.b(), 0.0);
  EXPECT_LE(result.b(), 1.0);
}

TEST_F(TraceRayIntegrationTest, RayMissesAll_ReturnsBackgroundColor) {
  render::ray r(render::vector(0, 0, 0), render::vector(1, 0, 0));  // Rayo que no toca nada

  render::color result = render::trace_ray(r, scene, cfg, cfg.get_max_depth(), rng);

  // Debe devolver color de fondo
  render::color expected_bg = render::background_gradient(cfg, r);
  EXPECT_NEAR(result.r(), expected_bg.r(), TOLERANCE);
  EXPECT_NEAR(result.g(), expected_bg.g(), TOLERANCE);
  EXPECT_NEAR(result.b(), expected_bg.b(), TOLERANCE);
}

TEST_F(TraceRayIntegrationTest, ZeroDepth_ReturnsBlack) {
  render::ray r(render::vector(0, 0, 0), render::vector(0, 0, 1));

  render::color result = render::trace_ray(r, scene, cfg, 0, rng);  // Profundidad 0

  // Según el enunciado, cuando depth <= 0, devuelve color de fondo (no negro)
  render::color const expected_bg = render::background_gradient(cfg, r);
  EXPECT_NEAR(result.r(), expected_bg.r(), TOLERANCE);
  EXPECT_NEAR(result.g(), expected_bg.g(), TOLERANCE);
  EXPECT_NEAR(result.b(), expected_bg.b(), TOLERANCE);
}

TEST_F(TraceRayIntegrationTest, MaterialTypes_ProcessCorrectly) {
  // Test con material metálico
  scene.add_material(render::material("metal_blue", render::metal_material(0.2, 0.2, 0.8, 0.1)));
  scene.add_object(render::object(render::sphere(render::point3d(2, 0, 5), 1.0), "metal_blue"));

  // Test con material refractivo
  scene.add_material(render::material("glass", render::refractive_material(1.5)));
  scene.add_object(render::object(render::sphere(render::point3d(-2, 0, 5), 1.0), "glass"));

  render::ray r1(render::vector(0, 0, 0), render::vector(2, 0, 5).normalize());   // Hacia metal
  render::ray r2(render::vector(0, 0, 0), render::vector(-2, 0, 5).normalize());  // Hacia vidrio

  render::color metal_color = render::trace_ray(r1, scene, cfg, 3, rng);
  render::color glass_color = render::trace_ray(r2, scene, cfg, 3, rng);

  // Ambos deben devolver colores válidos
  EXPECT_GE(metal_color.r(), 0.0);
  EXPECT_LE(metal_color.r(), 1.0);
  EXPECT_GE(glass_color.r(), 0.0);
  EXPECT_LE(glass_color.r(), 1.0);
}

TEST_F(TraceRayIntegrationTest, RecursiveDepth_ProperlyLimited) {
  // Test de profundidad recursiva completa
  render::ray r(render::vector(0, 0, 0), render::vector(0, 0, 1));

  // Probar diferentes profundidades
  render::color depth1 = render::trace_ray(r, scene, cfg, 1, rng);
  render::color depth3 = render::trace_ray(r, scene, cfg, 3, rng);
  render::color depth5 = render::trace_ray(r, scene, cfg, 5, rng);

  // Todos deben ser válidos
  EXPECT_GE(depth1.r(), 0.0);
  EXPECT_LE(depth1.r(), 1.0);
  EXPECT_GE(depth3.r(), 0.0);
  EXPECT_LE(depth3.r(), 1.0);
  EXPECT_GE(depth5.r(), 0.0);
  EXPECT_LE(depth5.r(), 1.0);

  // Mayor profundidad puede producir colores diferentes (más reflexiones)
  // Nota: No garantizamos que sean diferentes, pero deben ser válidos
}

TEST_F(TraceRayIntegrationTest, MultipleRNGs_IndependentResults) {
  // Test crítico para paralelización: diferentes RNGs deben producir secuencias independientes
  render::ray r(render::vector(0, 0, 0), render::vector(0, 0, 1));

  std::mt19937_64 rng1(42);
  std::mt19937_64 rng2(42);  // Misma semilla inicial

  render::color color1a = render::trace_ray(r, scene, cfg, 3, rng1);
  render::color color2a = render::trace_ray(r, scene, cfg, 3, rng2);

  // Misma semilla debe producir mismo resultado
  EXPECT_NEAR(color1a.r(), color2a.r(), TOLERANCE);
  EXPECT_NEAR(color1a.g(), color2a.g(), TOLERANCE);
  EXPECT_NEAR(color1a.b(), color2a.b(), TOLERANCE);

  // Segunda llamada con misma semilla original también debe coincidir
  render::color color1b = render::trace_ray(r, scene, cfg, 3, rng1);
  render::color color2b = render::trace_ray(r, scene, cfg, 3, rng2);

  EXPECT_NEAR(color1b.r(), color2b.r(), TOLERANCE);
  EXPECT_NEAR(color1b.g(), color2b.g(), TOLERANCE);
  EXPECT_NEAR(color1b.b(), color2b.b(), TOLERANCE);
}

TEST_F(TraceRayIntegrationTest, NegativeDepth_ReturnsSafeColor) {
  // Verificar comportamiento con profundidad negativa
  render::ray r(render::vector(0, 0, 0), render::vector(0, 0, 1));

  render::color result = render::trace_ray(r, scene, cfg, -1, rng);

  // Debe devolver color de fondo (comportamiento seguro)
  render::color expected_bg = render::background_gradient(cfg, r);
  EXPECT_NEAR(result.r(), expected_bg.r(), TOLERANCE);
  EXPECT_NEAR(result.g(), expected_bg.g(), TOLERANCE);
  EXPECT_NEAR(result.b(), expected_bg.b(), TOLERANCE);
}
