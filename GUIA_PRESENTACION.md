# GUÍA DE PRESENTACIÓN - JUEGO HEX CON MONTE CARLO PARALELO

## ESTRUCTURA SUGERIDA DE PRESENTACIÓN (10-15 minutos)

---

### DIAPOSITIVA 1: PORTADA (30 segundos)

**Contenido:**
- Título: "Juego HEX con IA Monte Carlo Paralela"
- Tu nombre
- Curso: Programación en Paralelo
- Fecha

**Qué decir:**
> "Buenos días/tardes. Les voy a presentar mi proyecto final: una implementación del juego HEX con inteligencia artificial usando simulaciones Monte Carlo paralelizadas con pthreads en C."

---

### DIAPOSITIVA 2: ¿QUÉ ES HEX? (1 minuto)

**Contenido:**
- Imagen del tablero 11x11
- Reglas básicas:
  * Rojo conecta arriba-abajo
  * Azul conecta izquierda-derecha
  * Primer camino completo gana
  * No hay empates (teorema matemático)

**Qué decir:**
> "HEX es un juego de estrategia inventado en los años 40. Se juega en un tablero hexagonal. Cada jugador intenta conectar sus bordes opuestos. Lo interesante es que HEX tiene garantía matemática de que no puede haber empates, lo cual lo hace perfecto para IA."

---

### DIAPOSITIVA 3: EL PROBLEMA (1 minuto)

**Contenido:**
- Complejidad del espacio de estados
- Número de posiciones posibles: ~10^56
- Inviable explorar todas las jugadas
- Necesidad de heurísticas inteligentes

**Qué decir:**
> "El problema con HEX es que tiene aproximadamente 10 elevado a 56 posiciones posibles. Es computacionalmente inviable explorar todo el árbol de decisión. Por eso usé un enfoque basado en simulaciones Monte Carlo: en lugar de analizar profundamente, simulo muchos juegos aleatorios para estimar qué jugada tiene más probabilidad de ganar."

---

### DIAPOSITIVA 4: MÉTODO MONTE CARLO (2 minutos)

**Contenido:**
- Diagrama del algoritmo:
  ```
  Para cada celda vacía:
     Simular N juegos aleatorios
     Contar victorias y derrotas
     score = victorias - derrotas
  
  Elegir celda con mejor score
  ```

- Ejemplo visual con números

**Qué decir:**
> "El método Monte Carlo funciona así: para cada posible jugada, simulo mil juegos aleatorios. Si en la mayoría de esas simulaciones gano, esa jugada recibe un score alto. Al final, elijo la jugada con el mejor score. Es un método estadístico: más simulaciones dan mejor precisión."

**Pregunta anticipada:** "¿Por qué funciona esto?"
> "Funciona porque aunque cada juego individual sea aleatorio, el promedio de miles de simulaciones converge a la probabilidad real de victoria. Es como lanzar una moneda: un lanzamiento es aleatorio, pero mil lanzamientos te dan la probabilidad real."

---

### DIAPOSITIVA 5: OPORTUNIDAD DE PARALELISMO (1 minuto)

**Contenido:**
- Problema embarazosamente paralelo
- Simulaciones independientes
- No hay dependencias entre threads
- Diagrama:
  ```
  Thread 1 → Simulaciones → Stats 1
  Thread 2 → Simulaciones → Stats 2    → Combinar → Mejor jugada
  Thread N → Simulaciones → Stats N
  ```

**Qué decir:**
> "Aquí está la clave del paralelismo: cada simulación es completamente independiente. No hay dependencias de datos entre threads. Cada thread puede ejecutar sus simulaciones en paralelo, y al final simplemente sumamos los resultados. Es lo que llamamos un 'problema embarazosamente paralelo'."

---

### DIAPOSITIVA 6: ARQUITECTURA DEL SISTEMA (2 minutos)

**Contenido:**
- Diagrama de módulos:
  ```
  GUI/Consola
      ↓
  Motor de Juego (tablero, reglas, DFS)
      ↓
  Motor IA Monte Carlo
      ↓
  Paralelización (pthreads)
      ↓
  Generador PCG (thread-safe)
  ```

