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
  int number_active_players;
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

/*********************************************************/

#define CLUBS 0
#define SPADES 1
#define DIAMONDS 2
#define HEARTS 3

#define KING 12
#define QUEEN 11
#define JACK 10
#define TEN 9
#define NINE 8
#define EIGHT 7
#define SEVEN 6
#define SIX 5
#define FIVE 4
#define FOUR 3
#define THREE 2
#define TWO 1
#define ACE 0

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
  HIGH_CARD = 0,
  UNDETERMINED = -1
} HandType;

typedef struct {
  int value;
  int value_secondary;
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

PokerHand *ph;

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

  int i = 0, j, winner_count = 1; int *winners = malloc(dealer->number_players*sizeof(int));

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
  int i=0;
  while (!dealer->players[i].active) i++;

  printf("\nCongrats on the win Player %d, You win pot of size %d.\n", i, dealer->pot);
  dealer->players[i].total_money += dealer->pot;
  dealer->pot = 0;
}


void test_rank_poker_hands() {
  int i;
  initialize_dealer(8);
  deal_hands();
  flop();
  turn();
  river();
  dealer->pot = 3000;
  // /* Testing data */
  // dealer->players[0].hand[0].value = JACK;
  // dealer->players[0].hand[0].suite = CLUBS;
  // dealer->players[0].hand[1].value = TEN;
  // dealer->players[0].hand[1].suite = CLUBS;

  // dealer->players[1].hand[0].value = TWO;
  // dealer->players[1].hand[0].suite = HEARTS;
  // dealer->players[1].hand[1].value = THREE;
  // dealer->players[1].hand[1].suite = CLUBS;

  // dealer->cards_on_table[0].value = KING;
  // dealer->cards_on_table[0].suite = CLUBS;
  // dealer->cards_on_table[1].value = QUEEN;
  // dealer->cards_on_table[1].suite = HEARTS;
  // dealer->cards_on_table[2].value = NINE;
  // dealer->cards_on_table[2].suite = DIAMONDS;
  // dealer->cards_on_table[3].value = EIGHT;
  // dealer->cards_on_table[3].suite = CLUBS;
  // dealer->cards_on_table[4].value = SEVEN;
  // dealer->cards_on_table[4].suite = CLUBS;

  // dealer->number_cards_on_table = 5;


  rank_poker_hands();
  split_pot();
}


/*********************************************************/

/* Main */
int main() {
  srand(time(NULL)); int i; // TODO change in de2 env
  GameState state = SETUP;

  //test_rank_poker_hands(); return 0;
  
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
          printf("\n\nDealt cards for Player %d: %s and %s", i,
            card_name(dealer->players[i].hand[0]),
            card_name(dealer->players[i].hand[1]));
        }

        state = BET;
        break;

      case FLOP:
        flop();
        printf("\n\n----------------------------------------");
        printf("\nFLOP %s, %s and %s", 
          card_name(dealer->cards_on_table[0]),
          card_name(dealer->cards_on_table[1]),
          card_name(dealer->cards_on_table[2]));

        state = BET;
        break;

      case TURN:
        turn();
        printf("\n\n----------------------------------------");
        printf("\nTURN %s", 
          card_name(dealer->cards_on_table[3]));

        state = BET;
        break;

      case RIVER:
        river();

        printf("\n\n----------------------------------------");
        printf("\nRIVER %s", 
          card_name(dealer->cards_on_table[4]));

        state = BET;
        break;

      case BET:
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

        free(dealer->deck);
        free(dealer->players);
        free(dealer);
        free(ph);

        printf("\n\nContinue play? (0 - No, 1 - Yes) ");
        int m;
        scanf("%d", &m);
        if (m==0) return 0;
        else state = SETUP;
        break;
    }
  }


}


