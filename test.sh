#!/bin/bash

# Script de pruebas automatizadas para el juego HEX

echo "=========================================="
echo "  PRUEBAS AUTOMÁTICAS - JUEGO HEX"
echo "=========================================="
echo ""

TESTS_PASSED=0
TESTS_FAILED=0

# Función para reportar test
test_result() {
   if [ $1 -eq 0 ]; then
      echo "✓ $2"
      ((TESTS_PASSED++))
   else
      echo "✗ $2"
      ((TESTS_FAILED++))
   fi
}

# Test 1: Verificar archivos fuente
echo "[1] Verificando archivos fuente..."
FILES="hex_game.h hex_game.c hex_ai.c hex_gui.c hex_console.c pcg_basic.h pcg_basic.c Makefile"
MISSING=0
for file in $FILES; do
   if [ ! -f "$file" ]; then
      echo "  Falta: $file"
      MISSING=1
   fi
done
test_result $MISSING "Archivos fuente completos"

# Test 2: Compilación limpia
echo ""
echo "[2] Compilando proyecto..."
make clean > /dev/null 2>&1
if make > /tmp/hex_compile.log 2>&1; then
   test_result 0 "Compilación exitosa"
else
   test_result 1 "Compilación fallida (ver /tmp/hex_compile.log)"
fi

# Test 3: Verificar ejecutables
echo ""
echo "[3] Verificando ejecutables..."
if [ -x "./hex" ] && [ -x "./hex_console" ]; then
   test_result 0 "Ejecutables generados"
else
   test_result 1 "Ejecutables no encontrados"
fi

# Test 4: Test básico de consola (auto-play simulado)
echo ""
echo "[4] Test funcional básico..."
# Crear input simulado (jugadas rápidas para terminar)
echo -e "quit\n" | timeout 5 ./hex_console -t 1 -s 10 > /tmp/hex_test.log 2>&1
if [ $? -eq 0 ] || [ $? -eq 124 ]; then  # 124 = timeout (esperado)
   test_result 0 "Ejecución de consola funcional"
else
   test_result 1 "Error en ejecución de consola"
fi

# Test 5: Verificar paralelismo (threads)
echo ""
echo "[5] Test de paralelismo..."
echo -e "A1\nquit\n" | timeout 10 ./hex_console -t 4 -s 50 > /tmp/hex_threads.log 2>&1
if grep -q "threads = 4" /tmp/hex_threads.log 2>/dev/null; then
   test_result 0 "Threads configurados correctamente"
else
   test_result 1 "Problema con configuración de threads"
fi

# Test 6: Verificar que la IA genera movimientos
echo ""
echo "[6] Test de IA..."
echo -e "A1\nquit\n" | timeout 15 ./hex_console -t 2 -s 100 > /tmp/hex_ai.log 2>&1
if grep -q "IA juega:" /tmp/hex_ai.log 2>/dev/null; then
   test_result 0 "IA genera movimientos"
else
   test_result 1 "IA no generó movimientos"
fi

# Test 7: Test de memory leaks (si valgrind está instalado)
echo ""
echo "[7] Test de memory leaks..."
if command -v valgrind &> /dev/null; then
   echo -e "quit\n" | timeout 20 valgrind --leak-check=full --error-exitcode=1 \
      ./hex_console -t 2 -s 10 > /tmp/hex_valgrind.log 2>&1
   if [ $? -eq 0 ]; then
      test_result 0 "Sin memory leaks detectados"
   else
      test_result 1 "Posibles memory leaks (ver /tmp/hex_valgrind.log)"
   fi
else
   echo "  (valgrind no instalado, test omitido)"
fi

# Test 8: Test de rendimiento básico
echo ""
echo "[8] Test de rendimiento..."
START=$(date +%s)
echo -e "A1\nB1\nquit\n" | timeout 30 ./hex_console -t 4 -s 500 > /tmp/hex_perf.log 2>&1
END=$(date +%s)
DURATION=$((END - START))

if [ $DURATION -lt 25 ]; then
   test_result 0 "Rendimiento aceptable (${DURATION}s)"
else
   test_result 1 "Rendimiento lento (${DURATION}s)"
fi

# Test 9: Verificar documentación
echo ""
echo "[9] Verificando documentación..."
DOCS="README.md REPORTE.md PRESENTACION.md"
MISSING_DOCS=0
for doc in $DOCS; do
   if [ ! -f "$doc" ]; then
      MISSING_DOCS=1
   fi
done
test_result $MISSING_DOCS "Documentación completa"

# Test 10: Test de ayuda
echo ""
echo "[10] Test de ayuda..."
./hex_console -h > /tmp/hex_help.log 2>&1
if grep -q "usage:" /tmp/hex_help.log; then
   test_result 0 "Mensaje de ayuda funcional"
else
   test_result 1 "Problema con mensaje de ayuda"
fi

# Resumen
echo ""
echo "=========================================="
echo "           RESUMEN DE PRUEBAS"
echo "=========================================="
echo "Tests exitosos: $TESTS_PASSED"
echo "Tests fallidos: $TESTS_FAILED"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
   echo "✓ TODAS LAS PRUEBAS PASARON"
   echo "El proyecto está listo para presentación."
   exit 0
else
   echo "✗ ALGUNAS PRUEBAS FALLARON"
   echo "Revisa los logs en /tmp/hex_*.log"
   exit 1
fi