**Qué decir:**
> "El sistema tiene una arquitectura modular. En la base está el generador de números aleatorios PCG, que es thread-safe. Los threads ejecutan simulaciones independientes. El motor de IA coordina todo y combina resultados. El motor de juego verifica ganadores usando DFS. Y arriba hay dos interfaces: una gráfica con raylib y otra de consola."

**Detalles técnicos importantes:**
1. **DFS para ganador**: "Uso búsqueda en profundidad para verificar si existe un camino entre bordes opuestos, en tiempo O(N²)."
2. **PCG**: "Elegí PCG en lugar de rand() porque es más rápido y tiene mejor calidad estadística."
3. **Sin mutex**: "No necesito mutex durante las simulaciones porque cada thread escribe en su propia memoria."

---

### DIAPOSITIVA 7: IMPLEMENTACIÓN - CÓDIGO CLAVE (1.5 minutos)

**Contenido:**
- Snippet de código del worker thread:
```c
void* monte_carlo_worker(void* args) {
   thread_args_t* targs = (thread_args_t*)args;
   
   // Cada thread ejecuta sus simulaciones
   run_simulations(
      targs->board, 
      targs->player, 
      targs->simulations,
      targs->stats, 
      targs->seed  // Semilla única
   );
   
   return NULL;
}
```

**Qué decir:**
> "Este es el código central: la función que ejecuta cada thread. Recibe una copia del tablero, el número de simulaciones a ejecutar, y una semilla única para su generador aleatorio. Cada thread trabaja de manera completamente independiente y guarda sus resultados en su propia estructura de estadísticas."

---

### DIAPOSITIVA 8: SINCRONIZACIÓN (1 minuto)

**Contenido:**
- Estrategia minimalista
- pthread_create() al inicio
- pthread_join() al final
- Sin mutex durante simulaciones
- Combinación secuencial de resultados

**Qué decir:**
> "La sincronización es muy simple. Lanzo todos los threads con pthread_create, los dejo trabajar sin interrupciones, y uso pthread_join para esperarlos. No hay mutex ni variables compartidas durante las simulaciones. Al final, combino los resultados de manera secuencial, lo cual es suficientemente rápido porque es solo una suma de estadísticas."

---

### DIAPOSITIVA 9: RESULTADOS - SPEEDUP (2 minutos)

**Contenido:**
- Tabla de resultados:
  | Threads | Tiempo (s) | Speedup | Eficiencia |
  |---------|------------|---------|------------|
  | 1       | [X]        | 1.00    | 100%       |
  | 2       | [X]        | [X]     | [X]%       |
  | 4       | [X]        | [X]     | [X]%       |
  | 8       | [X]        | [X]     | [X]%       |

- Gráfica de speedup (ideal vs. observado)

**Qué decir:**
> "Estos son los resultados de rendimiento. Con 4 threads obtengo un speedup de [X]x, lo cual representa una eficiencia de [X]%. El speedup es casi lineal hasta [N] threads, después comienza a saturarse debido al overhead de sincronización y la contención de memoria. La gráfica muestra que estamos muy cerca de la línea ideal hasta [N] threads."

**Análisis esperado:**
- Si eficiencia >90%: "Excelente escalabilidad"
- Si eficiencia 70-90%: "Buena escalabilidad con overhead razonable"
- Si eficiencia <70%: "Explicar causas: contención, overhead, etc."

---

### DIAPOSITIVA 10: RESULTADOS - CALIDAD DE IA (1 minuto)

**Contenido:**
- Tabla de calidad vs. simulaciones:
  | Simulaciones | Tiempo | Calidad de Decisión |
  |--------------|--------|---------------------|
  | 100          | ~1s    | Básica              |
  | 1000         | ~3s    | Buena               |
  | 5000         | ~15s   | Muy buena           |

**Qué decir:**
> "El número de simulaciones tiene impacto directo en la calidad de las decisiones. Con 100 simulaciones la IA juega decentemente pero comete errores. Con 1000 simulaciones es competitiva. Con 5000 es muy difícil de vencer. El costo es tiempo: cada aumento de 5x en simulaciones aumenta el tiempo 5x, pero el paralelismo mitiga esto."

