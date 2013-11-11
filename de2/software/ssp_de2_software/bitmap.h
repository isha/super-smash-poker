/*
 * bitmap.h
 *
 *  Created on: 2013-11-10
 *      Author: n1v7
 */

#ifndef BITMAP_H_
#define BITMAP_H_

#include "sdcard.h"
#include <stdlib.h>

typedef struct {
        int width;
        int height;
        int *data;
} Bitmap;

void skip(short int fp, int chars) {
   int i;
   for (i=0; i<chars; i++)
      sdcard_read(fp);
}

int sdcard_read_int (short int fp) {
	unsigned char arr[4];
	int i;
	for (i=0; i<4; i++) {
		arr[i] = sdcard_read(fp);
	}
	int num = (arr[3] << 24) + (arr[2] << 16) + (arr[1] << 8) + arr[0];
	return num;
}

void * load_bitmap(char *file) {
	Bitmap * bitmap = malloc(sizeof(Bitmap));
	short int fp;
	fp = sdcard_fopen(file, 0);
	if (fp == -1) {
		alt_printf("\nError opening bmp file %s", file);
	} else if (fp == -2) {
		alt_printf("\bmp file %s already open", file);
	}
	if (sdcard_read(fp)!='B' || sdcard_read(fp)!='M')
	{
		sdcard_fclose(fp);
		alt_printf("\n%s is not a bitmap file.\n",file);
	}

	long index;
	short int color_data, red, blue, green;
	int num_colors;
	int x;

	skip(fp,16);
	bitmap->width = sdcard_read_int(fp);
	bitmap->height = sdcard_read_int(fp);
	skip(fp,20);
	num_colors = sdcard_read_int(fp);
	skip(fp,4);

	/* assume we are working with an 8-bit file */
	if (num_colors == 0) num_colors = 256;

	/* try to allocate memory */
	if (( bitmap->data = malloc(bitmap->width*bitmap->height*sizeof(int))) == NULL) {
		sdcard_fclose(fp);
		alt_printf("\nError allocating memory for bitmap data %s", file);
	}

	/* Ignore the palette information for now */
	skip(fp, num_colors*4);

	/* read the bitmap */
	for (index = bitmap->height-1; index >=0; index--) {
		for (x = 0; x < bitmap->width; x++){
			color_data = sdcard_read(fp);
			red = ((color_data & 0xE0) >> 5) * 31 / 7;
			green = ((color_data & 0x1C) >> 2) * 63 / 7;
			blue = ((color_data & 0x03)) * 31 / 3;
			bitmap->data[(index*bitmap->width) + x] = (blue << 11 ) + (green << 5 ) + (red);
		}
		if (bitmap->width % 4) skip(fp, (4 - bitmap->width % 4));
	}
	sdcard_fclose(fp);
	return bitmap;
}

#endif /* BITMAP_H_ */
