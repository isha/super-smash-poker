package com.example.supersmashpoker;

public class Player {
	int number;
	int state;
	int bank;
	Card[] hand;
	
	public Player(int number) {
		this.number = number;
		this.state = START;
		this.bank = 1000;
	}
	
	public void dealHand(Card[] hand) {
		this.hand = hand;
	}
	
	public String getStateMessage() {
		switch (this.state){
		case START:
			return "Welcome to Super Smash Poker!";
		case WAITING:
			return "Just chill, Dealer is busy!";
		case TURN:
			return "Your turn! Ready to up the Ante?";
		case WIN:
			return "YOU WON!";
		case LOSE:
			return "You lost";
		default:
			return "";
		}
	}
	
	public static final int START = 0;
	public static final int WAITING = 1;
	public static final int TURN = 2;
	public static final int WIN = 3;
	public static final int LOSE = 4;
}
