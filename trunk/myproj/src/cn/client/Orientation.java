package cn.client;

import cn.client.Category;

public class Orientation {
	static Category cat = Category.getInstance("");
	public static final int NORTH = 0;
	public static final int EAST = 1;
	public static final int SOUTH = 2;
	public static final int WEST = 3;

	private int value = NORTH;

	public Orientation( int value) {
		cat.debug("> Orientation(): value = "+value);
		this.value = value %4;
		cat.debug("< Orientation()");
	}

	public int getOrientation() { return value; }
	public boolean isPartner( int v ) throws BadTypeException {
		cat.debug("isPartner(): is "+v+" partner of "+value+" ?");
		if ((v < NORTH) || (v > WEST)) 
			throw new BadTypeException("Incorrect orientation value: "+v);
		if ( ((value == NORTH) || (value == SOUTH)) && 
				((v == NORTH) || (v == SOUTH)))
			return true;
		if ( ((value == EAST) || (value == WEST)) &&
				((v == EAST) || (v == WEST)))
			return true;
		return false;
	}

	/**
	 * North and south are partners, East and West are partners.
	 * @param o an orientation object
	 * @return true if the orientation represented by o is a partner with this object's orientation.
	 */
	public boolean isPartner(Orientation o) {
		cat.debug("> isPartner(): o="+o.toString());
		boolean partner = false;
		try {
			partner = isPartner(o.getOrientation());
		}
		catch(BadTypeException exp){
			cat.fatal("This should not happen: this orientation has an incorrect value: "+exp);
			//System.exit(-1);
		}
		cat.debug("< isPartner(): "+partner);
		return partner;
	}

	public Orientation getPartner() {
		cat.debug("> getPartner(): "+value);
		Orientation partner = null;
		if (value==NORTH) partner =new Orientation(Orientation.SOUTH);
		if (value==SOUTH) partner = new Orientation(Orientation.NORTH);
		if (value==WEST) partner = new Orientation(Orientation.EAST);
		if (value==EAST) partner = new Orientation(Orientation.WEST);
		cat.debug("< getPartner(): "+partner.toString());
		return partner;
	}

	public static int next(int value) throws BadTypeException {
		cat.debug("next(): value= "+value);
		if ((value < NORTH) || (value > WEST)) throw new BadTypeException("Incorrect orientation value: "+value);
		if (value == WEST) return NORTH;
		return (value+1);
	}

	/**
	 * @return the next orientation value. Does not modify the current object.
	 */
	public Orientation next(){
		cat.debug("> next(): val="+value);
		Orientation o = null;
		try {
			int next_val = Orientation.next(value); 
			o = new Orientation(next_val);
		}
		catch(BadTypeException exp){
			cat.fatal("Should never occur: "+exp);
			//System.exit(-1);
		}
		cat.debug("< next(): "+o.getOrientation());
		return o;
	} 

	public static String show(International intl,int value) {
		cat.debug("> show()");
		String s = "";
		switch(value){
		case NORTH: s = intl.getString("NORTH"); break;
		case SOUTH: s = intl.getString("SOUTH"); break;
		case EAST: s = intl.getString("EAST"); break;
		case WEST: s = intl.getString("WEST"); break;
		default:
			cat.error("Unexpected value for orientation: "+value);
		throw new AssertionError();
		}
		cat.debug("< show(): "+s);
		return s;
	}

	public String show(International intl)  {
		return show(intl,this.value);
	}

	public String toString(){
		return Integer.toString(value);
	}
	public Orientation team(){
		return new Orientation(value %2);
	}
}
