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
		case LEAD:
			return "Your turn! Ready to up the Ante?";
		case FOLLOW:
			return "Your turn! Ready to up the Ante?";
		case WIN:
			return "YOU WON!";
		case LOSE:
			return "You lost...";
		case BROKE:
			return "You're broke and we don't take I-O-U's, get lost!";
		default:
			return "";
		}
	}
	

	// Player States
	public static final int
		JOIN 		= 0,	//State where we connect to DE2
		START 		= 1,	//State where we joined game but show the back of cards
		DEALT 		= 2,	//State where we first get our cards
		WAITING 	= 3,	//State where its not your turn
		ACTION		= 4,	//State where it is our turn
		LEAD 		= 4,	//Sub state of action for when the player before you has not bet
		FOLLOW		= 5,	//Sub state of action for when the player before you has bet
		ENDGAME		= 6,
		WIN 		= 6,	//State to say we won the game
		LOSE 		= 7,	//State to say we lost the game
		BROKE 		= 8;	//State for when you have no money
	
	// Player Actions
	public static final int
		FOLD		= 0,
		CALL		= 1,
		RAISE		= 2;
	
	public static final int
		STARTING_BANKROLL = 100;
}
