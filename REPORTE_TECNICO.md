# REPORTE TÉCNICO - JUEGO HEX CON MONTE CARLO PARALELO

**Autor**: [Tu Nombre]  
**Curso**: Programación en Paralelo  
**Fecha**: [Fecha de Entrega]  
**Institución**: [Tu Universidad]

---

## RESUMEN EJECUTIVO

Este proyecto implementa el juego de estrategia HEX con una inteligencia artificial basada en simulaciones Monte Carlo paralelizadas usando pthreads en C. El sistema permite jugar contra una IA que evalúa posiciones mediante simulaciones aleatorias distribuidas entre múltiples threads, logrando un speedup significativo y mejorando la calidad de las decisiones.

**Resultados principales:**
- Speedup de [X]x con [N] threads
- Tiempo de respuesta: [X] segundos por jugada
- Calidad de juego: [Descripción]
- Eficiencia de paralelización: [X]%

---

## 1. INTRODUCCIÓN

### 1.1 El Juego HEX

HEX es un juego de estrategia para dos jugadores inventado independientemente por Piet Hein (1942) y John Nash (1948). Se juega en un tablero de hexágonos donde:

- **Jugador ROJO**: Debe conectar el borde superior con el inferior
- **Jugador AZUL**: Debe conectar el borde izquierdo con el derecho
- Los jugadores alternan colocando fichas en celdas vacías
- El primer jugador en formar un camino continuo gana
- **Teorema matemático**: No existen empates en HEX

### 1.2 Motivación

Los juegos de estrategia como HEX son problemas computacionalmente complejos que se benefician significativamente del cómputo en paralelo. Este proyecto demuestra:

1. **Paralelización de simulaciones Monte Carlo**
2. **Uso efectivo de pthreads en C**
3. **Sincronización y combinación de resultados**
4. **Aplicación práctica de programación concurrente**

### 1.3 Objetivos

- Implementar el motor del juego HEX con detección de ganador
- Desarrollar IA basada en Monte Carlo
- Paralelizar las simulaciones usando pthreads
- Medir y analizar el rendimiento del sistema
- Proporcionar interfaz gráfica y de consola

---

## 2. FUNDAMENTOS TEÓRICOS

### 2.1 Algoritmo Monte Carlo

El método Monte Carlo para juegos consiste en:

```
Para cada movimiento posible:
   1. Realizar N simulaciones de juegos aleatorios
   2. Contar victorias y derrotas
   3. Calcular score = victorias - derrotas
   
Elegir el movimiento con mejor score
```

**Ventajas:**
- No requiere conocimiento específico del dominio
- Fácilmente paralelizable (simulaciones independientes)
- Se adapta automáticamente a la complejidad del tablero

**Limitaciones:**
- Requiere muchas simulaciones para ser efectivo
- No planifica estrategias a largo plazo
- Sensible a la calidad del generador aleatorio

### 2.2 Paralelización con pthreads

El programa divide el trabajo de simulación entre múltiples threads:

```c
// Cada thread ejecuta simulaciones independientes
for (int i = 0; i < n_threads; i++) {
   pthread_create(&threads[i], NULL, monte_carlo_worker, &args[i]);
}

// Sincronización
for (int i = 0; i < n_threads; i++) {
   pthread_join(threads[i], NULL);
}

// Combinación de resultados
for (cada celda) {
   total_wins = suma de wins de todos los threads
   total_losses = suma de losses de todos los threads
}
```

**Consideraciones:**
- Cada thread usa un generador PCG con semilla diferente
- No hay condiciones de carrera (cada thread escribe en su memoria)
- Sincronización solo al final con `pthread_join()`

### 2.3 Detección de Ganador (DFS)

Se usa Depth-First Search para verificar conectividad:

```c
// Para ROJO: buscar camino vertical
for (cada celda roja en fila superior) {
   DFS(celda)
   if (llegó a fila inferior) return ROJO_GANA
}

// Para AZUL: buscar camino horizontal  
for (cada celda azul en columna izquierda) {
   DFS(celda)
   if (llegó a columna derecha) return AZUL_GANA
}
```

**Complejidad**: O(N²) donde N = tamaño del tablero

---

## 3. DISEÑO E IMPLEMENTACIÓN

### 3.1 Arquitectura del Sistema

