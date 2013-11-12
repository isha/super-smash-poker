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