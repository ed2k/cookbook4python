package cn.client;

//import java.io.FileInputStream;
//import java.io.IOException;
//import java.util.MissingResourceException;
//import java.util.Properties;

import cn.client.Category;

/**
 * Computes the score for the current deal.
 * More complicated than what I had initially thought but pretty mathematical.
 * If rules change, modify the properties file scoring.
 * @author Axelle Apvrille
 * @version 0.1
 */
public class Score {
    static Category cat = Category.getInstance("");
    private static final String DEFAULT_BRIDGE_FILENAME = "bridge.properties";

    private Bid request = null; // the bid that has been asked
    private Bid done = null; // the bid that has really been done
    private boolean vulnerable = false;
    //private Properties prop;
    
    /**
     * Loads the score property file.
     * @param filename name of the file containing all the values for the scoring. Those constants
     * have been put in a text file so that it's possible to edit them if the rules change...
     * @throws IOException if something's wrong at reading, opening or closing the file.
     */
    public Score(String filename) {
	cat.debug("> Score()");
	cat.debug("Opening "+filename);
	//FileInputStream fis = new FileInputStream(filename);
	//prop = new Properties();
	cat.debug("Loading properties");
	//prop.load(fis);
	cat.debug("Closing "+filename);
	//fis.close();
	cat.debug("< Score()");
    }

    /**
     * Loads the default score property file.
     */
    public Score()  {
	this(DEFAULT_BRIDGE_FILENAME);
    }
    
    public void setBidRequest(Bid req) { request = req; }
    public void setBidDone(Bid b) {done = b; }
    public void setVulnerable(boolean vul) { vulnerable = vul; }

    public Bid getBidRequest() { return request; }
    public Bid getBidDone() { return done; }
    public boolean getVulnerable() { return vulnerable; }

    /**
     * Computes the score. You should call this only when the biddings have been set (both the requested bid
     * and whqt has really been achieved).
     * @return the score. If score is negative, this means that the contract has not been met and that the points
     * should be scored by the opponents.
     * @throws NullPointerException if you try to compute the score without having set the required bid and
     * the bid that has been done.
     * @throws MissingResourceException if we cannot load the right property from the score property file.
     */
    public int computeScore() throws NullPointerException {
	cat.debug("> computeScore()");
	if ((request == null) || (done == null)) throw new NullPointerException();

	int suit = done.getSuit();
	int value = done.getValue();
	int insult = done.getInsult();
	int score = 0;

	if (isDone()){
	    score = computeBasicScore(suit,value,insult);
	    score += computeBonusScore();
	    score += computeOvertrickScore();
	} else {
	    score = - computeUndertrickScore();
	}

	cat.debug("< computeScore(): score="+score);
	return score;
    }

    /**
     * To be called only if the contract has been met.
     * @param suit
     * @param value
     * @param insult
     * @throws MissingRessourceException 
     */
    private int computeBasicScore(int suit, int value, int insult)  {
		cat.debug("> computeBasicScore(): suit="+suit+" val="+value+" insult="+insult);
		int score = 0;
		score = computeTrickScore(suit,value);
		if (insult == Bid.DOUBLE) score = score * getProperty("score.trick.double.multiplier");
		if (insult == Bid.REDOUBLE) score = score * getProperty("score.trick.redouble.multiplier");
		cat.debug("< computeBasicScore(): "+score);
		return score;
    }
    
    /**
     * To be called only if the contract has been met 
     * @param suit
     * @param value
     * @throws MissingResourceException
     */
    private int computeTrickScore(int suit, int value)  {
	cat.debug("> computeTrickScore(): suit="+suit+" value="+value);
	int score = 0;
	// compute the basic score.
	switch(suit){
	case Card.CLUBS:
	case Card.DIAMOND:
	    score = value*getProperty("score.trick.minor");
	    break;
	case Card.HEART:
	case Card.SPADES:
	    score = value*getProperty("score.trick.major");
	    break;
	case Bid.NOTRUMP:
	    score = getProperty("score.trick.notrump.first") + (value-1)*getProperty("score.trick.notrump.next");
	    break;
	default:
	    cat.error("Invalid suit value !");
	    //System.exit(0);
	}
	cat.debug("< computeTrickScore(): "+score);
	return score;
    }

