package cn.client;

import java.util.Vector;
import java.util.Iterator;
import java.util.Collections;
import cn.client.Category;


/**
 * A hand of cards. 13 cards in bridge. Remembers cards you have in hand, and also those that have been played.
 * @author Axelle Apvrille
 * @version 0.1
 */
public class Hand {
	static Category cat = Category.getInstance("");

	private Vector cards;
	private boolean playedCards[];
	private static final int NUMBER = 13;

	public Hand(){
		cat.debug(">Hand()");
		cards = new Vector(Hand.NUMBER);
		playedCards = new boolean[Hand.NUMBER];
		cat.debug("<Hand()");
	}
	/*
	 * parse PBN format AKQ.JT.98.765432
	 */
	public Hand(String[] suits) {
		// TODO Auto-generated constructor stub
		this();
		for(int j=0;j<suits.length;j++){
			for(int k=0;k<suits[j].length();k++){
				Card c = new Card(4-j, Card.rank(suits[j].charAt(k)));
				addCard(c);
			}
		}		
	}
	public void resetHand(){
		cat.debug("> resetHand()");
		cards.clear();
		for (int i=0;i<Hand.NUMBER;i++)
			playedCards[i] = false;
		cat.debug("< resetHand()");
	}

	/**
	 * add a card to your hand. The card is marked 'not played'.
	 * @param c card to add
	 * @throws ArrayIndexOutOfBoundsException if you already have 13 cards in your hand
	 * @throws ImpossibleActionException if you're trying to add a card that already exists (cheater !)
	 */
	public void addCard(Card c)  {
		cat.debug("> Hand.addCard(): card is "+c.toString());
		if (cards.size()>= Hand.NUMBER) return;
		if (cards.indexOf(c) != -1) return;
		cards.add(c);
		playedCards[cards.indexOf((Object)c)]=false;
		cat.debug("< Hand.addCard()");
	}

	/**
	 * @return all cards not played of a given colour. If not cards, then an empty Vector is returned.
	 */
	public Vector selectColour(int colour){
		cat.debug("> selectColour(): colour="+colour);
		Vector v = new Vector(Hand.NUMBER);
		for (int i=0; i< cards.size() ; i++) {
			if (playedCards[i] == false){
				Card c = (Card) cards.get(i);
				if (c.getColour() == colour) {
					cat.debug("Selected: "+c.toString());
					v.add(c);
				}
			}
		}
		cat.debug("< selectColour()");
		Collections.sort(v);
		return v;
	}

	/**
	 * Marks a card as played. Cannot be played again.
	 * @param c card to play
	 * @throws ImpossibleActionException if you're trying to play a card that you don't have, or a card
	 * that has already been played.
	 */
	public void playCard(Card c)  {
		int index = cards.indexOf(c);
		if (index == -1) {
			cat.debug("hand playCard: index -1 card="+c.toString());			
			return;
		}
		if (playedCards[index]==true) {
			cat.debug("hand playCard already played "+c.toString());
			return;
		}
		playedCards[index]=true;
	}

	/**
	 * Displays the cards you haven't played yet in your hand
	 * Cards are displayed colour by colour.
	 * @param intl language to use
	 * @throws MissingResourceException if we cannot find the correct translation for the cards.
	 */
	public String showHand() {
		cat.debug("> showHand()");
		String display = "";

		for (int i=Card.CLUBS; i<=Card.SPADES; i++){
			Vector colour = selectColour(i);
			Iterator j = colour.iterator();
			display += Card.showCardColour(i) + " ";
			while (j.hasNext()){
				Card c = (Card) j.next();
				display += c.showCardValue();
				display += " ";
			}
			display += "\n";
		}

		cat.debug("< showHand(): "+display);
		return display;
	}

	public String toString(){
		cat.debug("> toString()");
		Iterator i = cards.iterator();
		int j=0;
		String s = "";
		while (i.hasNext()) {
			Card c = (Card)i.next();
			String thecard = c.toString();
			s = s+" "+thecard;
			if (playedCards[j])s = s+ "x"+j+"x";
			cat.debug("Card["+j+"]= "+thecard+" played="+playedCards[j]);
			j++;
		}
		cat.debug("< toString()"); 
		return s; 
	}
	public String toSuitString(){
		cat.debug("> showHand()");
		String display = "";
		for (int i=Card.CLUBS; i<=Card.SPADES; i++){
			Vector colour = selectColour(i);
			Iterator j = colour.iterator();
			display += Card.suit2str(i) + ":";
			while (j.hasNext()){
				Card c = (Card) j.next();
				display += c.rank();
				//display += " ";
			}
			display += "\n";
		}

		cat.debug("< showHand(): "+display);
		return display;
	}
	public String toSuitString(int suit){
		String display = "";

		Vector colour = selectColour(suit);
		Iterator j = colour.iterator();
		display += Card.suit2str(suit) + ":";
		while (j.hasNext()){
			Card c = (Card) j.next();
			display += c.rank();
		}
		return display;
	}

}
