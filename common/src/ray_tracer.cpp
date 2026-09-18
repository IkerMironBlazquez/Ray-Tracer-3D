#include "ray_tracer.hpp"
#include "intersection.hpp"
#include "material.hpp"
#include "object.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <random>

namespace render {

  namespace {

    // Constantes para evitar números mágicos
    constexpr double MIN_DISTANCE      = 1e-3;  // Misma constante que en object.cpp
    constexpr double REFLECTION_FACTOR = 2.0;

    inline auto lerp(color const & first_color, color const & second_color,
                     double interpolation_factor) -> color {
      return {((1.0 - interpolation_factor) * first_color.r()) +
                  (interpolation_factor * second_color.r()),
              ((1.0 - interpolation_factor) * first_color.g()) +
                  (interpolation_factor * second_color.g()),
              ((1.0 - interpolation_factor) * first_color.b()) +
                  (interpolation_factor * second_color.b())};
    }

    // Función helper para crear rayo reflejado y calcular color
    // Ahora recibe y pasa RNG
    auto calculate_reflected_color(ray const & reflected_ray, scene const & scene,
                                   config const & cfg, int depth, std::mt19937_64 & rng) -> color {
      return trace_ray(reflected_ray, scene, cfg, depth - 1, rng);
    }

    // Función helper
    auto process_matte_material([[maybe_unused]] ray const & input_ray,
                                Intersection const & intersection_data, scene const & scene_data,
                                config const & config_data, int reflection_depth,
                                std::mt19937_64 & random_generator) -> color {
      auto const & data = std::get<matte_material>(intersection_data.mat->get_data());

      // Ajustar la normal según el sentido: hacia afuera siempre
      vector const adjusted_normal =
          intersection_data.outward ? intersection_data.normal : -intersection_data.normal;

      // Calcular dirección de reflexión aleatoria
      vector const reflection_direction = reflect_matte(adjusted_normal, random_generator);

      // Crear nuevo rayo desde el punto de intersección (desplazado para evitar auto-intersección)
      vector const offset_point = intersection_data.point + MIN_DISTANCE * adjusted_normal;
      ray const reflected_ray(offset_point, reflection_direction);

      // Calcular color reflejado por el nuevo rayo decrementando profundidad en una unidad
      color const reflected_color = calculate_reflected_color(
          reflected_ray, scene_data, config_data, reflection_depth, random_generator);

      // Atenuación del color reflejado con la reflectancia del material
      return {reflected_color.r() * data.reflectance.r(),
              reflected_color.g() * data.reflectance.g(),
              reflected_color.b() * data.reflectance.b()};
    }

    // Función helper
    auto process_metal_material(ray const & input_ray, Intersection const & intersection_data,
                                scene const & scene_data, config const & config_data,
                                int reflection_depth, std::mt19937_64 & random_generator) -> color {
      auto const & data = std::get<metal_material>(intersection_data.mat->get_data());

      // Ajustar la normal según el sentido: hacia afuera siempre
      vector const adjusted_normal =
          intersection_data.outward ? intersection_data.normal : -intersection_data.normal;

      // Calcular dirección de reflexión metálica con difusión
      vector const reflection_direction =
          reflect_metal(input_ray.direction(), adjusted_normal, data.diffusion, random_generator);

      // Crear nuevo rayo desde el punto de intersección (desplazado para evitar auto-intersección)
      vector const offset_point = intersection_data.point + MIN_DISTANCE * adjusted_normal;
      ray const reflected_ray(offset_point, reflection_direction);

      // Calcular color reflejado por el nuevo rayo decrementando profundidad en una unidad
      color const reflected_color = calculate_reflected_color(
          reflected_ray, scene_data, config_data, reflection_depth, random_generator);

      // Atenuación del color reflejado con la reflectancia del material
      return {reflected_color.r() * data.reflectance.r(),
              reflected_color.g() * data.reflectance.g(),
              reflected_color.b() * data.reflectance.b()};
    }

