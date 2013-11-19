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
#define NUMBER_OF_PLAYERS 2

#define DEALT 0x02
#define ACTION 0x04

// Hold the player id / client id mapping
// index is player number, data inside is the client id
unsigned char player_id_mapping[NUMBER_OF_PLAYERS];

// Three global variables that hold the content and properties of the message
unsigned char message[MAX_MESSAGE_LENGTH];
int message_size;
unsigned char message_client_id;

alt_up_rs232_dev* uart;

void initialize_messaging() {
	 printf("UART Initialization\n");
	 uart = alt_up_rs232_open_dev("/dev/rs232_0");
}

// Receives message and sets message_client_id, message_size, and message
void receive_message() {
	 int i = 0;
	 int num_to_receive;
	 unsigned char data;
	 unsigned char parity;

	 printf("Clearing read buffer to start\n");
	 while (alt_up_rs232_get_used_space_in_read_FIFO(uart)) {
		 alt_up_rs232_read_data(uart, &data, &parity);
	 }

	 // wait for the client_id info
	 printf("Waiting for some data from the Middleman\n");
	 while (alt_up_rs232_get_used_space_in_read_FIFO(uart) == 0)
		 ;

	 // read the client_id
	 alt_up_rs232_read_data(uart, &data, &parity);
	 message_client_id = (int)data;

	 // wait for the size info
	 while (alt_up_rs232_get_used_space_in_read_FIFO(uart) == 0)
		 ;

	 // read the size
	 alt_up_rs232_read_data(uart, &data, &parity);
	 message_size = (int)data;

	 // read the rest of the information
	 printf("About to receive %d characters:", message_size);
	 for (i = 0; i < message_size; i++) {
		 while (alt_up_rs232_get_used_space_in_read_FIFO(uart) == 0)
			 ;
		 alt_up_rs232_read_data(uart, &data, &parity);
		 message[i] = data;
		 printf("%x ", message[i]);
	 }
	 printf("\n");
}

// Requires that message_client_id, message_size, and message are preset
void send_message() {
	int i;
	unsigned char parity;
	unsigned char data;

	printf("Clearing read buffer to start\n");
	while (alt_up_rs232_get_used_space_in_read_FIFO(uart)) {
		alt_up_rs232_read_data(uart, &data, &parity);
	}

	printf("Sending message: ");
	alt_up_rs232_write_data(uart, message_client_id);
	alt_up_rs232_write_data(uart, (unsigned char) message_size);
	for (i = 0; i < message_size; i++) {
		alt_up_rs232_write_data(uart, message[i]);
		printf("%d ", message[i]);
	}
	printf("\n");
}

// Converts four consecutive bytes (from MSB->LSB) into an int
unsigned int convert_bytes_to_int(unsigned char a, unsigned char b, unsigned char c, unsigned char d) {

	unsigned int temp_a, temp_b, temp_c, temp_d, result;

	temp_a = (unsigned int)a << 24;
	temp_b = (unsigned int)b << 16;
	temp_c = (unsigned int)c << 8;
	temp_d = (unsigned int)d;

	result = temp_a + temp_b + temp_c + temp_d;

	return result;

}

// Prints all values in the message, including client_id and size
void print_message() {
	int i;

	printf("Current message client_id: %x\n", message_client_id);
	printf("Current message size: %d\n", message_size);
	printf("Current message in hex: ");
	for (i = 0; i < message_size; i++) {
		printf("%x ", message[i]);
	}
	printf("\n");
}

// Reads the player action (and money, if needed)
int read_player_action_and_value(int pid) {

	int m_value = 0;

	dealer->players[pid].action = (int)message[0];
	printf("Action received from player %d: %x\n", pid, message[0]);

	// TODO Need to finalize what action enum values are between Android and DE2
	// if (dealer->players[pid].action == START_BET|| dealer->players[pid].action == RAISE) {
	if (dealer->players[pid].action == RAISE) { // 2 is RAISE in Android side
		m_value = convert_bytes_to_int(
				message[1],
				message[2],
				message[3],
				message[4]);
	}

	printf("m_value received from player %d: %d\n", pid, m_value);
	return m_value;
}

void read_initial_player_data(int current_number_of_players) {
	int temp_money = 0;

	// store the client_id
	player_id_mapping[current_number_of_players-1] = message_client_id;
	printf("The client_id of player %d is: %x\n", current_number_of_players-1, player_id_mapping[current_number_of_players-1]);

	// TODO: why do we need player enum?

	// convert 4 bytes from unsigned char to int
	temp_money = convert_bytes_to_int(
			message[1],
			message[2],
			message[3],
			message[4]);

	// store the total money of the player
	dealer->players[current_number_of_players-1].total_money = temp_money;
	printf("The money of player %d is: %d\n", current_number_of_players-1, dealer->players[current_number_of_players-1].total_money);
}

/* Wait for some time to give android devices chance to connect.
 * From the messages received, assign each player a client id
 */
void joining_period() {

	int current_number_of_players = 0;
	printf("\nWaiting for at least two players to connect...\n");
	for(;;) {

		// check if we should stop waiting for another player
		if (current_number_of_players == NUMBER_OF_PLAYERS) break;

		// store the data in current_message
		current_number_of_players++;
		receive_message();
		read_initial_player_data(current_number_of_players);

		printf("Player %d has joined, with client_id %x\n\n", current_number_of_players-1, message_client_id);
	}
	printf("Success! %d players are now connected.\n", current_number_of_players);

}

void set_action_state(int pid) {
	printf("\nRequesting action from player %d\n", pid);
	message_client_id = player_id_mapping[pid];
	message_size = 0x01;	// set msg size = 1 byte
	message[0] = ACTION; 		// set state = ACTION
}

/* First message to the clients. Send hand information and state
 * Money to be sent is the money they started with (received in a message before minus antes)
 */
void send_player_hands() {
	int i,j;
	for (i=0; i<dealer->number_players; i++) {

		printf("\nSEND PLAYER HANDS Initializing values for player %d...\n", i);

		message_client_id = player_id_mapping[i]; // send client_id

		message_size = 5;

		message[0] = DEALT; // state of player = DEALT

		// current hand of the player.
		// Need to add 1 to all values/suites to be compatible with Android side
		message[1] = dealer->players[i].hand[0].value+1;
		message[2] = dealer->players[i].hand[0].suite+1;
		message[3] = dealer->players[i].hand[1].value+1;
		message[4] = dealer->players[i].hand[1].suite+1;

		// currently not sending money

		//print_message();

		send_message();

		printf("Completed initialization for player %d\n", i);


	}
}

#endif /* MSG_H_ */
