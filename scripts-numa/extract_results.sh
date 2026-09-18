#!/bin/sh
set -Eeu pipefail
#SBATCH --job-name=extract
#SBATCH --output=./logs/extract.txt
#SBATCH --error=./logs/extract_results.err
# Script para extraer los resultados de las pruebas
# Se envía con sbatch a nodo estándar
export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
echo "=================================================="
echo "EXTRACCIÓN DE RESULTADOS DE PRUEBAS (NUMA)"
echo ""
# Verificar que estamos en el directorio correcto
if [ ! -d "logs" ]; then
  echo "No se encuentra el directorio logs/"
  exit 1
fi
OUTPUT_CSV="results.csv"
# Crear cabecera del CSV
echo "estrategia,hilos,grano,tiempo_s,energia_pkg_j,energia_ram_j,energia_total_j" > "$OUTPUT_CSV"
echo "Procesando archivos de log..."
# Procesar cada archivo de log
for STRATEGY in simple static auto; do
  for THREADS in 01 02 04 08 16 32 64 128 256; do
    LOGFILE="logs/${STRATEGY}_${THREADS}.txt"
    
    if [ ! -f "$LOGFILE" ]; then
      echo "Advertencia: No se encuentra $LOGFILE"
      continue
    fi
    echo "Procesando: $LOGFILE"
    # Extraer datos de cada grano
    # Este es un procesamiento básico - puede necesitar ajustes según el formato exacto de perf stat
    grep -A 20 "Grain:" "$LOGFILE" | grep -E "(seconds time elapsed|Joules power/energy)" | \
      awk -v strategy="$STRATEGY" -v threads="$THREADS" '
        BEGIN { grain=0; time=0; pkg=0; ram=0; count=0 }
        /Grain/ { grain=$2 }
        /seconds time elapsed/ { time=$1; count++ }
        /Joules.*pkg/ { pkg=$1 }
        /Joules.*ram/ { ram=$1; total=pkg+ram; 
          printf "%s,%d,%d,%.3f,%.3f,%.3f,%.3f\n", strategy, threads, grain, time, pkg, ram, total
        }
      ' || true
  done
done >> "$OUTPUT_CSV"
echo "Resultados consolidados en: $OUTPUT_CSV"
echo "Estadísticas rápidas:"
wc -l "$OUTPUT_CSV"
echo "Primeras líneas:"
head -n 10 "$OUTPUT_CSV"
echo "Para análisis completo, importar el CSV en Excel, Python, R, etc."
