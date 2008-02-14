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
 */
package cn.client;
import java.util.Vector;
import java.util.NoSuchElementException;
import java.util.Iterator;

/**
 * Bridge games start by a sequence of bids, to settle down to a given bid, and then play the game.
 * This class represents the sequence of bids.
 * @author Axelle Apvrille
 * @version 0.1
 */

public class Deal {
	static Category cat = Category.getInstance("");

	/**
	 * The dealer is the first one to make a bid.
	 */
	Orientation dealer;
	Vector bids;
	Orientation player;
	Bid contract;
	int passes;
	Orientation contractor;
	Orientation doubler;
	Orientation redoubler;
	Orientation declarer;
	Orientation dummy;
	Trick trick;
	boolean opening_lead;
	int tricks_taken[];
	Hand hands[];

	public Deal(Orientation dealer){
		cat.debug("> BidStage(): dealer="+dealer.toString());
		hands = new Hand[4];
		this.dealer = dealer;
		player = dealer;
		bids = new Vector();
		player = dealer; // dealer has not made his bid yet.
		trick = null;
	}
	public boolean bid(Bid bid){
		if(bid.is_pass())passes ++;
		else if(bid.is_double()){
			passes = 0;
			doubler = player.team();
		} else if (bid.is_redouble()){
			passes = 0;
			redoubler = player.team();
		} else {
			contract = bid;
			passes = 0;
			contractor = player.team();
			doubler = null;
			redoubler = null;
		}
		bids.add(bid);
		player = player.next();
		if ((passes < 3)||((passes==3)&&(contract == null))){
			return true;
		} else {
			prepare_trick_taking();
			return false;
		}
	}
	public Orientation find_declarer(){
		Orientation p = dealer;
		Iterator i = bids.iterator();
		while(i.hasNext()){
			Bid b = (Bid)i.next();
			if(p.team()==contractor.team()){
				if(b.getSuit()==contract.getSuit())	return p;
			}
			p = p.next();
		}
		return null;
	}
	public void prepare_trick_taking(){
		if (contract == null){
			contract = new Bid(Bid.PASS);
		} else {
			declarer = find_declarer();
			dummy = declarer.getPartner();
			player = declarer.next();
			trick = new Trick(player,contract);
		}
	}
	public boolean legal_card(Card card){
		if (trick.lead == null) return true;
		if (trick.lead.getColour()==card.getColour()) return true;
		Vector v = hands[player.getOrientation()].selectColour(card.getColour());
		if (v.isEmpty())return true;
		return false;
	}
	public void play_card(Card card){
		hands[player.getOrientation()].playCard(card);
		trick.play_card(card);
		player = player.next();
		opening_lead = true;
	}
	public void next_trick(){
		tricks_taken[trick.winner.team().getOrientation()] += 1;
		if (tricks_taken[0]+tricks_taken[1] < 13){
			player = trick.winner;
			trick = new Trick(trick.winner,contract);
		} else trick = null;
	}
	public Orientation getDealer() { return dealer; }
	public Vector getBids() { return bids; }
	public Bid getLastBid() throws NoSuchElementException {
		return (Bid) bids.lastElement();
	}

	/**
	 * @return the orientation of the next player to make a bid
	 */
	public Orientation getNextOrientation() { return player; }

	public void addBid(Bid b)  {
		cat.debug("> addBid(): b="+b);
		if (checkBid(b)){
			bids.add(b);	
			player = player.next();
		}
		cat.debug("< addBid()");
	}

	public boolean checkBid(Bid b)  {
		cat.debug("> checkBid(): b="+b);

		// it always possible to pass.
		if (b.getValue() != Bid.PASS){
			// no need to check that the bid is "possible" (ie no 8 heart)
			// as this is done when instantiating a Bid object.
			if (bids.isEmpty()){
				// for the first bid, to our knowledge, nearly everything is acceptable
				// except double or redouble.
				// saying 5 clubs straight away is acceptable (though it might be stupid)
				if (b.getInsult() != Bid.IGNORED){
					//throw new ImpossibleActionException(ImpossibleActionException.UNACCEPTABLE_BID);
				} else {
					// we need to check the bid is okay with the previous bid.
					Bid previous = null;
					try {
						previous = (Bid) bids.lastElement();
					}
					catch(NoSuchElementException exp){
						cat.fatal("The bids vector is not empty. Still we cannot get the last element. Strange !");
						return false;
					}		
					if ((previous.getValue() > b.getValue()) && 
							(b.getValue() != Bid.IGNORED) &&
							(previous.getValue() != Bid.PASS)){
						cat.debug("previous value="+previous.getValue());
						cat.debug("current value="+b.getValue());				    
						cat.warn("This bid is unacceptable : the current value is less than the previous bid.");
						return false;
					}				
					// TODO: check about the insults.		
					if ((previous.getValue()==b.getValue()) &&
							(previous.getSuit()>=b.getSuit()) &&
							(previous.getInsult()>=b.getInsult())){
						cat.warn("Bids always have to increase.");
						return false;
					}	
				}
			} // end if isEmpty
		} // end if PASS	
		cat.debug("< checkBid()");
		return true;
	}

