package cn.client;

//import java.util.MissingResourceException;

//import cn.client.*;


/**
 * @author Axelle Apvrille
 * @version 0.1
 */
public class Bid {
	static Category cat = Category.getInstance("Bid");
	public static final int PASS = 10;
	public static final int DOUBLE = 20; /* contre */
	public static final int REDOUBLE = 40; /* surcontre */

	public static final int IGNORED = -1;
	public static final int NOTRUMP = 0;
	private static final int MAX_BID = 7;
	private static final int MIN_BID = 1;
	/* use Card.CLUBS, Card.HEART etc for the others */

	private int value; // PASS, or the number of cards to do -6
	private int suit; // the colours, or No trump.
	private int insult; // double or redouble

	public Bid() {
		value = IGNORED;
		suit = IGNORED;
		insult = IGNORED;
	}
	public Bid(int b){
		value = b;
	}
	public void setValue(int v) throws BadTypeException {
		if ((v != PASS) && (v != IGNORED) && ((v < MIN_BID) || (v >MAX_BID)))
			throw new BadTypeException("Impossible bid value.");
		this.value = v;
	}

	public void setSuit(int s) throws BadTypeException {
		if ((s != IGNORED) && ((s < NOTRUMP) || (s > Card.SPADES)))
			throw new BadTypeException("Impossible bid suit");
		if ((value != PASS) && (value != IGNORED) && (s == IGNORED))
			throw new BadTypeException("When value is between "+MIN_BID+" and "+MAX_BID+", the suit cannot be Ignored. Reset the value first.");
		this.suit = s;
	}

	public void setInsult(int i) throws BadTypeException {
		if ((i != DOUBLE) && (i != REDOUBLE) && (i != IGNORED))
			throw new BadTypeException("Impossible bid insult.");
		this.insult = i;
	}

	public int getValue() { return value; }
	public int getSuit() { return suit; }
	public int getInsult() { return insult; }

	public String toString() {
		return "value="+value+" suit="+suit+" insult="+insult+" ";
	}

	public static String show(International intl, int value, int suit, int insult) {
		cat.debug("> Bid.show() val="+value+" suit="+suit+" insult="+insult);
		if (value == PASS) return intl.getString("PASS");

		String s = "";
		if (value != IGNORED) s = s + Integer.toString(value);
		if (suit != IGNORED){
			if (suit == NOTRUMP) s = s+intl.getString("NOTRUMP_SHORT");	
			else s = s + Card.showCardColour(intl,suit);
		}
		if (insult != IGNORED){
			if (insult == DOUBLE) s = s + intl.getString("DOUBLE");
			if (insult == REDOUBLE) s = s + intl.getString("REDOUBLE");
		}
		cat.debug("< Bid.show(): "+s);
		return s;
	}

	public String show(International intl) {
		return Bid.show(intl,this.value,this.suit,this.insult);
	}
	public boolean is_pass(){
		return value == PASS;
	}
	public boolean is_contract(){
		return value > 0;
	}
	public boolean is_double(){
		return value == DOUBLE;
	}
	public boolean is_redouble(){
		return value == REDOUBLE;
	}
}
