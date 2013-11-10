package com.example.supersmashpoker;

import java.io.Serializable;

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
			rank = "null";
			break;
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
			suit = "null";
		}
		
		return "ssp_" + suit + "_" + rank;
	}
	
	public static final int SPADES = 4;
	public static final int HEARTS = 3;
	public static final int CLUBS = 2;
	public static final int DIAMONDS = 1;
	
	public static final int ACE = 1;
	public static final int TWO = 2;
	public static final int THREE = 3;
	public static final int FOUR = 4;
	public static final int FIVE = 5;
	public static final int SIX = 6;
	public static final int SEVEN = 7;
	public static final int EIGHT = 8;
	public static final int NINE = 9;
	public static final int TEN = 10;
	public static final int JACK = 11;
	public static final int QUEEN = 12;
	public static final int KING = 13;
}
