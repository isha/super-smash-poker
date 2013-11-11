/*
 * sdcard.h
 *
 *  Created on: 2013-11-10
 *      Author: n1v7
 */

#ifndef SDCARD_H_
#define SDCARD_H_


#include <stdbool.h>
#include "altera_up_sd_card_avalon_interface.h"

alt_up_sd_card_dev* sdcard;


/* Initializes SD Card module */
void initialize_sdcard() {
	sdcard = alt_up_sd_card_open_dev("/dev/sdcard");
	if(sdcard == NULL) printf("\nError: SD Card Module cannot be initialized");
	if(!alt_up_sd_card_is_Present()){
		printf("\nSD Card not connected");
		return;
	}
	if(alt_up_sd_card_is_FAT16()) printf("\nSD Card Module initialized");
	else printf("Error: File system not recognized.\n");
}

/* Opens a file whose name is stored in filename.
 * If create is true, file will be created if it can't be found.
 * Returns: short int containing file handle if successful,
 * -1 if file could not be opened, -2 if file is already open
 */
short int sdcard_fopen(char* filename, bool create) {
	return alt_up_sd_card_fopen(filename, create);
}

/* Closes file denoted by handle in sdcard_file
 * Returns: true if successful, false if failed
 */
bool sdcard_fclose(short int sdcard_file) {
	return alt_up_sd_card_fclose(sdcard_file);
}


/* Writes a single byte of data to the file denoted by handle in sdcard_file
 * Returns: true if successful, false if failed
 */
bool sdcard_write(short int sdcard_file, unsigned char data) {
	return alt_up_sd_card_write(sdcard_file, data);
}

/* Writes a string to the file identified by sdcard_file
 * Returns: true if successful, false if failed
 */
bool sdcard_print(short int sdcard_file, unsigned char* string) {
	int i;
	for(i = 0; i < strlen(string); i++) {
		if(!sdcard_write(sdcard_file, string[i])) return false;
	}
	return true;
}

/* Reads a single byte of data to the file identified by sdcard_file
 * Returns: a data byte if successful, -1 if file handle is invalid, -2 if it could not read from SD card
 */
short int sdcard_read(short int sdcard_file) {
	return alt_up_sd_card_read(sdcard_file);
}

unsigned int sdcard_audio_read_little_end (short int fp) {
	unsigned int part_read;
	unsigned int full_read;

	part_read = sdcard_read(fp);
	full_read = part_read % 0x100;

	part_read = sdcard_read(fp);
	part_read = part_read % 0x100;
	full_read = part_read*16*16 + full_read;

	return full_read;
}
#endif /* SDCARD_H_ */
