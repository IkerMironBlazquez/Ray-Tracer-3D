#ifndef INTERSECTION_HPP
#define INTERSECTION_HPP

#include "vector.hpp"

namespace render {

  class material;

  // Estructura intersección rayo-objeto
  struct Intersection {
    bool hit        = false;         // ¿Hay intersección?
    double distance = 0.0;           // Distancia λ desde el origen (Rayo)
    vector point;                    // Punto de intersección I
    vector normal;                   // Vector normal en el punto de intersección
    material const * mat = nullptr;  // Material del objeto intersectado
    bool outward         = true;     // Sentido
  };

}  // namespace render

#endif
