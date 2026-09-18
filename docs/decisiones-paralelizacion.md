# Decisiones de paralelización

Documento de diseño que justifica **qué** se paralelizó, **qué no** y **por qué**, en
función de un análisis coste-beneficio, las dependencias de datos y el *overhead* de
gestión de tareas.

## Contexto del problema

El renderizado por *ray tracing* calcula el color de cada píxel trazando rayos desde la
cámara hacia la escena. El coste dominante es el cálculo de color por píxel, que implica:

- Generación de múltiples muestras por píxel (`samples_per_pixel`: 20–300).
- Trazado de rayos con reflexiones y refracciones recursivas (`max_depth`: 3–100).
- Cálculo de intersecciones contra todos los objetos de la escena.
- Generación de números aleatorios para el muestreo estocástico.

Para una imagen de 600×600 con 20 muestras se trazan ~7,2 millones de rayos, cada uno con
múltiples operaciones de coma flotante. El tiempo de I/O (lectura de configuración y escena,
escritura del PPM) representa menos del 1 % del total.

## Zona elegida: bucle de píxeles

Se paralelizó el **bucle externo sobre píxeles** en `par/src/main.cpp`. Razones:

- **Máximo paralelismo disponible.** Una imagen de 600×600 son 360 000 tareas
  independientes, suficientes para saturar cualquier número de núcleos.
- **Independencia total.** No hay dependencias de datos entre píxeles: cada uno escribe en
  su posición única y no requiere sincronización.
- **Carga desigual que favorece el balanceo dinámico.** El coste por píxel varía un orden de
  magnitud según su contenido (fondo/cielo muy rápido; objetos metálicos rápidos; objetos
  refractivos lentos). Esta variabilidad justifica el *work stealing*.
- **Escalabilidad.** Al no haber sincronización, el factor de aceleración teórico se
  aproxima al número de núcleos disponibles.

### Implementación

- **Algoritmo:** `tbb::parallel_for`, estándar para iteración paralela sobre rangos, con
  *work stealing* integrado.
- **Rango:** `tbb::blocked_range2d<int>` (filas × columnas), que preserva localidad
  espacial y permite controlar el tamaño de grano por dimensión.
- **Particionadores configurables en compilación**, para poder comparar estrategias:
  - `simple_partitioner`: división recursiva hasta el tamaño de grano mínimo.
  - `static_partitioner`: división estática (≈ un bloque por hilo).
  - `auto_partitioner` (por defecto): balanceo dinámico con *work stealing*.
- **Aleatoriedad reproducible y sin contención:** un generador `std::mt19937_64` por hilo
  mediante `tbb::enumerable_thread_specific`, sembrado desde un vector de semillas derivado
  de una semilla base. Así cada hilo tiene su propio RNG sin necesidad de bloqueos.
- **Control de paralelismo:** número de hilos y tamaño de grano (filas y columnas)
  ajustables por línea de comandos vía `tbb::global_control`.

## Zonas descartadas

**Muestras por píxel** (bucle interno de `render_pixel`). Grano demasiado fino: cada
muestra cuesta microsegundos, por lo que el *overhead* de crear tareas superaría la
ganancia. Además entraría en conflicto con la paralelización del bucle externo
(sobre-paralelización) y obligaría a multiplicar o sincronizar los generadores aleatorios.

**Objetos de la escena** (`closest_intersection_inner` en `common/src/ray_tracer.cpp`).
Las escenas típicas tienen 10–50 objetos y cada intersección cuesta nanosegundos:
iteraciones insuficientes para amortizar el *overhead*. Además la búsqueda del objeto más
cercano es una reducción (mínimo global) que complicaría la paralelización.

**Trazado recursivo de rayos** (`trace_ray`). Dependencia secuencial inherente: cada nivel
de recursión depende del resultado del anterior y la profundidad es baja (3–10 niveles
habituales). Paralelizarlo exigiría rediseñar el algoritmo.

**Entrada/salida.** Operaciones secuenciales que se ejecutan una sola vez y suponen menos
del 1 % del tiempo total; paralelizarlas no aporta beneficio y compromete la simplicidad.
