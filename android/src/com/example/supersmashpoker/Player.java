package com.example.supersmashpoker;

public class Player {
	int number;
	int state;
	int money;
	Card[] hand;
	
	public Player(int number) {
		this.number = number;
		this.state = WAITING;
		this.money = 0;
	}
	
	public void dealHand(Card[] hand) {
		this.hand = hand;
	}
	
	public static final int WAITING = 0;
	public static final int TURN = 0;
}
