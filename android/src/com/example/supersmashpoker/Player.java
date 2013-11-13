package com.example.supersmashpoker;

public class Player {
	int id;
	int state;
	int bank;
	Card[] hand;
	
	public Player(int id) {
		this.id = id;
		this.state = START;
		this.bank = STARTING_BANKROLL;
		this.hand = new Card[2];
		hand[0] = new Card(Card.BACK, 0);
		hand[1] = new Card(Card.BACK, 0);
	}
	
	public void dealHand(Card[] hand) {
		this.hand = hand;
	}
	
	public String getStateMessage() {
		switch (this.state){
		case START:
			return "Welcome to Super Smash Poker!";
		case DEALT:
			return "Here are your cards!";
		case WAITING:
			return "Just chill, Dealer is busy!";
		case BET:
			return "Your turn! Ready to up the Ante?";
		case WIN:
			return "YOU WON!";
		case LOSE:
			return "You lost";
		case BROKE:
			return "You're broke and we don't take I-O-U's, get lost!";
		default:
			return "";
		}
	}
	

	public static final int
		START = 0,
		DEALT = 1,
		WAITING = 2,
		BET = 3,
		WIN = 4,
		LOSE = 5,
		BROKE = 6;
	public static final int
		STARTING_BANKROLL = 1000;
}