    // Función helper
    // Añadido RNG para uniformidad
    auto process_refractive_material(ray const & input_ray, Intersection const & intersection_data,
                                     scene const & scene_data, config const & config_data,
                                     int reflection_depth, std::mt19937_64 & rng) -> color {
      auto const & data  = std::get<refractive_material>(intersection_data.mat->get_data());
      bool const outward = intersection_data.outward;

      // La función reflect_refractive toma la normal (orientada hacia fuera)
      // y el flag `outward` para determinar si el rayo entra o sale (índice directo/inverso).
      vector const physical_normal = intersection_data.normal;

      // Calcular dirección de reflexión/refracción usando la normal física y el flag outward
      vector const reflection_direction = reflect_refractive(input_ray.direction(), physical_normal,
                                                             data.refraction_index, outward);

      // Crear nuevo rayo desde el punto de intersección (desplazado para evitar auto-intersección).
      // Desplazamos ligeramente en la dirección del rayo saliente (refractado o reflejado)
      // en lugar de usar sólo la normal física: esto evita que el rayo se origine en el
      // lado equivocado de la superficie y vuelva a intersectar inmediatamente.
      vector const outgoing_dir = reflection_direction.normalize();
      vector const offset_point = intersection_data.point + MIN_DISTANCE * outgoing_dir;

      ray const reflected_ray(offset_point, outgoing_dir);

      // Calcular color reflejado por el nuevo rayo decrementando profundidad en una unidad
      color const reflected_color =
          calculate_reflected_color(reflected_ray, scene_data, config_data, reflection_depth, rng);

      // Para materiales refractivos, la reflectancia resultante es siempre (1,1,1)
      return reflected_color;
    }

    auto closest_intersection_inner(ray const & input_ray, scene const & scene_data)
        -> Intersection {
      Intersection best;
      best.distance = std::numeric_limits<double>::infinity();

      auto const & objects = scene_data.get_objects();
      for (auto const & obj : objects) {
        Intersection intersection_result;
        if (obj.get_type() == object_type::sphere) {
          intersection_result = std::get<sphere>(obj.get_data()).intersect(input_ray);
        } else {
          intersection_result = std::get<cylinder>(obj.get_data()).intersect(input_ray);
        }

        if (intersection_result.hit and
            intersection_result.distance > MIN_DISTANCE and
            intersection_result.distance < best.distance)
        {
          best = intersection_result;
          try {
            best.mat = &scene_data.get_material(obj.get_material_name());
          } catch (...) {
            best.mat = nullptr;
          }
        }
      }

      return best;
    }

    auto closest_intersection(ray const & input_ray, scene const & scene_data) noexcept
        -> Intersection {
      // Ya no necesitamos doble búsqueda - el material se asigna directamente
      return closest_intersection_inner(input_ray, scene_data);
    }

    // Helper para manejar procesamiento de materiales
    auto process_material_reflection(material_type const material_type_value,
                                     Intersection const & intersection_data, ray const & input_ray,
                                     scene const & scene_data, config const & config_data,
                                     int reflection_depth,
                                     std::mt19937_64 & material_random_generator) -> color {
      if (material_type_value == material_type::matte) {
        return process_matte_material(input_ray, intersection_data, scene_data, config_data,
                                      reflection_depth, material_random_generator);
      }

      if (material_type_value == material_type::metal) {
        return process_metal_material(input_ray, intersection_data, scene_data, config_data,
                                      reflection_depth, material_random_generator);
      }

      if (material_type_value == material_type::refractive) {
        return process_refractive_material(input_ray, intersection_data, scene_data, config_data,
                                           reflection_depth, material_random_generator);
      }

      // Excepción
      return {1.0, 0.0, 0.0};
    }

  }  // namespace

  // Calculamos el color para el cielo
  auto background_gradient(config const & cfg, ray const & r) -> color {
    vector const direction            = r.direction().normalize();
    double const interpolation_factor = 0.5 * (direction.get_y() + 1.0);  // map [-1,1] -> [0,1]

    color const dark  = cfg.get_background_dark_color();
    color const light = cfg.get_background_light_color();

    // Según especificación: ⃗c = (1−m)·⃗cl + m·⃗cd
    // lerp(a,b,t) = (1-t)*a + t*b, así que lerp(light,dark,m) = (1-m)*light + m*dark
    return lerp(light, dark, std::clamp(interpolation_factor, 0.0, 1.0));
  }

  // FUNCIÓN PRINCIPAL DE TRAZADO DE RAYOS
  // Ahora recibe RNG para unificar sistema con TBB (eliminar thread_local) (TBB)
  auto trace_ray(ray const & r, scene const & scene, config const & cfg, int depth,
                 std::mt19937_64 & rng) -> color {
    // Cuando la profundidad es menor o igual que 0, devolver color de fondo
    if (depth <= 0) {
      return background_gradient(cfg, r);
    }

    // Si la profundidad del rayo es positiva, se calcula la intersección del rayo con la escena
    Intersection const intersection_result = closest_intersection(r, scene);

    // Si no se produce intersección con ningún objeto de la escena
    // se genera como contribución el color de fondo
    if (!intersection_result.hit) {
      return background_gradient(cfg, r);
    }

    // Excepción
    if (intersection_result.mat == nullptr) {
      return {1.0, 0.0, 0.0};
    }

    // Obtener tipo de material y procesar según especificación
    material_type const material_type_value = intersection_result.mat->get_type();

    // Usar RNG para el hilo actual (eliminado thread_local material_rng) (TBB)
    return process_material_reflection(material_type_value, intersection_result, r, scene, cfg,
                                       depth, rng);
  }

