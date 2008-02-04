package cn.client;

//import java.util.Locale;
//import java.util.ResourceBundle;
//import java.util.MissingResourceException;


/**
 * Used to retrieve internationalized information, so that freebridge can be translated to (nearly) any language ;-)
 * Put a MessageBundle_<language>_<Country>.properties file somewhere in your class path so that freebridge can
 * go and fetch the various strings.
 * @author Axelle Apvrille
 * @version 0.1
 */
public class International {
    //private Locale locale;
    //private ResourceBundle messages;
    static Category cat = Category.getInstance("");
    
    public International(String language,String country) 
		 {
		cat.debug("> International: language="+language+" country="+country);
		//locale = new Locale(language, country);
		//messages = ResourceBundle.getBundle("MessageBundle",locale);
		cat.debug("< International");
    }

    /**
     * Builds a default object using French.
     */
    public International(){
    	this("en","EN");
    }

    public String getLanguage() { return "en"; }
    public String getCountry() { return "EN"; }

    /**
     * @param tag the string you want to retrieve, more exactly its tag.
     * @return the internationalized string.
     * @throws MissingResourceException if the given tag does not exist
     * @throws ClassCastException
     * @throws NullPointerException
     */
    public String getString(String tag) 
	throws NullPointerException, 
	       ClassCastException{
    	cat.debug(" getString(): "+tag);
    	return tag;
    	//return messages.getString(tag);
    }
}
