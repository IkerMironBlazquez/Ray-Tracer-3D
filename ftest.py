#!/usr/bin/env python3
"""
Script de automatización de pruebas funcionales
Prueba el funcionamiento completo de la aplicación render-par
"""

import os
import subprocess
import sys
import tempfile
import shutil
from pathlib import Path
from typing import Tuple, List, NamedTuple

# Colores para la salida
class Colors:
    GREEN = '\033[0;32m'
    RED = '\033[0;31m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    NC = '\033[0m'  # No Color

class TestResult(NamedTuple):
    name: str
    app: str
    passed: bool
    message: str

def print_header():
    """Imprime el encabezado del script"""
    print(f"{Colors.BLUE}========================================{Colors.NC}")
    print(f"{Colors.BLUE}  AUTOMATIZACIÓN DE PRUEBAS FUNCIONALES {Colors.NC}")
    print(f"{Colors.BLUE}========================================{Colors.NC}")
    print()

def ensure_project_built(project_root: Path):
    """Asegura que el proyecto esté compilado"""
    build_dir = project_root / "out" / "build" / "default"
    
    if not build_dir.exists():
        print(f"{Colors.YELLOW}Compilando el proyecto...{Colors.NC}")
        subprocess.run(["cmake", "--preset", "default"], cwd=project_root, check=True)
        subprocess.run(["cmake", "--build", "--preset", "gcc-debug"], cwd=project_root, check=True)

def find_executables(project_root: Path) -> Path:
    """Encuentra el ejecutable render-par"""
    base_path = project_root / "out" / "build" / "default"
    
    render_par = base_path / "par" / "Debug" / "render-par"
    
    if not render_par.exists():
        print(f"{Colors.RED}Error: No se encontró el ejecutable render-par{Colors.NC}")
        print("Recompilando...")
        subprocess.run(["cmake", "--build", "--preset", "gcc-debug"], cwd=project_root, check=True)
        
        if not render_par.exists():
            raise FileNotFoundError("No se pudo generar el ejecutable render-par")
    
    return render_par

def verify_ppm_format(ppm_file: Path) -> bool:
    """Verifica que un archivo tenga formato PPM válido básico"""
    try:
        with open(ppm_file, 'r') as f:
            first_line = f.readline().strip()
            if first_line != "P3":
                return False
            
            dimensions_line = f.readline().strip()
            if not dimensions_line or len(dimensions_line.split()) != 2:
                return False
                
            max_val_line = f.readline().strip()
            if max_val_line != "255":
                return False
                
        return ppm_file.stat().st_size > 0
    except Exception:
        return False

def run_functional_test(
    test_name: str,
    app_name: str,
    executable: Path,
    config_file: str,
    scene_file: str,
    output_file: Path,
    expect_success: bool,
    project_root: Path
) -> TestResult:
    """Ejecuta una prueba funcional individual"""
    
    print(f"{Colors.BLUE}[TEST] {test_name} ({app_name}){Colors.NC}")
    
    try:
        # Preparar argumentos
        args = []
        if config_file:
            args.append(str(project_root / config_file))
        if scene_file:
            args.append(str(project_root / scene_file))
        if output_file:
            args.append(str(output_file))
        
        # Ejecutar comando con timeout
        result = subprocess.run(
            [str(executable)] + args,
            capture_output=True,
            text=True,
            timeout=30,
            cwd=project_root
        )
        
        success = result.returncode == 0
        
        if expect_success:
            # Esperamos éxito
            if success and output_file and output_file.exists():
                if verify_ppm_format(output_file):
                    size = output_file.stat().st_size
                    message = f"✓ ÉXITO - Imagen generada correctamente ({size} bytes)"
                    print(f"  {Colors.GREEN}{message}{Colors.NC}")
                    return TestResult(test_name, app_name, True, message)
                else:
                    message = "✗ FALLO - Archivo PPM inválido o vacío"
                    print(f"  {Colors.RED}{message}{Colors.NC}")
                    return TestResult(test_name, app_name, False, message)
            else:
                message = f"✗ FALLO - No se generó la imagen (exit code: {result.returncode})"
                print(f"  {Colors.RED}{message}{Colors.NC}")
                if result.stderr:
                    print(f"    Error: {result.stderr.strip()}")
                return TestResult(test_name, app_name, False, message)
        else:
            # Esperamos fallo
            if not success:
                message = f"✓ ÉXITO - Falló como se esperaba (exit code: {result.returncode})"
                print(f"  {Colors.GREEN}{message}{Colors.NC}")
                return TestResult(test_name, app_name, True, message)
            else:
                message = "✗ FALLO - Debería haber fallado pero tuvo éxito"
                print(f"  {Colors.RED}{message}{Colors.NC}")
                return TestResult(test_name, app_name, False, message)
                
    except subprocess.TimeoutExpired:
        message = "✗ FALLO - Timeout (>30s)"
        print(f"  {Colors.RED}{message}{Colors.NC}")
        return TestResult(test_name, app_name, False, message)
    except Exception as e:
        message = f"✗ FALLO - Error: {str(e)}"
        print(f"  {Colors.RED}{message}{Colors.NC}")
        return TestResult(test_name, app_name, False, message)
    finally:
        print()

def main():
    """Función principal"""
    project_root = Path(__file__).parent
    os.chdir(project_root)
    
    print_header()
    
    try:
        # Asegurar que el proyecto esté compilado
        ensure_project_built(project_root)
        
        # Encontrar ejecutable
        render_par = find_executables(project_root)
        
        # Los archivos se generarán en el directorio raíz
        output_dir = project_root
        
        print(f"Directorio de trabajo: {output_dir}")
        print()
        
        results: List[TestResult] = []
        
        # ============================================================================
        # CASOS DE ÉXITO
        # ============================================================================
        
        print(f"{Colors.BLUE}=== CASOS DE ÉXITO ==={Colors.NC}")
        print()
        
        success_tests = [
            ("Caso básico", "test_files/test_config.txt", "test_files/test_scene.txt", "basic"),
            ("Escena compleja", "test_files/test_config.txt", "test_files/test_scene_complex.txt", "complex"),
            ("Config por defecto", "test_files/test_config_defaults.txt", "test_files/test_scene.txt", "defaults"),
            ("Config vacía", "test_files/test_config_empty.txt", "test_files/test_scene.txt", "empty_config"),
        ]
        
        for test_name, config, scene, output_prefix in success_tests:
            output_file = output_dir / f"{output_prefix}_par.ppm"
            result = run_functional_test(
                test_name, "PAR", render_par, config, scene, output_file, True, project_root
            )
            results.append(result)
        
        # ============================================================================
        # CASOS DE ERROR
        # ============================================================================
        
        print(f"{Colors.BLUE}=== CASOS DE ERROR ==={Colors.NC}")
        print()
        
        error_tests = [
            ("Sin argumentos", "", "", None),
            ("Pocos argumentos", "test_files/test_config.txt", "", None),
            ("Config inexistente", "nonexistent_config.txt", "test_files/test_scene.txt", "nonexistent_config"),
            ("Scene inexistente", "test_files/test_config.txt", "nonexistent_scene.txt", "nonexistent_scene"),
        ]
        
        # Pruebas de archivos con errores (si existen)
        for i in range(1, 7):
            config_error = f"test_files/test_config_error{i}.txt"
            if (project_root / config_error).exists():
                error_tests.append((f"Config error {i}", config_error, "test_files/test_scene.txt", f"config_error{i}"))
        
        for i in range(1, 8):
            scene_error = f"test_files/test_scene_error{i}.txt"  
            if (project_root / scene_error).exists():
                error_tests.append((f"Scene error {i}", "test_files/test_config.txt", scene_error, f"scene_error{i}"))
        
        for test_name, config, scene, output_prefix in error_tests:
            output_file = output_dir / f"{output_prefix}_par.ppm" if output_prefix else None
            result = run_functional_test(
                test_name, "PAR", render_par, config, scene, output_file, False, project_root
            )
            results.append(result)
        
        # ============================================================================
        # RESUMEN FINAL
        # ============================================================================
        
        print(f"{Colors.BLUE}========================================{Colors.NC}")
        print(f"{Colors.BLUE}      RESUMEN DE PRUEBAS FUNCIONALES    {Colors.NC}")
        print(f"{Colors.BLUE}========================================{Colors.NC}")
        
        total_tests = len(results)
        passed_tests = sum(1 for r in results if r.passed)
        failed_tests = [r for r in results if not r.passed]
        
        print(f"Archivos de resultado guardados en: {output_dir}/")
        print(f"Total de pruebas ejecutadas: {total_tests}")
        print(f"Total de pruebas exitosas: {passed_tests}")
        
        if not failed_tests:
            print(f"{Colors.GREEN}✓ TODAS LAS PRUEBAS FUNCIONALES PASARON{Colors.NC}")
            print()
            print("Imágenes generadas exitosamente:")
            ppm_files = list(output_dir.glob("*.ppm"))[:10]
            for ppm_file in ppm_files:
                size = ppm_file.stat().st_size
                print(f"  {ppm_file.name} ({size} bytes)")
            return 0
        else:
            print(f"{Colors.RED}✗ ALGUNAS PRUEBAS FUNCIONALES FALLARON{Colors.NC}")
            print("Pruebas fallidas:")
            for test in failed_tests:
                print(f"  - {test.name} ({test.app}): {test.message}")
            return 1
            
    except Exception as e:
        print(f"{Colors.RED}Error fatal: {str(e)}{Colors.NC}")
        return 1

if __name__ == "__main__":
    sys.exit(main())