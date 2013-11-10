package com.example.supersmashpoker;

public class Player {
	int number;
	int state;
	int bank;
	Card[] hand;
	
	public Player(int number) {
		this.number = number;
		this.state = WAITING;
		this.bank = 1000;
	}
	
	public void dealHand(Card[] hand) {
		this.hand = hand;
	}
	
	public String getStateMessage() {
		return "";
	}
	
	public static final int WAITING = 0;
	public static final int TURN = 0;
}