  // REFLEXIÓN EN MATERIALES MATE
  auto reflect_matte(vector const & normal, std::mt19937_64 & rng) -> vector {
    // Generador de números aleatorios entre -1 y 1
    std::uniform_real_distribution<double> distribution(-1.0, 1.0);

    // Calcular dirección de reflexión sumando al vector normal valores aleatorios
    vector const reflection_direction =
        normal + vector(distribution(rng), distribution(rng), distribution(rng));

    // Verificar si el vector resultante es demasiado pequeño (Sección 3.5.1)
    constexpr double epsilon = 1e-8;
    if (std::abs(reflection_direction.get_x()) < epsilon and
        std::abs(reflection_direction.get_y()) < epsilon and
        std::abs(reflection_direction.get_z()) < epsilon)
    {
      // Si es demasiado pequeño, usar el vector normal
      return normal;
    }

    return reflection_direction.normalize();
  }

  // REFLEXIÓN EN MATERIALES METÁLICOS
  auto reflect_metal(vector const & incident, vector const & normal, double diffusion,
                     std::mt19937_64 & rng) -> vector {
    // Calcular reflexión inicial: d⃗¹r = d⃗o - 2(d⃗o·d⃗n)d⃗n
    vector const initial_reflection = incident - REFLECTION_FACTOR * incident.dot(normal) * normal;

    // Normalizar la reflexión inicial
    vector const normalized_reflection = initial_reflection.normalize();

    // Generar vector de difusión aleatorio con factor Φ
    std::uniform_real_distribution<double> distribution(-diffusion, diffusion);
    vector const diffusion_vector(distribution(rng), distribution(rng), distribution(rng));

    // Calcular vector de reflexión final: d⃗r = (d⃗¹r/||d⃗¹r||) + φ⃗
    vector const final_reflection = normalized_reflection + diffusion_vector;

    // Normalizar el resultado final para mantener como vector unitario
    return final_reflection.normalize();
  }

  // REFLEXIÓN EN MATERIALES REFRACTIVOS
  auto reflect_refractive(vector const & incident_vector, vector const & normal_vector,
                          double refraction_index, bool outward_direction) noexcept -> vector {
    // Calcular vector unitario de dirección del rayo original
    vector const unit_direction = incident_vector.normalize();

    // Calcular cos θ = mín(-d̂o·d⃗n, 1)
    double const cos_theta = std::min(-unit_direction.dot(normal_vector), 1.0);
    double const sin_theta = std::sqrt(1.0 - (cos_theta * cos_theta));

    // Determinar el cociente de índices (n1 / n2) correcto.
    // Si el rayo viene desde fuera (outward_direction==true) entonces n1=1 (aire)
    // y n2=refraction_index, por tanto n_ratio = 1.0 / refraction_index.
    // Si viene desde dentro, n_ratio = refraction_index (1/n1*n2) según la convención usada.
    double const corrected_refraction =
        outward_direction ? (1.0 / refraction_index) : refraction_index;

    // Verificar si ρ'·sin θ > 1 (reflexión total interna)
    if (corrected_refraction * sin_theta > 1.0) {
      // Reflexión total interna: d⃗r = d̂o - 2(d̂o·d⃗n)d⃗n
      return unit_direction - REFLECTION_FACTOR * unit_direction.dot(normal_vector) * normal_vector;
    }

    // Refracción normal: calcular vectores u⃗ y v⃗
    vector const u_vector = corrected_refraction * (unit_direction + cos_theta * normal_vector);
    // Evitar pasar valores negativos a sqrt: clamp a 0.0
    double const inner       = 1.0 - u_vector.magnitude_squared();
    double const safe_inner  = std::max(0.0, inner);
    double const v_magnitude = std::sqrt(safe_inner);
    vector const v_vector    = -v_magnitude * normal_vector;

    return (u_vector + v_vector).normalize();
  }

}  // namespace render
