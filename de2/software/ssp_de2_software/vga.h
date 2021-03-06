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

extern char * card_name(Card card);
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
char * card_name(Card card) {
  char * c = malloc(sizeof(char)*20);
  if(card.value == BLANK_CARD_VALUE) {
	  strcpy(c, "---");
  } else {
	  switch (card.value) {
	      case 0: strcpy(c, "Ace"); break;
	      case 12: strcpy(c, "King"); break;
	      case 11: strcpy(c, "Queen"); break;
	      case 10: strcpy(c, "Jack"); break;
	      default: sprintf(c, "%d", card.value+1); break;
	    }
	    strcat(c, " of ");
	    switch (card.suite) {
	      case 0: strcat(c, "Clubs"); break;
	      case 1: strcat(c, "Spades"); break;
	      case 2: strcat(c, "Diamonds"); break;
	      case 3: strcat(c, "Hearts"); break;
	    }
	    strcat(c, "\0");
  }
  return c;
}

char * card_bitmap_name(Card card) {
  char * c = malloc(sizeof(char)*20);
  char * value_string = malloc(sizeof(char)*20);
  if(card.value == BLANK_CARD_VALUE) {
	  strcpy(c, "back.bmp");
  } else {
	  switch (card.suite) {
		  case 0: strcpy(c, "c"); break;
		  case 1: strcpy(c, "s"); break;
		  case 2: strcpy(c, "d"); break;
		  case 3: strcpy(c, "h"); break;
		}
	  switch (card.value) {
	      case 0: strcat(c, "a"); break;
	      case 12: strcat(c, "k"); break;
	      case 11: strcat(c, "q"); break;
	      case 10: strcat(c, "j"); break;
	      default:
	    	  sprintf(value_string, "%d", card.value+1);
	    	  strcat(c, value_string);
	    	  break;
	    }
	  strcat(c, ".bmp");
  }
  return c;
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
