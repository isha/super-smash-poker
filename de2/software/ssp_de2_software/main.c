#include <stdio.h>
#include "altera_up_avalon_rs232.h"
#include <string.h>

#define MAX_MESSAGE_LENGTH 30

int main()
{
	 int i;
	 int num_to_receive;
	 unsigned char data;
	 unsigned char parity;
	 unsigned char message[MAX_MESSAGE_LENGTH];

	 printf("UART Initialization\n");
	 alt_up_rs232_dev* uart = alt_up_rs232_open_dev("/dev/rs232_0");


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

	 return 0;
}
