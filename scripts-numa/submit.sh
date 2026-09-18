#!/bin/sh
#SBATCH --job-name=submit_all
#SBATCH --output=./logs/submit.txt
#SBATCH --error=./logs/submit.err
#SBATCH --time=00:10:00
set -Eeu pipefail

# Script maestro para lanzar todas las pruebas al clúster NUMA
# Se ejecuta en nodo normal y envía a STAN

export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

mkdir -p logs

# Verificar que estamos en el directorio (scripts-numa o raíz)
if [ ! -f "simple_01.sh" ]; then
  if [ -d "scripts-numa" ]; then
    cd scripts-numa
  else
    echo "X: No se encuentra el directorio scripts-numa"
    exit 1
  fi
fi

echo "=================================================="
echo "LANZAMIENTO DE PRUEBAS (STAN)"
echo "=================================================="
echo ""
echo "IMPORTANTE: Asegúrate de haber compilado antes"
echo "    ./scripts-numa/build_partitioners.sh"
echo ""
echo "Total de scripts a enviar: 27"
echo "  - scripts de pruebas (3 estrategias × 9 niveles de hilos)"
echo ""
echo "Tiempo estimado total: ~60-90 minutos"
echo ""
echo "Los resultados estarán en: logs/"
echo ""
echo "Continuando con el envío automático..."
echo ""

# Verificar que existen los 3 ejecutables
MISSING=0
for PART in simple static auto; do
  if [ ! -f "../build/par/render-par-${PART}" ]; then
    echo "❌ No se encuentra ../build/par/render-par-${PART}"
    MISSING=1
  fi
done

if [ $MISSING -eq 1 ]; then
  echo ""
  echo "Por favor, compila primero con:"
  echo "  ./scripts-numa/build_partitioners.sh"
  echo ""
  exit 1
fi

echo ""
echo "✅ Los 3 ejecutables encontrados:"
echo "   - ../build/par/render-par-simple"
echo "   - ../build/par/render-par-static"
echo "   - ../build/par/render-par-auto"
echo ""

# Simple partitioner (9 scripts)
echo "Enviando scripts de simple_partitioner..."
for THREADS in 01 02 04 08 16 32 64 128 256; do
  OUTPUT=$(sbatch -p stan simple_${THREADS}.sh)
  echo "  $OUTPUT"
  sleep 0.5
done
echo ""

# Static partitioner (9 scripts)
echo "Enviando scripts de static_partitioner..."
for THREADS in 01 02 04 08 16 32 64 128 256; do
  OUTPUT=$(sbatch -p stan static_${THREADS}.sh)
  echo "  $OUTPUT"
  sleep 0.5
done
echo ""

# Auto partitioner (9 scripts)
echo "Enviando scripts de auto_partitioner..."
for THREADS in 01 02 04 08 16 32 64 128 256; do
  OUTPUT=$(sbatch -p stan auto_${THREADS}.sh)
  echo "  $OUTPUT"
  sleep 0.5
done
echo ""

echo "=================================================="
echo "TODOS LOS TRABAJOS HAN SIDO ENVIADOS"
echo "=================================================="
echo ""
echo "Comandos útiles:"
echo "  - Ver estado de la cola:    squeue -u \$USER"
echo "  - Ver trabajos en stan:     squeue -p stan"
echo "  - Cancelar un trabajo:      scancel <JOB_ID>"
echo "  - Cancelar todos:           scancel -u \$USER"
echo "  - Ver logs en tiempo real:  tail -f logs/<nombre>.txt"
echo ""
echo "Los resultados estarán disponibles en: logs/"
echo ""
