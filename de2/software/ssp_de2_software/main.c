#include <stdio.h>
#include "altera_up_avalon_rs232.h"
#include <string.h>
#include "vga.h"
#include "sdcard.h"
#include "audio.h"
#include "bitmap.h"
#include "def.h"
#include "msg.h"
#include "sys/alt_alarm.h"
#include "sys/alt_timestamp.h"
#include "altera_avalon_timer_regs.h"
#include "altera_avalon_timer.h"
#include <time.h>

void init() {
	initialize_vga();
	initialize_sdcard();
	alt_timestamp_start();
	initialize_audio();
	start_audio();
	initialize_messaging();
}

/***************************************************************************/

Dealer * dealer;
int dealer_chip = 0;

/* Method definitions */
void initialize_dealer(int number_players) {
  dealer = malloc(sizeof(Dealer));

  /* Setup deck of cards */
  dealer->cards_in_deck = 52;
  dealer->deck = malloc(sizeof(Card) * 52);
  int i, j;
  for (i=0; i<4; i++) {
    for (j=0; j<13; j++) {
      dealer->deck[i*13+j].suite = i;
      dealer->deck[i*13+j].value = j;
    }
  }

  /* Shuffle cards in deck */
  Card temp;
  for (i=0; i<51; i++) {
    temp = dealer->deck[i];
    j = rand()%(52-(i+1)) + (i+1);
    dealer->deck[i] = dealer->deck[j];
    dealer->deck[j] = temp;
  }

  /* Setup players */
  dealer->number_players = number_players;
  dealer->players = malloc(sizeof(Player) * number_players);
  for (i=0; i<number_players; i++) {
    dealer->players[i].active = true;
    dealer->players[i].money = 0;
    dealer->players[i].total_money = 3000;
  }

  /* Cards on table, zero in the beginning */
  dealer->number_cards_on_table = 0;
  dealer->pot = 0;
  dealer->current_bet = 0;
}

/* Returns a random card from the deck */
Card get_card_from_deck() {
  dealer->cards_in_deck--;
  return dealer->deck[dealer->cards_in_deck];
}

/* Deals two cards to all players in the game */
void deal_hands() {
  int i;
  for (i=0; i<dealer->number_players; i++) {
    dealer->players[i].hand[0] = get_card_from_deck();
    dealer->players[i].hand[1] = get_card_from_deck();
  }
}

/* Plays the flop */
void flop() {
  dealer->cards_on_table[0] = get_card_from_deck();
  dealer->cards_on_table[1] = get_card_from_deck();
  dealer->cards_on_table[2] = get_card_from_deck();
  dealer->number_cards_on_table = 3;
}

/* Plays the turn */
void turn() {
  dealer->cards_on_table[3] = get_card_from_deck();
  dealer->number_cards_on_table = 4;
}

/* Plays the flop */
void river() {
  dealer->cards_on_table[4] = get_card_from_deck();
  dealer->number_cards_on_table = 5;
}

/* Gets player bet if player still playing
 * TODO change in de2 env
 */
unsigned int get_bet_for_player(int pid) {

  unsigned int m_input;

  printf("\n\n----------------------------------------");
  printf("\nYour total money %d and bet money %d", dealer->players[pid].total_money, dealer->players[pid].money);
  printf("\nPlayer %d\n Enter your action (0 - Bet, 1 - Call, 2 - Check, 3 - Raise, 4 - Fold): ", pid);

  set_action_state(pid);
  //send_message();

  receive_message();
  m_input = read_player_action_and_value(pid);

  if (dealer->players[pid].action == CALL) {
    dealer->players[pid].total_money -= (dealer->current_bet - dealer->players[pid].money);
    dealer->players[pid].money += (dealer->current_bet - dealer->players[pid].money);
  }

  if (dealer->players[pid].action == RAISE) {
    dealer->current_bet += m_input;
    dealer->players[pid].total_money -= (dealer->current_bet - dealer->players[pid].money);
    dealer->players[pid].money += (dealer->current_bet - dealer->players[pid].money);
  }

  if (dealer->players[pid].action == FOLD && dealer->current_bet > 0) {
    dealer->players[pid].active = false;

    int i, active_players = 0;
    for (i=0; i<dealer->number_players; i++) {
      if (dealer->players[i].active) active_players++;
    }
    if (active_players == 1) return -1;
  }
  printf("\nYour total money %d and bet money %d", dealer->players[pid].total_money, dealer->players[pid].money);

  return 0;
}

/* Returns true if players are still in the process of betting */
bool still_betting() {
  int i;
  for (i=0; i<dealer->number_players; i++) {
    if (dealer->players[i].money < dealer->current_bet && dealer->players[i].total_money > 0)
    return true;
  }
  return false;
}

