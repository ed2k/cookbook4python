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

import cn.client.Category;

/** Represents an abstract player.
 * @author Axelle Apvrille
 * @version 0.1
 */
public class Player {
	static Category cat = Category.getInstance("");

	/**
	 * The player's name. For instance John Doe.
	 */
	private String name;

	/**
	 * The cumulative score through all the matches of the player
	 */
	private int globalScore;

	/**
	 * The position of the player (north, east, south, west). 
	 */
	private Orientation currentPosition;

	/**
	 * Current playing cards of the player
	 */
	private Hand currentHand;

	/**
	 * Builds an object for the NORTH player. If you don't want to be north, use setOrientation() after.
	 * @param n name of the player
	 */
	public Player(String n){
		cat.debug("> Player() :"+n);
		name = n;
		globalScore = 0;
		currentHand = null;
		currentPosition = new Orientation(Orientation.NORTH);
		cat.debug("< Player() :"+n);
	}

	public void setOrientation(Orientation orient){
		cat.debug("> setOrientation()");
		currentPosition = orient; // previous object will be discarded.
		cat.debug("< setOrientation()");
	}
	public void setHand(Hand h) { currentHand = h; }

	public String getName() { return name;}
	public int getGlobalScore() { return globalScore; }

	/**
	 * @return the orientation of this player
	 */
	public Orientation getOrientation() { return currentPosition; }
	public Hand getHand() { return currentHand; }

	/**
	 * @param score score to add to the current global score for the player.
	 */
	public void addScore(int score) { globalScore += score; }

	/**
	 * Resets the global score to 0. Call when you've lost too much ;-)
	 */
	public void resetGlobalScore() { globalScore = 0; }

	/**
	 * Chooses a bid to announce
	 * @param intl language to use to print messages
	 * @return the bid to announce.
	 * @throws BadTypeException if the player specifies an incorrect value for the bid
	 * @throws MissingResourceException if we cannot print out the correct strings in the given language
	 */
	public  Bid doBid() {
		return null;
	}

	/**
	 * @return the card the player plays.
	 */
	public  Card doPlay(){ return null;}

	/**
	 * Displays the hand of the player
	 */
	public  void showHand() {};

	public String toString(){
		String s = "Name= "+name+" globalScore="+globalScore+" position="+currentPosition.toString();
		if (currentHand != null) s += " hand="+currentHand.toString();
		cat.debug(s);
		return s;
	}

}
