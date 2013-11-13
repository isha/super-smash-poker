package com.example.supersmashpoker;

public class Card {

	public int suit;
	public int rank;

	Card(int suit, int rank) {
		this.suit = suit;
		this.rank = rank;
	}

	public String getResourceName() {
		String rank;
		String suit;
		
		switch( this.rank ) {
		case ACE:
			rank = "ace";
			break;
		case TWO:
			rank = "2";
			break;
		case THREE:
			rank = "3";
			break;
		case FOUR:
			rank = "4";
			break;
		case FIVE:
			rank = "5";
			break;
		case SIX:
			rank = "6";
			break;
		case SEVEN:
			rank = "7";
			break;
		case EIGHT:
			rank = "8";
			break;
		case NINE:
			rank = "9";
			break;
		case TEN:
			rank = "10";
			break;
		case JACK:
			rank = "jack";
			break;
		case QUEEN:
			rank = "queen";
			break;
		case KING:
			rank = "king";
			break;
		default:
			return "ssp_back_card";
		}
		
		switch( this.suit ) {
		case SPADES:
			suit = "spade";
			break;
		case HEARTS:
			suit = "heart";
			break;
		case CLUBS:
			suit = "club";
			break;
		case DIAMONDS:
			suit = "diamond";
			break;
		default:
			return "ssp_back_card";
		}
		
		return "ssp_" + suit + "_" + rank;
	}
	
	
	//	Card Suits
	public static final int
		CLUBS		= 1,
		SPADES		= 2,
		DIAMONDS	= 3,
		HEARTS		= 4,
		BACK		= 0;

	//	Card Ranks 
	public static final int
		ACE		= 1,
		TWO		= 2,
		THREE	= 3,
		FOUR	= 4,
		FIVE	= 5,
		SIX		= 6,
		SEVEN	= 7,
		EIGHT	= 8,
		NINE	= 9,
		TEN		= 10,
		JACK	= 11,
		QUEEN	= 12,
		KING	= 13;
}
