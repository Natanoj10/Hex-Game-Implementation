# GUÍA RÁPIDA - JUEGO HEX CON MONTE CARLO PARALELO

## COMPILACIÓN Y EJECUCIÓN RÁPIDA

### 1. Compilar el proyecto

```bash
# Versión gráfica (requiere raylib)
make

# Solo versión consola (sin dependencias gráficas)
gcc -Wall -O3 -std=c11 -pthread hex_game.c hex_ai.c pcg_basic.c hex_console.c -o hex_console -lm -pthread
```

### 2. Ejecutar

```bash
# Versión consola - configuración rápida para pruebas
./hex_console -t 2 -s 100

# Versión consola - configuración normal
./hex_console -t 4 -s 1000

# Versión consola - máxima potencia (más lento)
./hex_console -t 8 -s 5000

# Versión gráfica (si compilaste con raylib)
./hex -t 4 -s 1000
```

## CÓMO JUGAR

### Reglas de HEX

1. **Tablero**: 11x11 hexágonos
2. **Objetivo**:
   - **ROJO (tú)**: Conectar borde superior con inferior
   - **AZUL (IA)**: Conectar borde izquierdo con derecho
3. **Turnos**: Los jugadores alternan colocando fichas
4. **Ganador**: Primer jugador en formar un camino continuo

### Controles

#### Versión Consola
- Escribe tu movimiento en formato `<Columna><Fila>`
- Ejemplos: `A1`, `B5`, `K11`
- Escribe `quit` o `q` para salir

#### Versión Gráfica
- **Click izquierdo**: Colocar ficha
- **R**: Reiniciar juego
- **ESC**: Salir

## PARÁMETROS DE CONFIGURACIÓN

### `-t <threads>` (Número de threads)
- **Rango**: 1-32
- **Default**: 4
- **Recomendado**: Número de núcleos de tu CPU

### `-s <simulations>` (Simulaciones por thread)
- **Rango**: 1 en adelante
- **Default**: 1000
- **Impacto**: 
  - Más simulaciones = IA más inteligente pero más lenta
  - Menos simulaciones = IA más rápida pero menos precisa

### Ejemplos de Configuración

```bash
# Para pruebas rápidas
./hex_console -t 2 -s 100       # ~1 segundo por jugada

# Balance recomendado
./hex_console -t 4 -s 1000      # ~3 segundos por jugada

# IA fuerte (requiere paciencia)
./hex_console -t 8 -s 5000      # ~15 segundos por jugada

# IA muy fuerte (muy lento)
./hex_console -t 16 -s 10000    # ~30+ segundos por jugada
```

## ESTRUCTURA DEL PROYECTO

```
hex/
├── hex_game.h          # Definiciones y estructuras
├── hex_game.c          # Motor del juego (tablero, ganador)
├── hex_ai.c            # IA Monte Carlo paralela
├── hex_gui.c           # Interfaz gráfica (raylib)
├── hex_console.c       # Interfaz de consola
├── pcg_basic.h         # Header generador PCG
├── pcg_basic.c         # Generador aleatorio PCG
├── Makefile            # Sistema de compilación
├── README.md           # Documentación completa
└── install.sh          # Script de instalación
```

## SOLUCIÓN DE PROBLEMAS

### Error: "raylib.h: No such file or directory"

**Solución**: Instala raylib o compila solo la versión de consola

```bash
# Ubuntu
sudo apt install libraylib-dev

# Fedora
sudo dnf install raylib-devel

# O compila solo consola
gcc -Wall -O3 -std=c11 -pthread hex_game.c hex_ai.c pcg_basic.c hex_console.c -o hex_console -lm -pthread
```

### La IA tarda mucho

**Solución**: Reduce simulaciones o threads

```bash
./hex_console -t 2 -s 100
```

### La IA juega muy mal

**Solución**: Aumenta simulaciones

```bash
./hex_console -t 8 -s 5000
```

## CARACTERÍSTICAS TÉCNICAS

### Algoritmo Monte Carlo
1. Para cada celda vacía, simula N juegos aleatorios
2. Cuenta victorias y derrotas
3. Elige la celda con mejor ratio

### Paralelización
- Usa **pthreads** para dividir el trabajo
- Cada thread ejecuta simulaciones independientes
- Generador **PCG** garantiza números aleatorios únicos por thread
- Combina resultados al final con `pthread_join()`

### Detección de Ganador
- Usa **DFS (Depth-First Search)**
- Verifica conectividad entre bordes opuestos
- Complejidad: O(N²) donde N = tamaño del tablero

## RENDIMIENTO ESPERADO

| CPU Cores | Threads | Sims | Tiempo/Jugada | Calidad IA |
|-----------|---------|------|---------------|------------|
| 2         | 2       | 100  | ~1s           | Básica     |
| 4         | 4       | 1000 | ~3s           | Buena      |
| 8         | 8       | 2000 | ~5s           | Muy Buena  |
| 16        | 16      | 5000 | ~12s          | Excelente  |

## ESTRATEGIAS PARA GANAR

1. **Controla el centro**: Las celdas centrales son más valiosas
2. **Bloquea al oponente**: Identifica los caminos potenciales del oponente
3. **Construye múltiples amenazas**: No dependas de un solo camino
4. **Conecta tus grupos**: Mantén tus fichas conectadas

## PARA EL REPORTE

### Métricas a medir
```bash
# Tiempo de ejecución con diferentes configuraciones
time ./hex_console -t 1 -s 1000
time ./hex_console -t 2 -s 1000
time ./hex_console -t 4 -s 1000
time ./hex_console -t 8 -s 1000
```

### Speedup
- Speedup = T(1 thread) / T(n threads)
- Eficiencia = Speedup / n

### Preguntas clave para el reporte
1. ¿Cómo escala el programa con más threads?
2. ¿Cuál es el overhead de sincronización?
3. ¿Cómo afectan las simulaciones a la calidad de juego?
4. ¿Qué bottlenecks identificaste?

## COMANDOS ÚTILES

```bash
# Ver ayuda
make help

# Limpiar compilación
make clean

# Compilar todo
make

# Ejecutar con valgrind (detección de memory leaks)
valgrind --leak-check=full ./hex_console -t 2 -s 100

# Compilar con debug
gcc -g -Wall -pthread hex_*.c pcg_*.c -o hex_console -lm -pthread

# Depurar con gdb
gdb ./hex_console
```

## MEJORAS FUTURAS

1. **MCTS con árbol**: Implementar búsqueda de árbol Monte Carlo completa
2. **Heurísticas**: Evaluar posiciones estratégicamente
3. **Opening book**: Pre-calcular mejores aperturas
4. **Paralelismo con procesos**: Usar fork() en lugar de threads
5. **MPI**: Distribuir en múltiples máquinas
6. **GPU**: Portar a CUDA para más velocidad

---

**¡Buena suerte con tu proyecto!**

Este es un excelente ejemplo de programación en paralelo aplicada a juegos de estrategia.