/* Returns true if player still has to put forward money */
bool player_still_playing(int pid) {
  if (!dealer->players[pid].active) return false;
  else if (dealer->players[pid].money < dealer->current_bet && dealer->players[pid].total_money > 0)
  return true;

  return false;
}

void test_case() {
	init();

	message_client_id = 0x01;
	message_size = 5;
	message[0] = 'A';
	message[1] = 'B';
	message[2] = 'C';
	message[3] = 'D';
	message[4] = 'E';

	printf("\n\nCurrent contents of message: \n");
	print_message();

	printf("\n\nSending message...");
	//send_message();

	while(1) {};
}

int main()
{
	// test_case();

	init();

 	srand(alt_timestamp()); // TODO change in de2 env
	int i;
	GameState state = SETUP;

	for (;;) {
	game_screen();
	switch (state) {
	  case SETUP:
		initialize_dealer(2);

		joining_period();

		/* Move dealer chip to the next player */
		if (dealer_chip == dealer->number_players-1) dealer_chip = 0;
		else dealer_chip++;

		state = DEAL_HANDS;
		break;

	  case DEAL_HANDS:
		deal_hands();
		send_player_hands();

		for (i=0; i<dealer->number_players; i++) {
		  printf("\n\nDealt cards for Player %d: Suite %d, Value %d & Suite %d, Value %d", i,
			dealer->players[i].hand[0].suite, dealer->players[i].hand[0].value,
			dealer->players[i].hand[1].suite, dealer->players[i].hand[1].value);
		}
		while(1);

		state = BET;
		break;

	  case FLOP:
		flop();
		printf("\n\n----------------------------------------");
		printf("\nFLOP (%d, %d), (%d, %d), (%d, %d)",
		  dealer->cards_on_table[0].suite, dealer->cards_on_table[0].value,
		  dealer->cards_on_table[1].suite, dealer->cards_on_table[1].value,
		  dealer->cards_on_table[2].suite, dealer->cards_on_table[2].value);

		state = BET;
		break;

	  case TURN:
		turn();
		printf("\n\n----------------------------------------");
		printf("\nTURN (%d, %d)",
		  dealer->cards_on_table[3].suite, dealer->cards_on_table[4].value);

		state = BET;
		break;

	  case RIVER:
		river();

		printf("\n\n----------------------------------------");
		printf("\nRIVER (%d, %d)",
		  dealer->cards_on_table[4].suite, dealer->cards_on_table[4].value);

		state = BET;
		break;

	  case BET:
		/* Betting Round 1 */
		get_bet_for_player(dealer_chip);
		if (dealer_chip == dealer->number_players-1) {
		  for (i=0; i<dealer->number_players-1; i++) {
			if (dealer->players[i].active) {
			  if (get_bet_for_player(i) == -1) {state = GAME_OVER; goto HOTSTUFF;}
			}
		  }
		} else {
		  for (i=dealer_chip+1; i<dealer->number_players; i++) {
			if (dealer->players[i].active) {
			  if (get_bet_for_player(i) == -1) {state = GAME_OVER; goto HOTSTUFF;}
			}
		  }
		  for (i=0; i<dealer_chip; i++) {
			if (dealer->players[i].active) {
			  if (get_bet_for_player(i) == -1) {state = GAME_OVER; goto HOTSTUFF;}
			}
		  }
		}

		/* If still betting, continue that */
		while (still_betting()) {
		  for (i=dealer_chip; i<dealer->number_players; i++) {
			if (player_still_playing(i)) {
			  if (get_bet_for_player(i) == -1) {state = GAME_OVER; goto HOTSTUFF;}
			}
		  }
		  for (i=0; i<dealer_chip; i++) {
			if (player_still_playing(i)) {
			  if (get_bet_for_player(i) == -1) {state = GAME_OVER; goto HOTSTUFF;}
			}
		  }
	   }

		/* Put bet money into the pot */
		for (i=0; i<dealer->number_players; i++) {
		  dealer->pot += dealer->players[i].money;
		  dealer->players[i].money = 0;
		  dealer->current_bet = 0;
		}

		printf("\nPOT %d", dealer->pot);
		printf("\n----------------------------------------\n");
		switch (dealer->number_cards_on_table) {
		  case 0: state = FLOP; break;
		  case 3: state = TURN; break;
		  case 4: state = RIVER; break;
		  case 5: state = GAME_OVER; break;
		}
		HOTSTUFF: break;

	  case GAME_OVER:
		printf("\n\nGAME OVER\n\n");

		free(dealer->deck);
		free(dealer->players);
		free(dealer);

		return 0;
	}
	}


	 return 0;
}
