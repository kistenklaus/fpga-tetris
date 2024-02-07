/**
 * @author      : kistenklaus (karlsasssie@gmail.com)
 * @created     : 07/02/2024
 * @filename    : main
 */
#include <linux/limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define CANVAS_WIDTH 32
#define CANVAS_HEIGHT 24
#define BLOCKS_PER_PIECE 4
#define MAX_PIECE_COUNT 128

typedef enum {
  COLOR_BLACK = 0,
  COLOR_RED = 1,
  COLOR_ORANGE = 2,
  COLOR_YELLOW = 3,
  COLOR_GREEN = 4,
  COLOR_CYAN = 5,
  COLOR_PURPLE = 6,
  COLOR_PINK = 7,
} color_t;

typedef struct {
  color_t m_colors[CANVAS_WIDTH * CANVAS_HEIGHT];
} mock_display;

mock_display g_display;

void mock_display_render() {
  for (unsigned int x = 0; x < CANVAS_WIDTH; x++) {
    printf("=");
  }
  printf("\n");
  for (unsigned int y = 0; y < CANVAS_HEIGHT; y++) {
    for (unsigned int x = 0; x < CANVAS_WIDTH; x++) {
      switch (g_display.m_colors[y * CANVAS_WIDTH + x]) {
      case COLOR_BLACK:
        printf("\033[0;30m#\033[0;0m");
        break;
      case COLOR_RED:
        printf("\033[0;31m#\033[0;0m");
        break;
      case COLOR_ORANGE:
        printf("\033[0;34m#\033[0;0m");
        break;
      case COLOR_YELLOW:
        printf("\033[0;33m#\033[0;0m");
        break;
      case COLOR_GREEN:
        printf("\033[0;32m#\033[0;0m");
        break;
      case COLOR_CYAN:
        printf("\033[0;36m#\033[0;0m");
        break;
      case COLOR_PURPLE:
        printf("\033[0;35m#\033[0;0m");
        break;
      case COLOR_PINK:
        printf("\033[0;37m#\033[0;0m");
        break;
      }
    }
    printf(" : %u\n", y);
  }
}

void set_pixel(unsigned int x, unsigned int y, color_t color) {
  g_display.m_colors[y * CANVAS_WIDTH + x] = color;
}

typedef struct {
  unsigned int x;
  unsigned int y;
} vec2;

vec2 vec2_new(unsigned int x, unsigned int y) {
  vec2 vec;
  vec.x = x;
  vec.y = y;
  return vec;
}

vec2 vec2_add(vec2 *a, vec2 *b) { return vec2_new(a->x + b->x, a->y + b->y); }

typedef vec2 tetris_piece_block;

typedef enum {
  PIECE_TYPE_I = 1,
  PIECE_TYPE_J,
  PIECE_TYPE_L,
  PIECE_TYPE_O,
  PIECE_TYPE_S,
  PIECE_TYPE_T,
  PIECE_TYPE_Z,
} tetris_piece_type;

color_t tetris_piece_type_to_color(tetris_piece_type type) {
  return (color_t)type;
}

typedef struct {
  tetris_piece_type m_tag;
  vec2 m_position;
  tetris_piece_block m_blocks[BLOCKS_PER_PIECE];
} tetris_piece_t;

void tetris_piece_new_i(tetris_piece_t *self, vec2 position) {
  self->m_tag = PIECE_TYPE_I;
  self->m_position = position;
  self->m_blocks[0].x = 0;
  self->m_blocks[0].y = 0;
  self->m_blocks[1].x = 1;
  self->m_blocks[1].y = 0;
  self->m_blocks[2].x = 2;
  self->m_blocks[2].y = 0;
  self->m_blocks[3].x = 3;
  self->m_blocks[3].y = 0;
}

void tetris_piece_new_j(tetris_piece_t *self, vec2 position) {
  self->m_tag = PIECE_TYPE_J;
  self->m_position = position;
  self->m_blocks[0].x = 0;
  self->m_blocks[0].y = 2;
  self->m_blocks[1].x = 1;
  self->m_blocks[1].y = 2;
  self->m_blocks[2].x = 1;
  self->m_blocks[2].y = 1;
  self->m_blocks[3].x = 1;
  self->m_blocks[3].y = 0;
}

