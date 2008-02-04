package cn.client;

import cn.client.Category;

/**
 * @author Axelle Apvrille
 * @version 0.1
 */
public class ImpossibleActionException extends Exception {
    static Category cat = Category.getInstance("");
    public static final int CARD_DOES_NOT_EXIST = 0;
    public static final int CARD_ALREADY_PLAYED = 1;
    public static final int CARD_ALREADY_EXISTS = 2;
    public static final int YOU_HAVE_CARDS_OF_THE_REQUESTED_SUIT = 3;
    public static final int UNACCEPTABLE_BID = 4;

    private int code = 0;
    public ImpossibleActionException(int code){
	super();
	this.code = code;
	cat.warn("ImpossibleActionException: code="+code);
    }
    public String toString(){
	cat.debug("> toString()");
	String s = "";
	switch(code){
	case CARD_DOES_NOT_EXIST: s = "Card does not exist"; break;
	case CARD_ALREADY_PLAYED: s = "Card already played"; break;
	case CARD_ALREADY_EXISTS: s = "Card already exists"; break;
	case YOU_HAVE_CARDS_OF_THE_REQUESTED_SUIT: s = "You have cards of the requested suit"; break;
	case UNACCEPTABLE_BID: s = "Your bid is unacceptable"; break;
	default:
	    s = "Unknown code"; break;
	}
	cat.debug(s);
	return s;
    }
}
