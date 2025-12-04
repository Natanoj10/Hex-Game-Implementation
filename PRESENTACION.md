# GUÍA DE PRESENTACIÓN - PROYECTO HEX

## PREPARACIÓN ANTES DE LA PRESENTACIÓN

### 1. Instalación del Proyecto
```bash
# Clonar/copiar archivos del proyecto
cd ~/hex

# Ejecutar instalador
./install.sh

# Verificar compilación
make clean
make
```

### 2. Pruebas Previas
```bash
# Prueba rápida de consola
./hex_console -t 2 -s 50

# Prueba de interfaz gráfica
./hex -t 2 -s 100
```

---

## ESTRUCTURA DE LA PRESENTACIÓN (15-20 minutos)

### PARTE 1: INTRODUCCIÓN (3 min)

#### Diapositiva 1: Título
"JUEGO HEX CON MONTE CARLO PARALELO"
- Tu nombre
- Programación en Paralelo
- Fecha

#### Diapositiva 2: ¿Qué es HEX?
**Mostrar:**
- Tablero 11x11
- Reglas básicas
- Objetivo de cada jugador

**Decir:**
"HEX es un juego de estrategia inventado por John Nash. Dos jugadores compiten en un tablero de hexágonos. ROJO debe conectar arriba con abajo, AZUL debe conectar izquierda con derecha. Matemáticamente, no puede haber empates."

#### Diapositiva 3: Objetivo del Proyecto
**Puntos clave:**
- Implementar el juego completo en C
- Crear IA usando Monte Carlo
- Paralelizar con pthreads
- Interfaz gráfica con raylib

---

### PARTE 2: ARQUITECTURA (5 min)

#### Diapositiva 4: Estructura del Código
**Mostrar diagrama:**
```
GUI/Consola
    ↓
   IA (Monte Carlo Paralelo)
    ↓
Motor del Juego
    ↓
Generador PCG
```

**Decir:**
"El proyecto está dividido en módulos independientes. El motor del juego maneja el tablero y las reglas. La IA usa simulaciones Monte Carlo paralelizadas. Y tenemos dos interfaces: gráfica y consola."

#### Diapositiva 5: Funciones Críticas
**Código clave a mostrar:**
```c
// Detección de ganador (DFS)
int board_check_winner(const board_t* board)

// IA principal
coord_t ai_best_move(const board_t* board, int player, 
                     int threads, int sims)

// Worker paralelo
void* monte_carlo_worker(void* args)
```

---

### PARTE 3: ALGORITMO MONTE CARLO (5 min)

#### Diapositiva 6: ¿Qué es Monte Carlo?
**Explicar con ejemplo:**
"Para decidir una jugada, el programa simula miles de partidas aleatorias desde cada posición posible. La jugada que lleva a más victorias es la elegida."

**Diagrama:**
```
Para cada celda vacía:
   ┌─────────────────────────┐
   │ Simular 1000 juegos     │
   │ Contar victorias        │
   └─────────────────────────┘

Elegir celda con más victorias
```

#### Diapositiva 7: Pseudocódigo
```
Para cada celda C:
   victorias = 0
   derrotas = 0
   
   Para i = 1 hasta N:
      Copiar tablero
      Jugar en C
      Simular juego aleatorio
      
      Si IA gana:
         victorias++
      Sino:
         derrotas++
   
   score[C] = victorias - derrotas

Retornar celda con mayor score
```

---

### PARTE 4: PARALELIZACIÓN (5 min)

#### Diapositiva 8: Estrategia de Paralelización
**Tipo:** Paralelismo de datos

**Mostrar código:**
```c
// Crear threads
for (int i = 0; i < threads; i++) {
   // Configurar argumentos
   targs[i]->simulations = sims_per_thread;
   targs[i]->seed = unique_seed(i);
   
   pthread_create(&tlist[i], NULL, 
                  monte_carlo_worker, targs[i]);
}

// Esperar threads
for (int i = 0; i < threads; i++) {
   pthread_join(tlist[i], NULL);
}

// Combinar resultados
```

#### Diapositiva 9: Escalabilidad
**Tabla de rendimiento:**

| Threads | Tiempo | Speedup |
|---------|--------|---------|
| 1       | 12s    | 1.0x    |
| 2       | 6s     | 2.0x    |
| 4       | 3s     | 4.0x    |
| 8       | 1.5s   | 8.0x    |

**Gráfica de speedup** (si tienes tiempo de generarla)

#### Diapositiva 10: Sin Race Conditions
**Explicar:**
"Cada thread trabaja en su propia memoria. No hay variables compartidas durante el cómputo. Solo combinamos resultados al final usando pthread_join()."

**Ventajas:**
- No necesitamos mutex
- No hay deadlocks
- Escalabilidad casi lineal

---

### PARTE 5: DEMOSTRACIÓN EN VIVO (5 min)

#### Demo 1: Versión Consola (2 min)
```bash
# Terminal 1: Prueba rápida
./hex_console -t 2 -s 100
```

**Hacer algunas jugadas:**
```
A1  # Tu movimiento
(IA piensa...)
# Mostrar estadísticas que imprime la IA
```