void tetris_piece_new_l(tetris_piece_t *self, vec2 position) {
  self->m_tag = PIECE_TYPE_L;
  self->m_position = position;
  self->m_blocks[0].x = 0;
  self->m_blocks[0].y = 0;
  self->m_blocks[1].x = 1;
  self->m_blocks[1].y = 0;
  self->m_blocks[2].x = 2;
  self->m_blocks[2].y = 0;
  self->m_blocks[3].x = 2;
  self->m_blocks[3].y = 1;
}

void tetris_piece_new_o(tetris_piece_t *self, vec2 position) {
  self->m_tag = PIECE_TYPE_O;
  self->m_position = position;
  self->m_blocks[0].x = 0;
  self->m_blocks[0].y = 0;
  self->m_blocks[1].x = 1;
  self->m_blocks[1].y = 0;
  self->m_blocks[2].x = 0;
  self->m_blocks[2].y = 1;
  self->m_blocks[3].x = 1;
  self->m_blocks[3].y = 1;
}

void tetris_piece_new_s(tetris_piece_t *self, vec2 position) {
  self->m_tag = PIECE_TYPE_Z;
  self->m_position = position;
  self->m_blocks[0].x = 0;
  self->m_blocks[0].y = 1;
  self->m_blocks[1].x = 1;
  self->m_blocks[1].y = 1;
  self->m_blocks[2].x = 1;
  self->m_blocks[2].y = 0;
  self->m_blocks[3].x = 2;
  self->m_blocks[3].y = 0;
}

void tetris_piece_new_t(tetris_piece_t *self, vec2 position) {
  self->m_tag = PIECE_TYPE_T;
  self->m_position = position;
  self->m_blocks[0].x = 0;
  self->m_blocks[0].y = 0;
  self->m_blocks[1].x = 1;
  self->m_blocks[1].y = 0;
  self->m_blocks[2].x = 2;
  self->m_blocks[2].y = 0;
  self->m_blocks[3].x = 1;
  self->m_blocks[3].y = 1;
}

void tetris_piece_new_z(tetris_piece_t *self, vec2 position) {
  self->m_tag = PIECE_TYPE_Z;
  self->m_position = position;
  self->m_blocks[0].x = 0;
  self->m_blocks[0].y = 0;
  self->m_blocks[1].x = 1;
  self->m_blocks[1].y = 0;
  self->m_blocks[2].x = 1;
  self->m_blocks[2].y = 1;
  self->m_blocks[3].x = 2;
  self->m_blocks[3].y = 1;
}

uint32_t hash(uint32_t seed) {
  uint32_t hash = 5381;
  hash = ((hash << 5) + hash) + ((char *)&seed)[0];
  hash = ((hash << 5) + hash) + ((char *)&seed)[1];
  return hash;
}

typedef color_t tetris_block;

typedef struct {
  tetris_block m_blocks[CANVAS_WIDTH * CANVAS_HEIGHT];
  tetris_piece_t m_fallingPiece;
  unsigned long m_gameSeed;
} tetris_game;

void tetris_game_new(tetris_game *self) {
  for (unsigned int i = 0; i < CANVAS_WIDTH * CANVAS_HEIGHT; i++) {
    self->m_blocks[i] = 0;
  }
  tetris_piece_new_z(&self->m_fallingPiece, vec2_new(CANVAS_WIDTH / 2 - 2, 0));
  self->m_gameSeed = 14123414123;
}

void tetris_game_render(tetris_game *self) {
  // render blocks
  for (unsigned int x = 0; x < CANVAS_WIDTH; x++) {
    for (unsigned int y = 0; y < CANVAS_HEIGHT; y++) {
      set_pixel(x, y, self->m_blocks[y * CANVAS_WIDTH + x]);
    }
  }
  // render falling piece
  for (unsigned int b = 0; b < BLOCKS_PER_PIECE; b++) {
    vec2 blockPos = vec2_add(&self->m_fallingPiece.m_position,
                             &self->m_fallingPiece.m_blocks[b]);
    set_pixel(blockPos.x, blockPos.y,
              tetris_piece_type_to_color(self->m_fallingPiece.m_tag));
  }
}

