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
	public static final int NOTRUMP = 4;
	private static final int MAX_BID = 7;
	private static final int MIN_BID = 1;
	/* use Card.CLUBS, Card.HEART etc for the others */

	private int value; // PASS, or the number of cards to do -6
	private int suit; // the colours, or No trump.
	//private int insult; // double or redouble

	public Bid() {
		value = IGNORED;
		suit = IGNORED;
		//insult = IGNORED;
	}
   public Bid(String b){
		this();
		String bid = b.toLowerCase();
		char first = b.charAt(0);
		String second = b.substring(1,2);
		if (bid.startsWith(" p")) value = PASS;
		else if (bid.startsWith(" x")) value = DOUBLE;
		else if (bid.startsWith("xx")) value = REDOUBLE;
		else {
			value = first - '0';
			suit = Card.str2suit(second);
		}
	}
	public Bid(int b){
		value = b;
	}
	public void setValue(int v) {
		//if ((v != PASS) && (v != IGNORED) && ((v < MIN_BID) || (v >MAX_BID)))
		//	return;
		this.value = v;
	}

	public void setSuit(int s)  {
		if ((s != IGNORED) && ((s < 0) || (s > Card.SPADES))){
			return;
		}
		if ((value != PASS) && (value != IGNORED) && (s == IGNORED)){		
			return;
		}
		this.suit = s;
	}

	public void setInsult(int i)  {
//		if ((i != DOUBLE) && (i != REDOUBLE) && (i != IGNORED))
//			return;
		this.value = i;
	}

	public int getValue() { return value; }
	public int getSuit() { return suit; }
	public int getInsult() { return value; }

	public String toString() {
		return value+" "+suit;
	}

	public static String show( int value, int suit, int insult) {
		cat.debug("> Bid.show() val="+value+" suit="+suit+" insult="+insult);
		if (value == PASS) return ("PASS");

		String s = "";
		if (value != IGNORED) s = s + Integer.toString(value);
		if (suit != IGNORED){
			if (suit == NOTRUMP) s = s+("NOTRUMP_SHORT");	
			else s = s + Card.showCardColour(suit);
		}
		if (insult != IGNORED){
			if (insult == DOUBLE) s = s + ("DOUBLE");
			if (insult == REDOUBLE) s = s + ("REDOUBLE");
		}
		cat.debug("< Bid.show(): "+s);
		return s;
	}

	public String show() {
		return Bid.show(this.value,this.suit,this.value);
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
