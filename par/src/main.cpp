#include <algorithm>
#include <atomic>  // Para std::atomic
#include <cmath>
#include <cstdint>  // Para std::uint64_t
#include <iostream>
#include <optional>  // Para std::optional
#include <print>
#include <random>  // Para std::mt19937_64
#include <ranges>  // Para std::ranges::generate
#include <span>
#include <vector>  // Para std::vector

#include <tbb/blocked_range2d.h>             // Para tbb::blocked_range2d
#include <tbb/enumerable_thread_specific.h>  // Para tbb::enumerable_thread_specific
#include <tbb/global_control.h>
#include <tbb/parallel_for.h>  // Para tbb::parallel_for
#include <tbb/partitioner.h>   // Para los particionadores (simple, static, auto)
#include <tbb/task_arena.h>

#include "camera.hpp"
#include "config.hpp"
#include "config_parser.hpp"
#include "image_par.hpp"  // PAR: Incluir estructura PAR
#include "ray.hpp"
#include "ray_tracer.hpp"
#include "scene.hpp"
#include "scene_parser.hpp"
#include "vector.hpp"

namespace {

  // Sd para cada hilo de renderizado
  // Usamos std::mt19937_64 y std::ranges::generate
  std::vector<std::uint64_t> generate_thread_sds(std::uint64_t base_seed, std::size_t num_threads) {
    std::vector<std::uint64_t> seeds(num_threads);
    std::mt19937_64 seed_generator{base_seed};
    std::ranges::generate(seeds, seed_generator);
    return seeds;
  }

  // Verifica si el píxel ha convergido
  bool has_converged(render::color const & current, render::color const & previous,
                     double threshold) {
    double const dr = std::abs(current.r() - previous.r());
    double const dg = std::abs(current.g() - previous.g());
    double const db = std::abs(current.b() - previous.b());

    double const max_component = std::max({current.r(), current.g(), current.b()});
    if (max_component > 0.0) {
      double const relative_change = std::max({dr, dg, db}) / max_component;
      return relative_change < threshold;
    }
    return false;
  }

  // Renderiza un píxel individual con muestreo adaptativo
  // Ahora recibe el generador RNG del hilo actual
  render::color render_pixel(int row, int col, render::camera & cam, render::scene const & scn,
                             render::config const & cfg, std::mt19937_64 & rng) {
    constexpr int MIN_SAMPLES              = 4;
    constexpr double CONVERGENCE_THRESHOLD = 0.001;

    render::color accum{0.0, 0.0, 0.0};
    render::color prev_avg{0.0, 0.0, 0.0};
    int samples_used = 0;

    for (int s = 0; s < cfg.get_samples_per_pixel(); ++s) {
      render::ray ray   = cam.get_ray(row, col, rng);  // Pasar RNG respecto al hilo
      render::color col = render::trace_ray(ray, scn, cfg, cfg.get_max_depth(), rng);  // Pasar RNG
      accum += col;
      samples_used++;

      // Después de MIN_SAMPLES, verificar convergencia
      if (s >= MIN_SAMPLES - 1) {
        render::color current_avg = accum / static_cast<double>(samples_used);
        if (has_converged(current_avg, prev_avg, CONVERGENCE_THRESHOLD)) {
          break;
        }
        prev_avg = current_avg;
      }
    }

    return accum / static_cast<double>(samples_used);
  }

