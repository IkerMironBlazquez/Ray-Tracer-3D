#include "image_par.hpp"
#include <gtest/gtest.h>

#include <cstddef>  // Para size_t
#include <cstdint>  // Para std::uint8_t
#include <filesystem>
#include <fstream>
#include <string>

using namespace render;

class ImagePARTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Limpiar archivos de test previos
    std::filesystem::remove("test_output.ppm");
  }

  void TearDown() override {
    // Limpiar archivos de test
    std::filesystem::remove("test_output.ppm");
  }
};

// ============================================================================
// TESTS DE CONSTRUCCIÓN Y DIMENSIONES
// ============================================================================

TEST_F(ImagePARTest, ConstructorValidDimensions) {
  image_par img(100, 50);
  auto [width, height] = img.get_dimensions();

  EXPECT_EQ(width, 100);
  EXPECT_EQ(height, 50);
}

TEST_F(ImagePARTest, ConstructorInvalidDimensions) {
  EXPECT_THROW(image_par(-1, 10), std::invalid_argument);
  EXPECT_THROW(image_par(10, -1), std::invalid_argument);
  EXPECT_THROW(image_par(0, 10), std::invalid_argument);
  EXPECT_THROW(image_par(10, 0), std::invalid_argument);
}

TEST_F(ImagePARTest, DefaultPixelValues) {
  image_par img(2, 2);

  // Verificar que los píxeles iniciales son negro (0,0,0)
  auto const & red_channel   = img.get_r_channel();
  auto const & green_channel = img.get_g_channel();
  auto const & blue_channel  = img.get_b_channel();

  EXPECT_EQ(red_channel.size(), 4);  // 2x2 = 4 píxeles
  EXPECT_EQ(green_channel.size(), 4);
  EXPECT_EQ(blue_channel.size(), 4);

  for (size_t i = 0; i < 4; ++i) {
    EXPECT_EQ(red_channel[i], 0);
    EXPECT_EQ(green_channel[i], 0);
    EXPECT_EQ(blue_channel[i], 0);
  }
}

// ============================================================================
// TESTS DE SET_PIXEL Y ESTRUCTURA PAR
// ============================================================================

TEST_F(ImagePARTest, SetPixelValidCoordinates) {
  image_par img(3, 2);  // 3 ancho x 2 alto

  // Establecer píxel en (1, 2) con color (255, 128, 64)
  EXPECT_NO_THROW(img.set_pixel(1, 2, 255, 128, 64));

  // Verificar que se almacenó correctamente en los vectores PAR
  auto const & red_channel   = img.get_r_channel();
  auto const & green_channel = img.get_g_channel();
  auto const & blue_channel  = img.get_b_channel();

  // Índice = row * width + col = 1 * 3 + 2 = 5
  size_t index = 1 * 3 + 2;
  EXPECT_EQ(red_channel[index], 255);
  EXPECT_EQ(green_channel[index], 128);
  EXPECT_EQ(blue_channel[index], 64);
}

TEST_F(ImagePARTest, SetPixelInvalidCoordinates) {
  image_par img(10, 10);

  EXPECT_THROW(img.set_pixel(-1, 5, 255, 255, 255), std::out_of_range);
  EXPECT_THROW(img.set_pixel(5, -1, 255, 255, 255), std::out_of_range);
  EXPECT_THROW(img.set_pixel(10, 5, 255, 255, 255), std::out_of_range);  // >= height
  EXPECT_THROW(img.set_pixel(5, 10, 255, 255, 255), std::out_of_range);  // >= width
}

