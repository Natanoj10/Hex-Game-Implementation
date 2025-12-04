#ifndef HEX_GAME_H
#define HEX_GAME_H

#include <stdint.h>
#include <pthread.h>

// Constantes del juego
#define BOARD_SIZE 11
#define MAX_THREADS 32
#define EMPTY 0
#define PLAYER_RED 1    // Conecta arriba-abajo
#define PLAYER_BLUE 2   // Conecta izquierda-derecha

// Estados del juego
#define GAME_CONTINUE 0
#define GAME_RED_WIN 1
#define GAME_BLUE_WIN 2

// Estructura del tablero
typedef struct {
   int cells[BOARD_SIZE][BOARD_SIZE];
   int moves_count;
} board_t;

// Estructura para coordenadas
typedef struct {
   int row;
   int col;
} coord_t;

// Estructura para estadísticas de simulación
typedef struct {
   int wins;
   int losses;
   int score;  // wins - losses
} stats_t;

// Estructura para argumentos de threads
typedef struct {
   int rank;
   board_t* board;
   int player;
   int simulations;
   stats_t stats[BOARD_SIZE][BOARD_SIZE];
   uint64_t seed;
} thread_args_t;

// Variables globales de configuración
extern int g_threads;
extern int g_simulations_per_thread;

// Funciones del motor del juego
void board_init(board_t* board);
void board_copy(board_t* dest, const board_t* src);
int board_is_valid_move(const board_t* board, int row, int col);
void board_make_move(board_t* board, int row, int col, int player);
int board_check_winner(const board_t* board);
void board_print(const board_t* board);
void board_get_empty_cells(const board_t* board, coord_t* cells, int* count);

// Funciones de IA Monte Carlo
coord_t ai_best_move(const board_t* board, int player, int threads, int sims);
void* monte_carlo_worker(void* args);
void run_simulations(board_t* board, int player, int simulations, 
                     stats_t stats[BOARD_SIZE][BOARD_SIZE], uint64_t seed);

// Funciones auxiliares
void print_help(void);
void error_msg(const char* msg);

#endif
