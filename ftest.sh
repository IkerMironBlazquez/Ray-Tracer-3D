#!/bin/bash

# Script de automatización de pruebas funcionales
# Prueba el funcionamiento completo de la aplicación render-par

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
echo -e "${BLUE}  AUTOMATIZACIÓN DE PRUEBAS FUNCIONALES ${NC}"
echo -e "${BLUE}========================================${NC}"
echo

# Verificar que el proyecto esté compilado
if [ ! -d "out/build/default" ]; then
    echo -e "${YELLOW}Compilando el proyecto...${NC}"
    cmake --preset default
    cmake --build --preset gcc-debug
fi

# Rutas a ejecutables
RENDER_PAR="./out/build/default/par/Debug/render-par"

# Verificar que el ejecutable existe
if [ ! -f "$RENDER_PAR" ]; then
    echo -e "${RED}Error: No se encontró el ejecutable render-par${NC}"
    echo "Recompilando..."
    cmake --build --preset gcc-debug
fi

# Los archivos se generarán en el directorio de trabajo actual
OUTPUT_DIR="."

echo "Directorio de trabajo: $OUTPUT_DIR"
echo

# Contadores
TESTS_TOTAL=0
TESTS_PASSED=0
FAILED_TESTS=()

# Función para ejecutar una prueba funcional
run_functional_test() {
    local test_name="$1"
    local app_name="$2"
    local executable="$3"
    local config_file="$4"
    local scene_file="$5"
    local output_file="$6"
    local expect_success="$7"  # true/false
    
    TESTS_TOTAL=$((TESTS_TOTAL + 1))
    
    echo -e "${BLUE}[TEST $TESTS_TOTAL] $test_name ($app_name)${NC}"
    
    local full_output="$OUTPUT_DIR/$output_file"
    
    # Ejecutar el comando y capturar salida
    if timeout 30 "$executable" "$config_file" "$scene_file" "$full_output" >/dev/null 2>&1; then
        local exit_code=0
    else
        local exit_code=$?
    fi
    
    if [ "$expect_success" = "true" ]; then
        # Esperamos éxito
        if [ $exit_code -eq 0 ] && [ -f "$full_output" ]; then
            # Verificar que el archivo PPM tenga contenido válido
            if head -3 "$full_output" | grep -q "P3" && [ -s "$full_output" ]; then
                echo -e "  ${GREEN}✓ ÉXITO - Imagen generada correctamente${NC}"
                TESTS_PASSED=$((TESTS_PASSED + 1))
                
                # Verificar formato PPM básico
                local header_lines=$(head -3 "$full_output")
                echo "    Formato PPM verificado: $(echo "$header_lines" | tr '\n' ' ')"
                echo "    Tamaño: $(wc -c < "$full_output") bytes"
            else
                echo -e "  ${RED}✗ FALLO - Archivo PPM inválido o vacío${NC}"
                FAILED_TESTS+=("$test_name")
            fi
        else
            echo -e "  ${RED}✗ FALLO - No se generó la imagen (exit code: $exit_code)${NC}"
            FAILED_TESTS+=("$test_name")
        fi
    else
        # Esperamos fallo
        if [ $exit_code -ne 0 ]; then
            echo -e "  ${GREEN}✓ ÉXITO - Falló como se esperaba (exit code: $exit_code)${NC}"
            TESTS_PASSED=$((TESTS_PASSED + 1))
        else
            echo -e "  ${RED}✗ FALLO - Debería haber fallado pero tuvo éxito${NC}"
            FAILED_TESTS+=("$test_name")
        fi
    fi
    echo
}

# ============================================================================
# PRUEBAS DE CASOS DE ÉXITO
# ============================================================================

echo -e "${BLUE}=== CASOS DE ÉXITO ===${NC}"
echo

# Caso básico con archivos de prueba estándar
run_functional_test "Caso básico" "PAR" "$RENDER_PAR" \
    "test_files/test_config.txt" "test_files/test_scene.txt" "basic_par.ppm" true

# Escena compleja
run_functional_test "Escena compleja" "PAR" "$RENDER_PAR" \
    "test_files/test_config.txt" "test_files/test_scene_complex.txt" "complex_par.ppm" true

# Configuración con valores por defecto
run_functional_test "Config por defecto" "PAR" "$RENDER_PAR" \
    "test_files/test_config_defaults.txt" "test_files/test_scene.txt" "defaults_par.ppm" true

# Configuración vacía (debe usar defaults)
run_functional_test "Config vacía" "PAR" "$RENDER_PAR" \
    "test_files/test_config_empty.txt" "test_files/test_scene.txt" "empty_config_par.ppm" true

# ============================================================================
# PRUEBAS DE CASOS DE ERROR
# ============================================================================

echo -e "${BLUE}=== CASOS DE ERROR ===${NC}"
echo

# Número incorrecto de argumentos
run_functional_test "Sin argumentos" "PAR" "$RENDER_PAR" \
    "" "" "" false

run_functional_test "Pocos argumentos" "PAR" "$RENDER_PAR" \
    "test_files/test_config.txt" "" "" false

# Archivos de configuración con errores
run_functional_test "Config error 1" "PAR" "$RENDER_PAR" \
    "test_files/test_config_error1.txt" "test_files/test_scene.txt" "error1_par.ppm" false

run_functional_test "Config error 2" "PAR" "$RENDER_PAR" \
    "test_files/test_config_error2.txt" "test_files/test_scene.txt" "error2_par.ppm" false

# Archivos de escena con errores
run_functional_test "Scene error 1" "PAR" "$RENDER_PAR" \
    "test_files/test_config.txt" "test_files/test_scene_error1.txt" "scene_error1_par.ppm" false

run_functional_test "Scene error 2" "PAR" "$RENDER_PAR" \
    "test_files/test_config.txt" "test_files/test_scene_error2.txt" "scene_error2_par.ppm" false

# Archivos inexistentes
run_functional_test "Config inexistente" "PAR" "$RENDER_PAR" \
    "nonexistent_config.txt" "test_files/test_scene.txt" "nonexistent_par.ppm" false

run_functional_test "Scene inexistente" "PAR" "$RENDER_PAR" \
    "test_files/test_config.txt" "nonexistent_scene.txt" "nonexistent_par.ppm" false

# ============================================================================
# RESUMEN FINAL
# ============================================================================

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}      RESUMEN DE PRUEBAS FUNCIONALES    ${NC}"
echo -e "${BLUE}========================================${NC}"

echo "Archivos de resultado guardados en: $OUTPUT_DIR/"
echo "Total de pruebas ejecutadas: $TESTS_TOTAL"
echo "Total de pruebas exitosas: $TESTS_PASSED"

if [ ${#FAILED_TESTS[@]} -eq 0 ]; then
    echo -e "${GREEN}✓ TODAS LAS PRUEBAS FUNCIONALES PASARON${NC}"
    echo
    echo "Imágenes generadas exitosamente:"
    ls -la "$OUTPUT_DIR"/*.ppm 2>/dev/null | head -10
    exit 0
else
    echo -e "${RED}✗ ALGUNAS PRUEBAS FUNCIONALES FALLARON${NC}"
    echo -e "Pruebas fallidas: ${FAILED_TESTS[*]}"
    exit 1
fi