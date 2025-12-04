#include "hex_game.h"
#include "pcg_basic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// ============================================================================
// SIMULACIÓN DE JUEGO ALEATORIO
// ============================================================================

static int simulate_random_game(board_t* sim_board, int current_player, pcg32_random_t* rng) {
   coord_t empty_cells[BOARD_SIZE * BOARD_SIZE];
   int empty_count;
   
   while (1) {
      // Verificar si hay ganador
      int winner = board_check_winner(sim_board);
      if (winner != GAME_CONTINUE) {
         return winner;
      }
      
      // Obtener celdas vacías
      board_get_empty_cells(sim_board, empty_cells, &empty_count);
      
      // Si no hay movimientos, es empate (no debería pasar en HEX)
      if (empty_count == 0) {
         return GAME_CONTINUE;
      }
      
      // Elegir movimiento aleatorio
      int move_idx = pcg32_boundedrand_r(rng, empty_count);
      coord_t move = empty_cells[move_idx];
      
      // Hacer el movimiento
      board_make_move(sim_board, move.row, move.col, current_player);
      
      // Alternar jugador
      current_player = (current_player == PLAYER_RED) ? PLAYER_BLUE : PLAYER_RED;
   }
}

// ============================================================================
// SIMULACIONES PARA UNA POSICIÓN
// ============================================================================

void run_simulations(board_t* board, int player, int simulations, 
                     stats_t stats[BOARD_SIZE][BOARD_SIZE], uint64_t seed) {
   pcg32_random_t rng;
   pcg32_srandom_r(&rng, seed, 0xa02bdbf7bb3c0a7ULL);
   
   // Para cada celda vacía
   for (int r = 0; r < BOARD_SIZE; r++) {
      for (int c = 0; c < BOARD_SIZE; c++) {
         if (board->cells[r][c] != EMPTY) {
            stats[r][c].wins = 0;
            stats[r][c].losses = 0;
            stats[r][c].score = -999999;
            continue;
         }
         
         stats[r][c].wins = 0;
         stats[r][c].losses = 0;
         
         // Hacer N simulaciones desde esta posición
         for (int sim = 0; sim < simulations; sim++) {
            board_t sim_board;
            board_copy(&sim_board, board);
            
            // Hacer el movimiento de prueba
            board_make_move(&sim_board, r, c, player);
            
            // Simular el resto del juego
            int opponent = (player == PLAYER_RED) ? PLAYER_BLUE : PLAYER_RED;
            int result = simulate_random_game(&sim_board, opponent, &rng);
            
            // Actualizar estadísticas
            if (result == player) {
               stats[r][c].wins++;
            } else if (result != GAME_CONTINUE) {
               stats[r][c].losses++;
            }
         }
         
         stats[r][c].score = stats[r][c].wins - stats[r][c].losses;
      }
   }
}

// ============================================================================
// WORKER THREAD PARA MONTE CARLO PARALELO
// ============================================================================

void* monte_carlo_worker(void* args) {
   thread_args_t* targs = (thread_args_t*)args;
   
   // Ejecutar simulaciones
   run_simulations(targs->board, targs->player, targs->simulations, 
                   targs->stats, targs->seed);
   
   return NULL;
}

// ============================================================================
// FUNCIÓN PRINCIPAL DE IA
// ============================================================================

coord_t ai_best_move(const board_t* board, int player, int threads, int sims) {
   pthread_t tlist[MAX_THREADS];
   thread_args_t* targs[MAX_THREADS];
   
   if (threads > MAX_THREADS) threads = MAX_THREADS;
   
   fprintf(stderr, "IA pensando con %d threads, %d simulaciones cada uno...\n", 
           threads, sims);
   
   // Crear argumentos para cada thread
   for (int i = 0; i < threads; i++) {
      targs[i] = malloc(sizeof(thread_args_t));
      if (targs[i] == NULL) error_msg("Allocation failure");
      
      targs[i]->rank = i;
      targs[i]->board = malloc(sizeof(board_t));
      if (targs[i]->board == NULL) error_msg("Allocation failure");
      board_copy(targs[i]->board, board);
      targs[i]->player = player;
      targs[i]->simulations = sims;
      targs[i]->seed = 0x123456789abcdefULL + i * 0x9e3779b97f4a7c15ULL;
      
      // Inicializar estadísticas
      for (int r = 0; r < BOARD_SIZE; r++) {
         for (int c = 0; c < BOARD_SIZE; c++) {
            targs[i]->stats[r][c].wins = 0;
            targs[i]->stats[r][c].losses = 0;
            targs[i]->stats[r][c].score = 0;
         }
      }
   }
   
   // Lanzar threads
   for (int i = 0; i < threads; i++) {
      pthread_create(&tlist[i], NULL, monte_carlo_worker, targs[i]);
   }
   
   // Esperar threads
   for (int i = 0; i < threads; i++) {
      pthread_join(tlist[i], NULL);
   }
   
   // Combinar estadísticas de todos los threads
   stats_t combined[BOARD_SIZE][BOARD_SIZE];
   for (int r = 0; r < BOARD_SIZE; r++) {
      for (int c = 0; c < BOARD_SIZE; c++) {
         combined[r][c].wins = 0;
         combined[r][c].losses = 0;
         combined[r][c].score = 0;
         
         for (int t = 0; t < threads; t++) {
            combined[r][c].wins += targs[t]->stats[r][c].wins;
            combined[r][c].losses += targs[t]->stats[r][c].losses;
         }
         
         combined[r][c].score = combined[r][c].wins - combined[r][c].losses;
      }
   }
   
   // Encontrar la mejor jugada
   coord_t best_move = {-1, -1};
   int best_score = -999999;
   
   for (int r = 0; r < BOARD_SIZE; r++) {
      for (int c = 0; c < BOARD_SIZE; c++) {
         if (board->cells[r][c] == EMPTY) {
            if (combined[r][c].score > best_score) {
               best_score = combined[r][c].score;
               best_move.row = r;
               best_move.col = c;
            }
         }
      }
   }
   
   // Debug: mostrar estadísticas de las mejores jugadas
   fprintf(stderr, "Top 5 jugadas:\n");
   for (int i = 0; i < 5; i++) {
      int max_score = -999999;
      coord_t max_coord = {-1, -1};
      
      for (int r = 0; r < BOARD_SIZE; r++) {
         for (int c = 0; c < BOARD_SIZE; c++) {
            if (board->cells[r][c] == EMPTY && combined[r][c].score > max_score) {
               int already_shown = 0;
               // No mostrar las que ya mostramos
               for (int j = 0; j < i; j++) {
                  // Este es un check simplificado
               }
               if (!already_shown) {
                  max_score = combined[r][c].score;
                  max_coord.row = r;
                  max_coord.col = c;
               }
            }
         }
      }
      
      if (max_coord.row >= 0) {
         fprintf(stderr, "  %c%d: wins=%d, losses=%d, score=%d\n",
                'A' + max_coord.col, max_coord.row + 1,
                combined[max_coord.row][max_coord.col].wins,
                combined[max_coord.row][max_coord.col].losses,
                combined[max_coord.row][max_coord.col].score);
      }
   }
   
   // Liberar memoria
   for (int i = 0; i < threads; i++) {
      free(targs[i]->board);
      free(targs[i]);
   }
   
   fprintf(stderr, "IA juega: %c%d\n", 'A' + best_move.col, best_move.row + 1);
   
   return best_move;
}
