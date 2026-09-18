#include "object.hpp"

#include <cmath>
#include <limits>

#include "intersection.hpp"
#include "ray.hpp"
#include "vector.hpp"

namespace render {

  // Constantes según el enunciado
  constexpr double MIN_DISTANCE = 1e-3;   // Distancia mínima válida según enunciado
  constexpr double EPSILON      = 1e-12;  // Tolerancia numérica más estricta

  Intersection sphere::intersect(ray const & r) const {
    Intersection result;

    // PASO 1: Calcular vector ⃗rc:
    // "el vector que va del origen del rayo Or hasta el centro de la esfera C"
    // ⃗rc = C - Or
    vector const rc = center - r.origin();

    // PASO 2: Calcular coeficientes de la ecuación cuadrática
    // Ecuación: a·λ² + b·λ + c = 0

    // a = d⃗r · d⃗r
    double const a = r.direction().dot(r.direction());

    // b = 2·d⃗r · ⃗rc
    double const b = 2.0 * r.direction().dot(rc);

    // c = ⃗rc · ⃗rc - r²
    double const c = rc.dot(rc) - radius * radius;

    // PASO 3: Calcular discriminante
    // Δ = b² - 4ac
    double const discriminant = b * b - 4.0 * a * c;

    // Si discriminante < 0
    if (discriminant < 0.0) {
      return result;  // result.hit = false por defecto
    }

    // Calcular soluciones de la ecuación cuadrática
    // Dado que la ecuación expandida es: d⃗r·d⃗r λ² - 2d⃗r·⃗rc λ + ⃗rc·⃗rc - r² = 0
    // Y definimos b = 2d⃗r·⃗rc (positivo), el término es -b·λ
    // Entonces ax² - bx + c = 0 se resuelve con: λ = (b ± √Δ) / 2a
    double const sqrt_discriminant = std::sqrt(discriminant);
    double const lambda1           = (b - sqrt_discriminant) / (2.0 * a);
    double const lambda2           = (b + sqrt_discriminant) / (2.0 * a);

    // Seleccionar la intersección más cercana válida (λ > MIN_DISTANCE)
    double lambda = -1.0;
    if (lambda1 > MIN_DISTANCE) {
      lambda = lambda1;  // Primera intersección (más cercana)
    } else if (lambda2 > MIN_DISTANCE) {
      lambda = lambda2;  // Segunda intersección
    } else {
      return result;  // No hay intersección válida
    }

    // Calcular punto de intersección
    // I = Or + d⃗r·λ
    result.point = r.at(lambda);

    // Calcular vector normal en el punto de intersección
    // d⃗n = (I - C) / r
    vector const normal = (result.point - center) / radius;

    // Determinar sentido
    // Si d⃗r · d⃗n < 0 → hacia afuera
    // Si d⃗r · d⃗n ≥ 0 → hacia adentro
    bool const outward = r.direction().dot(normal) < 0.0;

    // Resultado
    result.hit      = true;
    result.distance = lambda;
    result.normal   = outward ? normal : -normal;  // Cambiar signo si es hacia adentro
    result.outward  = outward;
    // result.material se asignará más adelante cuando tengamos materiales

    return result;
  }

  Intersection cylinder::intersect(ray const & r) const {
    Intersection best;
    best.distance = std::numeric_limits<double>::infinity();

    // Calcular eje unitario y altura del cilindro
    vector const axis_unit = axis.normalize();
    double const h         = height();

    // SUPERFICIE CURVA

    // Vector del origen al centro del cilindro
    vector const rc = r.origin() - center;

    // Calcular componentes perpendiculares al eje
    // ⃗v⊥â = ⃗v - (⃗v·â)â
    vector const dr_perp = r.direction() - r.direction().dot(axis_unit) * axis_unit;
    vector const rc_perp = rc - rc.dot(axis_unit) * axis_unit;

    // Coeficientes de la ecuación cuadrática para la superficie curva
    double const a = dr_perp.dot(dr_perp);
    double const b = 2.0 * rc_perp.dot(dr_perp);
    double const c = rc_perp.dot(rc_perp) - radius * radius;

    double const discriminant = b * b - 4.0 * a * c;

    // Si hay solución para la superficie curva
    if (discriminant >= 0.0 and std::abs(a) > EPSILON) {
      double const sqrt_disc = std::sqrt(discriminant);
      double const lambda1   = (-b - sqrt_disc) / (2.0 * a);
      double const lambda2   = (-b + sqrt_disc) / (2.0 * a);

      for (double const lambda : {lambda1, lambda2}) {
        if (lambda <= MIN_DISTANCE) {
          continue;
        }

        vector const point = r.at(lambda);

        // Verificar que el punto esté dentro de los límites de altura
        double const projection = (point - center).dot(axis_unit);

        if (std::abs(projection) <= h / 2.0) {
          // Calcular normal perpendicular al eje
          // IMPORTANTE: NO normalizar según enunciado página 15
          // d⃗n= (I−C)⊥ˆa= (I−C)−((I−C)·ˆa)ˆa
          vector const normal_vec = (point - center) - projection * axis_unit;

          bool const outward = r.direction().dot(normal_vec) < 0.0;

          // Actualizar si es más cercana
          if (lambda < best.distance) {
            best.hit      = true;
            best.distance = lambda;
            best.point    = point;
            best.normal   = outward ? normal_vec : -normal_vec;
            best.outward  = outward;
          }
        }
      }
    }

    // BASES

    // Base superior: centro en C + (h/2)â, normal = â
    // Base inferior: centro en C - (h/2)â, normal = -â
    for (int base_idx = 0; base_idx < 2; ++base_idx) {
      double const sign         = (base_idx == 0) ? 0.5 : -0.5;
      vector const base_center  = center + sign * h * axis_unit;
      vector const plane_normal = (base_idx == 0) ? axis_unit : -axis_unit;

      // Intersección con el plano de la base
      vector const rp          = base_center - r.origin();
      double const denominator = r.direction().dot(plane_normal);

      // Evitar división por cero (rayo paralelo al plano)
      if (std::abs(denominator) < EPSILON) {
        continue;
      }

      double const lambda = rp.dot(plane_normal) / denominator;

      // Validar distancia
      if (lambda <= MIN_DISTANCE) {
        continue;
      }
      if (best.hit and lambda >= best.distance) {
        continue;
      }

      vector const point = r.at(lambda);

      // Verificar que el punto esté dentro del radio de la base
      double const dist_from_center = (point - base_center).magnitude();

      if (dist_from_center <= radius) {
        // Corregir la determinación del sentido del normal
        // La normal siempre debe apuntar hacia afuera del cilindro
        bool const outward = r.direction().dot(plane_normal) < 0.0;

        // Para las bases, la normal correcta debe ser consistente con la geometría
        vector normal_final = plane_normal;
        if (!outward) {
          normal_final = -plane_normal;
        }

        best.hit      = true;
        best.distance = lambda;
        best.point    = point;
        best.normal   = normal_final;
        best.outward  = outward;
      }
    }

    return best;
  }

}  // namespace render
