#!/usr/bin/env python3
"""
Script de automatización de pruebas unitarias
Ejecuta todas las pruebas unitarias del proyecto
"""

import os
import subprocess
import sys
import re
from pathlib import Path
from typing import Tuple, Optional

# Colores para la salida
class Colors:
    GREEN = '\033[0;32m'
    RED = '\033[0;31m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    NC = '\033[0m'  # No Color

def print_header():
    """Imprime el encabezado del script"""
    print(f"{Colors.BLUE}========================================{Colors.NC}")
    print(f"{Colors.BLUE}  AUTOMATIZACIÓN DE PRUEBAS UNITARIAS  {Colors.NC}")
    print(f"{Colors.BLUE}========================================{Colors.NC}")
    print()

def ensure_project_built(project_root: Path):
    """Asegura que el proyecto esté compilado"""
    build_dir = project_root / "out" / "build" / "default"
    
    if not build_dir.exists():
        print(f"{Colors.YELLOW}Compilando el proyecto...{Colors.NC}")
        subprocess.run(["cmake", "--preset", "default"], cwd=project_root, check=True)
        subprocess.run(["cmake", "--build", "--preset", "gcc-debug"], cwd=project_root, check=True)

def find_test_executables(project_root: Path) -> dict:
    """Encuentra los ejecutables de pruebas"""
    base_path = project_root / "out" / "build" / "default"
    
    executables = {
        "COMMON": base_path / "utcommon" / "Debug" / "utcommon",
        "PAR": base_path / "utpar" / "Debug" / "utpar"
    }
    
    # Verificar que todos los ejecutables existen
    missing = [name for name, path in executables.items() if not path.exists()]
    
    if missing:
        print(f"{Colors.RED}Error: No se encontraron los ejecutables de pruebas para: {', '.join(missing)}{Colors.NC}")
        print("Recompilando...")
        subprocess.run(["cmake", "--build", "--preset", "gcc-debug"], cwd=project_root, check=True)
        
        # Verificar de nuevo
        still_missing = [name for name, path in executables.items() if not path.exists()]
        if still_missing:
            raise FileNotFoundError(f"No se pudieron generar los ejecutables: {', '.join(still_missing)}")
    
    return executables

def parse_test_output(output: str) -> Tuple[int, int]:
    """Parsea la salida de GoogleTest para extraer estadísticas"""
    # Buscar línea como "[==========] Running 114 tests from 13 test suites."
    running_match = re.search(r'\[==========\] Running (\d+) tests from \d+ test suites?\.', output)
    
    # Buscar línea como "[  PASSED  ] 114 tests."
    passed_match = re.search(r'\[\s+PASSED\s+\] (\d+) tests?\.', output)
    
    tests_run = int(running_match.group(1)) if running_match else 0
    tests_passed = int(passed_match.group(1)) if passed_match else 0
    
    return tests_run, tests_passed

def run_test_suite(name: str, executable: Path) -> Tuple[bool, int, int]:
    """Ejecuta una suite de pruebas y retorna (success, tests_run, tests_passed)"""
    print(f"{Colors.BLUE}Ejecutando pruebas de {name}...{Colors.NC}")
    print("=========================================")
    
    try:
        result = subprocess.run(
            [str(executable)], 
            capture_output=True, 
            text=True, 
            timeout=300  # 5 minutos de timeout
        )
        
        # Imprimir la salida
        print(result.stdout)
        if result.stderr:
            print(result.stderr)
        
        # Parsear estadísticas
        tests_run, tests_passed = parse_test_output(result.stdout)
        
        if result.returncode == 0:
            print(f"{Colors.GREEN}✓ {name}: TODAS LAS PRUEBAS PASARON{Colors.NC}")
            return True, tests_run, tests_passed
        else:
            print(f"{Colors.RED}✗ {name}: ALGUNAS PRUEBAS FALLARON{Colors.NC}")
            return False, tests_run, tests_passed
            
    except subprocess.TimeoutExpired:
        print(f"{Colors.RED}✗ {name}: TIMEOUT - Las pruebas tardaron demasiado{Colors.NC}")
        return False, 0, 0
    except Exception as e:
        print(f"{Colors.RED}✗ {name}: ERROR - {str(e)}{Colors.NC}")
        return False, 0, 0

def main():
    """Función principal"""
    project_root = Path(__file__).parent
    os.chdir(project_root)
    
    print_header()
    
    try:
        # Asegurar que el proyecto esté compilado
        ensure_project_built(project_root)
        
        # Encontrar ejecutables
        executables = find_test_executables(project_root)
        
        print("Iniciando ejecución de pruebas unitarias...")
        print()
        
        # Ejecutar todas las suites
        total_tests = 0
        total_passed = 0
        failed_suites = []
        
        for suite_name, executable in executables.items():
            success, tests_run, tests_passed = run_test_suite(suite_name, executable)
            
            total_tests += tests_run
            total_passed += tests_passed
            
            if not success:
                failed_suites.append(suite_name)
            
            print()
        
        # Resumen final
        print(f"{Colors.BLUE}========================================{Colors.NC}")
        print(f"{Colors.BLUE}         RESUMEN DE PRUEBAS             {Colors.NC}")
        print(f"{Colors.BLUE}========================================{Colors.NC}")
        
        if not failed_suites:
            print(f"{Colors.GREEN}✓ TODAS LAS PRUEBAS PASARON{Colors.NC}")
            print(f"Total de pruebas ejecutadas: {total_tests}")
            print(f"Total de pruebas exitosas: {total_passed}")
            return 0
        else:
            print(f"{Colors.RED}✗ ALGUNAS PRUEBAS FALLARON{Colors.NC}")
            print(f"Suites con fallos: {', '.join(failed_suites)}")
            print(f"Total de pruebas ejecutadas: {total_tests}")
            print(f"Total de pruebas exitosas: {total_passed}")
            return 1
            
    except Exception as e:
        print(f"{Colors.RED}Error fatal: {str(e)}{Colors.NC}")
        return 1

if __name__ == "__main__":
    sys.exit(main())