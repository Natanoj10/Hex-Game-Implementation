#include "hex_game.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Constantes gráficas
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 900
#define HEX_RADIUS 30.0f
#define BOARD_OFFSET_X 150.0f
#define BOARD_OFFSET_Y 150.0f

// Colores
#define COLOR_EMPTY      LIGHTGRAY
#define COLOR_RED        (Color){220, 50, 50, 255}
#define COLOR_BLUE       (Color){50, 100, 220, 255}
#define COLOR_HOVER      (Color){255, 255, 100, 100}
#define COLOR_BG         (Color){40, 40, 50, 255}
#define COLOR_BORDER_RED (Color){180, 30, 30, 255}
#define COLOR_BORDER_BLUE (Color){30, 70, 180, 255}

// ============================================================================
// FUNCIONES GRÁFICAS DE HEXÁGONOS
// ============================================================================

typedef struct {
   float x;
   float y;
} vec2_t;

vec2_t hex_to_pixel(int row, int col, float radius) {
   vec2_t pos;
   float width = sqrtf(3.0f) * radius;
   float height = 2.0f * radius;
   
   pos.x = BOARD_OFFSET_X + col * width + (row * width * 0.5f);
   pos.y = BOARD_OFFSET_Y + row * height * 0.75f;
   
   return pos;
}

void draw_hexagon(float cx, float cy, float radius, Color fill, Color outline) {
   Vector2 points[6];
   
   for (int i = 0; i < 6; i++) {
      float angle = (60.0f * i - 30.0f) * DEG2RAD;
      points[i].x = cx + radius * cosf(angle);
      points[i].y = cy + radius * sinf(angle);
   }
   
   // Dibujar relleno
   for (int i = 1; i < 5; i++) {
      DrawTriangle(
         (Vector2){cx, cy},
         points[i],
         points[i+1],
         fill
      );
   }
   DrawTriangle((Vector2){cx, cy}, points[0], points[1], fill);
   DrawTriangle((Vector2){cx, cy}, points[5], points[0], fill);
   
   // Dibujar contorno
   for (int i = 0; i < 6; i++) {
      DrawLineEx(points[i], points[(i+1)%6], 2.0f, outline);
   }
}

coord_t pixel_to_hex(float px, float py, float radius) {
   coord_t result = {-1, -1};
   float min_dist = 999999.0f;
   
   for (int r = 0; r < BOARD_SIZE; r++) {
      for (int c = 0; c < BOARD_SIZE; c++) {
         vec2_t hex_pos = hex_to_pixel(r, c, radius);
         float dx = px - hex_pos.x;
         float dy = py - hex_pos.y;
         float dist = sqrtf(dx*dx + dy*dy);
         
         if (dist < radius && dist < min_dist) {
            min_dist = dist;
            result.row = r;
            result.col = c;
         }
      }
   }
   
   return result;
}

void draw_board(const board_t* board, coord_t hover_cell) {
   // Dibujar bordes del tablero
   // Borde superior (RED)
   DrawRectangle(BOARD_OFFSET_X - 20, BOARD_OFFSET_Y - 40, 
                 600, 20, COLOR_BORDER_RED);
   // Borde inferior (RED)
   DrawRectangle(BOARD_OFFSET_X + 100, BOARD_OFFSET_Y + 660, 
                 600, 20, COLOR_BORDER_RED);
   
   // Borde izquierdo (BLUE)
   DrawRectangle(BOARD_OFFSET_X - 40, BOARD_OFFSET_Y - 20, 
                 20, 700, COLOR_BORDER_BLUE);
   // Borde derecho (BLUE)
   DrawRectangle(BOARD_OFFSET_X + 600, BOARD_OFFSET_Y + 250, 
                 20, 700, COLOR_BORDER_BLUE);
   
   // Dibujar celdas
   for (int r = 0; r < BOARD_SIZE; r++) {
      for (int c = 0; c < BOARD_SIZE; c++) {
         vec2_t pos = hex_to_pixel(r, c, HEX_RADIUS);
         
         Color fill = COLOR_EMPTY;
         Color outline = DARKGRAY;
         
         if (board->cells[r][c] == PLAYER_RED) {
            fill = COLOR_RED;
            outline = COLOR_BORDER_RED;
         } else if (board->cells[r][c] == PLAYER_BLUE) {
            fill = COLOR_BLUE;
            outline = COLOR_BORDER_BLUE;
         } else if (r == hover_cell.row && c == hover_cell.col) {
            fill = COLOR_HOVER;
         }
         
         draw_hexagon(pos.x, pos.y, HEX_RADIUS, fill, outline);
         
         // Dibujar coordenadas
         if (board->cells[r][c] == EMPTY) {
            char coord[8];
            sprintf(coord, "%c%d", 'A' + c, r + 1);
            DrawText(coord, pos.x - 12, pos.y - 8, 12, DARKGRAY);
         }
      }
   }
}

