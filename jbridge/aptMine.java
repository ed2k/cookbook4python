import java.awt.*;
import java.awt.event.*;
import java.applet.*;
import java.net.*;

public class aptMine extends Applet {
   Button a, b;
   int level=0;
   jFloaterPanel  assoc;


  boolean isStandalone = false;

  //Initialize the applet
  public void init() {
      setLayout(new BorderLayout());

      assoc = new jFloaterPanel();
      add("Center", assoc);
  }
   public void destroy() {
      remove(assoc);
      //  remove(p);        remove(controls);
   }

   public static void main(String args[]) {
      Frame f = new Frame("Mine");
      aptMine a = new aptMine();
      a.init();
      a.start();

      f.add("Center", a);
      f.setSize(600, 500);
      f.show();
   }

  //Get Applet information
  public String getAppletInfo() {
    return "Applet Information";
  }

  //Get parameter info
  public String[][] getParameterInfo() {
    return null;
  }


  
 


}

