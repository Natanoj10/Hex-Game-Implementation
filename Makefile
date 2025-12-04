# Makefile para el juego HEX con Monte Carlo paralelo

CC = gcc
CFLAGS = -Wall -O3 -std=c11 -pthread
LDFLAGS = -lraylib -lm -pthread
LDFLAGS_CONSOLE = -lm -pthread

# Archivos fuente
SOURCES_GUI = hex_game.c hex_ai.c pcg_basic.c hex_gui.c
SOURCES_CONSOLE = hex_game.c hex_ai.c pcg_basic.c hex_console.c
OBJECTS_GUI = $(SOURCES_GUI:.c=.o)
OBJECTS_CONSOLE = hex_game.o hex_ai.o pcg_basic.o hex_console.o
TARGET = hex
TARGET_CONSOLE = hex_console

# Regla principal
all: $(TARGET) $(TARGET_CONSOLE)

$(TARGET): hex_game.o hex_ai.o pcg_basic.o hex_gui.o
	$(CC) hex_game.o hex_ai.o pcg_basic.o hex_gui.o -o $(TARGET) $(LDFLAGS)

$(TARGET_CONSOLE): hex_game.o hex_ai.o pcg_basic.o hex_console.o
	$(CC) hex_game.o hex_ai.o pcg_basic.o hex_console.o -o $(TARGET_CONSOLE) $(LDFLAGS_CONSOLE)

# Compilación de objetos
hex_game.o: hex_game.c hex_game.h
	$(CC) $(CFLAGS) -c hex_game.c

hex_ai.o: hex_ai.c hex_game.h pcg_basic.h
	$(CC) $(CFLAGS) -c hex_ai.c

pcg_basic.o: pcg_basic.c pcg_basic.h
	$(CC) $(CFLAGS) -c pcg_basic.c

hex_gui.o: hex_gui.c hex_game.h
	$(CC) $(CFLAGS) -c hex_gui.c

hex_console.o: hex_console.c hex_game.h
	$(CC) $(CFLAGS) -c hex_console.c

# Limpiar archivos generados
clean:
	rm -f *.o $(TARGET) $(TARGET_CONSOLE)

# Ejecutar el juego
run: $(TARGET)
	./$(TARGET)

# Ejecutar versión consola
run-console: $(TARGET_CONSOLE)
	./$(TARGET_CONSOLE)

# Ejecutar con más threads
run-fast: $(TARGET)
	./$(TARGET) -t 8 -s 2000

# Ejecutar consola con más threads
run-console-fast: $(TARGET_CONSOLE)
	./$(TARGET_CONSOLE) -t 8 -s 2000

# Ejecutar con configuración de prueba rápida
run-test: $(TARGET)
	./$(TARGET) -t 2 -s 100

# Ejecutar consola con configuración de prueba rápida
run-console-test: $(TARGET_CONSOLE)
	./$(TARGET_CONSOLE) -t 2 -s 100

# Ayuda
help:
	@echo "Makefile para HEX"
	@echo ""
	@echo "Objetivos disponibles:"
	@echo "  make                - Compilar ambas versiones"
	@echo "  make run            - Ejecutar versión GUI (4 threads, 1000 sims)"
	@echo "  make run-console    - Ejecutar versión consola"
	@echo "  make run-fast       - Ejecutar GUI con más potencia (8 threads, 2000 sims)"
	@echo "  make run-console-fast - Ejecutar consola con más potencia"
	@echo "  make run-test       - Ejecutar GUI modo rápido para pruebas"
	@echo "  make run-console-test - Ejecutar consola modo rápido"
	@echo "  make clean          - Limpiar archivos compilados"
	@echo ""
	@echo "Opciones del programa:"
	@echo "  -t <num>   Número de threads (1-32, default 4)"
	@echo "  -s <num>   Simulaciones por thread (default 1000)"
	@echo ""
	@echo "Ejemplo: ./hex -t 8 -s 5000"
	@echo "Ejemplo: ./hex_console -t 4 -s 1000"

.PHONY: all clean run run-console run-fast run-console-fast run-test run-console-test help
