/*
 * def.h
 *
 *  Created on: 2013-11-10
 *      Author: n1v7
 */

#ifndef DEF_H_
#define DEF_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>


#define ANTY 5
#define BLANK_CARD_VALUE 0x55

/* Struct Definitions */
typedef struct {
  unsigned char value;
  unsigned char suite;
} Card;

typedef enum {
  CALL = 1,
  RAISE = 2,
  FOLD = 0
} PlayerAction;

typedef struct {
  Card hand[2];
  unsigned int total_money;

  bool active;
  PlayerAction action;
  unsigned int money;
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
extern Dealer * dealer;
extern int winner_count; extern int *winners;

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
#endif /* DEF_H_ */
