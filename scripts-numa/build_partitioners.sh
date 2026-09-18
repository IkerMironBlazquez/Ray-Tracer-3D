#!/bin/sh
set -Eeu pipefail

#SBATCH --job-name=build_parts
#SBATCH --output=scripts-numa/logs/build_partitioners.txt
#SBATCH --error=scripts-numa/logs/build_partitioners.err

# Script para compilar las 3 versiones del ejecutable (una por particionador)
# Se envía con sbatch a un nodo estándar

export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

echo "=================================================="
echo "COMPILACIÓN DE RENDER-PAR (3 particionadores)"
echo "=================================================="
echo ""
cd ..
# Función para compilar una versión
build_version() {
  local PARTITIONER=$1
  local BUILD_DIR=$2
  local DEFINE=$3
  
  echo "----------------------------------------"
  echo "Compilando: ${PARTITIONER}"
  echo "Build dir: ${BUILD_DIR}"
  echo "Define: ${DEFINE}"
  echo "----------------------------------------"
  
  # Limpiar directorio de build si existe
  rm -rf ${BUILD_DIR}
  
  # Crear directorio de build
  cmake -S . -B ${BUILD_DIR} \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=23 \
    -DCMAKE_CXX_FLAGS="-D${DEFINE}"
  
  # Compilar
  cmake --build ${BUILD_DIR} --config Release --parallel
  
  # Verificar que el ejecutable existe
  if [ -f "${BUILD_DIR}/par/render-par" ]; then
    echo "✅ ${PARTITIONER}: Compilado"
    
    # Copiar con nombre específico al directorio build/par/
    mkdir -p build/par
    cp "${BUILD_DIR}/par/render-par" "build/par/render-par-${PARTITIONER}"
    echo "   Copiado a: build/par/render-par-${PARTITIONER}"
  else
    echo "❌ ${PARTITIONER}"
    exit 1
  fi
  
  echo ""
}

# Compilar las 3 versiones
build_version "simple" "build/simple" "USE_SIMPLE_PARTITIONER"
build_version "static" "build/static" "USE_STATIC_PARTITIONER"
build_version "auto" "build/auto" "USE_AUTO_PARTITIONER"

echo "=================================================="
echo "COMPILACIÓN COMPLETADA"
echo "=================================================="
echo ""
echo "Ejecutables generados:"
echo "  - build/par/render-par-simple"
echo "  - build/par/render-par-static"
echo "  - build/par/render-par-auto"
echo ""
echo "Ahora puedes ejecutar: cd scripts-numa && ./submit.sh"
echo ""
