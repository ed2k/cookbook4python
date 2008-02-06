import java.lang.*;
import java.util.*;

class Test{
   public static void main(String[] argv){
      String a = "*";
      String[] b = a.split("\\.");
      if(a=="*") 
         System.out.println(b.length);
      System.out.println(a.substring(0,1).length());
   }
}