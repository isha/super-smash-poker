#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define ANTY 100

/* Struct Definitions */
typedef struct {
  char value;
  char suite;
} Card;


typedef enum {
  START_BET = 0,
  CALL = 1,
  CHECK = 2,
  RAISE = 3,
  FOLD = 4
} PlayerAction;

typedef struct {
  Card hand[2];
  int total_money;

  bool active;
  PlayerAction action;
  int money;
} Player;

typedef struct {
  Player * players;
  Card * deck;
  char number_players;
  char cards_in_deck;

  Card cards_on_table[5];
  char number_cards_on_table;

  int pot;
  int current_bet;
} Dealer;

/* Global variables */
Dealer * dealer;
int dealer_chip = 0;

/* Different states that a game can be in */
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
void get_bet_for_player(int pid) {
  printf("\n\n----------------------------------------");
  printf("\nYour total money %d and bet money %d", dealer->players[pid].total_money, dealer->players[pid].money);
  printf("\nPlayer %d\n Enter your action (0 - Bet, 1 - Call, 2 - Check, 3 - Raise, 4 - Fold): ", pid);
  scanf("%d", &dealer->players[pid].action);
  
  if (dealer->players[pid].action == START_BET) {
    int m;
    printf("\nBetting value: ");
    scanf("%d", &m);
    dealer->current_bet = m;
    dealer->players[pid].money = m;
    dealer->players[pid].total_money -= dealer->players[pid].money;
  }

  if (dealer->players[pid].action == CALL) {
    dealer->players[pid].total_money -= (dealer->current_bet - dealer->players[pid].money);
    dealer->players[pid].money += (dealer->current_bet - dealer->players[pid].money);
  }

  if (dealer->players[pid].action == RAISE) {
    int m;
    printf("\nHow much do you want to raise by: ");
    scanf("%d", &m);
    dealer->current_bet += m;
    dealer->players[pid].total_money -= (dealer->current_bet - dealer->players[pid].money);
    dealer->players[pid].money += (dealer->current_bet - dealer->players[pid].money);
  }

  if (dealer->players[pid].action == FOLD) {
    dealer->players[pid].active = false;

    int i, active_players = 0;
    for (i=0; i<dealer->number_players; i++) {
      if (dealer->players[i].active) active_players++;
    }
  }
  printf("\nYour total money %d and bet money %d", dealer->players[pid].total_money, dealer->players[pid].money);
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


/* Main */
int main() {
  srand(time(NULL)); int i; // TODO change in de2 env
  GameState state = SETUP;
  
  for (;;) {
    switch (state) {
      case SETUP:
        initialize_dealer(2);

        /* Move dealer chip to the next player */
        if (dealer_chip == dealer->number_players-1) dealer_chip = 0;
        else dealer_chip++;

        state = DEAL_HANDS;
        break;

      case DEAL_HANDS:
        deal_hands();

        for (i=0; i<dealer->number_players; i++) {
          printf("\n\nDealt cards for Player %d: Suite %d, Value %d & Suite %d, Value %d", i,
            dealer->players[i].hand[0].suite, dealer->players[i].hand[0].value,
            dealer->players[i].hand[1].suite, dealer->players[i].hand[1].value);
        }

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
              get_bet_for_player(i);
            }
          }
        } else {
          for (i=dealer_chip+1; i<dealer->number_players; i++) {
            if (dealer->players[i].active) {
              get_bet_for_player(i);
            }
          }
          for (i=0; i<dealer_chip; i++) {
            if (dealer->players[i].active) {
              get_bet_for_player(i);
            }
          }
        }

        /* If still betting, continue that */
        while (still_betting()) {
          for (i=dealer_chip; i<dealer->number_players; i++) {
            if (player_still_playing(i)) {
              get_bet_for_player(i);
            }
          }
          for (i=0; i<dealer_chip; i++) {
            if (player_still_playing(i)) {
              get_bet_for_player(i);
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
        break;

      case GAME_OVER:
        printf("\n\nGAME OVER\n\n");

        free(dealer->deck);
        free(dealer->players);
        free(dealer);

        return 0;
    }
  }


}


