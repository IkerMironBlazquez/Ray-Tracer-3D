#!/usr/bin/env python3
"""
Script para comparar 2 imágenes PPM según los umbrales de aceptación recibidos...

Umbrales de aceptación:
1. La diferencia máxima en un pixel debe estar por debajo de 150
   - dif = (abs(r1-r2) + abs(g1-g2) + abs(b1-b2)) / 3
2. El error cuadrático medio debe ser inferior a 10
   - RMSE = sqrt(suma de diferencias al cuadrado por pixel)

Deben cumplirse AMBAS condiciones para que las imágenes sean aceptables.

<LAS MÉTRICAS ESTÁN DESACTUALIZADAS PARA ESTA PARTE DE LA PRÁCTICA>
"""

import sys
import os
import math


def read_ppm(filename):
    """Archivo PPM y devuelve (ancho, alto, pixels)."""
    try:
        with open(filename, 'rb') as f:
            # Leer header
            magic = f.readline().decode('ascii').strip()
            
            # Soportar tanto P3 (ASCII) como P6 (binario)
            if magic not in ['P3', 'P6']:
                print(f"Error: {filename} no es un archivo PPM válido (debe ser P3 o P6)")
                return None
            
            is_binary = (magic == 'P6')
            
            # Saltar comentarios
            line = f.readline().decode('ascii').strip()
            while line.startswith('#'):
                line = f.readline().decode('ascii').strip()
            
            # Leer dimensiones
            width, height = map(int, line.split())
            
            # Leer valor máximo
            maxval = int(f.readline().decode('ascii').strip())
            if maxval != 255:
                print(f"Advertencia: {filename} tiene maxval={maxval}, esperado 255")
            
            # Leer datos de píxeles según el formato
            pixels = []
            
            if is_binary:
                # Formato P6 (binario)
                pixel_data = f.read()
                expected_size = width * height * 3
                if len(pixel_data) != expected_size:
                    print(f"Error: {filename} tiene {len(pixel_data)} bytes, esperados {expected_size}")
                    return None
                
                for i in range(0, len(pixel_data), 3):
                    r = pixel_data[i]
                    g = pixel_data[i + 1]
                    b = pixel_data[i + 2]
                    pixels.append((r, g, b))
            else:
                # Formato P3 (ASCII)
                remaining_data = f.read().decode('ascii')
                values = remaining_data.split()
                
                if len(values) != width * height * 3:
                    print(f"Error: {filename} tiene {len(values)} valores, esperados {width * height * 3}")
                    return None
                
                for i in range(0, len(values), 3):
                    r = int(values[i])
                    g = int(values[i + 1])
                    b = int(values[i + 2])
                    pixels.append((r, g, b))
            
            return (width, height, pixels)
    
    except FileNotFoundError:
        print(f"Error: Archivo {filename} no encontrado")
        return None
    except Exception as e:
        print(f"Error al leer {filename}: {e}")
        return None


