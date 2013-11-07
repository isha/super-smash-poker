#include <stdlib.h>
#include <stdio.h>

/* Struct Definitions */
typedef struct {
  char value;
  char suite;
} Card;

typedef struct {
  Card hand[2];
} Player;

typedef struct {
  Player * players;
  Card * deck;
  char number_players;
  char cards_in_deck;

  Card cards_on_table[5];
  char number_cards_on_table;
} Dealer;

/* Global variables */
Dealer * dealer;

typedef enum {
  SETUP,
  DEAL_HANDS,
  FLOP,
  TURN,
  RIVER,
  BET,
  GAME_OVER
} GameState;

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

  /* Cards on table, zero in the beginning */
  dealer->number_cards_on_table = 0;
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

/* Main */
int main() {
  GameState state = SETUP;
  
  for (;;) {
    switch (state) {
      case SETUP:
        initialize_dealer(2);
        state = DEAL_HANDS;
        break;

      case DEAL_HANDS:
        deal_hands();

        printf("\n\nDealt cards for Player 1: Suite %d, Value %d & Suite %d, Value %d", 
          dealer->players[0].hand[0].suite, dealer->players[0].hand[0].value,
          dealer->players[0].hand[1].suite, dealer->players[0].hand[1].value);
        printf("\nDealt cards for Player 2: Suite %d, Value %d & Suite %d, Value %d", 
          dealer->players[1].hand[0].suite, dealer->players[1].hand[0].value,
          dealer->players[1].hand[1].suite, dealer->players[1].hand[1].value);

        state = BET;
        break;

      case FLOP:
        flop();
        
        printf("\n\nFLOP (%d, %d), (%d, %d), (%d, %d)", 
          dealer->cards_on_table[0].suite, dealer->cards_on_table[0].value,
          dealer->cards_on_table[1].suite, dealer->cards_on_table[1].value,
          dealer->cards_on_table[2].suite, dealer->cards_on_table[2].value);

        state = BET;
        break;

      case TURN:
        turn();

        printf("\n\nTURN (%d, %d)", 
          dealer->cards_on_table[3].suite, dealer->cards_on_table[4].value);

        state = BET;
        break;

      case RIVER:
        river();

        printf("\n\nRIVER (%d, %d)", 
          dealer->cards_on_table[4].suite, dealer->cards_on_table[4].value);

        state = BET;
        break;

      case BET:
        switch (dealer->number_cards_on_table) {
          case 0: state = FLOP; break;
          case 3: state = TURN; break;
          case 4: state = RIVER; break;
          case 5: state = GAME_OVER; break;
        }
        break;

      case GAME_OVER:
        printf("\n\nGAME OVER\n\n");
        return 0;
    }
  }


}