TEST_F(ImagePARTest, SetMultiplePixelsPARStructure) {
  image_par img(2, 2);  // 2x2 = 4 píxeles

  // Establecer cada píxel con un color diferente
  img.set_pixel(0, 0, 255, 0, 0);      // Rojo
  img.set_pixel(0, 1, 0, 255, 0);      // Verde
  img.set_pixel(1, 0, 0, 0, 255);      // Azul
  img.set_pixel(1, 1, 255, 255, 255);  // Blanco

  // Verificar estructura PAR: canales separados
  auto const & red_channel   = img.get_r_channel();
  auto const & green_channel = img.get_g_channel();
  auto const & blue_channel  = img.get_b_channel();

  // Verificar que el canal rojo contiene: [255, 0, 0, 255]
  EXPECT_EQ(red_channel[0], 255);  // (0,0)
  EXPECT_EQ(red_channel[1], 0);    // (0,1)
  EXPECT_EQ(red_channel[2], 0);    // (1,0)
  EXPECT_EQ(red_channel[3], 255);  // (1,1)

  // Verificar que el canal verde contiene: [0, 255, 0, 255]
  EXPECT_EQ(green_channel[0], 0);    // (0,0)
  EXPECT_EQ(green_channel[1], 255);  // (0,1)
  EXPECT_EQ(green_channel[2], 0);    // (1,0)
  EXPECT_EQ(green_channel[3], 255);  // (1,1)

  // Verificar que el canal azul contiene: [0, 0, 255, 255]
  EXPECT_EQ(blue_channel[0], 0);    // (0,0)
  EXPECT_EQ(blue_channel[1], 0);    // (0,1)
  EXPECT_EQ(blue_channel[2], 255);  // (1,0)
  EXPECT_EQ(blue_channel[3], 255);  // (1,1)
}

// ============================================================================
// TESTS DE ESCRITURA PPM
// ============================================================================

TEST_F(ImagePARTest, WritePPMBasic) {
  image_par img(2, 1);  // 2 ancho x 1 alto

  // Establecer píxeles: rojo y verde
  img.set_pixel(0, 0, 255, 0, 0);
  img.set_pixel(0, 1, 0, 255, 0);

  // Escribir archivo PPM
  EXPECT_TRUE(img.write_ppm("test_output.ppm"));

  // Verificar que el archivo existe y tiene contenido correcto
  EXPECT_TRUE(std::filesystem::exists("test_output.ppm"));

  std::ifstream file("test_output.ppm");
  ASSERT_TRUE(file.is_open());

  std::string line;
  std::getline(file, line);
  EXPECT_EQ(line, "P3");  // Header PPM

  std::getline(file, line);
  EXPECT_EQ(line, "2 1");  // width height (PPM format correcto)

  std::getline(file, line);
  EXPECT_EQ(line, "255");  // Max color value

  // Primer píxel: rojo (255, 0, 0)
  std::getline(file, line);
  EXPECT_EQ(line, "255 0 0");

  // Segundo píxel: verde (0, 255, 0)
  std::getline(file, line);
  EXPECT_EQ(line, "0 255 0");
}

TEST_F(ImagePARTest, WritePPMInvalidFile) {
  image_par img(1, 1);

  // Intentar escribir en un directorio que no existe
  EXPECT_FALSE(img.write_ppm("/nonexistent/directory/test.ppm"));
}

// ============================================================================
// TESTS DE RENDIMIENTO PAR
// ============================================================================

TEST_F(ImagePARTest, PARChannelAccess) {
  constexpr int SIZE = 100;
  image_par img(SIZE, SIZE);

  // Llenar toda la imagen con gradiente
  for (int r = 0; r < SIZE; ++r) {
    for (int c = 0; c < SIZE; ++c) {
      auto const value = static_cast<std::uint8_t>((r + c) % 256);
      img.set_pixel(r, c, value, value, value);
    }
  }

  // Verificar acceso eficiente a canales PAR
  auto const & red_channel   = img.get_r_channel();
  auto const & green_channel = img.get_g_channel();
  auto const & blue_channel  = img.get_b_channel();

  EXPECT_EQ(red_channel.size(), SIZE * SIZE);
  EXPECT_EQ(green_channel.size(), SIZE * SIZE);
  EXPECT_EQ(blue_channel.size(), SIZE * SIZE);

  // Verificar algunos valores específicos
  EXPECT_EQ(red_channel[0], 0);                                      // (0,0) -> 0
  EXPECT_EQ(green_channel[99], 99);                                  // (0,99) -> 99
  EXPECT_EQ(blue_channel[SIZE * SIZE - 1], (2 * (SIZE - 1)) % 256);  // (99,99)
}

// ============================================================================
// TESTS DE COMPATIBILIDAD CON ENUNCIADO
// ============================================================================