  // PAR: Bucle de render paralelo para píxeles en estructura PAR
  // Usa tbb::parallel_for con blocked_range2d para paralelización óptima
  // Soporta 3 estrategias de particionador configurables por compilación:
  //   - USE_SIMPLE_PARTITIONER: División recursiva hasta grain size mínimo
  //   - USE_STATIC_PARTITIONER: División estática (1 bloque por hilo)
  //   - Por defecto: auto_partitioner (Work stealing)
  // FASE 4 PASO 4: Ahora recibe grain_rows y grain_cols como parámetros configurables
  void do_render_lp_par(render::image_par & image, render::camera & cam, render::scene const & scn,
                        render::config const & cfg,
                        tbb::enumerable_thread_specific<std::mt19937_64> & thread_rngs,
                        int grain_rows, int grain_cols) {
    int const width  = cfg.get_image_width();
    int const height = cfg.get_image_height();

    // Paralelización con tbb::parallel_for y blocked_range2d
    // FASE 4 PASO 4: Tamaño de grano configurable para filas y columnas
    tbb::parallel_for(
        tbb::blocked_range2d<int>(
            0, height, static_cast<size_t>(grain_rows),  // Rango de filas con grain size
            0, width, static_cast<size_t>(grain_cols)),  // Rango de columnas con grain size
        [&](tbb::blocked_range2d<int> const & range) {
          // Obtener RNG local para el hilo actual
          auto & rng = thread_rngs.local();

          // Iterar sobre el bloque asignado a este hilo
          for (int r = range.rows().begin(); r < range.rows().end(); ++r) {
            for (int c = range.cols().begin(); c < range.cols().end(); ++c) {
              render::color color = render_pixel(r, c, cam, scn, cfg, rng);
              color               = render::a_gamma(color, cfg.get_gamma());
              color               = render::clamp_color(color);
              render::vector rgb  = render::color_to_rgb(color);

              // PAR: Almacenar píxel en la estructura PAR (3 vectores separados)
              image.set_pixel(r, c, static_cast<std::uint8_t>(rgb.get_x()),
                              static_cast<std::uint8_t>(rgb.get_y()),
                              static_cast<std::uint8_t>(rgb.get_z()));
            }
          }
        },
#if defined(USE_SIMPLE_PARTITIONER)
        tbb::simple_partitioner()  // División recursiva
#elif defined(USE_STATIC_PARTITIONER)
        tbb::static_partitioner()  // División estática
#else
        tbb::auto_partitioner()  // Balanceo dinámico (DEFAULT)
#endif
    );
  }

  // Parsea y devuelve(config, scene).
  std::pair<render::config, render::scene> parse_config_and_scene(char const * config_file,
                                                                  char const * scene_file) {
    render::config cfg = render::config_parser::parse_file(std::string(config_file));
    render::scene scn  = render::scene_parser::parse_file(std::string(scene_file));
    return {cfg, scn};
  }

  // Parámetros de la configuración
  void log_config(render::config const & cfg) {
    std::println("Configuration loaded successfully:");
    std::println("  Image width: {}", cfg.get_image_width());
    std::println("  Image height: {}", cfg.get_image_height());
    std::println("  Gamma: {}", cfg.get_gamma());
    std::println("  Camera position: ({}, {}, {})", cfg.get_camera_position().get_x(),
                 cfg.get_camera_position().get_y(), cfg.get_camera_position().get_z());
    std::println("  Field of view: {}", cfg.get_field_of_view());
    std::println("  Samples per pixel: {}", cfg.get_samples_per_pixel());
    std::println("  Max depth: {}", cfg.get_max_depth());
  }

