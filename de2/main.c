#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

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
int get_bet_for_player(int pid) {
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

/*********************************************************/
typedef enum {
  ROYAL_FLUSH = 9,
  STRAIGHT_FLUSH = 8,
  FOUR_OF_A_KIND = 7,
  FULL_HOUSE = 6,
  FLUSH = 5,
  STRAIGHT = 4,
  THREE_OF_A_KIND = 3,
  TWO_PAIR = 2,
  PAIR = 1,
  HIGH_CARD = 0
} HandType;

typedef struct {
  int value;
  HandType type;

  Card hand[5];
  Card kicker;
} PokerHand;

char * card_name(Card card) {
  char * c = malloc(sizeof(char)*20);

  switch (card.value) {
    case 0: strcpy(c, "Ace"); break;
    case 12: strcpy(c, "King"); break;
    case 11: strcpy(c, "Queen"); break;
    case 10: strcpy(c, "Jack"); break;
    default: sprintf(c, "%d", card.value+1); break;
  }
  strcat(c, " of ");
  switch (card.suite) {
    case 0: strcat(c, "Clubs"); break;
    case 1: strcat(c, "Spades"); break;
    case 2: strcat(c, "Diamonds"); break;
    case 3: strcat(c, "Hearts"); break;
  }
  strcat(c, "\0");
  return c;
}

void split_pot() {
  int i, j, k, m; PokerHand ph[dealer->number_players];
  printf("\n");
  for (i=0; i<dealer->number_players; i++) {
    printf("\nPlayer %d: %s and %s", i,
      card_name(dealer->players[i].hand[0]), card_name(dealer->players[i].hand[1]));
  }
  printf("\n\nCards on table %s, %s, %s, %s and %s\n", 
    card_name(dealer->cards_on_table[0]), card_name(dealer->cards_on_table[1]),
    card_name(dealer->cards_on_table[2]), card_name(dealer->cards_on_table[3]),
    card_name(dealer->cards_on_table[4]) );

  for (i=0; i<dealer->number_players; i++) {

     /* Flush check */
    int suite_count[4]; Card by_suite[4][7];
    for (j=0; j<4; j++) suite_count[j]=0;

    by_suite[dealer->players[i].hand[0].suite][suite_count[dealer->players[i].hand[0].suite]] = dealer->players[i].hand[0];
    suite_count[dealer->players[i].hand[0].suite]++; 
    by_suite[dealer->players[i].hand[1].suite][suite_count[dealer->players[i].hand[1].suite]] = dealer->players[i].hand[1];
    suite_count[dealer->players[i].hand[1].suite]++; 
    for (j=0; j<dealer->number_cards_on_table; j++) {
      by_suite[dealer->cards_on_table[j].suite][suite_count[dealer->cards_on_table[j].suite]] = dealer->cards_on_table[j];
      suite_count[dealer->cards_on_table[j].suite]++;
    }
    for (j=0; j<4; j++) {
      if (suite_count[j] >= 5) {
        printf("\n Player %d: Flush with %s, %s, %s, %s and %s ", i, card_name(by_suite[j][0]), card_name(by_suite[j][1]), 
          card_name(by_suite[j][2]), card_name(by_suite[j][3]), card_name(by_suite[j][4]) );
      }
    }

    /* Full house, 4 of a kind, 3 of a kind, two pair, single pair */
    int value_count[13]; Card by_value[13][4];
    for (j=0; j<13; j++) value_count[j]=0;

    by_value[dealer->players[i].hand[0].value][value_count[dealer->players[i].hand[0].value]] = dealer->players[i].hand[0];
    value_count[dealer->players[i].hand[0].value]++; 
    by_value[dealer->players[i].hand[1].value][value_count[dealer->players[i].hand[1].value]] = dealer->players[i].hand[1];
    value_count[dealer->players[i].hand[1].value]++; 
    for (j=0; j<dealer->number_cards_on_table; j++) {
      by_value[dealer->cards_on_table[j].value][value_count[dealer->cards_on_table[j].value]] = dealer->cards_on_table[j];
      value_count[dealer->cards_on_table[j].value]++;
    }

    for (j=0; j<13; j++) {
      if (value_count[j] ==  4) {
        printf("\nPlayer %d: 4 of a kind with %s, %s, %s and %s ", i, card_name(by_value[j][0]), card_name(by_value[j][1]), 
          card_name(by_value[j][2]), card_name(by_value[j][3]) );
      } 
    }

    for (j=0; j<13; j++) {
      if (value_count[j] == 3) {
        /* Check for 3 of a kind */
        for (k=0; k<13 && k!=j; k++) {
          if (value_count[k] == 2) {
            printf("\nPlayer %d: Full House with %s, %s, %s, %s and %s ", i, card_name(by_value[j][0]), card_name(by_value[j][1]), 
              card_name(by_value[j][2]), card_name(by_value[k][0]), card_name(by_value[k][1]) );
          }
        }

        /* If not full house, is a 3 of a kind */
        printf("\nPlayer %d: 3 of a kind with %s, %s and %s ",i, card_name(by_value[j][0]), card_name(by_value[j][1]), 
          card_name(by_value[j][2]) );
      }
    }


    for (j=0; j<13; j++) {
      if (value_count[j] == 2) {
        /* Check for two pair */
        for (k=0; k<13 && k!=j; k++) {
          if (value_count[k] == 2) {
            printf("\nPlayer %d: Two pair with %s, %s, %s and %s ", i, card_name(by_value[j][0]), card_name(by_value[j][1]), 
              card_name(by_value[k][0]), card_name(by_value[k][1]) );
          }
        }

        /* If not 2 pair, is a single pair */
        printf("\nPlayer %d: Single pair with %s and %s ", i,card_name(by_value[j][0]), card_name(by_value[j][1]) );
      }
    }
    
  }

  printf("\n");
}

void test_split_pot() {
  int i;
  initialize_dealer(2);
  deal_hands();
  for (i=0; i<dealer->number_players; i++) {
    printf("\nDealt cards for Player %d: %s and %s", i,
      card_name(dealer->players[i].hand[0]), card_name(dealer->players[i].hand[1]));
  }
  flop();
  turn();
  river();
  printf("\n\nCards on table %s, %s, %s, %s and %s\n", 
    card_name(dealer->cards_on_table[0]), card_name(dealer->cards_on_table[1]),
    card_name(dealer->cards_on_table[2]), card_name(dealer->cards_on_table[3]),
    card_name(dealer->cards_on_table[4]) );
  split_pot();
}


/*********************************************************/

/* Main */
int main() {
  srand(time(NULL)); int i; // TODO change in de2 env
  GameState state = SETUP;

  //test_split_pot(); return 0;
  
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
          printf("\n\nDealt cards for Player %d: %s & %s", i,
            card_name(dealer->players[i].hand[0]),
            card_name(dealer->players[i].hand[1]));
        }

        state = BET;
        break;

      case FLOP:
        flop();
        printf("\n\n----------------------------------------");
        printf("\nFLOP (%s), (%s), (%s)", 
          card_name(dealer->cards_on_table[0]),
          card_name(dealer->cards_on_table[1]),
          card_name(dealer->cards_on_table[2]));

        state = BET;
        break;

      case TURN:
        turn();
        printf("\n\n----------------------------------------");
        printf("\nTURN (%s)", 
          card_name(dealer->cards_on_table[3]));

        state = BET;
        break;

      case RIVER:
        river();

        printf("\n\n----------------------------------------");
        printf("\nRIVER (%s)", 
          card_name(dealer->cards_on_table[4]));

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
        split_pot();
        printf("\n\nGAME OVER\n\n");

        free(dealer->deck);
        free(dealer->players);
        free(dealer);

        return 0;
    }
  }


}