void draw_ui(int turn, int game_state) {
   // Panel de información
   DrawRectangle(10, 10, 300, 120, (Color){30, 30, 40, 230});
   
   DrawText("JUEGO HEX", 20, 20, 24, WHITE);
   
   if (game_state == GAME_CONTINUE) {
      if (turn == PLAYER_RED) {
         DrawText("Turno: ROJO (Humano)", 20, 55, 18, COLOR_RED);
         DrawText("Conecta arriba-abajo", 20, 80, 14, LIGHTGRAY);
      } else {
         DrawText("Turno: AZUL (IA)", 20, 55, 18, COLOR_BLUE);
         DrawText("Conecta izq-derecha", 20, 80, 14, LIGHTGRAY);
      }
   } else if (game_state == GAME_RED_WIN) {
      DrawText("GANO ROJO!", 20, 55, 24, COLOR_RED);
      DrawText("Presiona R para reiniciar", 20, 85, 14, LIGHTGRAY);
   } else if (game_state == GAME_BLUE_WIN) {
      DrawText("GANO AZUL!", 20, 55, 24, COLOR_BLUE);
      DrawText("Presiona R para reiniciar", 20, 85, 14, LIGHTGRAY);
   }
   
   DrawText("ESC - Salir", 20, 105, 12, DARKGRAY);
   
   // Leyenda
   DrawRectangle(WINDOW_WIDTH - 250, 10, 240, 100, (Color){30, 30, 40, 230});
   DrawText("LEYENDA:", WINDOW_WIDTH - 240, 20, 16, WHITE);
   
   draw_hexagon(WINDOW_WIDTH - 220, 55, 15, COLOR_RED, COLOR_BORDER_RED);
   DrawText("Rojo (Humano)", WINDOW_WIDTH - 190, 47, 14, WHITE);
   
   draw_hexagon(WINDOW_WIDTH - 220, 85, 15, COLOR_BLUE, COLOR_BORDER_BLUE);
   DrawText("Azul (IA)", WINDOW_WIDTH - 190, 77, 14, WHITE);
}

// ============================================================================
// MAIN LOOP
// ============================================================================

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
   coord_t hover_cell = {-1, -1};
   int ai_thinking = 0;
   
   // Inicializar ventana
   InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "HEX - Monte Carlo Paralelo");
   SetTargetFPS(60);
   
   fprintf(stderr, "Juego HEX iniciado\n");
   fprintf(stderr, "Threads: %d, Simulaciones: %d\n", 
           g_threads, g_simulations_per_thread);
   fprintf(stderr, "RED (Humano) conecta arriba-abajo\n");
   fprintf(stderr, "BLUE (IA) conecta izquierda-derecha\n\n");
   
   while (!WindowShouldClose()) {
      // =====================================================================
      // UPDATE
      // =====================================================================
      
      Vector2 mouse_pos = GetMousePosition();
      hover_cell = pixel_to_hex(mouse_pos.x, mouse_pos.y, HEX_RADIUS);
      
      // Reiniciar juego
      if (IsKeyPressed(KEY_R)) {
         board_init(&board);
         current_turn = PLAYER_RED;
         game_state = GAME_CONTINUE;
         fprintf(stderr, "\n=== NUEVO JUEGO ===\n\n");
      }
      
      // Turno del humano
      if (game_state == GAME_CONTINUE && current_turn == PLAYER_RED && !ai_thinking) {
         if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (hover_cell.row >= 0 && hover_cell.col >= 0) {
               if (board_is_valid_move(&board, hover_cell.row, hover_cell.col)) {
                  fprintf(stderr, "Humano juega: %c%d\n", 
                         'A' + hover_cell.col, hover_cell.row + 1);
                  board_make_move(&board, hover_cell.row, hover_cell.col, PLAYER_RED);
                  
                  // Verificar ganador
                  game_state = board_check_winner(&board);
                  if (game_state == GAME_CONTINUE) {
                     current_turn = PLAYER_BLUE;
                  } else {
                     fprintf(stderr, "\n¡GANÓ ROJO!\n");
                  }
               }
            }
         }
      }
      
      // Turno de la IA (hacerlo en el draw para no bloquear)
      
      // =====================================================================
      // DRAW
      // =====================================================================
      
      BeginDrawing();
      ClearBackground(COLOR_BG);
      
      draw_board(&board, hover_cell);
      draw_ui(current_turn, game_state);
      
      // Turno de la IA
      if (game_state == GAME_CONTINUE && current_turn == PLAYER_BLUE && !ai_thinking) {
         ai_thinking = 1;
         coord_t ai_move = ai_best_move(&board, PLAYER_BLUE, 
                                        g_threads, g_simulations_per_thread);
         
         if (ai_move.row >= 0 && ai_move.col >= 0) {
            board_make_move(&board, ai_move.row, ai_move.col, PLAYER_BLUE);
            
            // Verificar ganador
            game_state = board_check_winner(&board);
            if (game_state == GAME_CONTINUE) {
               current_turn = PLAYER_RED;
            } else {
               fprintf(stderr, "\n¡GANÓ AZUL!\n");
            }
         }
         
         ai_thinking = 0;
      }
      
      // Mensaje de pensamiento
      if (ai_thinking) {
         DrawText("IA PENSANDO...", WINDOW_WIDTH/2 - 80, 
                  WINDOW_HEIGHT - 50, 20, YELLOW);
      }
      
      EndDrawing();
   }
   
   CloseWindow();
   return 0;
}
