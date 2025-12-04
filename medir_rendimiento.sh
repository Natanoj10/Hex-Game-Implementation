#!/bin/bash

# Script de pruebas automatizadas para el proyecto HEX
# Mide tiempos de ejecución y genera datos para el reporte

echo "=========================================="
echo "  SCRIPT DE MEDICIÓN DE RENDIMIENTO"
echo "  Juego HEX con Monte Carlo Paralelo"
echo "=========================================="
echo ""

# Verificar que el ejecutable existe
if [ ! -f ./hex_console ]; then
   echo "ERROR: No se encuentra ./hex_console"
   echo "Ejecuta primero: make"
   exit 1
fi

# Crear directorio para resultados
RESULTS_DIR="resultados_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$RESULTS_DIR"
echo "Los resultados se guardarán en: $RESULTS_DIR"
echo ""

# Crear archivo de entrada para pruebas consistentes
# Esto asegura que todas las mediciones usen la misma secuencia de jugadas
cat > "$RESULTS_DIR/input.txt" << 'EOF'
F6
E5
F5
G6
E6
quit
EOF

echo "Archivo de entrada creado: $RESULTS_DIR/input.txt"
echo ""

# ============================================================================
# PRUEBA 1: Speedup con simulaciones fijas
# ============================================================================

echo "=========================================="
echo "PRUEBA 1: Medición de Speedup"
echo "Simulaciones fijas: 1000 por thread"
echo "=========================================="
echo ""

SIMS=1000

for THREADS in 1 2 4 8 16; do
   echo "Ejecutando con $THREADS threads..."
   
   # Archivo de log individual
   LOG_FILE="$RESULTS_DIR/test_t${THREADS}_s${SIMS}.log"
   
   # Ejecutar 3 veces y promediar (para resultados más estables)
   TOTAL_TIME=0
   for RUN in 1 2 3; do
      echo "  Run $RUN/3..."
      
      # Capturar tiempo con /usr/bin/time (más preciso que bash time)
      # Formato: tiempo_real tiempo_usuario tiempo_sistema
      TIME_OUTPUT=$( { /usr/bin/time -f "%e %U %S" \
         ./hex_console -t $THREADS -s $SIMS < "$RESULTS_DIR/input.txt" \
         > /dev/null; } 2>&1 )
      
      # Extraer tiempo real
      REAL_TIME=$(echo $TIME_OUTPUT | awk '{print $1}')
      TOTAL_TIME=$(echo "$TOTAL_TIME + $REAL_TIME" | bc)
      
      echo "    Tiempo: ${REAL_TIME}s"
   done
   
   # Calcular promedio
   AVG_TIME=$(echo "scale=3; $TOTAL_TIME / 3" | bc)
   echo "  Promedio: ${AVG_TIME}s"
   echo ""
   
   # Guardar resultado
   echo "$THREADS $AVG_TIME" >> "$RESULTS_DIR/speedup_data.txt"
   
   # Guardar log detallado
   echo "Threads: $THREADS, Sims: $SIMS, Avg Time: ${AVG_TIME}s" >> "$LOG_FILE"
done

echo "Datos de speedup guardados en: $RESULTS_DIR/speedup_data.txt"
echo ""

# ============================================================================
# PRUEBA 2: Impacto de simulaciones (threads fijos)
# ============================================================================

echo "=========================================="
echo "PRUEBA 2: Impacto de Simulaciones"
echo "Threads fijos: 4"
echo "=========================================="
echo ""

THREADS=4

for SIMS in 100 500 1000 2000 5000; do
   echo "Ejecutando con $SIMS simulaciones..."
   
   LOG_FILE="$RESULTS_DIR/test_t${THREADS}_s${SIMS}.log"
   
   # Una sola ejecución (para ahorrar tiempo)
   TIME_OUTPUT=$( { /usr/bin/time -f "%e %U %S" \
      ./hex_console -t $THREADS -s $SIMS < "$RESULTS_DIR/input.txt" \
      > /dev/null; } 2>&1 )
   
   REAL_TIME=$(echo $TIME_OUTPUT | awk '{print $1}')
   echo "  Tiempo: ${REAL_TIME}s"
   echo ""
   
   # Guardar resultado
   echo "$SIMS $REAL_TIME" >> "$RESULTS_DIR/sims_impact_data.txt"
   
   # Guardar log detallado
   echo "Threads: $THREADS, Sims: $SIMS, Time: ${REAL_TIME}s" >> "$LOG_FILE"
done

echo "Datos de impacto de simulaciones guardados en: $RESULTS_DIR/sims_impact_data.txt"
echo ""

# ============================================================================
# PRUEBA 3: Eficiencia (comparación directa)
# ============================================================================

echo "=========================================="
echo "PRUEBA 3: Cálculo de Eficiencia"
echo "=========================================="
echo ""

# Leer tiempo de 1 thread como baseline
T1=$(head -1 "$RESULTS_DIR/speedup_data.txt" | awk '{print $2}')
echo "Tiempo con 1 thread (baseline): ${T1}s"
echo ""