	/**
	 * @return true if the bid stage is finished (people have passed)
	 */
	public boolean isFinished(){
		cat.debug("> isFinished()");

		// everybody must get a chance to bid before the stage is finished.
		int len = bids.size();
		if (len < 4) {
			cat.debug("< isFinished(): not enough bids: "+len);
			return false;
		}
		cat.debug("Length of bids vector="+len);

		for (int i= len ;i > (len-4)  ; i--) {
			Bid b = (Bid) bids.get(i-1);
			cat.debug("Check bid i="+i+": "+b.toString());
			if (b.getValue() != Bid.PASS) {
				cat.debug("< isFinished(): false");
				return false;
			}
		}

		cat.debug("< isFinished(): true");
		return true;
	}

	/**
	 * @return the bid that we're going to play. If the bid stage is not finished yet,
	 * then this bid might still be challenged.
	 * returns null if nobody has made a bid yet.
	 */
	public Bid getBidToPlay(){
		cat.debug("> getBidToPlay()");
		if (bids.isEmpty()) return null;

		boolean found = false;
		int i = bids.size()-1;
		Bid b, bidToPlay;
		bidToPlay = new Bid();


		while ((i >= 0) && (found == false)){
			cat.debug("i="+i);
			b = (Bid) bids.get(i);
			if ((b.getValue() != Bid.IGNORED) &&
					(b.getValue() != Bid.PASS)) {
				cat.debug("Found the bid we're going to play");
				bidToPlay.setValue(b.getValue());
				bidToPlay.setSuit(b.getSuit());
				if (bidToPlay.getInsult()==Bid.IGNORED) bidToPlay.setInsult(b.getInsult());
				found = true;
			} else {
				if (b.getInsult() != Bid.IGNORED) {
					cat.debug("Recording the insult...");
					bidToPlay.setInsult(b.getInsult());
				}
			}
			i--;
		} // end of while
		if ((bidToPlay.getValue() == Bid.IGNORED) && 
				(bidToPlay.getSuit() == Bid.IGNORED) &&
				(bidToPlay.getInsult() == Bid.IGNORED)) {
			cat.debug("< getBidToPlay(): Nobody has made a bid yet: null.");
			return null;
		}


		cat.debug("< getBidToPlay(): bidToPlay="+bidToPlay.toString());
		return bidToPlay; //TO CHANGE
	}

	/**
	 * The "dead" is the the partner of the first of one camp that has announced the suit that
	 * is being played.
	 * @return orientation of the dead. Returns null if bid stage isn't finished yet.
	 *
	 */
	public Orientation getDead(){
		cat.debug("> getDead()");
		Bid bidToPlay = getBidToPlay();
		if (isFinished()==false){
			cat.debug("< getDead(): bid stage isn't finished yet. It's impossible to get the dead. Null");
			return null;
		}

		// We search for the orientation of the player who announced the bid to play.
		// Beware, this does NOT determine the dead yet.
		Orientation current = dealer;
		Orientation bidOrientation = null;
		Orientation firstTalk = null;
		for (Iterator i = bids.iterator(); (i.hasNext()) && (bidOrientation == null); ){
			Bid b = (Bid) i.next();
			cat.debug(current.toString()+" announced "+b.toString());
			if ((b.getValue() == bidToPlay.getValue()) &&
					(b.getSuit() == bidToPlay.getSuit()))
				bidOrientation = current;
			current = current.next();
		}
		if (bidOrientation == null) {
			cat.fatal("Something's wrong...bidOrientation==null");
			//System.exit(-1);
		}
		cat.debug("Orientation of the person who announced the bid is "+bidOrientation);

		// The dead is the partner of the person who spoke of the suit first.
		current = dealer;
		for (Iterator j = bids.iterator(); (j.hasNext()) && (firstTalk == null) ; ){
			Bid b = (Bid) j.next();
			cat.debug(b.toString());
			if (b.getSuit() == bidToPlay.getSuit()){
				if (current.isPartner(bidOrientation))
					firstTalk = current;
			}
			current = current.next();
		}
		if (firstTalk == null){
			cat.fatal("Something's wrong... firstTalk==null");
			//System.exit(-1);
		}
		cat.debug("Orientation of the first person to speak of suit "+bidToPlay.getSuit()+" is "+firstTalk.toString());

		cat.debug("< getDead() is partner of "+firstTalk);
		return firstTalk.getPartner();
	}

	public String toString(){
		cat.debug("> toString()");

		// dumping the bid
		for (Iterator i= bids.iterator(); i.hasNext() ; ) {
			Bid b = (Bid) i.next();
			cat.debug("bid= "+b.toString());
		}
		cat.debug("< toString()");
		return "Dealer = "+dealer.toString()+ " bids="+bids;
	}
	public boolean currenTrickCompleted() {
		// TODO Auto-generated method stub
		return trick.completed();
	}
}
