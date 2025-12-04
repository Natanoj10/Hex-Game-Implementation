# JUEGO HEX CON MONTE CARLO PARALELO

Implementación del juego HEX con inteligencia artificial usando simulaciones Monte Carlo paralelizadas con pthreads.

## DESCRIPCIÓN DEL JUEGO

HEX es un juego de estrategia para dos jugadores en un tablero de hexágonos:

- **Tablero**: 11x11 hexágonos
- **Jugadores**: 
  - ROJO (Humano): Debe conectar el borde superior con el inferior
  - AZUL (IA): Debe conectar el borde izquierdo con el derecho
- **Reglas**:
  - Los jugadores alternan turnos colocando fichas
  - Una vez colocada, una ficha no se puede mover
  - No hay empates (teorema matemático)
  - El primer jugador que forme un camino continuo gana

## COMPILACIÓN

### Requisitos
- GCC con soporte C11
- pthread
- raylib (para interfaz gráfica)
- Linux (Fedora, Ubuntu, etc.)

### Instalar raylib en Fedora
```bash
sudo dnf install raylib-devel
```

### Instalar raylib en Ubuntu
```bash
sudo apt install libraylib-dev
```

### Compilar
```bash
make
```

## EJECUCIÓN

### Modo básico (4 threads, 1000 simulaciones)
```bash
./hex
```

### Modo rápido (8 threads, 2000 simulaciones)
```bash
./hex -t 8 -s 2000
```

### Modo personalizado
```bash
./hex -t <threads> -s <simulaciones>
```

**Opciones:**
- `-t <num>`: Número de threads (1-32, default 4)
- `-s <num>`: Simulaciones por thread (default 1000)
- `-h`: Mostrar ayuda

### Ejemplos
```bash
# Modo prueba rápida
./hex -t 2 -s 100

# Modo competitivo (más lento pero más inteligente)
./hex -t 16 -s 5000

# Balance recomendado
./hex -t 8 -s 2000
```

## CONTROLES

- **Mouse**: Click izquierdo para colocar ficha
- **R**: Reiniciar juego
- **ESC**: Salir

## ARQUITECTURA

### Estructura de Archivos
```
hex/
├── hex_game.h       - Definiciones y estructuras
├── hex_game.c       - Motor del juego (tablero, reglas, ganador)
├── hex_ai.c         - IA con Monte Carlo paralelo
├── hex_gui.c        - Interfaz gráfica con raylib
├── pcg_basic.h      - Header del generador PCG
├── pcg_basic.c      - Generador de números aleatorios PCG
├── Makefile         - Sistema de compilación
└── README.md        - Este archivo
```

### Módulos

#### 1. Motor del Juego (hex_game.c)
- Gestión del tablero
- Validación de movimientos
- Detección de ganador usando DFS (Depth-First Search)
- Los hexágonos tienen 6 vecinos conectados

#### 2. IA Monte Carlo (hex_ai.c)
- **Algoritmo**:
  1. Para cada celda vacía del tablero
  2. Simular N juegos aleatorios desde esa posición
  3. Contar victorias y derrotas
  4. Elegir la celda con mejor ratio victoria/derrota

- **Paralelización**:
  - Divide el trabajo entre múltiples threads
  - Cada thread ejecuta simulaciones independientes
  - Usa generador PCG para números aleatorios (thread-safe)
  - Combina resultados al final
  - Sincronización con pthread_join()

#### 3. Interfaz Gráfica (hex_gui.c)
- Renderizado de hexágonos con raylib
- Conversión pixel ↔ coordenadas hexagonales
- Sistema de hover para feedback visual
- Panel de información y leyenda

### Algoritmo Monte Carlo

```
Para cada celda vacía (r, c):
   Para i = 1 hasta N simulaciones:
      - Copiar tablero
      - Hacer movimiento (r, c) para la IA
      - Simular juego aleatorio hasta el final
      - Si IA gana: +1 victoria
      - Si IA pierde: +1 derrota
   
   score[r][c] = victorias - derrotas

Elegir celda con mayor score
```

## RENDIMIENTO

### Tiempos Aproximados (por turno de IA)

| Threads | Simulaciones | Tiempo  | Calidad |
|---------|-------------|---------|---------|
| 2       | 100         | ~1s     | Básica  |
| 4       | 1000        | ~3s     | Buena   |
| 8       | 2000        | ~5s     | Muy buena|
| 16      | 5000        | ~12s    | Excelente|

*Tiempos en CPU de 8 núcleos a 3.0 GHz

### Escalabilidad
El programa escala casi linealmente con el número de threads:
- 2 threads → 2x velocidad
- 4 threads → 4x velocidad
- 8 threads → 8x velocidad

Limitado por:
- Número de núcleos físicos
- Overhead de sincronización
- Cache coherence

## ESTRATEGIA DE LA IA

La IA usa Monte Carlo para evaluar posiciones:

1. **Exploración**: Simula muchos juegos aleatorios
2. **Explotación**: Elige la jugada con más victorias
3. **Estadística**: Más simulaciones = mejor decisión

**Ventajas:**
- No necesita evaluar cada posible secuencia
- Fácilmente paralelizable
- Se adapta a la complejidad del tablero

**Limitaciones:**
- Requiere muchas simulaciones para ser competitiva
- No planifica estrategias a largo plazo (vs MCTS con árbol)

## DETECCIÓN DE GANADOR

Usa DFS (Depth-First Search) para verificar conectividad:

### Para ROJO (vertical):
1. Iniciar DFS desde cada celda roja en la fila superior
2. Explorar vecinos hexagonales recursivamente
3. Si llega a la fila inferior → ROJO gana

### Para AZUL (horizontal):
1. Iniciar DFS desde cada celda azul en la columna izquierda
2. Explorar vecinos hexagonales recursivamente
3. Si llega a la columna derecha → AZUL gana

## MEJORAS POSIBLES

1. **MCTS con árbol**: Implementar Monte Carlo Tree Search completo
2. **Apertura book**: Pre-calcular mejores aperturas
3. **Heurísticas**: Evaluar posiciones con funciones heurísticas
4. **Balanceo de carga**: Asignación dinámica de trabajo a threads
5. **GPU**: Portar simulaciones a CUDA/OpenCL
6. **Red neuronal**: Entrenar NN para evaluación de posiciones

## DEPURACIÓN

### Compilar con símbolos de debug
```bash
gcc -g -Wall -pthread hex_*.c pcg_*.c -o hex -lraylib -lm
```

### Ejecutar con valgrind
```bash
valgrind --leak-check=full ./hex -t 2 -s 100
```

### Ver estadísticas de la IA
La IA imprime en stderr:
- Top 5 mejores jugadas
- Wins, losses, score de cada jugada
- Tiempo de cómputo

## CRÉDITOS

- **Generador PCG**: Melissa O'Neill (pcg-random.org)
- **Raylib**: Ramon Santamaria (@raysan5)
- **Juego HEX**: Inventado por Piet Hein (1942) y John Nash (1948)

## LICENCIA

Este proyecto es código educativo para el curso de Programación en Paralelo.

## AUTOR

Jonatan - Proyecto Final de Programación en Paralelo
