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


#define ANTY 100

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

  unsigned int pot;
  unsigned int current_bet;
} Dealer;

/* Global variables */
extern Dealer * dealer;

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

#endif /* DEF_H_ */
