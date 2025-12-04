#include "hex_game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// JUEGO POR CONSOLA (SIN GUI)
// ============================================================================

void parse_move(const char* input, coord_t* move) {
   move->row = -1;
   move->col = -1;
   
   if (strlen(input) < 2) return;
   
   // Formato: A1, B2, K11, etc.
   char col_char = input[0];
   if (col_char >= 'a' && col_char <= 'z') {
      col_char = col_char - 'a' + 'A';
   }
   
   if (col_char < 'A' || col_char > 'A' + BOARD_SIZE - 1) return;
   
   int row = atoi(input + 1) - 1;
   int col = col_char - 'A';
   
   if (row < 0 || row >= BOARD_SIZE) return;
   
   move->row = row;
   move->col = col;
}

int main(int argc, char** argv) {
   // Procesar argumentos
   for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-t") == 0 && i+1 < argc) {
         g_threads = atoi(argv[++i]);
         if (g_threads < 1 || g_threads > MAX_THREADS) {
            error_msg("Wrong number of threads (1-32)");
         }
      } else if (strcmp(argv[i], "-s") == 0 && i+1 < argc) {
         g_simulations_per_thread = atoi(argv[++i]);
         if (g_simulations_per_thread < 1) {
            error_msg("Simulations must be >= 1");
         }
      } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
         print_help();
      }
   }
   
   // Inicializar juego
   board_t board;
   board_init(&board);
   
   int current_turn = PLAYER_RED;  // Humano empieza
   int game_state = GAME_CONTINUE;
   
   printf("========================================\n");
   printf("         JUEGO HEX (Consola)           \n");
   printf("========================================\n");
   printf("Configuración:\n");
   printf("  Threads: %d\n", g_threads);
   printf("  Simulaciones por thread: %d\n", g_simulations_per_thread);
   printf("\n");
   printf("ROJO (Humano) conecta arriba-abajo\n");
   printf("AZUL (IA) conecta izquierda-derecha\n");
   printf("\n");
   printf("Formato de jugada: <Columna><Fila>\n");
   printf("Ejemplo: A1, B2, K11\n");
   printf("Escriba 'quit' para salir\n");
   printf("========================================\n\n");
   
   char input[64];
   
   while (game_state == GAME_CONTINUE) {
      board_print(&board);
      
      if (current_turn == PLAYER_RED) {
         // Turno del humano
         printf("Tu turno (ROJO): ");
         if (fgets(input, sizeof(input), stdin) == NULL) break;
         
         // Quitar newline
         input[strcspn(input, "\n")] = 0;
         
         // Verificar quit
         if (strcmp(input, "quit") == 0 || strcmp(input, "q") == 0) {
            printf("Saliendo...\n");
            break;
         }
         
         // Parse movimiento
         coord_t move;
         parse_move(input, &move);
         
         if (move.row < 0 || move.col < 0) {
            printf("Movimiento inválido. Formato: A1, B2, etc.\n\n");
            continue;
         }
         
         if (!board_is_valid_move(&board, move.row, move.col)) {
            printf("Celda ocupada o fuera del tablero.\n\n");
            continue;
         }
         
         // Hacer movimiento
         board_make_move(&board, move.row, move.col, PLAYER_RED);
         printf("Jugaste: %c%d\n\n", 'A' + move.col, move.row + 1);
         
         // Verificar ganador
         game_state = board_check_winner(&board);
         if (game_state != GAME_CONTINUE) break;
         
         current_turn = PLAYER_BLUE;
      }
      else {
         // Turno de la IA
         printf("Turno de la IA (AZUL)...\n");
         coord_t ai_move = ai_best_move(&board, PLAYER_BLUE, 
                                        g_threads, g_simulations_per_thread);
         
         if (ai_move.row < 0 || ai_move.col < 0) {
            printf("Error: IA no pudo encontrar movimiento.\n");
            break;
         }
         
         board_make_move(&board, ai_move.row, ai_move.col, PLAYER_BLUE);
         printf("\n");
         
         // Verificar ganador
         game_state = board_check_winner(&board);
         if (game_state != GAME_CONTINUE) break;
         
         current_turn = PLAYER_RED;
      }
   }
   
   // Mostrar resultado final
   board_print(&board);
   printf("========================================\n");
   if (game_state == GAME_RED_WIN) {
      printf("       ¡GANASTE! (ROJO)                \n");
   } else if (game_state == GAME_BLUE_WIN) {
      printf("       ¡GANÓ LA IA! (AZUL)             \n");
   } else {
      printf("       JUEGO CANCELADO                 \n");
   }
   printf("========================================\n");
   
   return 0;
}
