#include "hex_game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Variables globales
int g_threads = 4;
int g_simulations_per_thread = 1000;

// ============================================================================
// FUNCIONES AUXILIARES
// ============================================================================

void print_help(void) {
   fprintf(stderr, "usage: hex [options]\n");
   fprintf(stderr, "Options:\n");
   fprintf(stderr, "  -t <threads>      Number of threads (1-32, default 4)\n");
   fprintf(stderr, "  -s <simulations>  Simulations per thread (default 1000)\n");
   exit(1);
}

void error_msg(const char* msg) {
   fprintf(stderr, "ERROR: %s\n", msg);
   exit(2);
}

// ============================================================================
// FUNCIONES DEL TABLERO
// ============================================================================

void board_init(board_t* board) {
   for (int r = 0; r < BOARD_SIZE; r++) {
      for (int c = 0; c < BOARD_SIZE; c++) {
         board->cells[r][c] = EMPTY;
      }
   }
   board->moves_count = 0;
}

void board_copy(board_t* dest, const board_t* src) {
   memcpy(dest, src, sizeof(board_t));
}

int board_is_valid_move(const board_t* board, int row, int col) {
   if (row < 0 || row >= BOARD_SIZE) return 0;
   if (col < 0 || col >= BOARD_SIZE) return 0;
   return board->cells[row][col] == EMPTY;
}

void board_make_move(board_t* board, int row, int col, int player) {
   board->cells[row][col] = player;
   board->moves_count++;
}

void board_print(const board_t* board) {
   printf("\n  ");
   for (int c = 0; c < BOARD_SIZE; c++) {
      printf(" %c", 'A' + c);
   }
   printf("\n");
   
   for (int r = 0; r < BOARD_SIZE; r++) {
      printf("%2d", r + 1);
      for (int i = 0; i < r; i++) printf(" ");
      for (int c = 0; c < BOARD_SIZE; c++) {
         char symbol = '.';
         if (board->cells[r][c] == PLAYER_RED) symbol = 'R';
         else if (board->cells[r][c] == PLAYER_BLUE) symbol = 'B';
         printf(" %c", symbol);
      }
      printf("\n");
   }
   printf("\n");
}

void board_get_empty_cells(const board_t* board, coord_t* cells, int* count) {
   *count = 0;
   for (int r = 0; r < BOARD_SIZE; r++) {
      for (int c = 0; c < BOARD_SIZE; c++) {
         if (board->cells[r][c] == EMPTY) {
            cells[*count].row = r;
            cells[*count].col = c;
            (*count)++;
         }
      }
   }
}

// ============================================================================
// DETECCIÓN DE GANADOR (DFS)
// ============================================================================

// Direcciones de vecinos en hexágono
static const int HEX_DIRS[6][2] = {
   {-1,  0}, {-1,  1},  // arriba-izq, arriba-der
   { 0, -1}, { 0,  1},  // izquierda, derecha
   { 1, -1}, { 1,  0}   // abajo-izq, abajo-der
};

static void dfs_red(const board_t* board, int r, int c, int visited[BOARD_SIZE][BOARD_SIZE], int* reached_bottom) {
   if (r < 0 || r >= BOARD_SIZE || c < 0 || c >= BOARD_SIZE) return;
   if (visited[r][c]) return;
   if (board->cells[r][c] != PLAYER_RED) return;
   
   visited[r][c] = 1;
   
   // Si llegamos a la fila inferior, ganó RED
   if (r == BOARD_SIZE - 1) {
      *reached_bottom = 1;
      return;
   }
   
   // Explorar vecinos
   for (int i = 0; i < 6; i++) {
      dfs_red(board, r + HEX_DIRS[i][0], c + HEX_DIRS[i][1], visited, reached_bottom);
      if (*reached_bottom) return;
   }
}

static void dfs_blue(const board_t* board, int r, int c, int visited[BOARD_SIZE][BOARD_SIZE], int* reached_right) {
   if (r < 0 || r >= BOARD_SIZE || c < 0 || c >= BOARD_SIZE) return;
   if (visited[r][c]) return;
   if (board->cells[r][c] != PLAYER_BLUE) return;
   
   visited[r][c] = 1;
   
   // Si llegamos a la columna derecha, ganó BLUE
   if (c == BOARD_SIZE - 1) {
      *reached_right = 1;
      return;
   }
   
   // Explorar vecinos
   for (int i = 0; i < 6; i++) {
      dfs_blue(board, r + HEX_DIRS[i][0], c + HEX_DIRS[i][1], visited, reached_right);
      if (*reached_right) return;
   }
}

int board_check_winner(const board_t* board) {
   int visited[BOARD_SIZE][BOARD_SIZE];
   
   // Verificar victoria de RED (arriba -> abajo)
   memset(visited, 0, sizeof(visited));
   for (int c = 0; c < BOARD_SIZE; c++) {
      if (board->cells[0][c] == PLAYER_RED) {
         int reached = 0;
         dfs_red(board, 0, c, visited, &reached);
         if (reached) return GAME_RED_WIN;
      }
   }
   
   // Verificar victoria de BLUE (izquierda -> derecha)
   memset(visited, 0, sizeof(visited));
   for (int r = 0; r < BOARD_SIZE; r++) {
      if (board->cells[r][0] == PLAYER_BLUE) {
         int reached = 0;
         dfs_blue(board, r, 0, visited, &reached);
         if (reached) return GAME_BLUE_WIN;
      }
   }
   
   return GAME_CONTINUE;
}
