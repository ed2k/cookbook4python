package cn.client;

import java.util.Vector;
import java.lang.Comparable;
import cn.client.Category;

/**
 * Card of a standard bridge game. No jokers.
 * @author Axelle Apvrille
 * @version 0.1
 */
public class Card implements Comparable{
	static Category cat = Category.getInstance("");
	static String[] SUIT2STR = {"C","D","H","S","N"}; 
	/**
	 * The various bridge colours. They are set in the bridge order.
	 */
	public static final int CLUBS = 1;
	public static final int DIAMOND = 2;
	public static final int HEART = 3;
	public static final int SPADES = 4;

	/**
	 * The various card numbers. The 1 is the ace. But it's the highest card in HonourValue
	 */
	public static final int ACE = 1;
	public static final int KING = 13;
	public static final int QUEEN = 12;
	public static final int JACK = 11;

	/**
	 * Provided for help within this class
	 */
	private static final int MIN_VALUE = 1;
	private static final int MAX_VALUE = 13;

	private int value = 1;
	private int colour = CLUBS;


	/**
	 * Creates a card.
	 * @param value the card number, from 1 to 13. May use the ACE, KING, QUEEN and JACK constants too.
	 * @param colour the suit of the card.
	 * @throws BadTypeException if you set wrong input values.
	 */ 
	public Card(int value, int colour) {
		cat.debug("> Card(): value = " + value + " colour = "+ colour);

		if ((value < MIN_VALUE) || (value > MAX_VALUE))
			return;
		if ((colour < CLUBS) || (colour > SPADES))
			return;
		this.value = value;
		this.colour = colour;
		cat.debug("< Card()");
	}

	public int getValue() { return value; }

	/**
	 * @return the suit of the card.
	 */
	public int getColour() { return colour; }

	public static boolean isMinor(int colour) { 
		if ((colour == CLUBS) || (colour == DIAMOND))
			return true;
		return false;
	}

	public static boolean isMajor(int colour){
		return (!isMinor(colour));
	}

	/**
	 * @return Honour points associated with this card. Ace is the highest with 4. Down to Jack 1, and the rest 0.
	 */
	public int getHonourValue() {
		cat.debug("getHonourValue(): card is = "+value);
		if (value == Card.JACK) return 1;
		if (value == Card.QUEEN) return 2;
		if (value == Card.KING) return 3;
		if (value == Card.ACE) return 4;
		return 0;
	}
	public static int rank(char pbn){
		char c = pbn;
		if (c=='A') return ACE;
		if (c=='K') return KING;
		if (c=='Q') return QUEEN;
		if (c=='J') return JACK;
		if (c=='T') return 10;
		return (c - '0');
	}
	/*
	 * @return number of the card (in letters).
	 */
	public static String showCardValue(int value) {
		cat.debug("> showCardValue(): value = "+ value);
		String s = "";
		if ( value < 11) {
			Integer i = new Integer(value);
			s = i.toString();
		}

		if (value == Card.ACE)
			s = "ACE_SHORT";
		if (value == Card.JACK)
			s = "JACK_SHORT";
		if (value == Card.QUEEN)
			s = ("QUEEN_SHORT");
		if (value == Card.KING)
			s = ("KING_SHORT");

		if (! s.equals(""))
			cat.debug("< showCardValue(): value = "+value+" string= "+s);
		else
			cat.error("< showCardValue(): value = "+value+" unknown tag");
		return s;
	}

	public String showCardValue()  {
		return showCardValue(this.value);
	}

	/**
	 * @return colour of the card, in a short string. (abreviated). Depends on your country.
	 */
	public static String showCardColour(int colour) {
		cat.debug("> showCardColour(): colour = "+colour);
		String s = "";

		switch(colour){
		case CLUBS:
			s = ("CLUBS_SHORT");
			break;
		case DIAMOND:
			s = ("DIAMOND_SHORT");
			break;
		case HEART:
			s = ("HEART_SHORT");
			break;
		case SPADES:
			s = ("SPADES_SHORT");
			break;
		default:
			cat.error("Unknown tag");
		break;
		}

		cat.debug("< showCardColour(): colour = "+ colour+ " string= "+s);
		return s;
	}

	public String showCardColour()  {
		return showCardColour(this.colour);
	}

	/**
	 * For debugging
	 */
	public String toString(){
		//return "Value = " + value + " Colour=" + colour;
		return suit2str(colour)+rank2str(value);
	}
	public static String rank2str(int rank){
		if ((rank > 1) && (rank < 10)) return Integer.toString(rank);
		switch (rank){
		case 10: return "T";
		case ACE: return "A";
		case JACK: return "J";
		case QUEEN: return "Q";
		case KING: return "K";
		}
		return Integer.toString(rank);
	}
	public String rank(){ return rank2str(value);}
	public String suit(){ return suit2str(colour);}
	public static String suit2str(int suit){
		switch(suit){
		case CLUBS: return "C";
		case HEART: return "H";
		case SPADES: return "S";
		case DIAMOND: return "D";
		}
		return Integer.toString(suit);
	}
	public boolean equals(Object obj){
		cat.debug("Card.equals()");
		if (obj instanceof Card) {
			Card c = (Card) obj;
			if ((c.getColour()==colour) && (c.getValue()==value))
				return true;
		} 
		return false;
	}
	public int compareTo(Object obj){
		Card c = (Card)obj;
		if (colour != c.getColour()) return c.getColour()-colour;
		return Card.compare(this, c, new Bid(Bid.NOTRUMP));
	}
	public static int rank_diff(Card a, Card b){
		if (a.getValue()==ACE){
			if (b.getValue()==ACE)return 0;
			return 14-b.getValue();
		}
		if (b.getValue()==ACE){
			return a.getValue()-14;
		}
		return a.getValue()-b.getValue();
	}
	// 0 includes case that a, b has different suit.
	public static int compare(Card a, Card b, Bid trump){
		if (a.getColour()==b.getColour()) return Card.rank_diff(a,b);
		if (a.getColour()==trump.getValue()){
			return 1; 
		}
		return 0;
	}
}
