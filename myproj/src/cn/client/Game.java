/**
 *
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

import java.util.MissingResourceException;

import cn.client.Category;


/**
 * 4 games is a match.
 * @author Axelle Apvrille
 * @version 0.1
 */
public class Game {
	static Category cat = Category.getInstance("");

	/**
	 * The leader is the first player to play a card. It is different from the dealer i.e the player
	 * who deals the cards and is the first to make (possibly) a bid.
	 */
	private Orientation leader = null;
	private Match currentMatch = null;
	private Deal bids = null;
	private Dealer deal = null;

	/**
	 * Initiates a game with the given players
	 */
	public Game(){
		this(new Match());
	}

	/**
	 * @param m the current match. The dealer of the match must be set.
	 */
	public Game(Match m){
		cat.debug("> Game()");
		currentMatch = m;
		bids = new Deal(m.getDealer());
		deal = new Dealer();
		cat.debug("< Game()");
	}

	public Match getCurrentMatch() { return currentMatch; }

	public void setLeader(Orientation l) { leader = l; }
	public Orientation getLeader() { return leader; }

	/**
	 * Shuffles and deals the card using the current card dealer
	 */
	public void doDeal(){
		cat.debug("> doDeal()");

		// we deal the cards
		Hand n = new Hand();
		Hand e = new Hand();
		Hand s = new Hand();
		Hand w = new Hand();


		deal.deal(n,e,s,w,currentMatch.getDealer());

		// set the hands to the players
		currentMatch.getNorth().setHand(n);
		currentMatch.getEast().setHand(e);
		currentMatch.getSouth().setHand(s);
		currentMatch.getWest().setHand(w);

		cat.debug("< doDeal()");
	}

	/**
	 * Will start asking the dealer to play, and then continue until the bid stage is finished.
	 * @param intl 
	 * @throws MissingResourceException 
	 * @return the bid to play
	 */
	public Bid doBidStage() throws MissingResourceException {
		cat.debug("> doBidStage()");

		Orientation current = currentMatch.getDealer();
		Player p = null;
		Bid b = null;

		while (! bids.isFinished()){
			boolean ok = false;

			// get the current player.
			switch(current.idx()){
			case Orientation.NORTH: p = currentMatch.getNorth();  break;
			case Orientation.EAST: p = currentMatch.getEast(); break;
			case Orientation.SOUTH: p = currentMatch.getSouth(); break;
			case Orientation.WEST: p = currentMatch.getWest(); break;
			default:
				cat.fatal("Unknown orientation ! ");
			//System.exit(-1);
			}
			cat.debug("It's "+current+"'s turn.");

			// show the hand of the player 
			p.showHand();

			// ask the current player his bid.
			do {

					b = p.doBid(); 
					bids.addBid(b);
					ok = true;

//				catch(BadTypeException exp1){
//					cat.warn("[Game] BadTypeException caught: "+exp1);
//					// TODO: add a method to send an error message.
//					// loop until we get a correct bid
//					ok = false;
//				}
//				catch(ImpossibleActionException exp){
//				cat.warn("[Game] ImpossibleActionException: "+exp);
//				// TODO: add a method to send an error message.
//				// loop until we get a correct bid
//				ok = false;
//				}
//				catch(Exception exp2){
//					// for instance a Missing Resource Exception 
//					cat.fatal("[Game] Fatal exception caught: "+exp2);
//					//System.exit(-1);
//				}
			} while (! ok);

			// next player
			current = current.next();
		}

		cat.debug("< doBidStage()");
		return bids.getBidToPlay();
	}

	/**
	 * Starts playing cards
	 */
	public void doPlay()  {
		// get everybody to play
	}

	public String toString(){
		String s = currentMatch.toString();
		if (leader != null) s += " leader="+leader.idx();
		cat.debug(s);
		return s;
	}
}
