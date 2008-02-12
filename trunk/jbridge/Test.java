import java.lang.*;
import java.util.*;

class Test{
   public static void main(String[] argv){
      String a = "*\r\nb";
      String[] b = a.split("\\r\\n");
      System.out.println(b.length);
      System.out.println(a.substring(0,1).length());
   }
}