package cn.client;

import java.util.Vector;
import java.lang.Math;

import cn.client.Category;

/**
 * This is a simple implementation of a dealer of 52 bridge cards.
 * Not very realistic currently, but should be efficient.
 * @author Axelle Apvrille
 * @version 0.1
 */
public class Dealer {
    static Category cat = Category.getInstance("");
    private static final int DECK_SIZE = 52;
    private Vector deck;
    //private Random random; 

    /**
     * Creates the dealer object, and initializes the deck
     */
    public Dealer(){
		deck = new Vector(DECK_SIZE);
		//random = new Random();
		initDeck();
    }

    /**
     * Initializes the deck, creating all bridge cards, per suit.
     * Call this only once, to instantiate the bridge cards.
     */
    private void initDeck(){
    	for (int i=Card.CLUBS;i<=Card.SPADES;i++){
			for (int j=1;j<=13;j++){
				Card c = new Card(j,i);
				deck.add(c);
			}
		}
    }

    /**
     * There are several ways of implementing a shuffler. This is certainly not the most
     * realistic but it should provide good shuffling of the cards.
     * Basically, we work on the previous deck, and we choose a random number. The card
     * at this position in the deck is added into the current position in the shuffled deck.
     * We remove the selected card from the deck.
     * At the end, we just replace the shuffled deck with the current deck.
     * Beware: if multiple threads, you must make sure we do not start dealing a deck that has not completely
     * been shuffled... or it might not have the right number of cards.
     */
    private void shuffle(){
	cat.debug("> shuffle()");
	Vector shuffled = new Vector(DECK_SIZE);
	int left = deck.size();

	while (left > 0) {
	    cat.debug("Cards lefts in the deck: "+left);
	    if (left == 1) { // just a short cut. It's stupid to take a random number between 1 and 1...
		cat.debug("We select the last card left.");
		shuffled.add(deck.firstElement());
		deck.remove(0);
	    } else {
		int position = random(left); /* position: 0 to left-1 */
		cat.debug("We've selected card number "+position);
		shuffled.add(deck.get(position));
		deck.remove(position);
	    }
	    left = deck.size();
	}

	// at this point, deck should be empty, and shuffle is the shuffled deck.
	cat.debug("Post assertions...");
	if (! deck.isEmpty()) {
	    cat.error("Deck is not empty ?!");
	    throw new AssertionError();
	}
	deck = shuffled;
	
	cat.debug("< shuffle()");
    }

    /**
     * This will deal the cards. It first starts by shuffling the deck, and then starts dealing them
     * starting from the person after the dealer.
     * Normally, no exception is thrown, except if I'm a bad developper ;-)
     * @param north hand of the guy sitting in north position
     * @param east
     * @param south
     * @param west
     * @param dealer orientation of the guy supposed to deal the cards.
     */
    public void deal(Hand north, Hand east, Hand south, Hand west, Orientation dealer){
		cat.debug("> deal()");
	
		cat.debug("Let's first shuffle the cards.");
		shuffle();
	
		cat.debug("We check we have "+DECK_SIZE+" cards or it's useless to go on...");
		if (deck.size() != DECK_SIZE){
		    cat.error("Deck size is : "+deck.size());
		    throw new AssertionError();
		}
	
		try {
		    int orientation = Orientation.next(dealer.idx());
		    cat.debug("Dealing the cards starting with "+orientation);
	
		    for (int i=0; i< deck.size();  i++){
			switch(orientation){
			case Orientation.NORTH:
			    north.addCard((Card)deck.get(i)); break;
			case Orientation.EAST:
			    east.addCard((Card)deck.get(i)); break;
			case Orientation.SOUTH:
			    south.addCard((Card)deck.get(i)); break;
			case Orientation.WEST:
			    west.addCard((Card)deck.get(i)); break;
			default:
			    cat.error("Wrong orientation !");
			    throw new AssertionError();
			}
			orientation = Orientation.next(orientation);
		    }
		}
		catch(Exception exp){
		    // this should never occur (or there's a bug)
		    cat.error(exp.toString());
		    throw new AssertionError();
		}
	
		cat.debug("< deal()");
    }

    /**
     * This is merely a debugging function to show the current content of the deck.
     * The deck may be shuffled or not. If called in the middle of a shuffling, the deck might
     * even not have 52 cards.
     * Output is sent to log facility.
     */
    public void dumpDeck(){
		cat.debug("> dumpDeck()");
		for (int i=0; i< deck.size(); i++){
		    Card c = (Card) deck.get(i);
		    cat.debug("Card["+i+"]= "+c.toString());
		}
		cat.debug("< dumpDeck()");
    }
    public int random(int max){
    	double r = java.lang.Math.random() * (double)(max);
    	return (int)(r);
    }
}
