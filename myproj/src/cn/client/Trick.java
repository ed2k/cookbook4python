/**
 * FreeBridge is a Java-Based application to play bridge on your computer.
 * Copyright (C) Axelle and Ludovic Apvrille
 * 
 * This program is free software; you can redistribute it and/or modify it under the 
 * terms of the GNU General Public License as published by the Free Software Foundation; 
 * either version 2 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with this 
 * program; if not, write to the 
 *	 Free Software Foundation, Inc., 59 Temple Place, 
 *	 Suite 330, Boston, MA 02111-1307 USA
 *
 */
package cn.client;

import java.util.Vector;
import java.util.Iterator;

import cn.client.Category;

/** 
 * @author Axelle Apvrille
 * @version 0.1
 */
public class Trick {
	static Category cat = Category.getInstance("");

	private static final int NUMBER_OF_CARDS = 4;
	/**
	 * This represents the orientation of the first player initiating the lift 
	 */
	private Orientation player;
	private Card cards[];
	Bid trump;
	Orientation winner;
	Card lead;
	Orientation leader;

	public Trick(Orientation player, Bid trump) {
		cat.debug("> Trick()");
		cards = new Card[NUMBER_OF_CARDS];
		cat.debug("< Trick()");
		this.player = player;
		this.trump = trump;
		leader = player;
	}

	public void setStart(Orientation s) { player =s ; }
	public Orientation getStart(){ return player; }
	public Card[] getCards() { return cards; }
	public boolean play_card(Card card){
		cards[player.getOrientation()] = card;
		Card win = cards[winner.getOrientation()];
		if(lead == null){
			lead = card;
			winner = player;
		} else if(Card.compare(card,win,trump) > 0){
			winner = player;
		}
		player = player.next();
		return cards[player.getOrientation()] == null;
	}
	/**
	 * This checks that the card can be played. If so, the card is marked played and added to the lift.
	 * @param b the bid that is being played (bid requested)
	 * @param c the card to add to the lift
	 * @param h the hand of the player playing card c.
	 * @throws ImpossibleActionException if you're playing a wrong suit, or if you don't have the card, or if you've
	 * already played it.
	 */
//	public void addCard(Bid b, Card c, Hand h)  {
//		cat.debug("> addCard(): b="+b+" c="+c+" h="+h);
//		checkCard(b,c,h); // throws ImpossibleActionException
//		cards.add(c);
//		h.playCard(c); // throws Impossible Action Exception
//		cat.debug("< addCard()");
//	}

	/**
	 * Checks that you're playing the right suit.
	 * This does not check that card c belongs to hand h. Nor that the card has already been played.
	 * @param b the bid that is being played (bid requested)
	 * @param c the card to add to the lift
	 * @param h the hand of the player playing card c.
	 * @throws ImpossibleActionException You cannot play another suit if you still have cards of that suit.
	 */
//	void checkCard(Bid b, Card c, Hand h) throws ImpossibleActionException {
//		cat.debug("> checkCard(): b="+b+" c="+c+" h="+h);
//		if (! cards.isEmpty()){
//			Card first = (Card) cards.firstElement();
//			if (first.getColour()!=c.getColour()) {
//				// it's okay to play another colour if you haven't got the requested colour
//				if (! h.selectColour(first.getColour()).isEmpty()) 
//					throw new ImpossibleActionException(ImpossibleActionException.YOU_HAVE_CARDS_OF_THE_REQUESTED_SUIT);
//			}
//		}
//		cat.debug("< checkCard()");
//	}

	/**
	 * @param b the bid that is being requested
	 * @return the person who's currently winning the lift. If nothing played yet, returns null.
	 */
//	public Orientation wins(Bid b) {
//		cat.debug("> wins(): b= "+b.toString());
//		Orientation winner = null;
//
//		if (! cards.isEmpty()){
//			Iterator i = cards.iterator();
//			Card highest = (Card) i.next();
//			Orientation current_orientation = player;
//			winner = player; // if there's only one card, then he's the winner.
//
//			while (i.hasNext()){
//				Card current = (Card) i.next();
//				current_orientation = current_orientation.next();
//				cat.debug("Current card is : "+current);
//				cat.debug("Current highest card = "+highest);
//				if (current.getColour()==highest.getColour()){
//					// if the current card is higher, then it's the temporary winner.
//					if (current.getValue()>highest.getValue()) {
//						cat.debug("The current card is higher");
//						winner = current_orientation;
//						highest = current;
//					}
//				} else {
//					// if the current card doesn't have the same suit, it can only win if it's trump.
//					if (current.getColour()==b.getSuit()){
//						cat.debug("The current card is a trump, whereas the highest is not");
//						winner = current_orientation;
//						highest = current;
//					}
//				}
//			} while (i.hasNext());
//		}
//		cat.debug("< wins(): "+winner);
//		return winner;
//	}

	public String toString(){
		String s = "start = "+ player.toString();
//		for (Iterator i = cards.iterator();  i.hasNext() ; ){
//			Card c = (Card)i.next();
//			s = s + " card="+c.toString();
//		}
		for (int i = 0;i<4;i++){
			if (cards[i] == null)s = s+"-";
			else s = s + cards[i].toString();
		}
		return s;
	}

}