```
┌─────────────────┐
│  Interfaz       │
│  (GUI/Consola)  │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Motor de Juego │
│  - Tablero      │
│  - Reglas       │
│  - Ganador (DFS)│
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Motor IA       │
│  Monte Carlo    │
└────────┬────────┘
         │
         ▼
┌──────────────────────────────┐
│   Paralelización (pthreads)  │
│                               │
│  Thread 1  Thread 2  Thread N │
│     │          │         │    │
│     ▼          ▼         ▼    │
│   PCG 1     PCG 2    PCG N    │
│  (Sims)    (Sims)   (Sims)    │
└──────────────────────────────┘
         │
         ▼
┌─────────────────┐
│  Combinación    │
│  de Resultados  │
└─────────────────┘
```

### 3.2 Módulos Principales

#### 3.2.1 Motor del Juego (hex_game.c)

**Funciones clave:**
- `board_init()`: Inicializa tablero vacío
- `board_make_move()`: Ejecuta un movimiento
- `board_check_winner()`: Detecta ganador con DFS
- `board_is_valid_move()`: Valida movimientos

**Estructura del tablero:**
```c
typedef struct {
   int cells[BOARD_SIZE][BOARD_SIZE];  // 0=vacío, 1=rojo, 2=azul
   int moves_count;                     // Número de jugadas realizadas
} board_t;
```

#### 3.2.2 IA Monte Carlo (hex_ai.c)

**Funciones clave:**
- `ai_best_move()`: Encuentra mejor jugada (punto de entrada)
- `monte_carlo_worker()`: Función ejecutada por cada thread
- `run_simulations()`: Ejecuta N simulaciones para un thread
- `simulate_random_game()`: Simula un juego completo aleatorio

**Flujo de ejecución:**
```
ai_best_move()
   │
   ├─► Crear threads (pthread_create)
   │      │
   │      ├─► Thread 1: run_simulations()
   │      ├─► Thread 2: run_simulations()
   │      └─► Thread N: run_simulations()
   │             │
   │             └─► simulate_random_game() (M veces)
   │
   ├─► Sincronizar (pthread_join)
   │
   └─► Combinar estadísticas y elegir mejor movimiento
```

#### 3.2.3 Generador Aleatorio PCG (pcg_basic.c)

**¿Por qué PCG?**
- Rápido y ligero
- Buena calidad estadística
- Thread-safe con estado independiente
- Mejor que rand() de libc

**Uso en el proyecto:**
```c
// Cada thread tiene su propio generador
pcg32_random_t rng;
pcg32_srandom_r(&rng, seed_base + thread_id * offset, sequence);

// Generar número aleatorio acotado
int random_move = pcg32_boundedrand_r(&rng, num_moves);
```

### 3.3 Sincronización y Comunicación

**Estrategia de sincronización:**

1. **Sin mutex durante simulaciones**: Cada thread escribe en su propia memoria
2. **Barrera implícita**: `pthread_join()` espera a todos los threads
3. **Combinación secuencial**: El thread principal combina resultados

**Ventajas:**
- Mínimo overhead de sincronización
- No hay condiciones de carrera
- Escalabilidad casi lineal

**Análisis de dependencias:**
```
Tiempo total = T_launch + T_simulation + T_join + T_combine

Donde:
- T_launch ≈ O(n_threads) - Crear threads
- T_simulation ≈ O(sims * cells) / n_threads - Trabajo paralelo
- T_join ≈ O(n_threads) - Esperar threads
- T_combine ≈ O(cells * n_threads) - Combinar resultados
```

---

## 4. EXPERIMENTOS Y RESULTADOS

### 4.1 Configuración Experimental

**Hardware utilizado:**
- **CPU**: [Especificar modelo, ej: Intel Core i7-9700K]
- **Cores**: [Número de núcleos físicos]
- **Threads**: [Número de hilos lógicos]
- **RAM**: [Cantidad de RAM]
- **OS**: Linux [Versión/Distribución]

**Configuración de software:**
- **Compilador**: GCC [versión] con flags `-O3 -pthread`
- **Tablero**: 11x11 hexágonos
- **Simulaciones base**: 1000 por thread

### 4.2 Mediciones de Tiempo

**Tabla 1: Tiempo de ejecución por jugada (segundos)**

