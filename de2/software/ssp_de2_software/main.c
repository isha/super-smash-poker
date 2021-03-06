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
//	initialize_audio();
//	start_audio();
	initialize_messaging();
}

/***************************************************************************/

Dealer * dealer;
int dealer_chip = 0;
PokerHand *ph;
int winner_count; int *winners;

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

  /* Initialize cards on table to blank */
  for(i = 0; i < 5; i++) {
	  dealer->cards_on_table[i].value = BLANK_CARD_VALUE;
  }

  /* Cards on table, zero in the beginning */
  dealer->number_cards_on_table = 0;
  dealer->pot = 0;
  dealer->current_bet = ANTY;
  dealer->number_active_players = dealer->number_players;
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
  printf("\nPlayer %d\n Enter your action (0 - Fold, 1 - Call, 2 - Raise): ", pid);

  set_action_state(pid);
  send_message();

  receive_message();
  m_input = read_player_action_and_value(pid);

  if (dealer->players[pid].action == CALL) {
	  if ((dealer->players[pid].total_money - (dealer->current_bet - dealer->players[pid].money)) > 0) {
		dealer->players[pid].total_money -= (dealer->current_bet - dealer->players[pid].money);
		dealer->players[pid].money += (dealer->current_bet - dealer->players[pid].money);
		sprintf(dealer->players[pid].message, "CALL %d", dealer->players[pid].money);
	  } else {
		dealer->players[pid].money += dealer->players[pid].total_money;
		sprintf(dealer->players[pid].message, "ALL IN", dealer->players[pid].total_money);
		dealer->players[pid].total_money = 0;
	  }
  }

  if (dealer->players[pid].action == RAISE) {
    dealer->current_bet += m_input;
    dealer->players[pid].total_money -= (dealer->current_bet - dealer->players[pid].money);
    dealer->players[pid].money += (dealer->current_bet - dealer->players[pid].money);


    if (dealer->players[pid].total_money == 0)
		sprintf(dealer->players[pid].message, "ALL IN", m_input, dealer->players[pid].money);
    else
    	sprintf(dealer->players[pid].message, "RAISE %d (BET %d)", m_input, dealer->players[pid].money);
  }

  if (dealer->players[pid].action == FOLD && dealer->current_bet > 0) {
	dealer->players[pid].active = false;
	dealer->number_active_players--;
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

/* Update pixel buffer */

void game_screen() {
	alt_up_char_buffer_clear(char_buffer);

	char str1[50], str2[30], str3[50];

	sprintf(str1, "Player 1: $%d", dealer->players[0].total_money);
	alt_up_char_buffer_string(char_buffer, str1, 5, 5);

	sprintf(str1, "Player 2: $%d", dealer->players[1].total_money);
	alt_up_char_buffer_string(char_buffer, str1, 5, 10);

	Bitmap* flop_card0_bmp = load_bitmap(card_bitmap_name(dealer->cards_on_table[0]));
	Bitmap* flop_card1_bmp = load_bitmap(card_bitmap_name(dealer->cards_on_table[1]));
	Bitmap* flop_card2_bmp = load_bitmap(card_bitmap_name(dealer->cards_on_table[2]));
	Bitmap* turn_card_bmp  = load_bitmap(card_bitmap_name(dealer->cards_on_table[3]));
	Bitmap* river_card_bmp = load_bitmap(card_bitmap_name(dealer->cards_on_table[4]));

	draw_bitmap(flop_card0_bmp, 25, 60);
	draw_bitmap(flop_card1_bmp, 55, 60);
	draw_bitmap(flop_card2_bmp, 85, 60);
	draw_bitmap(turn_card_bmp, 115, 60);
	draw_bitmap(river_card_bmp, 145, 60);

	sprintf(str3, "Pot: $%d", dealer->pot);
	alt_up_char_buffer_string(char_buffer, str3, 5, 50);

	sprintf(str3, "Current Bet: $%d", dealer->current_bet);
	alt_up_char_buffer_string(char_buffer, str3, 5, 51);
	draw_to_screen();
}

void test_case() {
	init();
}


// YOLOYOLO
/* Return 1 if a is greater than b, -1 if otherwise and 0 if they are equal */
int comp_value(int a, int b) {
  if (a == 0 && b == 0) return 0;
  else if (a == 0) return 1;
  else if (b == 0) return -1;
  else if (a > b) return 1;
  else return -1;
}

void rank_poker_hands() {
  int i, j, k, m, l; ph = malloc(sizeof(PokerHand)*dealer->number_players);

  Card null_card;
  null_card.value = -1;
  null_card.suite = -1;

  printf("\n");
  for (i=0; i<dealer->number_players; i++) {
    printf("\nPlayer %d: %s and %s", i,
      card_name(dealer->players[i].hand[0]), card_name(dealer->players[i].hand[1]));
  }
  printf("\n\nCards on table %s, %s, %s, %s and %s\n",
    card_name(dealer->cards_on_table[0]), card_name(dealer->cards_on_table[1]),
    card_name(dealer->cards_on_table[2]), card_name(dealer->cards_on_table[3]),
    card_name(dealer->cards_on_table[4]) );

  for (i=0; i<dealer->number_players && dealer->players[i].active; i++) {
    ph[i].type = UNDETERMINED;
    ph[i].value = -1;
    ph[i].value_secondary = -1;
    for (j=0; j<5; j++) ph[i].hand[j] = null_card;
    ph[i].kicker = null_card;
    int num_filled = 0;


    /* Create hash maps */
    int suite_count[4]; Card by_suite[4][7];
    for (j=0; j<4; j++) suite_count[j]=0;
    int value_count[13]; Card by_value[13][4]; // For value count, 0-12 represents Two to Ace
    for (j=0; j<13; j++) value_count[j]=0;
    by_suite[dealer->players[i].hand[0].suite][suite_count[dealer->players[i].hand[0].suite]] = dealer->players[i].hand[0];
    suite_count[dealer->players[i].hand[0].suite]++;
    by_suite[dealer->players[i].hand[1].suite][suite_count[dealer->players[i].hand[1].suite]] = dealer->players[i].hand[1];
    suite_count[dealer->players[i].hand[1].suite]++;
    for (j=0; j<dealer->number_cards_on_table; j++) {
      by_suite[dealer->cards_on_table[j].suite][suite_count[dealer->cards_on_table[j].suite]] = dealer->cards_on_table[j];
      suite_count[dealer->cards_on_table[j].suite]++;
    }

    int val = dealer->players[i].hand[0].value == 0 ? 12 : dealer->players[i].hand[0].value - 1;
    by_value[val][value_count[val]] = dealer->players[i].hand[0];
    value_count[val]++;
    val = dealer->players[i].hand[1].value == 0 ? 12 : dealer->players[i].hand[1].value - 1;
    by_value[val][value_count[val]] = dealer->players[i].hand[1];
    value_count[val]++;
    for (j=0; j<dealer->number_cards_on_table; j++) {
      val = dealer->cards_on_table[j].value == 0 ? 12 : dealer->cards_on_table[j].value - 1;
      by_value[val][value_count[val]] = dealer->cards_on_table[j];
      value_count[val]++;
    }

    /* Check for best hand */
    for (j=0; j<4; j++) {
      if (suite_count[j] >= 5) {
        /* Flush */
        if (ph[i].type < FLUSH) {
          ph[i].type = FLUSH;
          ph[i].hand[0] = by_suite[j][0];
          ph[i].hand[1] = by_suite[j][1];
          ph[i].hand[2] = by_suite[j][2];
          ph[i].hand[3] = by_suite[j][3];
          ph[i].hand[4] = by_suite[j][4];

          ph[i].value = ph[i].hand[0].value;
          for (k=0; k<5; k++) {
            if (comp_value(ph[i].hand[k].value, ph[i].value) == 1) ph[i].value = ph[i].hand[k].value;
          }
          num_filled = 5;
        }
      }
    }

    /* Straight */
    for (j=12; j>=4; j--) {
      if (j == 4 && value_count[12]>=1 && value_count[j]>=1 && value_count[j-1]>=1 && value_count[j-2]>=1
        && value_count[j-3]>=1) {
          ph[i].type = STRAIGHT;
          ph[i].value = j;
          ph[i].hand[0] = by_value[j][0];
          ph[i].hand[1] = by_value[j-1][0];
          ph[i].hand[2] = by_value[j-2][0];
          ph[i].hand[3] = by_value[j-3][0];
          ph[i].hand[4] = by_value[12][0];
          num_filled = 5;
          break;
      } else if (value_count[j]>=1 && value_count[j-1]>=1 && value_count[j-2]>=1
        && value_count[j-3]>=1 && value_count[j-4]>=1) {
          ph[i].type = STRAIGHT;
          ph[i].value = j;
          ph[i].hand[0] = by_value[j][0];
          ph[i].hand[1] = by_value[j-1][0];
          ph[i].hand[2] = by_value[j-2][0];
          ph[i].hand[3] = by_value[j-3][0];
          ph[i].hand[4] = by_value[j-4][0];
          num_filled = 5;
          break;
      }
    }

    /* 4 of a kind */
    for (j=12; j>=0; j--) {
      if (value_count[j] == 4 && ph[i].type < FOUR_OF_A_KIND) {
          ph[i].type = FOUR_OF_A_KIND;
          ph[i].value = j;
          ph[i].hand[0] = by_value[j][0];
          ph[i].hand[1] = by_value[j][1];
          ph[i].hand[2] = by_value[j][2];
          ph[i].hand[3] = by_value[j][3];
          num_filled = 4;
          break;
      }
    }


    /* Full house and 3 of a kind */
    for (j=12; j>=0; j--) {
      if (value_count[j] == 3) {
        /* Check for 3 of a kind */
        for (k=12; k>=0 && k!=j; k--) {
          if (value_count[k] == 2
              && ph[i].type < FULL_HOUSE) {
            ph[i].type = FULL_HOUSE;
            ph[i].value = j;
            ph[i].value_secondary = k;
            ph[i].hand[0] = by_value[j][0];
            ph[i].hand[1] = by_value[j][1];
            ph[i].hand[2] = by_value[j][2];
            ph[i].hand[3] = by_value[k][0];
            ph[i].hand[4] = by_value[k][1];
            num_filled = 5;
            break;
          }
        }

        /* If not full house, is a 3 of a kind */
        if (ph[i].type < THREE_OF_A_KIND || (ph[i].type == THREE_OF_A_KIND && comp_value(ph[i].value, j) == -1)) {
          ph[i].type = THREE_OF_A_KIND;
          ph[i].value = j;
          ph[i].hand[0] = by_value[j][0];
          ph[i].hand[1] = by_value[j][1];
          ph[i].hand[2] = by_value[j][2];
          num_filled = 3;
        }
      }
    }

    for (j=12; j>=0; j--) {
      if (value_count[j] == 2) {
        /* Check for two pair */
        for (k=12; k>=0 && k!=j; k--) {
          if (value_count[k] == 2 && ph[i].type < TWO_PAIR) {
            ph[i].type = TWO_PAIR;
            ph[i].value = j;
            ph[i].value_secondary = k;
            ph[i].hand[0] = by_value[j][0];
            ph[i].hand[1] = by_value[j][1];
            ph[i].hand[2] = by_value[k][0];
            ph[i].hand[3] = by_value[k][1];
            num_filled = 4;
            break;
          }
        }

        /* If not 2 pair, is a single pair */
        if (ph[i].type < PAIR) {
          ph[i].type = PAIR;
          ph[i].value = j;
          ph[i].hand[0] = by_value[j][0];
          ph[i].hand[1] = by_value[j][1];
          num_filled = 2;
          break;
        }
      }
    }

    for (j=12; j>=0; j--) {
      if (value_count[j] >= 1 && ph[i].type < HIGH_CARD ) {
          ph[i].type = HIGH_CARD;
          ph[i].value = j;
          ph[i].hand[0] = by_value[j][0];
          num_filled = 1;
          break;
      }
    }

    /* Fill up remaining spots in best hand */
    for (j=12; j>=0 && num_filled<5; j--) {
      for (k=0; k<value_count[j] && num_filled<5; k++) {
        int flag = 0;
        for (l=0; l<num_filled; l++) {
          if (by_value[j][k].value == ph[i].hand[l].value && by_value[j][k].suite == ph[i].hand[l].suite) flag = 1;
        }
        if (flag == 0) {
          ph[i].hand[num_filled] = by_value[j][k];
          num_filled++;
        }
      }
    }

    /* Kicker */
    for (j=12; j>=0; j--) {
      for (k=0; k<value_count[j] && ph[i].kicker.value == -1; k++) {
        int flag = 0;
        for (l=0; l<num_filled; l++) {
          if (by_value[j][k].value == ph[i].hand[l].value && by_value[j][k].suite == ph[i].hand[l].suite) flag = 1;
        }
        if (flag == 0) {
          ph[i].kicker = by_value[j][k];
        }
      }
    }
  }


  for (i=0; i<dealer->number_players && dealer->players[i].active; i++) {
    char *s;
    switch (ph[i].type) {
        case ROYAL_FLUSH: s = "Royal Flush";
          break;
        case STRAIGHT_FLUSH: s = "Straight Flush";
          break;
        case FOUR_OF_A_KIND: s = "Four of a Kind";
          break;
        case FULL_HOUSE: s = "Full House";
          break;
        case FLUSH: s = "Flush";
          break;
        case STRAIGHT: s = "Straight";
          break;
        case THREE_OF_A_KIND: s = "Three of a Kind";
          break;
        case TWO_PAIR: s = "Two Pair";
          break;
        case PAIR: s = "Pair";
          break;
        case HIGH_CARD: s = "High Card";
          break;
        case UNDETERMINED: printf("\nError: Player hand should not be UNDETERMINED"); break;
    }
    printf("\nPlayer %d: %s with %s, %s, %s, %s, %s and kicker %s", i, s, card_name(ph[i].hand[0]), card_name(ph[i].hand[1]),
      card_name(ph[i].hand[2]), card_name(ph[i].hand[3]), card_name(ph[i].hand[4]), card_name(ph[i].kicker));
  }
  printf("\n");
}

void split_pot() {
  if (ph == NULL) { printf("\nHand standing not determined yet!"); return; }
  winner_count = 1;
  int i = 0, j;
  winners = malloc(dealer->number_players*sizeof(int));

  while (!dealer->players[i].active) i++;
  winners[0] = i;

  for (i=winners[0]+1; i<dealer->number_players && dealer->players[i].active; i++) {
    int flag = 0;
    if (ph[i].type > ph[winners[0]].type) {
      winners[0] = i;
      winner_count = winner_count > 1 ? 1 : winner_count;
    }
    else if (ph[i].type == ph[winners[0]].type && comp_value(ph[i].value, ph[winners[0]].value) == 1) {
        winners[0] = i;
        winner_count = winner_count > 1 ? 1 : winner_count;
      }
    else if (ph[i].type == ph[winners[0]].type && ph[i].value == ph[winners[0]].value
      && comp_value(ph[i].value_secondary, ph[winners[0]].value_secondary) == 1) {

        winners[0] = i;
        winner_count = winner_count > 1 ? 1 : winner_count;
      }
    else if (ph[i].type == ph[winners[0]].type && ph[i].value == ph[winners[0]].value &&
      ph[i].value_secondary == ph[winners[0]].value_secondary) {

      for (j=0; j<5 && flag == 0; j++) {
        if (comp_value(ph[i].hand[j].value, ph[winners[0]].hand[j].value) == 1) {
          winners[0] = i; flag = 1;
          winner_count = winner_count > 1 ? 1 : winner_count;
        }
      }
      if (flag == 0 && ph[i].type == ph[winners[0]].type) {
          if (comp_value(ph[i].kicker.value, ph[winners[0]].kicker.value) == 1) {
            winners[0] = i; flag = 1;
            winner_count = winner_count > 1 ? 1 : winner_count;
          }
      }
    } else if (ph[i].type == ph[winners[0]].type && ph[i].value == ph[winners[0]].value &&
      ph[i].value_secondary == ph[winners[0]].value_secondary && flag == 0) {
      winners[winner_count] = i;
      winner_count++;
    }
  }

  printf("\nCongrats on the win ");
  for (j=0; j<winner_count; j++) {
    printf("Player %d, ", winners[j]);
    dealer->players[winners[j]].total_money += dealer->pot/winner_count;
  }
  printf("You win pot of size %d.\n", dealer->pot/winner_count);
  dealer->pot = 0;
}

void last_man_standing() {
  winners = malloc(sizeof(int));
  int i=0;
  while (!dealer->players[i].active) i++;

  printf("\nCongrats on the win Player %d, You win pot of size %d.\n", i, dealer->pot);
  dealer->players[i].total_money += dealer->pot;
  dealer->pot = 0;

  winner_count = 1;
  winners[0] = i;
}

// YOLOYOLO
int main()
{
	printf("\n\n\n----- STARTING A NEW GAME -----");
	init();
	printf("\nInitialization complete.");

 	srand(alt_timestamp()); // TODO change in de2 env
	int i;
	int new_player_count = 2;

	bool first_game = true;
	GameState state = SETUP;

	for (;;) {

	switch (state) {
	  case SETUP:
		printf("\n\n----- ENTERING SETUP STATE -----\n\n");
		initialize_dealer(new_player_count);
		printf("Initialized dealer.");

		joining_period();

		/* Move dealer chip to the next player */
		if (dealer_chip == dealer->number_players-1) dealer_chip = 0;
		else dealer_chip++;

		state = DEAL_HANDS;
		break;

	  case DEAL_HANDS:

		printf("\n\n ----- ENTERING DEAL_HANDS STATE -----\n\n");

		deal_hands();
		send_player_hands();

		for (i=0; i<dealer->number_players; i++) {
		  printf("\n\nDealt cards for Player %d: Suite %d, Value %d & Suite %d, Value %d", i,
			dealer->players[i].hand[0].suite, dealer->players[i].hand[0].value,
			dealer->players[i].hand[1].suite, dealer->players[i].hand[1].value);
		}

		state = ANTY_CALL;
		break;

	  case FLOP:
		flop();
		printf("\n\n----------------------------------------");
		printf("\nFLOP (%s) (%s) (%s) \n",
		  card_name(dealer->cards_on_table[0]),
		  card_name(dealer->cards_on_table[1]),
		  card_name(dealer->cards_on_table[2]));

		state = BET;
		break;

	  case TURN:
		turn();
		printf("\n\n----------------------------------------");
		printf("\nTURN (%s) (%s) (%s) (%s) \n",
		  card_name(dealer->cards_on_table[0]),
		  card_name(dealer->cards_on_table[1]),
		  card_name(dealer->cards_on_table[2]),
		  card_name(dealer->cards_on_table[3]));

		state = BET;
		break;

	  case RIVER:
		river();

		printf("\n\n----------------------------------------");
		printf("\nRIVER (%s) (%s) (%s) (%s) (%s) \n",
		  card_name(dealer->cards_on_table[0]),
		  card_name(dealer->cards_on_table[1]),
		  card_name(dealer->cards_on_table[2]),
		  card_name(dealer->cards_on_table[3]),
		  card_name(dealer->cards_on_table[4]));

		state = BET;
		break;

	  case ANTY_CALL:
		printf("\nCalling anty from all players");

		get_bet_for_player(dealer_chip);
		if (dealer_chip == dealer->number_players-1) {
		  for (i=0; i<dealer->number_players-1; i++) {
			if (dealer->players[i].active) {
			  get_bet_for_player(i);
			  if (dealer->number_active_players == 1) {state = GAME_OVER; goto HOTSTUFF;}
			}
		  }
		} else {
		  for (i=dealer_chip+1; i<dealer->number_players; i++) {
			if (dealer->players[i].active) {
			  get_bet_for_player(i);
			  if (dealer->number_active_players == 1) {state = GAME_OVER; goto HOTSTUFF;}
			}
		  }
		  for (i=0; i<dealer_chip; i++) {
			if (dealer->players[i].active) {
			  get_bet_for_player(i);
			  if (dealer->number_active_players == 1) {state = GAME_OVER; goto HOTSTUFF;}
			}
		  }
		}
		/* Put bet money into the pot */
		for (i=0; i<dealer->number_players; i++) {
		  dealer->pot += dealer->players[i].money;
		  dealer->players[i].money = 0;
		  dealer->current_bet = 0;
		}

		printf("\n----------------------------------------\n");
		printf("\nPOT %d", dealer->pot);
		printf("\n----------------------------------------\n");

	  state = BET;
	  break;

	  case BET:
		  for (i=0; i<dealer->number_players; i++) {
			  if (dealer->players[i].active && dealer->players[i].total_money > 0)
			  	sprintf(dealer->players[i].message, " ");
		  }

	  /* Betting Round 1 */
		get_bet_for_player(dealer_chip);
		if (dealer_chip == dealer->number_players-1) {
		  for (i=0; i<dealer->number_players-1; i++) {
			if (dealer->players[i].active) {
			  get_bet_for_player(i);
			  if (dealer->number_active_players == 1) {state = GAME_OVER; goto HOTSTUFF;}
			}
		  }
		} else {
		  for (i=dealer_chip+1; i<dealer->number_players; i++) {
			if (dealer->players[i].active) {
			  get_bet_for_player(i);
			  if (dealer->number_active_players == 1) {state = GAME_OVER; goto HOTSTUFF;}
			}
		  }
		  for (i=0; i<dealer_chip; i++) {
			if (dealer->players[i].active) {
			  get_bet_for_player(i);
			  if (dealer->number_active_players == 1) {state = GAME_OVER; goto HOTSTUFF;}
			}
		  }
		}

		/* If still betting, continue that */
		while (still_betting()) {
		  for (i=dealer_chip; i<dealer->number_players; i++) {
			if (player_still_playing(i)) {
			  get_bet_for_player(i);
			  if (dealer->number_active_players == 1) {state = GAME_OVER; goto HOTSTUFF;}
			}
		  }
		  for (i=0; i<dealer_chip; i++) {
			if (player_still_playing(i)) {
			  get_bet_for_player(i);
			  if (dealer->number_active_players == 1) {state = GAME_OVER; goto HOTSTUFF;}
			}
		  }
	   }
		switch (dealer->number_cards_on_table) {
		  case 0: state = FLOP; break;
		  case 3: state = TURN; break;
		  case 4: state = RIVER; break;
		  case 5: state = GAME_OVER; break;
		}


	   HOTSTUFF:;

		/* Put bet money into the pot */
		for (i=0; i<dealer->number_players; i++) {
		  dealer->pot += dealer->players[i].money;
		  dealer->players[i].money = 0;
		  dealer->current_bet = 0;
		}

		printf("\n----------------------------------------\n");
		printf("\nPOT %d", dealer->pot);
		printf("\n----------------------------------------\n");

		break;

	  case GAME_OVER:
		if (dealer->number_active_players > 1) {
		  rank_poker_hands();
		  split_pot();
		} else {
		  last_man_standing();
		}

		send_game_results();
		printf("\n\nWaiting for response from all players.\n");
		receive_replay_status();

		state = SETUP;

		free(dealer->deck);
		free(dealer->players);
		free(dealer);
		free(ph);

		break;
	}
	game_screen();
	}


	 return 0;
}