    /**
     * To be called only if the contract has been met. This computes the various bonus if you have
     * achieved a grand slam etc.
     * Bonus scores are usually only scored if they have been BOTH ASKED and MET.
     * @throws MissingResourceException
     * @return score to add to other scores such as basic trick score.
     */
    private int computeBonusScore()  {
	cat.debug("> computeBonusScore()");
	int score = 0;

	if (!isGame()) score = getProperty("score.bonus.nongame");
	else {
	    if (request.getValue() == 6){ // SMALL SLAM
		if (vulnerable) score = getProperty("score.bonus.smallslam.vulnerable");
		else score = getProperty("score.bonus.smallslam");
	    } else {
		if (request.getValue() == 7) { // GRAND SLAM
		    if (vulnerable) score = getProperty("score.bonus.grandslam.vulnerable");
		    else score = getProperty("score.bonus.grandslam");
		} else {
		    // value is < 6, but we've done a game.
		    if (vulnerable) score = getProperty("score.bonus.game.vulnerable");
		    else score = getProperty("score.bonus.game");
		}
	    }
	}
	if (done.getInsult()==Bid.DOUBLE) score = score + getProperty("score.bonus.double");
	if (done.getInsult()==Bid.REDOUBLE) score = score + getProperty("score.bonus.redouble");

	cat.debug("< computeBonusScore(): "+score);
	return score;
    }

    /**
     * Computes the additional score if you have done more than requested.
     * Call only if contract has been met.
     * @return 0 if no additional score, or the additional score
     * @throws MissingResourceException 
     */
    private int computeOvertrickScore()  {
	cat.debug("> computeOvertrickScore()");
	int score = 0;
	int overtrick = done.getValue()-request.getValue();
	if (overtrick <= 0) return 0;

	if (done.getInsult()==Bid.IGNORED) score = computeTrickScore(done.getSuit(),overtrick);
	if (done.getInsult()==Bid.DOUBLE) {
	    if (isGame()) score = getProperty("score.overtrick.game.double");
	    else score = getProperty("score.overtrick.nongame.double");
	}
	if (done.getInsult()==Bid.REDOUBLE) {
	    if (isGame()) score = getProperty("score.overtrick.game.redouble");
	    else score = getProperty("score.overtrick.nongame.redouble");
	}
	
	cat.debug("< computeOvertrickScore(): "+score);
	return score;
    }

    /**
     * Call this only if the contract has not been met. This is penalty score. Score will be
     * marked by the opponents.
     * @return positive integer. But score should be marked by the opponent.
     */
    private int computeUndertrickScore() {
	cat.debug("> computeUndertrickScore()");
	int score = 0;
	int undertrick = request.getValue()-done.getValue();
	if (undertrick <=0) return 0;
	
	if (done.getInsult()==Bid.IGNORED){
	    if (vulnerable) score = undertrick * getProperty("score.undertrick.vulnerable");
	    else score = undertrick * getProperty("score.undertrick");
	} else {
	    if (vulnerable) {
		score = getProperty("score.undertrick.double.vulnerable.first");
		if (undertrick >1) score += (undertrick-1)* getProperty("score.undertrick.double.vulnerable.next");
	    } else {
		score = getProperty("score.undertrick.double.first");
		if (undertrick >1) score += getProperty("score.undertrick.double.second");
		if (undertrick >2) score += (undertrick-2) * getProperty("score.undertrick.double.next");
	    }
	    if (done.getInsult()==Bid.REDOUBLE) score *= getProperty("score.undertrick.redouble.multiplier");
	}
	cat.debug("< computeUndertrickScore(): "+score);
	return score;
    }

    /**
     * Says if the contract has been met or not.
     * @return true if met. False otherwise.
     */
    private boolean isDone() {
	cat.debug("isDone()");
	if (done.getValue() >= request.getValue()) return true;
	return false;
    }

    /**
     * A game is 3 NT, or 4 spades, or 4 hearts or 5 in the minors.
     * @return true is this is a game (MANCHE), false otherwise.
     */
    private boolean isGame() throws NullPointerException {
	cat.debug("isGame()");
	if (! isDone()) return false;
	if ((request.getValue()>=3) && (request.getSuit()==Bid.NOTRUMP)) return true;
	if ((request.getValue()>=4) && (Card.isMajor(request.getSuit()))) return true;
	if ((request.getValue()>=5) && (Card.isMinor(request.getSuit()))) return true;
	return false;
    }

    /**
     * Helpful method to retrieve the value of the tag as an integer.
     * @throws MissingResourceException if we cannot find the requested tag.
     */
    private int getProperty(String tag)  {
	cat.debug("> getProperty(): tag="+tag);
	// replace with dictionary String s = prop.getProperty(tag);
	String s = "100";
	if (s == null) {
	    cat.error("Tag "+tag+" is mising in the bridge property file.");
	    //throw new MissingResourceException("Tag "+tag+" is mising in the bridge property file.","",tag);
	}
	cat.debug("< getProperty(): "+s);
	return Integer.valueOf(s).intValue();

    }

    public String toString(){
	String s = "";
	if (request != null) s = s+ " requested bid= "+request.toString();
	if (done != null) s = s+ " bid done= "+done.toString();
	s = s+ vulnerable;
	return s;
    }
}