TEST_F(ImagePARTest, EnunciadoPARRequirements) {
  // Según el enunciado: "Se representarán los píxeles de una imagen como tres secuencias
  // independientes. Cada una de las secuencias contendrá elementos que deberán estar
  // en el rango de 0 a 255."

  image_par img(10, 10);

  // Verificar que tenemos tres vectores independientes
  auto const & red_channel   = img.get_r_channel();
  auto const & green_channel = img.get_g_channel();
  auto const & blue_channel  = img.get_b_channel();

  // Verificar que son vectores separados (diferentes direcciones de memoria)
  EXPECT_NE(&red_channel, &green_channel);
  EXPECT_NE(&green_channel, &blue_channel);
  EXPECT_NE(&red_channel, &blue_channel);

  // Verificar que todos los elementos están en rango [0-255]
  img.set_pixel(5, 5, 0, 128, 255);  // Valores límite

  EXPECT_GE(red_channel[5 * 10 + 5], 0);
  EXPECT_LE(red_channel[5 * 10 + 5], 255);
  EXPECT_GE(green_channel[5 * 10 + 5], 0);
  EXPECT_LE(green_channel[5 * 10 + 5], 255);
  EXPECT_GE(blue_channel[5 * 10 + 5], 0);
  EXPECT_LE(blue_channel[5 * 10 + 5], 255);
}

// ============================================================================
// TESTS DE PARALELIZACIÓN Y CONCURRENCIA
// ============================================================================

