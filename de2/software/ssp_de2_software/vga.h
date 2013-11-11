/*
 * vga.h
 *
 *  Created on: 2013-11-10
 *      Author: n1v7
 */

#ifndef VGA_H_
#define VGA_H_


#include "altera_up_avalon_video_character_buffer_with_dma.h"
#include "altera_up_avalon_video_pixel_buffer_dma.h"
#include "io.h"
#include "def.h"

#define RESOLUTION_X 320
#define RESOLUTION_Y 240

int pixel_colors[RESOLUTION_X][RESOLUTION_Y];
int old_pixel_colors[RESOLUTION_X][RESOLUTION_Y];
int initial_pixel_colors[RESOLUTION_X][RESOLUTION_Y];

alt_up_pixel_buffer_dma_dev* pixel_buffer;
alt_up_char_buffer_dev *char_buffer;

unsigned int pixel_buffer_addr1 = PIXEL_BUFFER_BASE;
unsigned int pixel_buffer_addr2 = PIXEL_BUFFER_BASE + (320 * 240 * 2);

void initialize_vga() {
	pixel_buffer = alt_up_pixel_buffer_dma_open_dev("/dev/pixel_buffer_dma");
	alt_up_pixel_buffer_dma_change_back_buffer_address(pixel_buffer, pixel_buffer_addr1);
	alt_up_pixel_buffer_dma_swap_buffers(pixel_buffer);
	while (alt_up_pixel_buffer_dma_check_swap_buffers_status(pixel_buffer));

	// Set the 2nd buffer address
	alt_up_pixel_buffer_dma_change_back_buffer_address(pixel_buffer,
	pixel_buffer_addr2);
	alt_up_pixel_buffer_dma_clear_screen(pixel_buffer, 0);
	alt_up_pixel_buffer_dma_clear_screen(pixel_buffer, 1);

	char_buffer = alt_up_char_buffer_open_dev("/dev/char_drawer");
	alt_up_char_buffer_init(char_buffer);
}

void draw_to_screen() {
	int i, j;
	for (i=0; i<RESOLUTION_X; i++){
		for (j=0; j<RESOLUTION_Y; j++){
			if (pixel_colors[i][j] != old_pixel_colors[i][j]) {
				draw_pixel_fast(pixel_buffer, pixel_colors[i][j], i, j);
				old_pixel_colors[i][j] = pixel_colors[i][j];
				alt_up_pixel_buffer_dma_swap_buffers(pixel_buffer);
				draw_pixel_fast(pixel_buffer, pixel_colors[i][j], i, j);
				alt_up_pixel_buffer_dma_swap_buffers(pixel_buffer);
			}
		}
	}
}

void game_screen() {
	alt_up_char_buffer_clear(char_buffer);

	char str1[50], str2[30];

	sprintf(str1, "P1: %d", dealer->players[0].total_money);
	sprintf(str2, "%d of %d and %d of %d", dealer->players[0].hand[0].value, dealer->players[0].hand[0].suite,
			dealer->players[0].hand[1].value, dealer->players[0].hand[1].suite);

	alt_up_char_buffer_string(char_buffer, str1, 12, 50);
	alt_up_char_buffer_string(char_buffer, str2, 12, 51);

	sprintf(str1, "P2: %d", dealer->players[0].total_money);
	sprintf(str2, "%d of %d and %d of %d", dealer->players[1].hand[0].value, dealer->players[1].hand[0].suite,
			dealer->players[1].hand[1].value, dealer->players[1].hand[1].suite);

	alt_up_char_buffer_string(char_buffer, str1, 49, 50);
	alt_up_char_buffer_string(char_buffer, str2, 49, 51);
}

/* Author : Jeff Goeders
 *
 * This funcion draws a pixel to the background buffer, and assumes:
 * 1. Your pixel buffer DMA is set to CONSECUTIVE
 * 2. The resolution is 320x240
 * 3. x and y are within the screen (0,0)->(319, 239)
 * 4. You are using 16-bit color
 *
 * DO NOT USE THIS FUNCTION IF ANY OF THE ABOVE ARE NOT GUARANATEED, OR YOU
 * MAY WRITE TO INVALID MEMORY LOCATIONS, CRASHING YOUR PROGRAM, OR
 * CAUSING UNEXPECTED BEHAVIOR.
 */
int draw_pixel_fast(alt_up_pixel_buffer_dma_dev *pixel_buffer,
	unsigned int color, unsigned int x, unsigned int y) {
	unsigned int addr;

	addr = ((x & pixel_buffer->x_coord_mask) << 1);
	addr += (((y & pixel_buffer->y_coord_mask) * 320) << 1);

	IOWR_16DIRECT(pixel_buffer->back_buffer_start_address, addr, color);

	return 0;
}

#endif /* VGA_H_ */
