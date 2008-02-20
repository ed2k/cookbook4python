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

import cn.client.Category;

/**
 * A match is 4 games with the same players, using different vulnerabilities at
 * each turn.
 * 
 * @author Axelle Apvrille
 * @version 0.1
 */

public class Match {
	static Category cat = Category.getInstance("");

	protected Player north,east,south,west;
	protected Vulnerability vuln;
	protected Orientation dealer;

	/**
	 * A match is a sequencee of 4 games with the same players.
	 * The first dealer of the match will always be north. And at first, nobody
	 * is vulnerable.
	 */
	public Match(){
		cat.debug("> Match()");
		vuln = new Vulnerability();

			dealer = new Orientation(Orientation.NORTH);

	}

	public void setNorth(Player n){
		north = n;
	}

	public void setEast(Player n){
		cat.debug("> setEast(): n="+n);
		east = n;
		east.setOrientation(new Orientation(Orientation.EAST));	
		cat.debug("< setEast()");
	}

	public void setSouth(Player n){
		cat.debug("> setSouth(): n="+n);
		south = n;
		south.setOrientation(new Orientation(Orientation.SOUTH));	
		cat.debug("< setSouth()");
	}

	public void setWest(Player n){
		cat.debug("> setWest(): n="+n);
		west = n;
		west.setOrientation(new Orientation(Orientation.WEST));	
		cat.debug("< setWest()");
	}

	public Player getNorth() { return north; }
	public Player getEast() { return east; }
	public Player getSouth() { return south; }
	public Player getWest() { return west; }

	public Vulnerability getVulnerability() { return vuln; }
	public Orientation getDealer() { return dealer; }

	public void nextGame() {
		//cat.debug("> nextGame()");

		vuln.setVulnerability(vuln.next());
		dealer = dealer.next();


		//cat.debug("< nextGame(): vuln="+vuln+" dealer="+dealer);
	}
}