# Calcular speedup y eficiencia para cada configuración
echo "Threads | Tiempo(s) | Speedup | Eficiencia(%)" > "$RESULTS_DIR/efficiency_report.txt"
echo "--------|-----------|---------|---------------" >> "$RESULTS_DIR/efficiency_report.txt"

while read THREADS TIME; do
   SPEEDUP=$(echo "scale=2; $T1 / $TIME" | bc)
   EFFICIENCY=$(echo "scale=2; ($SPEEDUP / $THREADS) * 100" | bc)
   
   printf "%7d | %9.3f | %7.2f | %13.2f\n" \
      $THREADS $TIME $SPEEDUP $EFFICIENCY \
      >> "$RESULTS_DIR/efficiency_report.txt"
   
   echo "Threads: $THREADS -> Speedup: ${SPEEDUP}x, Eficiencia: ${EFFICIENCY}%"
done < "$RESULTS_DIR/speedup_data.txt"

echo ""
echo "Reporte de eficiencia guardado en: $RESULTS_DIR/efficiency_report.txt"
echo ""

# ============================================================================
# GENERAR RESUMEN
# ============================================================================

echo "=========================================="
echo "GENERANDO RESUMEN"
echo "=========================================="
echo ""

cat > "$RESULTS_DIR/RESUMEN.txt" << EOF
RESUMEN DE MEDICIONES - JUEGO HEX
Fecha: $(date)
========================================

CONFIGURACIÓN:
- Ejecutable: ./hex_console
- Tablero: 11x11
- Entrada: input.txt (secuencia fija de jugadas)

PRUEBA 1: SPEEDUP
- Simulaciones fijas: $SIMS por thread
- Configuraciones: 1, 2, 4, 8, 16 threads
- Repeticiones: 3 veces, promediado

Ver archivo: speedup_data.txt

PRUEBA 2: IMPACTO DE SIMULACIONES
- Threads fijos: 4
- Simulaciones: 100, 500, 1000, 2000, 5000
- Repeticiones: 1 vez por configuración

Ver archivo: sims_impact_data.txt

PRUEBA 3: EFICIENCIA
- Baseline: 1 thread
- Métricas: Speedup y Eficiencia

Ver archivo: efficiency_report.txt

========================================
ARCHIVOS GENERADOS:

- speedup_data.txt       : Datos para gráfica de speedup
- sims_impact_data.txt   : Datos para impacto de simulaciones
- efficiency_report.txt  : Tabla de eficiencia
- test_*.log             : Logs detallados por configuración
- input.txt              : Entrada usada para las pruebas
- RESUMEN.txt            : Este archivo

========================================
USO DE LOS DATOS:

1. Para gráficas en Excel/LibreOffice:
   - Importar speedup_data.txt (columnas: threads, tiempo)
   - Importar sims_impact_data.txt (columnas: sims, tiempo)

2. Para cálculos en el reporte:
   - efficiency_report.txt tiene speedup y eficiencia

3. Para verificar ejecuciones:
   - Revisar logs individuales test_*.log

========================================
PRÓXIMOS PASOS:

1. Revisar los datos en efficiency_report.txt
2. Crear gráficas para el reporte
3. Analizar tendencias y anomalías
4. Comparar con predicciones teóricas (Ley de Amdahl)
5. Documentar en el reporte técnico

========================================
EOF

cat "$RESULTS_DIR/RESUMEN.txt"

echo ""
echo "=========================================="
echo "  MEDICIONES COMPLETADAS"
echo "=========================================="
echo ""
echo "Todos los archivos están en: $RESULTS_DIR/"
echo ""
echo "Para ver el reporte de eficiencia:"
echo "  cat $RESULTS_DIR/efficiency_report.txt"
echo ""
echo "Para generar gráficas, usa los archivos:"
echo "  - $RESULTS_DIR/speedup_data.txt"
echo "  - $RESULTS_DIR/sims_impact_data.txt"
echo ""
echo "¡Buena suerte con tu reporte!"
echo ""

# ============================================================================
# GENERAR SCRIPTS DE GRAFICACIÓN (BONUS)
# ============================================================================

# Script de gnuplot para speedup
cat > "$RESULTS_DIR/plot_speedup.gnu" << 'EOF'
set terminal png size 800,600
set output 'speedup_graph.png'
set title 'Speedup vs. Threads'
set xlabel 'Number of Threads'
set ylabel 'Speedup'
set grid
set key left top

# Línea ideal (speedup = threads)
plot 'speedup_data.txt' using 1:(BASELINE/$2) with linespoints title 'Observed' lw 2 pt 7 ps 1.5, \
     x with lines title 'Ideal (Linear)' lw 2 dt 2

# Nota: Reemplaza BASELINE con el tiempo de 1 thread
EOF

echo "Script de gnuplot creado: $RESULTS_DIR/plot_speedup.gnu"
echo "Para generar gráfica (requiere gnuplot):"
echo "  cd $RESULTS_DIR && gnuplot plot_speedup.gnu"
echo ""

exit 0