---

### DIAPOSITIVA 11: DEMOSTRACIÓN (2 minutos)

**Contenido:**
- Video pregrabado O demostración en vivo
- Mostrar:
  1. Compilación
  2. Ejecución con diferentes configuraciones
  3. Una jugada completa
  4. Salida de estadísticas de la IA

**Qué decir:**
> "Les voy a mostrar el sistema en acción. [Ejecutar demo]. Aquí pueden ver la interfaz de consola. Hago mi jugada en F6. La IA piensa con 4 threads y 1000 simulaciones, tarda unos 3 segundos, y muestra las 5 mejores jugadas evaluadas con sus estadísticas. Finalmente hace su jugada."

**Tips para demo:**
- Usa configuración rápida (-t 2 -s 100) para que no tarde
- Prepara capturas de pantalla por si falla
- Muestra también la versión gráfica si es posible

---

### DIAPOSITIVA 12: DESAFÍOS Y SOLUCIONES (1.5 minutos)

**Contenido:**
Tabla de desafíos:

| Desafío | Solución |
|---------|----------|
| Detección de ganador lenta | Implementé DFS eficiente |
| rand() es lento y no thread-safe | Usé generador PCG |
| Overhead de sincronización | Minimicé uso de mutex |
| Balanceo de carga | División equitativa de trabajo |

**Qué decir:**
> "Enfrenté varios desafíos. Primero, la detección de ganador era lenta, lo solucioné con un algoritmo DFS optimizado. Segundo, el generador rand() de C es lento y no thread-safe, cambié a PCG que es 3-4x más rápido y diseñado para multithreading. Tercero, reduje la sincronización al mínimo para evitar overhead."

---

### DIAPOSITIVA 13: LECCIONES APRENDIDAS (1 minuto)

**Contenido:**
- Bullet points de conclusiones:
  * Identificar código paralelizable es clave
  * Minimizar sincronización mejora rendimiento
  * Calidad del generador aleatorio importa
  * Speedup real vs. teórico (Ley de Amdahl)

**Qué decir:**
> "Las lecciones principales que aprendí: primero, no todo es paralelizable - identificar las partes independientes es crucial. Segundo, cada mutex es un cuello de botella potencial. Tercero, en simulaciones Monte Carlo, la calidad del generador aleatorio afecta tanto la velocidad como la precisión. Y finalmente, la Ley de Amdahl se cumple: el speedup tiene límites reales."

---

### DIAPOSITIVA 14: MEJORAS FUTURAS (30 segundos)

**Contenido:**
- Lista breve:
  1. MCTS completo (árbol de búsqueda)
  2. Opening book pre-computado
  3. Aceleración con GPU (CUDA)
  4. Thread pool para reutilizar threads

**Qué decir:**
> "Como mejoras futuras, podría implementar Monte Carlo Tree Search completo que mantiene un árbol de búsqueda entre jugadas. También podría pre-computar una base de aperturas. Para mayor velocidad, las simulaciones podrían portarse a GPU. Y usar un thread pool eliminaría el overhead de crear threads en cada jugada."

---

### DIAPOSITIVA 15: CONCLUSIONES (1 minuto)

**Contenido:**
- Logros principales:
  * ✓ Sistema HEX completo y funcional
  * ✓ IA competitiva con Monte Carlo
  * ✓ Paralelización efectiva (speedup [X]x)
  * ✓ Escalabilidad demostrada
  * ✓ Código limpio y documentado

**Qué decir:**
> "En conclusión, logré implementar un sistema completo del juego HEX con una IA basada en simulaciones Monte Carlo paralelizadas. El sistema escala eficientemente hasta [N] threads con un speedup de [X]x. La IA es competitiva y toma decisiones inteligentes. El código está bien estructurado y documentado. Este proyecto demuestra que los métodos Monte Carlo son excelentes candidatos para paralelización, y que pthreads en C nos da el control necesario para optimizar el rendimiento."

---

### DIAPOSITIVA 16: PREGUNTAS (Resto del tiempo)

**Contenido:**
- "¿Preguntas?"
- Tu email/contacto

