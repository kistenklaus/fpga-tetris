/******************************************************************************
 *
 * Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
 ******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "sleep.h"

typedef enum {
	COLOR_BLACK = 0,
	COLOR_RED = 1,
	COLOR_BLUE = 2,
	COLOR_ORANGE = 3,
	COLOR_YELLOW = 4,
	COLOR_PINK = 5,
	COLOR_CYAN = 6,
	COLOR_GREEN = 7,
} color_t;

#define DISPLAY_COLOR *(color_t*)(0x41220000)
#define DISPLAY_PIXEL *(unsigned int*)(0x41210000)
#define DISPLAY_SET *(int*)(0x41200000)

#define LEFT_INPUT   *(int*)(0x41230000)
#define RIGHT_INPUT  *(int*)(0x41240000)
#define ROTATE_INPUT *(int*)(0x41250000)

#define CANVAS_WIDTH 10
#define CANVAS_HEIGHT 22
#define BLOCKS_PER_PIECE 4
#define MAX_PIECE_COUNT 128

color_t g_pixel_buffer[CANVAS_WIDTH * CANVAS_HEIGHT];

void set_pixel(unsigned int x, unsigned int y, color_t color) {
	g_pixel_buffer[y * CANVAS_WIDTH + x] = color;
}

void clear(color_t fill) {
	for (unsigned int i = 0; i < CANVAS_WIDTH * CANVAS_HEIGHT; ++i) {
		g_pixel_buffer[i] = fill;
	}
}

void render() {
	for (unsigned int i = 0; i < CANVAS_WIDTH * CANVAS_HEIGHT; ++i) {
		DISPLAY_COLOR = g_pixel_buffer[i];
		DISPLAY_PIXEL = i;
		DISPLAY_SET = 1;
		usleep(1);
		DISPLAY_SET = 0;
	}
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

vec2 vec2_add(vec2 *a, vec2 *b) {
	return vec2_new(a->x + b->x, a->y + b->y);
}

void vec2_madd(vec2* self, int dx, int dy) {
	self->x += dx;
	self->y += dy;
}

typedef vec2 tetris_piece_block;


typedef enum {
	PIECE_TYPE_I = COLOR_RED,
	PIECE_TYPE_J = COLOR_BLUE,
	PIECE_TYPE_L = COLOR_ORANGE,
	PIECE_TYPE_O = COLOR_YELLOW,
	PIECE_TYPE_S = COLOR_PINK,
	PIECE_TYPE_T = COLOR_CYAN,
	PIECE_TYPE_Z = COLOR_GREEN,
} tetris_piece_type;

color_t tetris_piece_type_to_color(tetris_piece_type type) {
	return (color_t) type;
}

typedef struct {
	tetris_piece_type m_tag;
	vec2 m_position;
	tetris_piece_block m_blocks[BLOCKS_PER_PIECE];
	int m_rotation;
} tetris_piece_t;

void tetris_piece_new_i(tetris_piece_t *self, vec2 position) {
	self->m_tag = PIECE_TYPE_I;
	self->m_position = position;
	self->m_blocks[0] = vec2_new(0, 2);
	self->m_blocks[1] = vec2_new(1, 2);
	self->m_blocks[2] = vec2_new(2, 2);
	self->m_blocks[3] = vec2_new(3, 2);
	self->m_rotation = 0;
}

void tetris_piece_new_j(tetris_piece_t *self, vec2 position) {
	self->m_tag = PIECE_TYPE_J;
	self->m_position = position;
	self->m_blocks[0] = vec2_new(0, 1);
	self->m_blocks[1] = vec2_new(1, 1);
	self->m_blocks[2] = vec2_new(2, 1);
	self->m_blocks[3] = vec2_new(2, 2);
	self->m_rotation = 0;
}

void tetris_piece_new_l(tetris_piece_t *self, vec2 position) {
	self->m_tag = PIECE_TYPE_L;
	self->m_position = position;
	self->m_blocks[0] = vec2_new(0, 2);
	self->m_blocks[1] = vec2_new(0, 1);
	self->m_blocks[2] = vec2_new(1, 1);
	self->m_blocks[3] = vec2_new(2, 1);
	self->m_rotation = 0;
}

void tetris_piece_new_o(tetris_piece_t *self, vec2 position) {
	self->m_tag = PIECE_TYPE_O;
	self->m_position = position;
	self->m_blocks[0].x = 0 + 1;
	self->m_blocks[0].y = 0 + 1;
	self->m_blocks[1].x = 1 + 1;
	self->m_blocks[1].y = 0 + 1;
	self->m_blocks[2].x = 0 + 1;
	self->m_blocks[2].y = 1 + 1;
	self->m_blocks[3].x = 1 + 1;
	self->m_blocks[3].y = 1 + 1;
	self->m_rotation = 0;
}

void tetris_piece_new_s(tetris_piece_t *self, vec2 position) {
	self->m_tag = PIECE_TYPE_Z;
	self->m_position = position;
	self->m_blocks[0] = vec2_new(0, 2);
	self->m_blocks[1] = vec2_new(1, 2);
	self->m_blocks[2] = vec2_new(1, 1);
	self->m_blocks[3] = vec2_new(2, 1);
	self->m_rotation = 0;
}

void tetris_piece_new_t(tetris_piece_t *self, vec2 position) {
	self->m_tag = PIECE_TYPE_T;
	self->m_position = position;
	self->m_blocks[0] = vec2_new(0, 1);
	self->m_blocks[1] = vec2_new(1, 1);
	self->m_blocks[2] = vec2_new(2, 1);
	self->m_blocks[3] = vec2_new(1, 2);
	self->m_rotation = 0;
}

void tetris_piece_new_z(tetris_piece_t *self, vec2 position) {
	self->m_tag = PIECE_TYPE_Z;
	self->m_position = position;
	self->m_blocks[0] = vec2_new(0, 1);
	self->m_blocks[1] = vec2_new(1, 1);
	self->m_blocks[2] = vec2_new(1, 2);
	self->m_blocks[3] = vec2_new(2, 2);
	self->m_rotation = 0;
}

uint32_t hash(uint32_t seed) {
	uint32_t hash = 5381;
	hash = ((hash << 5) + hash) + ((char *) &seed)[0];
	hash = ((hash << 5) + hash) + ((char *) &seed)[1];
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
	tetris_piece_new_z(&self->m_fallingPiece,
			vec2_new(CANVAS_WIDTH / 2 - 2, 0));
	self->m_gameSeed = 0;
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
	if (self->m_fallingPiece.m_position.y == 0) {
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
	while ((newType = ((self->m_gameSeed % 7) + 1))
			== self->m_fallingPiece.m_tag) {
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

int g_prev_right_control = 0;

int getRightControll() {
	int right_control = RIGHT_INPUT;
	int pressed = !g_prev_right_control && right_control;
	g_prev_right_control = right_control;
	return pressed;
}

int g_prev_left_control = 0;

int getLeftControll() {
	int left_control = LEFT_INPUT;
	int pressed = !g_prev_left_control && left_control;
	g_prev_left_control = left_control;
	return pressed;
}/******************************************************************************
 *
 * Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
 ******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "sleep.h"

typedef enum {
	COLOR_BLACK = 0,
	COLOR_RED = 1,
	COLOR_BLUE = 2,
	COLOR_ORANGE = 3,
	COLOR_YELLOW = 4,
	COLOR_PINK = 5,
	COLOR_CYAN = 6,
	COLOR_GREEN = 7,
} color_t;

#define DISPLAY_COLOR *(color_t*)(0x41220000)
#define DISPLAY_PIXEL *(unsigned int*)(0x41210000)
#define DISPLAY_SET *(int*)(0x41200000)

#define LEFT_INPUT   *(int*)(0x41230000)
#define RIGHT_INPUT  *(int*)(0x41240000)
#define ROTATE_INPUT *(int*)(0x41250000)

#define CANVAS_WIDTH 10
#define CANVAS_HEIGHT 22
#define BLOCKS_PER_PIECE 4
#define MAX_PIECE_COUNT 128

color_t g_pixel_buffer[CANVAS_WIDTH * CANVAS_HEIGHT];

void set_pixel(unsigned int x, unsigned int y, color_t color) {
	g_pixel_buffer[y * CANVAS_WIDTH + x] = color;
}

void clear(color_t fill) {
	for (unsigned int i = 0; i < CANVAS_WIDTH * CANVAS_HEIGHT; ++i) {
		g_pixel_buffer[i] = fill;
	}
}

void render() {
	for (unsigned int i = 0; i < CANVAS_WIDTH * CANVAS_HEIGHT; ++i) {
		DISPLAY_COLOR = g_pixel_buffer[i];
		DISPLAY_PIXEL = i;
		DISPLAY_SET = 1;
		usleep(1);
		DISPLAY_SET = 0;
	}
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

vec2 vec2_add(vec2 *a, vec2 *b) {
	return vec2_new(a->x + b->x, a->y + b->y);
}

void vec2_madd(vec2* self, int dx, int dy) {
	self->x += dx;
	self->y += dy;
}

typedef vec2 tetris_piece_block;


typedef enum {
	PIECE_TYPE_I = COLOR_RED,
	PIECE_TYPE_J = COLOR_BLUE,
	PIECE_TYPE_L = COLOR_ORANGE,
	PIECE_TYPE_O = COLOR_YELLOW,
	PIECE_TYPE_S = COLOR_PINK,
	PIECE_TYPE_T = COLOR_CYAN,
	PIECE_TYPE_Z = COLOR_GREEN,
} tetris_piece_type;

color_t tetris_piece_type_to_color(tetris_piece_type type) {
	return (color_t) type;
}

typedef struct {
	tetris_piece_type m_tag;
	vec2 m_position;
	tetris_piece_block m_blocks[BLOCKS_PER_PIECE];
	int m_rotation;
} tetris_piece_t;

void tetris_piece_new_i(tetris_piece_t *self, vec2 position) {
	self->m_tag = PIECE_TYPE_I;
	self->m_position = position;
	self->m_blocks[0] = vec2_new(0, 2);
	self->m_blocks[1] = vec2_new(1, 2);
	self->m_blocks[2] = vec2_new(2, 2);
	self->m_blocks[3] = vec2_new(3, 2);
	self->m_rotation = 0;
}

void tetris_piece_new_j(tetris_piece_t *self, vec2 position) {
	self->m_tag = PIECE_TYPE_J;
	self->m_position = position;
	self->m_blocks[0] = vec2_new(0, 1);
	self->m_blocks[1] = vec2_new(1, 1);
	self->m_blocks[2] = vec2_new(2, 1);
	self->m_blocks[3] = vec2_new(2, 2);
	self->m_rotation = 0;
}

void tetris_piece_new_l(tetris_piece_t *self, vec2 position) {
	self->m_tag = PIECE_TYPE_L;
	self->m_position = position;
	self->m_blocks[0] = vec2_new(0, 2);
	self->m_blocks[1] = vec2_new(0, 1);
	self->m_blocks[2] = vec2_new(1, 1);
	self->m_blocks[3] = vec2_new(2, 1);
	self->m_rotation = 0;
}

void tetris_piece_new_o(tetris_piece_t *self, vec2 position) {
	self->m_tag = PIECE_TYPE_O;
	self->m_position = position;
	self->m_blocks[0].x = 0 + 1;
	self->m_blocks[0].y = 0 + 1;
	self->m_blocks[1].x = 1 + 1;
	self->m_blocks[1].y = 0 + 1;
	self->m_blocks[2].x = 0 + 1;
	self->m_blocks[2].y = 1 + 1;
	self->m_blocks[3].x = 1 + 1;
	self->m_blocks[3].y = 1 + 1;
	self->m_rotation = 0;
}

void tetris_piece_new_s(tetris_piece_t *self, vec2 position) {
	self->m_tag = PIECE_TYPE_Z;
	self->m_position = position;
	self->m_blocks[0] = vec2_new(0, 2);
	self->m_blocks[1] = vec2_new(1, 2);
	self->m_blocks[2] = vec2_new(1, 1);
	self->m_blocks[3] = vec2_new(2, 1);
	self->m_rotation = 0;
}

void tetris_piece_new_t(tetris_piece_t *self, vec2 position) {
	self->m_tag = PIECE_TYPE_T;
	self->m_position = position;
	self->m_blocks[0] = vec2_new(0, 1);
	self->m_blocks[1] = vec2_new(1, 1);
	self->m_blocks[2] = vec2_new(2, 1);
	self->m_blocks[3] = vec2_new(1, 2);
	self->m_rotation = 0;
}

void tetris_piece_new_z(tetris_piece_t *self, vec2 position) {
	self->m_tag = PIECE_TYPE_Z;
	self->m_position = position;
	self->m_blocks[0] = vec2_new(0, 1);
	self->m_blocks[1] = vec2_new(1, 1);
	self->m_blocks[2] = vec2_new(1, 2);
	self->m_blocks[3] = vec2_new(2, 2);
	self->m_rotation = 0;
}

uint32_t hash(uint32_t seed) {
	uint32_t hash = 5381;
	hash = ((hash << 5) + hash) + ((char *) &seed)[0];
	hash = ((hash << 5) + hash) + ((char *) &seed)[1];
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
	tetris_piece_new_z(&self->m_fallingPiece,
			vec2_new(CANVAS_WIDTH / 2 - 2, 0));
	self->m_gameSeed = 0;
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
	if (self->m_fallingPiece.m_position.y == 0) {
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
	while ((newType = ((self->m_gameSeed % 7) + 1))
			== self->m_fallingPiece.m_tag) {
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

int g_prev_right_control = 0;

int getRightControll() {
	int right_control = RIGHT_INPUT;
	int pressed = !g_prev_right_control && right_control;
	g_prev_right_control = right_control;
	return pressed;
}

int g_prev_left_control = 0;

int getLeftControll() {
	int left_control = LEFT_INPUT;
	int pressed = !g_prev_left_control && left_control;
	g_prev_left_control = left_control;
	return pressed;
}

int g_prev_rotate_control = 0;

int getRotateControll() {
	int rotate_control = ROTATE_INPUT;
	int pressed = !g_prev_rotate_control && rotate_control;
	g_prev_rotate_control = rotate_control;
	return pressed;
}

int subticks = 0;

void tetris_game_update(tetris_game *self) {
	vec2 potentialPos = self->m_fallingPiece.m_position;

	if (getRightControll()) {
		vec2_madd(&potentialPos, 1, 0);
	}
	if (getLeftControll()) {
		vec2_madd(&potentialPos, -1, 0);
	}

	unsigned int max_x = 0; // unsigned! checks both borders!
	for (unsigned int i = 0; i < BLOCKS_PER_PIECE; i++) {
		unsigned int x = self->m_fallingPiece.m_blocks[i].x + potentialPos.x;
		if (x > max_x)
			max_x = x;
	}

	if (max_x >= CANVAS_WIDTH) {
		potentialPos = self->m_fallingPiece.m_position; // ignore movement!
	}
	// check for block collisions
	for (unsigned int b = 0; b < BLOCKS_PER_PIECE; b++) {
		vec2 potentialBlockPos = vec2_add(&potentialPos,
				&self->m_fallingPiece.m_blocks[b]);
		if (self->m_blocks[(potentialBlockPos.y) * CANVAS_WIDTH
				+ potentialBlockPos.x] != COLOR_BLACK) {
			potentialPos = self->m_fallingPiece.m_position;
		}
	}

	if (getRotateControll()) {
		int potRot = (self->m_fallingPiece.m_rotation + 1) % 4;
		// determine new blocks!
		vec2 potBlocks[BLOCKS_PER_PIECE] = { };
		switch (self->m_fallingPiece.m_tag) {
		case PIECE_TYPE_I:
			switch (potRot) {
			case 0:
			case 2:
				potBlocks[0] = vec2_new(0, 2);
				potBlocks[1] = vec2_new(1, 2);
				potBlocks[2] = vec2_new(2, 2);
				potBlocks[3] = vec2_new(3, 2);
				break;
			case 1:
			case 3:
				potBlocks[0] = vec2_new(2, 0);
				potBlocks[1] = vec2_new(2, 1);
				potBlocks[2] = vec2_new(2, 2);
				potBlocks[3] = vec2_new(2, 3);
			}
			break;
		case PIECE_TYPE_J:
			switch (potRot) {
			case 0:
				potBlocks[0] = vec2_new(0, 1);
				potBlocks[1] = vec2_new(1, 1);
				potBlocks[2] = vec2_new(2, 1);
				potBlocks[3] = vec2_new(2, 2);
				break;
			case 1:
				potBlocks[0] = vec2_new(1, 0);
				potBlocks[1] = vec2_new(1, 1);
				potBlocks[2] = vec2_new(1, 2);
				potBlocks[3] = vec2_new(0, 2);
				break;
			case 2:
				potBlocks[0] = vec2_new(0, 0);
				potBlocks[1] = vec2_new(0, 1);
				potBlocks[2] = vec2_new(1, 1);
				potBlocks[3] = vec2_new(2, 1);
				break;
			case 3:
				potBlocks[0] = vec2_new(2, 0);
				potBlocks[1] = vec2_new(1, 0);
				potBlocks[2] = vec2_new(1, 1);
				potBlocks[3] = vec2_new(1, 2);
				break;
			}
			break;
		case PIECE_TYPE_L:
			switch (potRot) {
			case 0:
				potBlocks[0] = vec2_new(0, 2);
				potBlocks[1] = vec2_new(0, 1);
				potBlocks[2] = vec2_new(1, 1);
				potBlocks[3] = vec2_new(2, 1);
				break;
			case 1:
				potBlocks[0] = vec2_new(0, 0);
				potBlocks[1] = vec2_new(1, 0);
				potBlocks[2] = vec2_new(1, 1);
				potBlocks[3] = vec2_new(1, 2);
				break;
			case 2:
				potBlocks[0] = vec2_new(0, 1);
				potBlocks[1] = vec2_new(1, 1);
				potBlocks[2] = vec2_new(2, 1);
				potBlocks[3] = vec2_new(2, 0);
				break;
			case 3:
				potBlocks[0] = vec2_new(1, 0);
				potBlocks[1] = vec2_new(1, 1);
				potBlocks[2] = vec2_new(1, 2);
				potBlocks[3] = vec2_new(2, 2);
				break;
			}
			break;
		case PIECE_TYPE_O:
			for (unsigned int i = 0; i < BLOCKS_PER_PIECE; ++i) {
				potBlocks[i] = self->m_fallingPiece.m_blocks[i];
			}
			break;
		case PIECE_TYPE_S:
			switch (potRot) {
			case 0:
			case 2:
				potBlocks[0] = vec2_new(0, 2);
				potBlocks[1] = vec2_new(1, 2);
				potBlocks[2] = vec2_new(1, 1);
				potBlocks[3] = vec2_new(2, 1);
				break;
			case 1:
			case 3:
				potBlocks[0] = vec2_new(1, 0);
				potBlocks[1] = vec2_new(1, 1);
				potBlocks[2] = vec2_new(2, 1);
				potBlocks[3] = vec2_new(2, 2);
				break;
			}
			break;
		case PIECE_TYPE_T:
			switch (potRot) {
			case 0:
				potBlocks[0] = vec2_new(0, 1);
				potBlocks[1] = vec2_new(1, 1);
				potBlocks[2] = vec2_new(2, 1);
				potBlocks[3] = vec2_new(1, 2);
				break;
			case 1:
				potBlocks[0] = vec2_new(1, 0);
				potBlocks[1] = vec2_new(1, 1);
				potBlocks[2] = vec2_new(1, 2);
				potBlocks[3] = vec2_new(0, 1);
				break;
			case 2:
				potBlocks[0] = vec2_new(0, 1);
				potBlocks[1] = vec2_new(1, 1);
				potBlocks[2] = vec2_new(2, 1);
				potBlocks[3] = vec2_new(1, 0);
				break;
			case 3:
				potBlocks[0] = vec2_new(1, 0);
				potBlocks[1] = vec2_new(1, 1);
				potBlocks[2] = vec2_new(1, 2);
				potBlocks[3] = vec2_new(2, 1);
				break;
			}
			break;
		case PIECE_TYPE_Z:
			switch (potRot) {
			case 0:
			case 2:
				potBlocks[0] = vec2_new(0, 1);
				potBlocks[1] = vec2_new(1, 1);
				potBlocks[2] = vec2_new(1, 2);
				potBlocks[3] = vec2_new(2, 2);
				break;
			case 1:
			case 3:
				potBlocks[0] = vec2_new(2, 0);
				potBlocks[1] = vec2_new(2, 1);
				potBlocks[2] = vec2_new(1, 1);
				potBlocks[3] = vec2_new(1, 2);
				break;
			}
			break;
		}
		printf("rotation = %u\n", potRot);
		// collision detection!
		// including [bounds check, block collision!]
		self->m_fallingPiece.m_rotation = potRot;
		for (unsigned int i = 0; i < BLOCKS_PER_PIECE; ++i) {
			self->m_fallingPiece.m_blocks[i] = potBlocks[i];
		}
		printf("survived\n");

	}

	if (subticks == 15) {
		vec2_madd(&potentialPos, 0, 1);
	}

	unsigned int max_y = 0;
	for (unsigned int i = 0; i < BLOCKS_PER_PIECE; ++i) {
		unsigned int y = self->m_fallingPiece.m_blocks[i].y + potentialPos.y;
		if (y > max_y)
			max_y = y;
	}
	if (max_y >= CANVAS_HEIGHT) {
		tetris_game_place(self);
		return;
	}
// block collisions
	for (unsigned int b = 0; b < BLOCKS_PER_PIECE; b++) {
		vec2 potentialBlockPos = vec2_add(&potentialPos,
				&self->m_fallingPiece.m_blocks[b]);
		if (self->m_blocks[(potentialBlockPos.y) * CANVAS_WIDTH
				+ potentialBlockPos.x] != COLOR_BLACK) {
			tetris_game_place(self);
			return;
		}
	}

	self->m_fallingPiece.m_position = potentialPos;

	subticks = (subticks + 1) % 16;
	printf("foo\n");
}

tetris_game g_tetris;

int main() {

	tetris_game_new(&g_tetris);

	while (1) {
		tetris_game_update(&g_tetris);
		tetris_game_render(&g_tetris);
		render();
		usleep(1000 * 10);
	}
	return 0;
}


int g_prev_rotate_control = 0;

int getRotateControll() {
	int rotate_control = ROTATE_INPUT;
	int pressed = !g_prev_rotate_control && rotate_control;
	g_prev_rotate_control = rotate_control;
	return pressed;
}

int subticks = 0;

void tetris_game_update(tetris_game *self) {
	vec2 potentialPos = self->m_fallingPiece.m_position;

	if (getRightControll()) {
		vec2_madd(&potentialPos, 1, 0);
	}
	if (getLeftControll()) {
		vec2_madd(&potentialPos, -1, 0);
	}

	unsigned int max_x = 0; // unsigned! checks both borders!
	for (unsigned int i = 0; i < BLOCKS_PER_PIECE; i++) {
		unsigned int x = self->m_fallingPiece.m_blocks[i].x + potentialPos.x;
		if (x > max_x)
			max_x = x;
	}

	if (max_x >= CANVAS_WIDTH) {
		potentialPos = self->m_fallingPiece.m_position; // ignore movement!
	}
	// check for block collisions
	for (unsigned int b = 0; b < BLOCKS_PER_PIECE; b++) {
		vec2 potentialBlockPos = vec2_add(&potentialPos,
				&self->m_fallingPiece.m_blocks[b]);
		if (self->m_blocks[(potentialBlockPos.y) * CANVAS_WIDTH
				+ potentialBlockPos.x] != COLOR_BLACK) {
			potentialPos = self->m_fallingPiece.m_position;
		}
	}

	if (getRotateControll()) {
		int potRot = (self->m_fallingPiece.m_rotation + 1) % 4;
		// determine new blocks!
		vec2 potBlocks[BLOCKS_PER_PIECE] = { };
		switch (self->m_fallingPiece.m_tag) {
		case PIECE_TYPE_I:
			switch (potRot) {
			case 0:
			case 2:
				potBlocks[0] = vec2_new(0, 2);
				potBlocks[1] = vec2_new(1, 2);
				potBlocks[2] = vec2_new(2, 2);
				potBlocks[3] = vec2_new(3, 2);
				break;
			case 1:
			case 3:
				potBlocks[0] = vec2_new(2, 0);
				potBlocks[1] = vec2_new(2, 1);
				potBlocks[2] = vec2_new(2, 2);
				potBlocks[3] = vec2_new(2, 3);
			}
			break;
		case PIECE_TYPE_J:
			switch (potRot) {
			case 0:
				potBlocks[0] = vec2_new(0, 1);
				potBlocks[1] = vec2_new(1, 1);
				potBlocks[2] = vec2_new(2, 1);
				potBlocks[3] = vec2_new(2, 2);
				break;
			case 1:
				potBlocks[0] = vec2_new(1, 0);
				potBlocks[1] = vec2_new(1, 1);
				potBlocks[2] = vec2_new(1, 2);
				potBlocks[3] = vec2_new(0, 2);
				break;
			case 2:
				potBlocks[0] = vec2_new(0, 0);
				potBlocks[1] = vec2_new(0, 1);
				potBlocks[2] = vec2_new(1, 1);
				potBlocks[3] = vec2_new(2, 1);
				break;
			case 3:
				potBlocks[0] = vec2_new(2, 0);
				potBlocks[1] = vec2_new(1, 0);
				potBlocks[2] = vec2_new(1, 1);
				potBlocks[3] = vec2_new(1, 2);
				break;
			}
			break;
		case PIECE_TYPE_L:
			switch (potRot) {
			case 0:
				potBlocks[0] = vec2_new(0, 2);
				potBlocks[1] = vec2_new(0, 1);
				potBlocks[2] = vec2_new(1, 1);
				potBlocks[3] = vec2_new(2, 1);
				break;
			case 1:
				potBlocks[0] = vec2_new(0, 0);
				potBlocks[1] = vec2_new(1, 0);
				potBlocks[2] = vec2_new(1, 1);
				potBlocks[3] = vec2_new(1, 2);
				break;
			case 2:
				potBlocks[0] = vec2_new(0, 1);
				potBlocks[1] = vec2_new(1, 1);
				potBlocks[2] = vec2_new(2, 1);
				potBlocks[3] = vec2_new(2, 0);
				break;
			case 3:
				potBlocks[0] = vec2_new(1, 0);
				potBlocks[1] = vec2_new(1, 1);
				potBlocks[2] = vec2_new(1, 2);
				potBlocks[3] = vec2_new(2, 2);
				break;
			}
			break;
		case PIECE_TYPE_O:
			for (unsigned int i = 0; i < BLOCKS_PER_PIECE; ++i) {
				potBlocks[i] = self->m_fallingPiece.m_blocks[i];
			}
			break;
		case PIECE_TYPE_S:
			switch (potRot) {
			case 0:
			case 2:
				potBlocks[0] = vec2_new(0, 2);
				potBlocks[1] = vec2_new(1, 2);
				potBlocks[2] = vec2_new(1, 1);
				potBlocks[3] = vec2_new(2, 1);
				break;
			case 1:
			case 3:
				potBlocks[0] = vec2_new(1, 0);
				potBlocks[1] = vec2_new(1, 1);
				potBlocks[2] = vec2_new(2, 1);
				potBlocks[3] = vec2_new(2, 2);
				break;
			}
			break;
		case PIECE_TYPE_T:
			switch (potRot) {
			case 0:
				potBlocks[0] = vec2_new(0, 1);
				potBlocks[1] = vec2_new(1, 1);
				potBlocks[2] = vec2_new(2, 1);
				potBlocks[3] = vec2_new(1, 2);
				break;
			case 1:
				potBlocks[0] = vec2_new(1, 0);
				potBlocks[1] = vec2_new(1, 1);
				potBlocks[2] = vec2_new(1, 2);
				potBlocks[3] = vec2_new(0, 1);
				break;
			case 2:
				potBlocks[0] = vec2_new(0, 1);
				potBlocks[1] = vec2_new(1, 1);
				potBlocks[2] = vec2_new(2, 1);
				potBlocks[3] = vec2_new(1, 0);
				break;
			case 3:
				potBlocks[0] = vec2_new(1, 0);
				potBlocks[1] = vec2_new(1, 1);
				potBlocks[2] = vec2_new(1, 2);
				potBlocks[3] = vec2_new(2, 1);
				break;
			}
			break;
		case PIECE_TYPE_Z:
			switch (potRot) {
			case 0:
			case 2:
				potBlocks[0] = vec2_new(0, 1);
				potBlocks[1] = vec2_new(1, 1);
				potBlocks[2] = vec2_new(1, 2);
				potBlocks[3] = vec2_new(2, 2);
				break;
			case 1:
			case 3:
				potBlocks[0] = vec2_new(2, 0);
				potBlocks[1] = vec2_new(2, 1);
				potBlocks[2] = vec2_new(1, 1);
				potBlocks[3] = vec2_new(1, 2);
				break;
			}
			break;
		}
		printf("rotation = %u\n", potRot);
		// collision detection!
		// including [bounds check, block collision!]
		self->m_fallingPiece.m_rotation = potRot;
		for (unsigned int i = 0; i < BLOCKS_PER_PIECE; ++i) {
			self->m_fallingPiece.m_blocks[i] = potBlocks[i];
		}
		printf("survived\n");

	}

	if (subticks == 15) {
		vec2_madd(&potentialPos, 0, 1);
	}

	unsigned int max_y = 0;
	for (unsigned int i = 0; i < BLOCKS_PER_PIECE; ++i) {
		unsigned int y = self->m_fallingPiece.m_blocks[i].y + potentialPos.y;
		if (y > max_y)
			max_y = y;
	}
	if (max_y >= CANVAS_HEIGHT) {
		tetris_game_place(self);
		return;
	}
// block collisions
	for (unsigned int b = 0; b < BLOCKS_PER_PIECE; b++) {
		vec2 potentialBlockPos = vec2_add(&potentialPos,
				&self->m_fallingPiece.m_blocks[b]);
		if (self->m_blocks[(potentialBlockPos.y) * CANVAS_WIDTH
				+ potentialBlockPos.x] != COLOR_BLACK) {
			tetris_game_place(self);
			return;
		}
	}

	self->m_fallingPiece.m_position = potentialPos;

	subticks = (subticks + 1) % 16;
	printf("foo\n");
}

tetris_game g_tetris;

int main() {

	tetris_game_new(&g_tetris);

	while (1) {
		tetris_game_update(&g_tetris);
		tetris_game_render(&g_tetris);
		render();
		usleep(1000 * 10);
	}
	return 0;
}