TEST_F(ImagePARTest, LargeImage_MemoryLayout) {
  // Test de imagen grande para verificar layout de memoria PAR
  constexpr int LARGE_SIZE = 1'000;
  image_par img(LARGE_SIZE, LARGE_SIZE);

  auto [width, height] = img.get_dimensions();
  EXPECT_EQ(width, LARGE_SIZE);
  EXPECT_EQ(height, LARGE_SIZE);

  // Verificar que los canales tienen el tamaño correcto
  auto const & r_chan = img.get_r_channel();
  auto const & g_chan = img.get_g_channel();
  auto const & b_chan = img.get_b_channel();

  size_t const expected_size = static_cast<size_t>(LARGE_SIZE) * static_cast<size_t>(LARGE_SIZE);
  EXPECT_EQ(r_chan.size(), expected_size);
  EXPECT_EQ(g_chan.size(), expected_size);
  EXPECT_EQ(b_chan.size(), expected_size);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_F(ImagePARTest, SimulatedConcurrentWrites_DifferentPixels) {
  // Simular escrituras concurrentes en diferentes píxeles (sin race condition)
  image_par img(100, 100);

  // Simular que diferentes "hilos" escriben en diferentes regiones
  // Thread 1: cuadrante superior izquierdo
  for (int r = 0; r < 50; ++r) {
    for (int c = 0; c < 50; ++c) {
      img.set_pixel(r, c, 255, 0, 0);  // Rojo
    }
  }

  // Thread 2: cuadrante superior derecho
  for (int r = 0; r < 50; ++r) {
    for (int c = 50; c < 100; ++c) {
      img.set_pixel(r, c, 0, 255, 0);  // Verde
    }
  }

  // Thread 3: cuadrante inferior izquierdo
  for (int r = 50; r < 100; ++r) {
    for (int c = 0; c < 50; ++c) {
      img.set_pixel(r, c, 0, 0, 255);  // Azul
    }
  }

  // Thread 4: cuadrante inferior derecho
  for (int r = 50; r < 100; ++r) {
    for (int c = 50; c < 100; ++c) {
      img.set_pixel(r, c, 255, 255, 0);  // Amarillo
    }
  }

  // Verificar que todos los cuadrantes tienen los colores correctos
  auto const & r_chan = img.get_r_channel();
  auto const & g_chan = img.get_g_channel();
  auto const & b_chan = img.get_b_channel();

  // Verificar esquinas
  EXPECT_EQ(r_chan[0], 255);  // (0,0) rojo
  EXPECT_EQ(g_chan[0], 0);
  EXPECT_EQ(b_chan[0], 0);

  EXPECT_EQ(r_chan[99], 0);  // (0,99) verde - último de la primera fila
  EXPECT_EQ(g_chan[99], 255);
  EXPECT_EQ(b_chan[99], 0);

  EXPECT_EQ(r_chan[5'000], 0);  // (50,0) azul
  EXPECT_EQ(g_chan[5'000], 0);
  EXPECT_EQ(b_chan[5'000], 255);

  EXPECT_EQ(r_chan[9'999], 255);  // (99,99) amarillo
  EXPECT_EQ(g_chan[9'999], 255);
  EXPECT_EQ(b_chan[9'999], 0);
}

TEST_F(ImagePARTest, PARStructure_ContiguousMemory) {
  // Verificar que cada canal es contiguo en memoria (importante para paralelización)
  image_par img(10, 10);

  auto const & r_chan = img.get_r_channel();
  auto const & g_chan = img.get_g_channel();
  auto const & b_chan = img.get_b_channel();

  // Los elementos de cada canal deben estar en memoria contigua
  // Verificamos que las direcciones son consecutivas
  for (size_t i = 1; i < r_chan.size(); ++i) {
    ptrdiff_t const diff = &r_chan[i] - &r_chan[i - 1];
    EXPECT_EQ(diff, 1);  // Elementos consecutivos
  }

  for (size_t i = 1; i < g_chan.size(); ++i) {
    ptrdiff_t const diff = &g_chan[i] - &g_chan[i - 1];
    EXPECT_EQ(diff, 1);
  }

  for (size_t i = 1; i < b_chan.size(); ++i) {
    ptrdiff_t const diff = &b_chan[i] - &b_chan[i - 1];
    EXPECT_EQ(diff, 1);
  }
}

TEST_F(ImagePARTest, EdgePixels_CorrectIndexing) {
  // Test de indexación en bordes (crítico para blocked_range2d)
  image_par img(100, 100);

  // Establecer píxeles en los bordes
  img.set_pixel(0, 0, 1, 1, 1);    // Esquina superior izquierda
  img.set_pixel(0, 99, 2, 2, 2);   // Esquina superior derecha
  img.set_pixel(99, 0, 3, 3, 3);   // Esquina inferior izquierda
  img.set_pixel(99, 99, 4, 4, 4);  // Esquina inferior derecha
  img.set_pixel(50, 0, 5, 5, 5);   // Borde izquierdo medio
  img.set_pixel(50, 99, 6, 6, 6);  // Borde derecho medio
  img.set_pixel(0, 50, 7, 7, 7);   // Borde superior medio
  img.set_pixel(99, 50, 8, 8, 8);  // Borde inferior medio

  auto const & r_chan = img.get_r_channel();

  // Verificar índices calculados correctamente
  EXPECT_EQ(r_chan[0], 1);      // (0,0) -> índice 0
  EXPECT_EQ(r_chan[99], 2);     // (0,99) -> índice 99
  EXPECT_EQ(r_chan[9'900], 3);  // (99,0) -> índice 9900
  EXPECT_EQ(r_chan[9'999], 4);  // (99,99) -> índice 9999
  EXPECT_EQ(r_chan[5'000], 5);  // (50,0) -> índice 5000
  EXPECT_EQ(r_chan[5'099], 6);  // (50,99) -> índice 5099
  EXPECT_EQ(r_chan[50], 7);     // (0,50) -> índice 50
  EXPECT_EQ(r_chan[9'950], 8);  // (99,50) -> índice 9950
}

TEST_F(ImagePARTest, AllPixelValues_0to255) {
  // Verificar que todos los valores [0-255] se almacenan correctamente
  image_par img(256, 1);  // 256 píxeles en una fila

  for (int i = 0; i < 256; ++i) {
    auto const val = static_cast<std::uint8_t>(i);
    img.set_pixel(0, i, val, val, val);
  }

  auto const & r_chan = img.get_r_channel();
  auto const & g_chan = img.get_g_channel();
  auto const & b_chan = img.get_b_channel();

  // Verificar que todos los valores se almacenaron correctamente
  for (int i = 0; i < 256; ++i) {
    EXPECT_EQ(r_chan[static_cast<size_t>(i)], static_cast<std::uint8_t>(i));
    EXPECT_EQ(g_chan[static_cast<size_t>(i)], static_cast<std::uint8_t>(i));
    EXPECT_EQ(b_chan[static_cast<size_t>(i)], static_cast<std::uint8_t>(i));
  }
}