**Preguntas frecuentes esperadas:**

**P1: "¿Por qué no usaste OpenMP?"**
> "OpenMP es más simple sintácticamente, pero pthreads me da más control fino sobre la sincronización y el manejo de threads. Como las simulaciones son completamente independientes, no necesitaba las directivas paralelas de OpenMP."

**P2: "¿Cómo aseguras que los generadores aleatorios son independientes?"**
> "Cada thread usa un generador PCG con semilla diferente. Específicamente, uso: seed_base + thread_id * offset_grande, donde el offset es un número primo grande. PCG garantiza que con semillas diferentes, las secuencias son estadísticamente independientes."

**P3: "¿Qué pasa si hay más threads que cores?"**
> "El sistema operativo hará context switching, lo cual agrega overhead. Por eso el speedup se satura. En mi caso, vi que después de [N] threads (que coincide con mis núcleos físicos), el speedup deja de mejorar significativamente."

**P4: "¿La IA siempre gana?"**
> "No siempre. Con 100 simulaciones es beatable. Con 1000 es competitiva. Con 5000+ es muy difícil de vencer pero no imposible. Monte Carlo puro no es óptimo - MCTS completo sería más fuerte."

**P5: "¿Cuánta memoria usa?"**
> "Cada thread mantiene una copia del tablero (121 ints = ~500 bytes) y estadísticas (121 structs = ~1.5KB). Para 8 threads son ~16KB total, muy manejable."

---

## TIPS DE PRESENTACIÓN

### Lenguaje Corporal
- Mantén contacto visual con la audiencia
- Usa gestos naturales para enfatizar puntos
- No leas las diapositivas textualmente

### Timing
- Practica para mantener 10-15 minutos
- Deja 5 minutos para preguntas
- Si te quedas corto, expande en "Desafíos y Soluciones"
- Si te pasas, reduce "Implementación" y "Código"

### Manejo de Nervios
- Respira profundo antes de empezar
- Habla despacio y claro
- Si te bloqueas, di "Permítanme reformular..." y continúa

### Durante Demo
- Ten un Plan B (video pregrabado) por si falla
- Usa configuración rápida (-t 2 -s 100) para que no tarde
- Explica qué está haciendo el programa mientras ejecuta

### Material de Apoyo
- Imprime una copia de tus notas
- Ten el código abierto en un editor (por si preguntan detalles)
- Ten calculadora para responder preguntas numéricas

---

## CHECKLIST PRE-PRESENTACIÓN

- [ ] Diapositivas completas y revisadas
- [ ] Demo probada y funcionando
- [ ] Video de backup preparado
- [ ] Código compilado y listo
- [ ] Datos de rendimiento actualizados
- [ ] Practicaste al menos 3 veces
- [ ] Cronometraste tu presentación
- [ ] Laptop cargada
- [ ] Adaptadores/cables necesarios
- [ ] Llegaste 10 minutos antes

---

## ESTRUCTURA ALTERNATIVA (SI TIENES SOLO 5 MINUTOS)

1. **Introducción** (30s): Qué es HEX y el objetivo
2. **Método Monte Carlo** (1m): Cómo funciona el algoritmo
3. **Paralelización** (1m): Por qué es paralelizable
4. **Resultados** (1.5m): Speedup y calidad
5. **Demo** (1m): Mostrar funcionando
6. **Conclusiones** (30s): Logros principales

---

## EXTRAS PARA IMPRESIONAR

### Métricas Adicionales
- Simulations per second
- Memory bandwidth utilization
- Cache miss rate (si mediste con perf)

### Comparaciones
- "Mi implementación es [X]% más rápida que una versión naive"
- "Equivalente a tener una IA [X] veces más inteligente en el mismo tiempo"

### Contexto Histórico
- "Hex fue usado por John Nash (el del Premio Nobel) para estudiar teoría de juegos"
- "Algoritmos Monte Carlo fueron clave en AlphaGo de DeepMind"

---

**¡Buena suerte con tu presentación!**

Recuerda: conoces tu proyecto mejor que nadie. Habla con confianza sobre lo que construiste.
