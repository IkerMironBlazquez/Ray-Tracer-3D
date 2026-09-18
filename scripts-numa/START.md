# Scripts NUMA

## Pre-requisitos Verificados

Antes de enviar los trabajos a STAN
1. **Compilar las 3 versiones** (una por particionador) en un nodo estándar:
```bash
./scripts-numa/build_partitioners.sh
```

Este script compila automáticamente:
- `build/stan/par/render-par-simple`
- `build/stan/par/render-par-static`
- `build/stan/par/render-par-auto`

## Envío de Trabajos (Opción Recomendada)

```bash
cd scripts-numa
./submit.sh
```

## Qué hace cada trabajo

### Compilación Previa (MANUAL)
- **Debes compilar ANTES**
- Comando: `cmake -S . -B build/stan -DCMAKE_BUILD_TYPE=Release && cmake --build build/stan --config Release --parallel`
- Tiempo estimado: 2-5 minutos
- El ejecutable debe estar en: `build/stan/par/render-par`

### Pruebas - **90 pruebas optimizadas**
Cada script ejecuta **entre 1 y 5 pruebas** según nivel de hilos:

**Distribución Optimizada**:
- **1, 2, 4 hilos**: Solo grano 64 (1 prueba) - demostrar ineficiencia
- **8 hilos**: Granos 16, 64, 256 (3 pruebas)
- **16 hilos**: Granos 4, 16, 64, 256 (4 pruebas)
- **32-256 hilos**: Granos 1, 4, 16, 64, 256 (5 pruebas)

**Simple Partitioner** (9 scripts) - 30 pruebas:
- 1, 2, 4, 8, 16, 32, 64, 128, 256 hilos

**Static Partitioner** (9 scripts) - 30 pruebas:
- 1, 2, 4, 8, 16, 32, 64, 128, 256 hilos

**Auto Partitioner** (9 scripts) - 30 pruebas:
- 1, 2, 4, 8, 16, 32, 64, 128, 256 hilos

Tiempo estimado por script: 2-5 minutos (según nivel de hilos)

## Monitorización

Ver estado de trabajos:
```bash
squeue -u $USER
```

Ver trabajos en la partición stan:
```bash
squeue -p stan
```

Ver progreso en tiempo real:
```bash
tail -f scripts-numa/logs/simple_16.txt
```

##  Obtención de Resultados

Una vez completadas todas las pruebas:

```bash
cd scripts-numa
./extract_results.sh
```

Esto generará: `scripts-numa/results_all.csv`

##  Importante

1. **No modificar los scripts después de enviarlos**
2. **Verificar render-par** antes de enviar
3. **Los logs están en** `scripts-numa/logs/`
4. **Límite estricto de 5 minutos** - Scripts optimizados para cumplirlo
5. **Solo se usa `power/energy-pkg/`** - Evita colapso del sistema
6. **Compilación en nodos estándar** - No requiere stan

## Solución de Problemas

### Si submit.sh falla diciendo que no encuentra los ejecutables
```bash
./scripts-numa/build_partitioners.sh

# Verificar que existen
ls -lh build/stan/par/render-par-*
```

### Cancelar todos los trabajos
```bash
scancel -u $USER
```

##  Checklist Pre-Envío

- [ ] **Compilar las 3 versiones** con `./scripts-numa/build_partitioners.sh`
- [ ] Verificar que los 3 ejecutables existen:
  - `build/stan/par/render-par-simple`
  - `build/stan/par/render-par-static`
  - `build/stan/par/render-par-auto`
- [ ] Confirmar que `config5.txt` y `scene5.txt` existen
- [ ] Asegurar que hay espacio
- [ ] Estar preparado para esperar a los resultados

##  Tamaño Estimado de Resultados

- Cada log: ~1-2 MB
- Total logs: ~30-60 MB
- CSV final: ~100 KB

## Después de Obtener Resultados

1. Importar `results.csv` en Python/Excel/R
2. Generar gráficas de:
   - Tiempo vs Hilos (por estrategia)
   - Energía vs Hilos (por estrategia)
   - Sup vs Hilos
   - Eficiencia vs Hilos
3. Identificar configuración óptima
4. Documentar...