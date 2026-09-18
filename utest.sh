#!/bin/bash

# Script de automatización de pruebas unitarias
# Como especificado en el enunciado, ejecuta todas las pruebas unitarias del proyecto

set -e  # Terminar en caso de error

# Colores para la salida
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Directorio base del proyecto
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_ROOT"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  AUTOMATIZACIÓN DE PRUEBAS UNITARIAS  ${NC}"
echo -e "${BLUE}========================================${NC}"
echo

# Verificar que el proyecto esté compilado
if [ ! -d "out/build/default" ]; then
    echo -e "${YELLOW}Compilando el proyecto...${NC}"
    cmake --preset default
    cmake --build --preset gcc-debug
fi

# Buscar ejecutables de pruebas
UTCOMMON_EXE="./out/build/default/utcommon/Debug/utcommon"
UTPAR_EXE="./out/build/default/utpar/Debug/utpar"

# Verificar que los ejecutables existen
if [ ! -f "$UTCOMMON_EXE" ] || [ ! -f "$UTPAR_EXE" ]; then
    echo -e "${RED}Error: No se encontraron los ejecutables de pruebas${NC}"
    echo "Recompilando..."
    cmake --build --preset gcc-debug
fi

# Inicializar contadores
TOTAL_TESTS=0
TOTAL_PASSED=0
FAILED_TESTS=()

# Función para ejecutar una suite de pruebas
run_test_suite() {
    local test_name="$1"
    local test_exe="$2"
    
    echo -e "${BLUE}Ejecutando pruebas de $test_name...${NC}"
    echo "========================================="
    
    if [ ! -f "$test_exe" ]; then
        echo -e "${RED}Error: No se encontró el ejecutable $test_exe${NC}"
        FAILED_TESTS+=("$test_name")
        return 1
    fi
    
    # Ejecutar las pruebas y capturar la salida
    if output=$("$test_exe" 2>&1); then
        echo "$output"
        
        # Extraer estadísticas
        local tests_run=$(echo "$output" | grep -o '\[[0-9]* tests\]' | grep -o '[0-9]*' | head -1)
        local tests_passed=$(echo "$output" | grep 'PASSED' | grep -o '[0-9]*' | head -1)
        
        if [ -n "$tests_run" ] && [ -n "$tests_passed" ]; then
            TOTAL_TESTS=$((TOTAL_TESTS + tests_run))
            TOTAL_PASSED=$((TOTAL_PASSED + tests_passed))
        fi
        
        echo -e "${GREEN}✓ $test_name: TODAS LAS PRUEBAS PASARON${NC}"
        return 0
    else
        echo "$output"
        echo -e "${RED}✗ $test_name: ALGUNAS PRUEBAS FALLARON${NC}"
        FAILED_TESTS+=("$test_name")
        return 1
    fi
}

# Ejecutar todas las suites de pruebas
echo "Iniciando ejecución de pruebas unitarias..."
echo

# Pruebas de common (librería común)
run_test_suite "COMMON" "$UTCOMMON_EXE"
echo

# Pruebas de PAR (Parallel)
run_test_suite "PAR" "$UTPAR_EXE"
echo

# Resumen final
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}         RESUMEN DE PRUEBAS             ${NC}"
echo -e "${BLUE}========================================${NC}"

if [ ${#FAILED_TESTS[@]} -eq 0 ]; then
    echo -e "${GREEN}✓ TODAS LAS PRUEBAS PASARON${NC}"
    echo -e "Total de pruebas ejecutadas: ${TOTAL_TESTS}"
    echo -e "Total de pruebas exitosas: ${TOTAL_PASSED}"
    exit 0
else
    echo -e "${RED}✗ ALGUNAS PRUEBAS FALLARON${NC}"
    echo -e "Suites con fallos: ${FAILED_TESTS[*]}"
    echo -e "Total de pruebas ejecutadas: ${TOTAL_TESTS}"
    echo -e "Total de pruebas exitosas: ${TOTAL_PASSED}"
    exit 1
fi