| Threads | Simulaciones/Thread | Tiempo (s) | Speedup | Eficiencia |
|---------|---------------------|------------|---------|------------|
| 1       | 1000                | [medir]    | 1.00    | 100%       |
| 2       | 1000                | [medir]    | [calc]  | [calc]     |
| 4       | 1000                | [medir]    | [calc]  | [calc]     |
| 8       | 1000                | [medir]    | [calc]  | [calc]     |
| 16      | 1000                | [medir]    | [calc]  | [calc]     |

**Cálculos:**
- Speedup(n) = Tiempo(1 thread) / Tiempo(n threads)
- Eficiencia(n) = Speedup(n) / n × 100%

**Comandos para medición:**
```bash
# Ejemplo con time
time ./hex_console -t 1 -s 1000 < input.txt
time ./hex_console -t 2 -s 1000 < input.txt
time ./hex_console -t 4 -s 1000 < input.txt
# etc...
```

### 4.3 Escalabilidad

**Gráfica de Speedup** [Insertar gráfica]

Análisis:
- El speedup observado es [lineal/sublineal/superlineal]
- La eficiencia se mantiene en [X]% hasta [N] threads
- El overhead de sincronización es [despreciable/significativo]

**Ley de Amdahl:**
```
Speedup máximo = 1 / (f_serial + (1-f_serial)/p)

Donde:
- f_serial = fracción del código que es secuencial
- p = número de procesadores
```

En nuestro caso, f_serial ≈ [calcular] debido a:
1. Lanzamiento de threads: [X]%
2. Combinación de resultados: [X]%
3. Detección de ganador: [X]%

### 4.4 Impacto de Simulaciones en Calidad

**Tabla 2: Calidad de decisiones vs. simulaciones**

| Simulaciones | Win Rate vs Humano | Tiempo/Jugada |
|--------------|-------------------|---------------|
| 100          | [estimar]%        | [medir]s      |
| 500          | [estimar]%        | [medir]s      |
| 1000         | [estimar]%        | [medir]s      |
| 5000         | [estimar]%        | [medir]s      |
| 10000        | [estimar]%        | [medir]s      |

**Observaciones:**
- Con 100 simulaciones: [describir comportamiento]
- Con 1000 simulaciones: [describir comportamiento]
- Con 10000 simulaciones: [describir comportamiento]
- Retornos decrecientes después de [X] simulaciones

### 4.5 Análisis de Carga

**Balanceo de carga:**
- Cada thread procesa el mismo número de simulaciones
- No hay desbalanceo significativo
- Variación en tiempo de ejecución: [X]%

**Tabla 3: Tiempo por thread (4 threads, 1000 sims c/u)**

| Thread ID | Tiempo (s) | Variación |
|-----------|------------|-----------|
| 0         | [medir]    | -         |
| 1         | [medir]    | [calc]%   |
| 2         | [medir]    | [calc]%   |
| 3         | [medir]    | [calc]%   |

---

## 5. ANÁLISIS Y DISCUSIÓN

### 5.1 Rendimiento Observado

**Fortalezas:**
1. Escalabilidad casi lineal hasta [N] threads
2. Overhead de sincronización mínimo (<[X]%)
3. No hay condiciones de carrera
4. Uso eficiente de la caché CPU

**Limitaciones:**
1. Speedup se satura después de [N] threads debido a:
   - Contención de memoria
   - Overhead de context switching
   - Límite de núcleos físicos

### 5.2 Comparación con Enfoque Secuencial

**Ventajas del paralelismo:**
- Tiempo de respuesta reducido en [X]x
- Permite más simulaciones en tiempo razonable
- Mejor calidad de decisiones

**Desventajas del paralelismo:**
- Mayor uso de memoria (estado por thread)
- Complejidad adicional en el código
- Requiere hardware multinúcleo

### 5.3 Optimizaciones Realizadas

1. **Eliminación de mutex durante simulaciones**
   - Cada thread trabaja en memoria independiente
   - Solo sincronización al final

2. **Generador PCG en lugar de rand()**
   - 3-4x más rápido que rand()
   - Mejor calidad estadística
   - Thread-safe por diseño

3. **Compilación con -O3**
   - Optimizaciones del compilador
   - Vectorización automática
   - Inlining de funciones

4. **Localidad de caché**
   - Estructuras compactas (board_t)
   - Acceso secuencial a memoria

### 5.4 Bottlenecks Identificados

