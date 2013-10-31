
/* Struct Definitions */
typedef struct {
  char value;
  char suite;
  struct Card * next;
} Card;

typedef struct {
  Card * hand;
  struct Player * next;
} Player;

typedef struct {
  Player * players;
  Card * deck;
  char cards_in_deck;
} Dealer;

/* Global variables */
Dealer * dealer;


/* Method definitions */
void initialize_dealer(int number_players) {
  dealer = malloc(sizeof(Dealer));

  /* Setup deck of cards */
  dealer->cards_in_deck = 0;
  Card * card = malloc(sizeof(Card)); dealer->deck = card;
  int i, j;
  for (i=0; i<4; i++) {
    for (j=0; j<13; j++) {
      dealer->cards_in_deck++;
      card->suite = i; card->value = j;
      dealer->cards_in_deck == 52 ? card->next = NULL : card->next = malloc(sizeof(Card));
      card = card->next;
    }
  }

  /* Setup players */
  Player * player = malloc(sizeof(Player)); dealer->players = player;
  for (i=0; i<number_players; i++) {
    i==number_players-1 ? player->next = NULL : player->next = malloc(sizeof(Player));
    player = player->next;
  }

}

/* Returns a random card from the deck */
Card * get_card_from_deck() {
  Card * current = dealer->deck, * prev = NULL;

  int r = rand() % dealer->cards_in_deck, i=0;
  while(i != r) {
    prev = current; current = current->next;
    i++;
  }

  prev->next = current->next;
  dealer->cards_in_deck--;
  return current;
}

/* Deals two cards to all players in the game */
void deal_hands() {
  Player * player = dealer->players;

  /* Card 1 */
  while (player != NULL) { 
    player->hand = get_card_from_deck();
    player = player->next;
  }

  /* Card 2 */
  player = dealer->players;
  while (player != NULL) {
    (Card *) (player->hand)->next = get_card_from_deck();
    player = player->next;
  }
}

/* Main */
int main() {
  initialize_dealer(2);

  deal_hands();
}


