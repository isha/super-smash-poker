package com.example.supersmashpoker;

public class Player {
	int character_id;
	int state;
	int bank;
	Card[] hand;
	
	public Player(int character_id) {
		this.character_id = character_id;
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
		case JOIN:
			return "Welcome to Super Smash Poker!";
		case START:
			return "Welcome to Super Smash Poker!";
		case DEALT:
			return "Here are your cards!";
		case WAITING:
			return "Just chill, Dealer is busy!";
		case ACTION:
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
	

	// Player States
	public static final int
		JOIN = 0,
		START = 1,
		DEALT = 2,
		WAITING = 3,
		ACTION = 4,
		WIN = 5,
		LOSE = 6,
		BROKE = 7;
	
	// Player Actions
	public static final int
		FOLD = 0,
		CALL = 1,
		RAISE = 2;
	
	public static final int
		STARTING_BANKROLL = 100;
}
