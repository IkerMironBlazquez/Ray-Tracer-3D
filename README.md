# Sistema de renderizado paralelo de escenas 3D

Renderizador de escenas 3D por *ray tracing* con paralelización a nivel de píxel sobre
**Intel oneTBB**. El proyecto explora cómo escalar el cálculo de imágenes fotorrealistas
sobre arquitecturas multinúcleo y multi-zócalo (NUMA), comparando estrategias de
partición de trabajo y midiendo rendimiento y consumo energético.

Desarrollado para la asignatura de Arquitectura de Computadores de la Universidad Carlos III
de Madrid.

## Características

- **Motor de *ray tracing*** con muestreo estocástico por píxel, reflexiones y refracciones
  recursivas, y corrección gamma.
- **Muestreo adaptativo:** cada píxel deja de muestrear en cuanto el color converge
  (umbral relativo), evitando trabajo innecesario en zonas planas de la imagen.
- **Paralelización a nivel de píxel** con `tbb::parallel_for` sobre un
  `blocked_range2d` bidimensional, con *work stealing* y tamaño de grano configurable.
- **Tres estrategias de partición** seleccionables en compilación (`simple`, `static`,
  `auto`) para comparar su comportamiento bajo distintos números de hilos.
- **Generación aleatoria reproducible y sin contención:** un generador `std::mt19937_64`
  por hilo (`tbb::enumerable_thread_specific`), sembrado de forma determinista.
- **Imagen en disposición SoA** (tres vectores de canal R/G/B independientes) para mejorar
  la localidad de caché, con salida en formato PPM (P3).
- **Suite de pruebas unitarias** con GoogleTest y **scripts de *benchmarking*** para
  clúster SLURM que miden tiempo y energía (`perf`).

## Arquitectura

| Directorio    | Contenido |
|---------------|-----------|
| `common/`     | Núcleo del motor: vectores, rayos, cámara, materiales, objetos, intersecciones, *parsers* de configuración/escena y el trazador de rayos. |
| `par/`        | Renderizador paralelo: bucle de píxeles con TBB, estructura de imagen SoA y `main`. |
| `utcommon/`   | Pruebas unitarias del núcleo. |
| `utpar/`      | Pruebas unitarias de la estructura de imagen paralela. |
| `scenes/`     | Escenas y configuraciones de ejemplo. |
| `test_files/` | Entradas para las pruebas de los *parsers* (casos válidos y de error). |
| `scripts-numa/` | Scripts SLURM para evaluación de rendimiento y energía en clúster. |
| `docs/`       | Documentación de diseño. |

Las decisiones de qué paralelizar (y, sobre todo, qué **no**) y su justificación se
detallan en [docs/decisiones-paralelizacion.md](docs/decisiones-paralelizacion.md).

## Requisitos

- Compilador con soporte de **C++23** (probado con GCC 14).
- **CMake ≥ 3.30** y **Ninja**.
- **Intel oneTBB** (`find_package(TBB)`).

GoogleTest y Microsoft GSL se descargan automáticamente mediante `FetchContent` durante la
configuración de CMake.

## Compilación

```bash
cmake --preset default
cmake --build --preset gcc-release --config Release --target all --parallel
```

El preajuste `default` activa optimizaciones agresivas (`-O3 -march=native -flto`,
*loop unrolling*) y compila con avisos como errores (`-Wall -Wextra -Werror -pedantic`).

Para seleccionar la estrategia de partición se define el *flag* correspondiente al
configurar (por defecto, `auto`):

- `-DUSE_SIMPLE_PARTITIONER` — división recursiva hasta el grano mínimo.
- `-DUSE_STATIC_PARTITIONER` — división estática (≈ un bloque por hilo).
- *(sin flag)* — `auto_partitioner` con balanceo dinámico.

## Ejecución

```bash
render-par <config> <scene> <salida.ppm> [num_hilos] [grano_filas] [grano_columnas]
```

- `num_hilos` — número de hilos (`0` = automático, lo decide TBB).
- `grano_filas` / `grano_columnas` — tamaño de grano del `blocked_range2d`.

Ejemplo:

```bash
./out/build/default/par/render-par scenes/config1.txt scenes/scene1.txt salida.ppm 8 4 4
```

## Pruebas

```bash
ctest --test-dir out/build/default --build-config Release
```

Los *scripts* `utest.py`/`utest.sh` (pruebas unitarias) y `ftest.py`/`ftest.sh` y
`compare.py` (pruebas funcionales y comparación de imágenes) automatizan la verificación.

## Evaluación de rendimiento

`scripts-numa/` contiene una batería de trabajos SLURM que ejecuta el renderizador para las
tres estrategias de partición a través de distintos números de hilos (1–256) y tamaños de
grano, midiendo tiempo de ejecución y energía (`perf`, contador `power/energy-pkg/`). Los
resultados se consolidan en un CSV para analizar aceleración, eficiencia y consumo. Véase
[scripts-numa/START.md](scripts-numa/START.md) para el flujo completo.

## Licencia

Distribuido bajo los términos del archivo [LICENSE](LICENSE).