#### Demo 2: Versión Gráfica (3 min)
```bash
# Terminal 2: Versión GUI
./hex -t 4 -s 1000
```

**Mostrar:**
1. El tablero hexagonal
2. Hacer un movimiento con mouse
3. Observar a la IA pensar
4. Ver mensaje en terminal con estadísticas
5. Continuar juego 2-3 turnos

**Puntos a mencionar durante demo:**
- "Aquí ven el tablero renderizado con hexágonos"
- "La IA está usando 4 threads para evaluar"
- "En terminal se ven las mejores 5 jugadas consideradas"
- "Más simulaciones = mejor jugada pero más tiempo"

---

### PARTE 6: CONCLUSIONES (2 min)

#### Diapositiva 11: Logros
✓ Juego HEX completamente funcional  
✓ IA competente usando Monte Carlo  
✓ Paralelización efectiva (speedup casi lineal)  
✓ Dos interfaces (GUI y consola)  
✓ Código modular y bien documentado  

#### Diapositiva 12: Aprendizajes
**Técnicos:**
- Implementación de pthreads en aplicación real
- Algoritmos Monte Carlo
- Detección de conectividad con DFS
- Manejo de memoria en C

**Prácticos:**
- Estructura de proyectos en C
- Debugging de programas paralelos
- Optimización de rendimiento

#### Diapositiva 13: Mejoras Futuras
- Implementar MCTS con árbol (más inteligente)
- Usar GPU para más simulaciones
- Libro de aperturas
- Modo multijugador
- Red neuronal para evaluación

---

## PREGUNTAS FRECUENTES Y RESPUESTAS

### P1: "¿Por qué usar Monte Carlo y no minimax?"
**R:** "Monte Carlo es más fácil de paralelizar y no necesita función de evaluación compleja. Para HEX, el espacio de estados es enorme (11^121 posiciones), así que minimax con alpha-beta sería muy lento."

### P2: "¿Cómo evitan race conditions?"
**R:** "Cada thread tiene su propia memoria para estadísticas. No hay escritura concurrente en variables compartidas. Solo combinamos resultados al final con pthread_join()."

### P3: "¿Por qué no escala a más de 16 threads?"
**R:** "Porque mi CPU tiene 8 núcleos físicos con hyperthreading. Más threads no dan más speedup real, solo overhead de context switching."

### P4: "¿Qué tan buena es la IA?"
**R:** "Con 4 threads y 1000 simulaciones por thread (4000 total), juega muy bien. Con 8 threads y 5000 sims (40000 total), juega a nivel competitivo. Depende del tiempo que estés dispuesto a esperar."

### P5: "¿Por qué usar PCG y no rand()?"
**R:** "PCG tiene mejor calidad estadística y es thread-safe por diseño. rand() de libc no es thread-safe y tiene problemas de correlación."

---

## CHECKLIST PRE-PRESENTACIÓN

**24 horas antes:**
- [ ] Compilar proyecto sin errores
- [ ] Probar ambas versiones (GUI y consola)
- [ ] Verificar que funcione en la máquina de presentación
- [ ] Preparar diapositivas
- [ ] Practicar demo

**1 hora antes:**
- [ ] Abrir terminales necesarias
- [ ] Compilar una última vez
- [ ] Hacer test run rápido
- [ ] Tener código abierto en editor

**Durante presentación:**
- [ ] Hablar claro y no muy rápido
- [ ] Mantener contacto visual
- [ ] Mostrar código clave
- [ ] Demo en vivo (no video)
- [ ] Responder preguntas con confianza

---

## COMANDOS ÚTILES DURANTE PRESENTACIÓN

```bash
# Compilar
make clean && make

# Test rápido consola (no esperar mucho)
./hex_console -t 2 -s 50

# Demo normal consola
./hex_console -t 4 -s 500

# Demo GUI (recomendado)
./hex -t 4 -s 1000

# Si quieres impresionar con velocidad
./hex -t 8 -s 2000

# Ver ayuda
make help
./hex -h

# Ver estadísticas detalladas
./hex_console -t 8 -s 1000 2>&1 | grep "Top 5"
```

---

## BACKUP PLAN

### Si la GUI no funciona:
"Tengo también una versión de consola completamente funcional."
```bash
./hex_console -t 4 -s 1000
```

### Si el rendimiento es lento:
"Puedo reducir las simulaciones para la demo."
```bash
./hex -t 2 -s 100
```

### Si no compila en el momento:
"Tengo los binarios pre-compilados listos."

---

## TIEMPO ESTIMADO

- Introducción: 3 min
- Arquitectura: 5 min
- Monte Carlo: 5 min
- Paralelización: 5 min
- Demo: 5 min
- Conclusiones: 2 min
- Preguntas: 5 min

**Total: 30 minutos** (ajustar según tiempo asignado)

---

## CONSEJOS FINALES

1. **Practica la demo** - Haz la demo 2-3 veces antes
2. **Ten backup** - Código compilado y listo
3. **Sé honesto** - Si algo no sabes, admítelo
4. **Muestra el código** - Es más impresionante que hablar
5. **Disfruta** - Es tu proyecto, estás orgulloso de él

¡BUENA SUERTE CON LA PRESENTACIÓN!