void tetris_game_place(tetris_game *self) {
  if(self->m_fallingPiece.m_position.y == 0) { 
    tetris_game_new(self);
    return;
  }
  for (unsigned int b = 0; b < BLOCKS_PER_PIECE; b++) {
    vec2 blockPos = vec2_add(&self->m_fallingPiece.m_position,
                             &self->m_fallingPiece.m_blocks[b]);
    self->m_blocks[blockPos.y * CANVAS_WIDTH + blockPos.x] =
        tetris_piece_type_to_color(self->m_fallingPiece.m_tag);
    printf("placing block %u %u\n", blockPos.x, blockPos.y);
  }
  // select next piece (never select a piece twice)

  self->m_gameSeed = hash(self->m_gameSeed);
  tetris_piece_type newType;
  while ((newType = ((self->m_gameSeed % 7) + 1)) ==
         self->m_fallingPiece.m_tag) {
    self->m_gameSeed = hash(self->m_gameSeed ^ 12397198471298371);
  }

  vec2 spawnPos = vec2_new(CANVAS_WIDTH / 2 - 2, 0);
  switch (newType) {
  case PIECE_TYPE_I:
    tetris_piece_new_i(&self->m_fallingPiece, spawnPos);
    break;
  case PIECE_TYPE_J:
    tetris_piece_new_j(&self->m_fallingPiece, spawnPos);
    break;
  case PIECE_TYPE_L:
    tetris_piece_new_l(&self->m_fallingPiece, spawnPos);
    break;
  case PIECE_TYPE_O:
    tetris_piece_new_o(&self->m_fallingPiece, spawnPos);
    break;
  case PIECE_TYPE_S:
    tetris_piece_new_s(&self->m_fallingPiece, spawnPos);
    break;
  case PIECE_TYPE_T:
    tetris_piece_new_t(&self->m_fallingPiece, spawnPos);
    break;
  case PIECE_TYPE_Z:
    tetris_piece_new_z(&self->m_fallingPiece, spawnPos);
    break;
  }
}

void tetris_game_update(tetris_game *self) {
  vec2 potentialPos = vec2_new(self->m_fallingPiece.m_position.x,
                               self->m_fallingPiece.m_position.y + 1);

  unsigned int height;
  switch (self->m_fallingPiece.m_tag) {
  case PIECE_TYPE_I:
    height = 1;
    break;
  case PIECE_TYPE_J:
    height = 2;
    break;
  case PIECE_TYPE_L:
    height = 3;
    break;
  case PIECE_TYPE_O:
    height = 2;
    break;
  case PIECE_TYPE_S:
    height = 2;
    break;
  case PIECE_TYPE_T:
    height = 2;
    break;
  case PIECE_TYPE_Z:
    height = 2;
    break;
  }
  // boundary box collisions
  if (potentialPos.y > CANVAS_HEIGHT - height) {
    tetris_game_place(self);
    return;
  }

  // block collisions
  for (unsigned int b = 0; b < BLOCKS_PER_PIECE; b++) {
    vec2 potentialBlockPos =
        vec2_add(&potentialPos, &self->m_fallingPiece.m_blocks[b]);
    if (self->m_blocks[(potentialBlockPos.y) * CANVAS_WIDTH + potentialBlockPos.x] !=
        COLOR_BLACK) {
      tetris_game_place(self);
      return;
    }
  }

  self->m_fallingPiece.m_position = potentialPos;
}

tetris_game g_tetris;

int main() {

  tetris_game_new(&g_tetris);

  while (1) {
    tetris_game_update(&g_tetris);
    tetris_game_render(&g_tetris);
    mock_display_render();
    usleep(1000 * 100);
  }

  printf("Hello World\n");
  return 0;
}