  // PAR: Parsea, crea la cámara, renderiza a PAR, escribe PPM. (0-exito / 1-error)
  // FASE 4 PASO 4 y 5: Ahora recibe parámetros de paralelización configurables
  int render_to_file(char const * config_file, char const * scene_file, char const * out_file,
                     int num_threads, int grain_rows, int grain_cols) {
    try {
      auto [cfg, scn] = parse_config_and_scene(config_file, scene_file);

      // FASE 4 PASO 5: Implementar control de número de hilos
      // Si num_threads > 0, limitar el paralelismo. Si es 0, TBB decide automáticamente.
      std::optional<tbb::global_control> thread_limit;
      if (num_threads > 0) {
        thread_limit.emplace(tbb::global_control::max_allowed_parallelism,
                             static_cast<std::size_t>(num_threads));
        std::println("Thread control: Limited to {} threads", num_threads);
      } else {
        std::println("Thread control: Automatic (TBB decides)");
      }

      // Determinar el número de hilos reales disponibles para generar semillas
      int const actual_threads = tbb::this_task_arena::max_concurrency();
      std::println("TBB Max Concurrency (hilos efectivos): {}", actual_threads);

      // Sd por hilo
      auto const base_seed = static_cast<std::uint64_t>(cfg.get_ray_rng_seed());
      std::vector<std::uint64_t> const thread_seeds =
          generate_thread_sds(base_seed, static_cast<std::size_t>(actual_threads));
      std::println("Vector de semillas generado: {} semillas (base: {})", thread_seeds.size(),
                   base_seed);

      // Generadores locales por hilo
      // Usamos tbb::enumerable_thread_specific con lambda
      // Cada hilo obtendrá su propio generador con una semilla única del vector
      tbb::enumerable_thread_specific<std::mt19937_64> thread_rngs{[&thread_seeds] {
        // Usar un contador atómico para asignar secuencialmente las semillas
        static std::atomic<std::size_t> thread_counter{0};
        std::size_t const thread_index = thread_counter++;
        // Asegurar que el índice esté dentro del rango del vector
        std::size_t const seed_index = thread_index % thread_seeds.size();
        return std::mt19937_64{thread_seeds[seed_index]};
      }};
      std::println("Generadores locales por hilo configurados");

      render::camera cam(cfg);

      // PAR: Crear estructura PAR para almacenar la imagen completa
      render::image_par image(cfg.get_image_width(), cfg.get_image_height());

      // PAR: Renderizar a la estructura PAR (3 vectores separados)
      // FASE 4 PASO 4: Pasar grain_rows y grain_cols configurables
      std::println("Grain size: rows={}, cols={}", grain_rows, grain_cols);
      do_render_lp_par(image, cam, scn, cfg, thread_rngs, grain_rows, grain_cols);

      // PAR: Escribir desde la estructura PAR al archivo PPM
      if (!image.write_ppm(std::string(out_file))) {
        std::println(std::cerr, "Error: cannot write output file {}", std::string(out_file));
        return 1;
      }

      log_config(cfg);

    } catch (std::exception const & exception) {
      std::println(std::cerr, "Error: {}", exception.what());
      return 1;
    }

    return 0;
  }

}  // namespace

// NOLINTNEXTLINE(bugprone-exception-escape)
int main(int argc, char * argv[]) {
  // Mostrar la estrategia de particionador compilada
#if defined(USE_SIMPLE_PARTITIONER)
  std::println("Starting PAR rendering with SIMPLE_PARTITIONER");
#elif defined(USE_STATIC_PARTITIONER)
  std::println("Starting PAR rendering with STATIC_PARTITIONER");
#else
  std::println("Starting PAR rendering with AUTO_PARTITIONER (default)");
#endif

  std::span<char *> const args(argv, static_cast<size_t>(argc));

  // FASE 4 PASO 4 y 5: Parámetros opcionales para control de paralelización
  // Uso: render-par <config> <scene> <output> [num_threads] [grain_rows] [grain_cols]
  // - num_threads: Número de hilos (0 = automático, default)
  // - grain_rows: Tamaño de grano para filas (default = 1)
  // - grain_cols: Tamaño de grano para columnas (default = 1)
  if (args.size() < 4 or args.size() > 7) {
    std::println(std::cerr,
                 "Usage: {} <config> <scene> <output> [num_threads] [grain_rows] "
                 "[grain_cols]",
                 args[0]);
    std::println(std::cerr, "  num_threads: Number of threads (0=automatic, default=0)");
    std::println(std::cerr, "  grain_rows: Grain size for rows (default=1)");
    std::println(std::cerr, "  grain_cols: Grain size for columns (default=1)");
    return 1;
  }

  // Valores por defecto para paralelización
  int num_threads = 256;  // Número de hilos por defecto
  int grain_rows  = 4;    // Grain size por defecto para filas
  int grain_cols  = 4;    // Grain size por defecto para columnas

  // Parsear parámetros opcionales
  if (args.size() >= 5) {
    num_threads = std::stoi(args[4]);
    if (num_threads < 0) {
      std::println(std::cerr, "Error: num_threads must be >= 0");
      return 1;
    }
  }
  if (args.size() >= 6) {
    grain_rows = std::stoi(args[5]);
    if (grain_rows <= 0) {
      std::println(std::cerr, "Error: grain_rows must be > 0");
      return 1;
    }
  }
  if (args.size() >= 7) {
    grain_cols = std::stoi(args[6]);
    if (grain_cols <= 0) {
      std::println(std::cerr, "Error: grain_cols must be > 0");
      return 1;
    }
  }

  return render_to_file(args[1], args[2], args[3], num_threads, grain_rows, grain_cols);
}
