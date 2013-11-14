/*
 * audio.h
 *
 *  Created on: 2013-10-03
 *      Author: Kevin
 */

#ifndef AUDIO_H_
#define AUDIO_H_

#include "altera_up_avalon_audio_and_video_config.h"
#include "altera_up_avalon_audio.h"
#include <sys/alt_alarm.h>

short int fp;
unsigned int *data;
int globalInd = 0;
int globalMaxInd = 0;

alt_u32 handle_timer_interrupts(alt_up_audio_dev * device)
{
	//int index;

	if (alt_up_audio_write_fifo_space(device, ALT_UP_AUDIO_LEFT) > 33){
//		for (index = 0; index < 96; index++){
//			data[index] = sdcard_audio_read_little_end(fp);
//		}
		alt_up_audio_write_fifo (device, &(data[globalInd*32]), 32, ALT_UP_AUDIO_LEFT);
		alt_up_audio_write_fifo (device, &(data[globalInd*32]), 32, ALT_UP_AUDIO_RIGHT);
		globalInd++;
		if (globalInd > globalMaxInd){
			globalInd = 0;
		}
	}
	return 1;
}

void initialize_audio() {
	alt_up_av_config_dev * av_config = alt_up_av_config_open_dev("/dev/audio_config");
	if (!alt_up_av_config_read_ready(av_config)) {
		printf("Audio can not be initialised\n");
	}
}

alt_up_audio_dev * start_audio(){
	// open the Audio port;
	alt_up_audio_dev * audio_device = alt_up_audio_open_dev("/dev/audio");
	if ( audio_device == NULL)
		alt_printf ("\nError: could not open audio device \n");
	else
		alt_printf ("\nOpened audio device \n");

	alt_up_audio_reset_audio_core(audio_device);

	alt_up_audio_enable_write_interrupt(audio_device);

	char * file = "Kirby5.wav";
	fp = sdcard_fopen(file, 0);
	if (fp == -1) {
		alt_printf("\nError opening wav file %s", file);
	} else if (fp == -2) {
		alt_printf("\nwav file %s already open", file);
	}

	unsigned int read;
	while(read != 0x6174){
		read = sdcard_audio_read_little_end(fp);
	}
	read = sdcard_audio_read_little_end(fp);
	read = read + sdcard_audio_read_little_end(fp)*16*16*16*16;

	unsigned int data2[read][32];

	int i, j;
	for (i=0; i < read/64; i++){
		for (j=0; j < 32; j++){
			data2[i][j] = sdcard_audio_read_little_end(fp);
		}
	}


	data = data2;

	globalMaxInd = read/64;

	// Start the interrupt
	static alt_alarm alarm;
	alt_alarm_start(&alarm, 1, handle_timer_interrupts, audio_device);
}

#endif /* AUDIO_H_ */