def compare_images(file1, file2, verbose=False):
    """
    Compara dos imágenes PPM según los umbrales.
    
    Criterios de aceptación:
    1. Diferencia máxima por pixel < 150
    2. Error cuadrático medio (RMSE) < 10
    
    Retorna True si las imágenes son aceptables, False si no.
    """
    print(f"\n{'='*70}")
    print(f"COMPARACIÓN DE IMÁGENES PPM - UMBRALES")
    print(f"{'='*70}")
    print(f"Imagen 1: {file1}")
    print(f"Imagen 2: {file2}")
    print(f"{'='*70}\n")
    
    img1 = read_ppm(file1)
    img2 = read_ppm(file2)
    
    if img1 is None or img2 is None:
        print("Excepción de lectura")
        return None
    
    width1, height1, pixels1 = img1
    width2, height2, pixels2 = img2
    
    # Verificar dimensiones
    print(f"Dimensiones imagen 1: {width1}x{height1}")
    print(f"Dimensiones imagen 2: {width2}x{height2}\n")
    
    if width1 != width2 or height1 != height2:
        print(f"Excepción de dimensiones")
        print(f"  Imagen 1: {width1}x{height1}")
        print(f"  Imagen 2: {width2}x{height2}")
        return None
    
    # Comparar píxel por píxel según especificaciones del profesor
    total_pixels = width1 * height1
    
    max_pixel_diff = 0.0  # Diferencia máxima en un pixel
    sum_squared_diffs = 0.0  # Suma de diferencias al cuadrado por pixel
    
    different_pixels = 0
    pixel_diffs = []  # Para mostrar detalles si verbose=True
    
    for i in range(total_pixels):
        r1, g1, b1 = pixels1[i]
        r2, g2, b2 = pixels2[i]
        
        # Calcular diferencia del pixel según fórmula del profesor:
        # dif = (abs(r1-r2) + abs(g1-g2) + abs(b1-b2)) / 3
        pixel_diff = (abs(r1 - r2) + abs(g1 - g2) + abs(b1 - b2)) / 3.0
        
        # Actualizar diferencia máxima
        if pixel_diff > max_pixel_diff:
            max_pixel_diff = pixel_diff
        
        # Calcular cuadrado de la diferencia para RMSE
        sum_squared_diffs += pixel_diff * pixel_diff
        
        # Contar píxeles diferentes (para estadísticas)
        if pixel_diff > 0:
            different_pixels += 1
            row = i // width1
            col = i % width1
            pixel_diffs.append((row, col, pixels1[i], pixels2[i], pixel_diff))
    
    # Calcular error cuadrático medio (RMSE)
    # RMSE = sqrt(suma de diferencias al cuadrado)
    rmse = math.sqrt(sum_squared_diffs)
    
    # Calcular porcentaje de píxeles diferentes (estadística adicional)
    pixel_diff_percent = (different_pixels / total_pixels) * 100
    
    # Mostrar resultados
    print(f"{'='*70}")
    print(f"RESULTADOS SEGÚN UMBRALES DEL PROFESOR")
    print(f"{'='*70}")
    print(f"Total de píxeles:                    {total_pixels:,}")
    print(f"Píxeles diferentes:                  {different_pixels:,} ({pixel_diff_percent:.2f}%)")
    print(f"")
    print(f"CRITERIO 1: Diferencia máxima por pixel")
    print(f"  Valor calculado:                   {max_pixel_diff:.4f}")
    print(f"  Umbral máximo permitido:           150.0")
    criterio1_ok = max_pixel_diff < 150.0
    print(f"  Estado:                            {'✓ CUMPLE' if criterio1_ok else '✗ NO CUMPLE'}")
    print(f"")
    print(f"CRITERIO 2: Error cuadrático medio (RMSE)")
    print(f"  Valor calculado:                   {rmse:.4f}")
    print(f"  Umbral máximo permitido:           10.0")
    criterio2_ok = rmse < 10.0
    print(f"  Estado:                            {'✓ CUMPLE' if criterio2_ok else '✗ NO CUMPLE'}")
    print(f"{'='*70}\n")
    
    # Resultado final (deben cumplirse AMBOS criterios)
    aceptable = criterio1_ok and criterio2_ok
    
    if different_pixels == 0:
        print("✓✓✓ RESULTADO: Las imágenes son IDÉNTICAS")
    elif aceptable:
        print("✓ RESULTADO: Las imágenes son ACEPTABLES")
        print(f"  → Ambos criterios se cumplen")
    else:
        print(f"✗ RESULTADO: Las imágenes NO SON ACEPTABLES")
        if not criterio1_ok:
            print(f"  → Criterio 1 FALLA: diferencia máxima {max_pixel_diff:.4f} >= 150")
        if not criterio2_ok:
            print(f"  → Criterio 2 FALLA: RMSE {rmse:.4f} >= 10")
        
        if verbose and len(pixel_diffs) > 0:
            print(f"\nPíxeles con mayor diferencia (primeros 20):")
            # Ordenar por diferencia descendente
            pixel_diffs.sort(key=lambda x: x[4], reverse=True)
            for row, col, px1, px2, diff in pixel_diffs[:20]:
                print(f"  Posición ({row:4d},{col:4d}): "
                      f"RGB1={px1} RGB2={px2} diff={diff:.2f}")
    
    print(f"{'='*70}\n")
    
    return aceptable


def main():
    if len(sys.argv) < 3:
        print("Uso: python compare_ppm.py <imagen1.ppm> <imagen2.ppm> [--verbose]")
        print("\nUmbrales de aceptación del profesor:")
        print("  1. Diferencia máxima por pixel < 150")
        print("  2. Error cuadrático medio (RMSE) < 10")
        print("\nAmbos criterios deben cumplirse para que las imágenes sean aceptables.")
        sys.exit(1)
    
    file1 = sys.argv[1]
    file2 = sys.argv[2]
    verbose = "--verbose" in sys.argv or "-v" in sys.argv
    
    result = compare_images(file1, file2, verbose)
    
    if result is None:
        sys.exit(1)  # Error al leer archivos
    elif result is True:
        sys.exit(0)  # Imágenes aceptables (idénticas o dentro de umbrales)
    else:
        sys.exit(2)  # Imágenes no aceptables


if __name__ == "__main__":
    main()