1. **Combinación de resultados**: O(cells × threads)
   - Actualmente secuencial
   - Podría paralelizarse con reducción

2. **Creación de threads**: Overhead constante
   - Podría usar thread pool
   - Reutilizar threads entre jugadas

3. **Detección de ganador**: O(N²)
   - Se ejecuta después de cada jugada
   - No paralelizado actualmente

### 5.5 Alternativas de Diseño Consideradas

**1. Procesos en lugar de threads (fork())**
- Ventaja: Aislamiento completo de memoria
- Desventaja: Mayor overhead, necesita IPC
- Decisión: Threads son más eficientes para este caso

**2. MPI para clusters**
- Ventaja: Escalabilidad a múltiples máquinas
- Desventaja: Latencia de red, complejidad
- Decisión: Overkill para una sola máquina

**3. OpenMP**
- Ventaja: Sintaxis más simple (#pragma)
- Desventaja: Menos control fino
- Decisión: Pthreads ofrecen más flexibilidad

---

## 6. CONCLUSIONES

### 6.1 Logros Principales

1. **Implementación exitosa** del juego HEX completo
2. **Paralelización efectiva** con speedup de [X]x
3. **IA competitiva** usando Monte Carlo
4. **Interfaz funcional** (GUI y consola)
5. **Código limpio y documentado**

### 6.2 Lecciones Aprendidas

**Técnicas:**
- Diseño de algoritmos paralelizables
- Minimización de sincronización
- Importancia de buenos generadores aleatorios
- Medición y análisis de rendimiento

**Conceptos:**
- Speedup y eficiencia
- Ley de Amdahl en la práctica
- Balanceo de carga
- Escalabilidad de sistemas paralelos

### 6.3 Trabajo Futuro

**Mejoras a corto plazo:**
1. Thread pool para reutilizar threads
2. Paralelizar combinación de resultados
3. Optimizar detección de ganador

**Extensiones avanzadas:**
1. **MCTS completo**: Implementar Monte Carlo Tree Search
2. **Opening book**: Pre-computar mejores aperturas
3. **Heurísticas**: Evaluar posiciones estratégicamente
4. **GPU acceleration**: Portar a CUDA/OpenCL
5. **Neural networks**: IA con aprendizaje automático

---

## 7. REFERENCIAS

1. **HEX Game Theory**
   - Browne, C. et al. (2012). "A Survey of Monte Carlo Tree Search Methods"
   - Nash, J. (1952). "Some Games and Machines for Playing Them"

2. **Monte Carlo Methods**
   - Chaslot, G. et al. (2008). "Monte-Carlo Tree Search: A New Framework"
   - Coulom, R. (2007). "Efficient Selectivity and Backup Operators in MC Tree Search"

3. **Generadores Aleatorios**
   - O'Neill, M.E. (2014). "PCG: A Family of Simple Fast Space-Efficient Statistically Good Algorithms for Random Number Generation"

4. **Programación Paralela**
   - Pacheco, P. (2011). "An Introduction to Parallel Programming"
   - Butenhof, D. (1997). "Programming with POSIX Threads"

5. **Raylib**
   - raylib.com - Documentación oficial

---

## APÉNDICES

### Apéndice A: Código Completo

[El código fuente completo está disponible en los archivos adjuntos]

### Apéndice B: Manual de Usuario

Ver archivo `GUIA_RAPIDA.md` para instrucciones detalladas de uso.

### Apéndice C: Scripts de Medición

```bash
#!/bin/bash
# script_medicion.sh - Medir rendimiento con diferentes configuraciones

for threads in 1 2 4 8 16; do
   echo "Midiendo con $threads threads..."
   time ./hex_console -t $threads -s 1000 < input.txt 2>&1 | tee log_${threads}.txt
done
```

### Apéndice D: Formato de Entrada para Pruebas

```
# input.txt - Secuencia de movimientos para pruebas repetibles
A1
B2
C3
...
```

---

**Fin del Reporte**

---

## CHECKLIST DE ENTREGA

- [ ] Código fuente completo y comentado
- [ ] Makefile funcional
- [ ] README con instrucciones
- [ ] Este reporte técnico completado
- [ ] Gráficas de rendimiento
- [ ] Archivos de medición
- [ ] Scripts de prueba
- [ ] Presentación (si es requerida)
- [ ] Demo en video (opcional)

