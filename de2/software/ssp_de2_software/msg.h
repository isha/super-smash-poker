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

alt_up_rs232_dev* uart;
unsigned char current_message[MAX_MESSAGE_LENGTH]; // holds the current msg in the form: [client_ID][data.size][data]
unsigned char player_id_mapping[NUMBER_OF_PLAYERS]; // index is player number, data inside is the client id
int current_number_of_players = 0;

void initialize_messaging() {
	 printf("UART Initialization\n");
	 uart = alt_up_rs232_open_dev("/dev/rs232_0");
}


/* Convert 4 byte values in unsigned format to unsigned int
 *
 */
unsigned int convert_bytes_to_int(unsigned char a, unsigned char b, unsigned char c, unsigned char d) {

	unsigned int temp_a, temp_b, temp_c, temp_d, result;

	temp_a = (unsigned int)a << 24;
	temp_b = (unsigned int)b << 16;
	temp_c = (unsigned int)c << 8;
	temp_d = (unsigned int)d;

	result = temp_a + temp_b + temp_c + temp_d;

	return result;

}

/* Prints out whatever is stored in current_message
 *
 */
void print_current_message() {
	int i;

	printf("Current message (in hex): ");
	for (i = 0; current_message[i] != '\0'; i++) {
		printf("%x ", current_message[i]);
	}
	printf("\n");
}

/* Waits for the next message
 * And stores the full message in the form [client_ID][data.size][data]
 */
void receive_message() {
	 int i = 0;
	 int num_to_receive;
	 unsigned char data;
	 int client_id;
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
	 client_id = (int)data;
	 current_message[0] = data;

	 // wait for the size info
	 while (alt_up_rs232_get_used_space_in_read_FIFO(uart) == 0)
		 ;

	 // read the size
	 alt_up_rs232_read_data(uart, &data, &parity);
	 current_message[1] = data;
	 num_to_receive = (int)data;

	 // read the rest of the information
	 printf("About to receive %d characters:\n", num_to_receive);
	 for (i = 2; i < num_to_receive+2; i++) {
		 while (alt_up_rs232_get_used_space_in_read_FIFO(uart) == 0)
			 ;
		 alt_up_rs232_read_data(uart, &data, &parity);
		 printf("%x", data);
		 current_message[i] = data;
	 }

	 // append a null character to the end of the message
	 current_message[i] = '\0';
	 printf("\n");

	 print_current_message();
}

/* Sends whatever is in current_message to the Middleman
* Current message must be in the form: [client_ID][data.size][data]
*/
void send_message() {
	int i;
	printf("Sending message: ");
	alt_up_rs232_write_data(uart, (unsigned char) strlen(current_message));
	for (i = 0; i < strlen(current_message); i++) {
		alt_up_rs232_write_data(uart, current_message[i]);
		printf("%x", current_message[i]);
	}
	printf("\n");
}

/* Read the initial player information
 * Stores the player_id mapping and total_money of the player
 * Assumes the following format was sent: [client_id][size][player_enum][total_money1-4]
 */
void read_initial_player_information() {

	int temp_money;

	receive_message();

	// store the client_id
	player_id_mapping[current_number_of_players-1] = current_message[0];
	printf("The client_id of player %d is: %x\n", current_number_of_players-1, player_id_mapping[current_number_of_players-1]);

	// why do we need player enum?

	// convert 4 bytes from unsigned char to int
	temp_money = convert_bytes_to_int(
			current_message[3],
			current_message[4],
			current_message[5],
			current_message[6]);

	// store the total money of the player
	dealer->players[current_number_of_players-1].total_money = temp_money;
	printf("The money of player %d is: %x\n", current_number_of_players-1, dealer->players[current_number_of_players-1].total_money);
}

/* First message to the clients. Send hand information, state and money
 * Money to be sent is the money they started with (received in a message before minus antes)
 */
void send_player_hands() {
	int i;
	for (i=0; i<dealer->number_players; i++) {

		printf("Initializing values for player %d...\n", i);
		current_message[0] = player_id_mapping[i]; // send client_id
		current_message[1] = 0x05; // size of message (following this byte)
		current_message[2] = 0x01; // state of player = DEALT

		// current hand of the player.
		// Need to add 1 to all values/suites to be compatible with Android side
		current_message[3] = ++(dealer->players[i].hand[0].value);
		current_message[4] = ++(dealer->players[i].hand[0].suite);
		current_message[5] = ++(dealer->players[i].hand[1].value);
		current_message[6] = ++(dealer->players[i].hand[1].suite);

		// currently not sending money. Is this required?

		print_current_message();
		send_message();
		printf("Completed initialization for player %d\n", i);
	}
}

/* Read the response from a player
* This assumes that enum for actions on both sides are the same!
* Also assumes following format of information: [client_id][size][action][value1-4(optional, depending on action)]
*/
int read_player_action(int pid) {
	int m_value = 0;

	printf("About to receive message from player %d...\n", pid);
	receive_message();

	dealer->players[pid].action = current_message[2];
	printf("Action received from player %d: %x\n", pid, current_message[2]);

	if (dealer->players[pid].action == START_BET || dealer->players[pid].action == RAISE) {
		m_value = convert_bytes_to_int(
				current_message[3],
				current_message[4],
				current_message[5],
				current_message[6]);
	}

	printf("m_value received from player %d: %d\n", pid, m_value);
	return m_value;
}

/* Request action from a single given player by setting their state to BET
* Sends information in the following format: [client_id][size][state = ACTION]
*/
void request_action(int pid) {
	printf("Requesting action from player %d\n", pid);
	current_message[0] = player_id_mapping[pid];
	current_message[1] = 0x01;
	current_message[2] = 0x03; // set state = ACTION

	print_current_message();
	send_message();
	printf("Request sent.\n");
}

/* Changes the state of a given player to waiting
* Sends information in the following format: [client_id][size][state = WAITING]
*/
void set_waiting_state(int pid) {
	printf("Setting player %d to waiting state\n", pid);
	current_message[0] = player_id_mapping[pid];
	current_message[1] = 0x01;
	current_message[2] = 0x02; // set state = WAITING

	print_current_message();
	send_message();
	printf("Successfully sent player %d waiting state info\n", pid);
}

/* Wait for some time to give android devices chance to connect.
 * From the messages received, assign each player a client id
 */
void joining_period() {

	printf("Waiting for at least two players to connect...\n");

	for(;;) {

		// check if we should continue waiting for another player
		if (current_number_of_players == NUMBER_OF_PLAYERS) break;

		// store the data in current_message
		read_initial_player_information();
		current_number_of_players++;
	}

	printf("Success! %d players are now connected.\n", current_number_of_players);

}

/* Tell each device whether they won or lost and assign their state accordingly
 * Sends information in the following format: [client_id][size][state = WIN or LOSE]
 */
void send_results() {
	int i;
	printf("Sending results of game to all players...");
	for (i=0; i<dealer->number_players; i++) {
		current_message[0] = player_id_mapping[i];
		current_message[1] = 0x01;
		// TODO: if this player wins,
			current_message[2] = 0x04; // set state = WIN
		// TODO: else this player loses,
			current_message[2] = 0x05; // set state = LOSE

		print_current_message();
		send_message();
	}
}

#endif /* MSG_H_ */
