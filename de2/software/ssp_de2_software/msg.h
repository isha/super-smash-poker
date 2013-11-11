/*
 * msg.h
 *
 *  Created on: 2013-11-10
 *      Author: n1v7
 */

#ifndef MSG_H_
#define MSG_H_

#include "def.h"
#define MAX_MESSAGE_LENGTH 30

alt_up_rs232_dev* uart;

void initialize_messaging() {
	 printf("UART Initialization\n");
	 uart = alt_up_rs232_open_dev("/dev/rs232_0");
}

/* Wait for some time to give android devices chance to connect.
 * From the messages received, assign each player a client id
 */
void init_players() {
	 printf("Clearing read buffer to start\n");
	 while (alt_up_rs232_get_used_space_in_read_FIFO(uart)) {
		 alt_up_rs232_read_data(uart, &data, &parity);
	 }

	 // wait for something to be in the buffer
	 printf("Waiting for some data from the Middleman\n");
	 while (alt_up_rs232_get_used_space_in_read_FIFO(uart) == 0);

	 // save data in a character array
	 alt_up_rs232_read_data(uart, &data, &parity);
	 num_to_receive = (int)data;

	 printf("About to receive %d characters:\n", num_to_receive);
	 for (i = 0; i < num_to_receive; i++) {
		 while (alt_up_rs232_get_used_space_in_read_FIFO(uart) == 0);
		 alt_up_rs232_read_data(uart, &data, &parity);
		 message[i] = data;
	 }
	 message[i] = '\0';

	 printf("\n%s", message);
}

/* First message to the clients. Send hand information, state and money
 * Money to be sent is the money they started with (received in a message before minus antes)
 */
void send_player_hands() {
	int i;
	for (i=0; i<dealer->number_players; i++) {

	}
}
void messaging() {
	 int i;
	 int num_to_receive;
	 unsigned char data;
	 unsigned char parity;
	 unsigned char message[MAX_MESSAGE_LENGTH];


	 // main loop to send back data
	 while (1) {

		 printf("Clearing read buffer to start\n");
		 while (alt_up_rs232_get_used_space_in_read_FIFO(uart)) {
		 alt_up_rs232_read_data(uart, &data, &parity);
		 }

		 // wait for something to be in the buffer
		 printf("Waiting for some data from the Middleman\n");
		 while (alt_up_rs232_get_used_space_in_read_FIFO(uart) == 0)
			 ;

		 // save data in a character array
		 alt_up_rs232_read_data(uart, &data, &parity);
		 num_to_receive = (int)data;

		 printf("About to receive %d characters:\n", num_to_receive);
		 for (i = 0; i < num_to_receive; i++) {
			 while (alt_up_rs232_get_used_space_in_read_FIFO(uart) == 0)
				 ;
			 alt_up_rs232_read_data(uart, &data, &parity);
			 printf("%c", data);
			 message[i] = data;
		 }
		 message[i] = '\0';

		 printf("\n%s", message);

		 // Now send the actual message to the Middleman
		 alt_up_rs232_write_data(uart, (unsigned char) strlen(message));
		 for (i = 0; i < strlen(message); i++) {
			 alt_up_rs232_write_data(uart, message[i]);
		 }

		 printf("\nMessage has been sent! Please send another message\n");

	 }
}

#endif /* MSG_H_ */
