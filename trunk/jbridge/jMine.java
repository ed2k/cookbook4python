import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.io.*;
import java.net.*; 
import java.text.DateFormat;
import java.lang.*;


//Panel to hold buttons leds and mine field
public class jMine extends Panel {
   MineCanvas mineCanvas=new MineCanvas();
   Panel palette = new Panel(new FlowLayout());

   jMine() {
      setLayout(new BorderLayout());
      // Layout components
      add(mineCanvas, BorderLayout.CENTER);
      //        setSize(500, 300);
   }

}//end of class 

class MineCanvas extends Canvas{
   int width, height;
   int status = 0;
//    Image imgButton;
    
   int xnum=10, ynum=10;
   int xGrid=16, yGrid=16;

   //represent table layout
   String[][] tdata;
   int[][] table_layout = {{150,30},{300,100},{150,200},{0,100}};
   // bid layout
   int[] bidinput_layout = {450,150};
   String [][] bidinput_data;
   int[] bidhistory_layout = {450,50};
   int hand_id = 0;
   int myseat = 0;
   String bidhistory = "";
   // trick layout
   int[] trick_layout = {30,300};
   String trick_data = "";
   static String[] suit2str = {"S","H","D","C","N"};
   static String[] seat2str = {"N","E","S","W"};
   MineCanvas() {
      tdata = new String[4][4];
      for (int i = 0;i<4;i++)for (int j = 0;j<4;j++){
               tdata[i][j] = "";
      }
      bidinput_data = new String[8][5];
      for (int i = 0;i<7;i++){
         for (int j = 0;j<5;j++){
            bidinput_data[i][j]=String.valueOf(i+1)+suit2str[j];
         }
      }
      bidinput_data[7][0] = "";
      bidinput_data[7][1] = "p";
      bidinput_data[7][2] = "x";
      bidinput_data[7][3] = "xx";      
      bidinput_data[7][4] = ""; 
      xnum=30;ynum=16;
//       MediaTracker tracker = new MediaTracker(this);
//       imgButton =getToolkit().getImage("mButton.gif");
//       try {
//          tracker.addImage(imgButton, 0);
//          tracker.waitForAll();
//       } catch (Exception e) {
//          e.printStackTrace();
//       }
      width = xGrid*xnum;
      height = xGrid*ynum + 32; // + panel height
      addMouseListener(new MouseEventHandler());
      //addMouseMotionListener(new MouseMotionEventHandler());
      Timer t = new Timer();
      t.schedule(new MessageClient(this),1000);
      String[] s = {"N"};
      FloaterMessage m = new FloaterMessage("request_seat",s);
      MessageClient.send(m.toString());
   }
   public void sendBid(String bid){
      String[] s= {String.valueOf(hand_id),bidhistory+bid};
      MessageClient.send(new FloaterMessage("auction_status",s).toString());
   }
   public void sendPlay(String p){
      String[] s= {String.valueOf(hand_id),trick_data+p};
      MessageClient.send(new FloaterMessage("play",s).toString());
   }  
   public void paint(Graphics g) {
      update(g);
   }
   public void drawBidHistory(Graphics g){
      for(int i=0;i<4;i++){
         int x = bidhistory_layout[0]+i*xGrid;
         int y = bidhistory_layout[1];
         g.drawString(seat2str[i],x,y);
      }
      int y = bidhistory_layout[1]+yGrid;
      int pos = (hand_id-1) % 4;
      for(int i=0;i<(bidhistory.length()/2);i++){
         if (pos > 3) {
            y += yGrid;
            pos = 0;
         }
         int x = bidhistory_layout[0]+pos*xGrid;
         g.drawString(bidhistory.substring(2*i,2*i+2),x,y);
         pos++;
      }
   }
   public void drawTrick(Graphics g){
      int x = trick_layout[0];
      int y = trick_layout[1];
      int pos = 0;
      //g.drawString(seat2str[i],x,y);
      for(int i=0;i<(trick_data.length()/2);i++){
         if (pos > 3) {
            y += yGrid;
            x = trick_layout[0];
            pos = 0;
         } 
         x += xGrid;
         g.drawString(trick_data.substring(2*i,2*i+2),x,y); 
         pos++;
      }
   }   
   public void drawBidInput(Graphics g){
      for(int i=0;i<8;i++){
         int y = bidinput_layout[1]+i*yGrid;
         for(int j=0;j<5;j++){
            int x = bidinput_layout[0]+j*xGrid;
            g.drawString(bidinput_data[i][j],x,y);
         }
      }
   }
   public void drawTable(Graphics g){
      for(int i=0;i<4;i++)for(int j=0;j<4;j++)
         for(int k=0;k<tdata[i][j].length();k++){
            //int sw=xGrid,sh=yGrid, sx=tdata[i][j][k]*sw;
            int dx=table_layout[i][0]+k*xGrid;
            int dy=table_layout[i][1]+yGrid+j*yGrid;
            //g.drawImage(imgButton,dx,dy,dx+sw,dy+sh,sx,0,sx+sw,sh,this);
            String c = tdata[i][j].substring(k,k+1);
            g.drawString(c,dx,dy);
         }

   }

   public void update(Graphics g) {
      // draw 4 hands on the table
      drawBidInput(g);
      drawTable(g);
      drawBidHistory(g);
      drawTrick(g);
   }
   
   public Dimension getPreferredSize() {
      return new Dimension(width, height);
   }

   class MouseEventHandler extends MouseAdapter {
      public void mouseClicked(MouseEvent evt){
         //System.out.print("button "+evt.getX()+","+evt.getY());
         if((evt.getModifiers() & InputEvent.BUTTON1_MASK)
            != InputEvent.BUTTON1_MASK) return;
         int x = evt.getX(); int y = evt.getY();
         
         if (MyUtil.inRect(x,y,table_layout[2],13*xGrid,4*yGrid)) {
            int pos = (x-table_layout[2][0])/xGrid; 
            int suit = (y-table_layout[2][1])/yGrid;            
            if (pos < tdata[2][suit].length()){
               String rank = tdata[2][suit].substring(pos,pos+1);
               String trick = suit2str[suit]+rank;
               System.out.println(trick);
               sendPlay(trick.toLowerCase());
               //repaint();
            }
         } else if (MyUtil.inRect(x,y,bidinput_layout,4*xGrid,8*yGrid)) {
            int suit = (x-bidinput_layout[0])/xGrid;
            int rank = ((y-bidinput_layout[1]+yGrid)/yGrid)+1;     
            String bid = " p";
            if (rank < 8){ 
               bid = String.valueOf(rank)+suit2str[suit];
            } else if (suit == 2) bid = " x";
            else if (suit == 3) bid = "xx";
            System.out.println(bid);
            sendBid(bid.toLowerCase());
         } else if (MyUtil.inRect(x,y,table_layout[0],13*xGrid,4*yGrid)){
            System.out.println("in 0");
            int pos = (x-table_layout[0][0])/xGrid; 
            int suit = (y-table_layout[0][1])/yGrid;            
            if (pos < tdata[0][suit].length()){
               String rank = tdata[0][suit].substring(pos,pos+1);
               String trick = suit2str[suit]+rank;
               sendPlay(trick.toLowerCase());    
            }       
         }
      }
   }
   public void handleData(FloaterMessage msg){
      // ??? not working with substring(0,1) == "*"
      char msgname = msg.name.charAt(0);
      if(msgname == '*'){
         hand_id = Integer.parseInt(msg.args[0]);
         String hands = msg.args[2];
         bidhistory = msg.args[3];
         trick_data = msg.args[4];
         System.out.println(hands);
         String[] hs = hands.split("\\|");
         System.out.println(hs.length);
         for(int i=0;i<hs.length;i++){
            //System.out.println(hs[i]);
            int seat = Integer.parseInt(hs[i].substring(0,1));
            if (i == 0) myseat = seat;
            int fix = (2+seat-myseat) %4;
            String[] suits = hs[i].substring(2).split("\\.");
            for(int j=0;j<suits.length;j++){
               StringBuffer sb = new StringBuffer();
               for(int k=0;k<suits[j].length();k++){
                  sb.append(suits[j].charAt(k));
               }
               tdata[fix][j] = sb.toString();
            }
         }
         repaint();
      } else if (msgname == 'a'){
         bidhistory = msg.args[1];
         repaint();
      } else if (msgname == 'p'){
         trick_data = msg.args[1];
         repaint();
      }
   }
}//end of mineCanvas

class MessageClient extends TimerTask {
   Vector inbox;
   static String website = "http://127.0.0.1:8080/postit.yaws?flproxyB=";
   MineCanvas owner;
   MessageClient(MineCanvas caller){
      owner = caller;
   }
   public static void send(String m){
      try{
         System.out.println(m);
         String u = website+URLEncoder.encode(m+"\r\n");
         URL url = new URL(u);
         InputStream in = url.openStream();
         // Create a buffered input stream for efficency
         BufferedInputStream bufIn = new BufferedInputStream(in);
         // Repeat until end of file
         String line = "";
         for (;;) {
            int data = bufIn.read();
            // Check for EOF
            if (data == -1)  break;
            else {
               System.out.print((char)data);
            }
         }
      }catch (MalformedURLException mue) {
         System.err.println ("Invalid URL");
      } catch (IOException ioe) {
         System.err.println ("I/O Error - " + ioe);
      }
   }

   public void run(){
      try {
         URL url = new URL(website);
         InputStream in = url.openStream();
         // Create a buffered input stream for efficency
         BufferedInputStream bufIn = new BufferedInputStream(in);
         // Repeat until end of file
         String line = "";
         for (;;) {
            int data = bufIn.read();
            // Check for EOF
            if (data == -1)  break;
            else {
               if (data == 13){
                  //System.out.print("<xd>");
                  if(line.startsWith("nothing")) break; 
                  if(line == "") continue;
                  if(line.startsWith("T4")){
                     line = "";
                     continue;
                  }
                  System.out.println (line);
                  //inbox.add(new FloaterMessage(line));
                  FloaterMessage m = new FloaterMessage(line);
                  line = "";
                  System.out.print(m.name+":");
                  if(m.args != null)
                     for(int i=0;i<m.args.length;i++)
                        System.out.print(m.args[i]+"|");
                  System.out.println("");
                  owner.handleData(m);
               } else if (data != 10){
                  //System.out.print((char)data);
                  //System.out.print("_");
                  line += (char)data;
               }
            }
         }
      }catch (MalformedURLException mue) {
         System.err.println ("Invalid URL");
      } catch (IOException ioe) {
         System.err.println ("I/O Error - " + ioe);
      }

      Timer t = new Timer();
      t.schedule(new MessageClient(owner), 1000);
   }
}

class FloaterMessage {
   String name = "";
   String mfrom = "mfrom";
   String mid ="33";
   String[] args;
   static String[] seat2str = {"N","E","S","W"};
   FloaterMessage(String cmd, String[] seqs){
      if(cmd == "request_seat"){
         name = "S";
         args = new String[4];
         args[0] = mfrom;
         args[1] = seqs[0];
         args[2] = "hostname";
         args[3] = "port";
      } else if (cmd == "auction_status"){
         name = "a";
         args = seqs;
      } else if (cmd == "play"){
         name = "p";
         args = seqs;
      }
   }
   FloaterMessage(String line){
      name = "";
      if(line.startsWith("Floater '")){ 
         name = "Floater '";
         return;
      } 
      name = line.substring(0,1);
      String fsep = "\\\\";
      String[] f = line.split(fsep);
      int num_of_args = Integer.parseInt(f[0].substring(1));
      int len_pfrom = Integer.parseInt(f[1]);
      int len_pid = Integer.parseInt(f[2]);
      String reline = MyUtil.join(f,3,"\\");
      mfrom = reline.substring(0,len_pfrom);
      String tmp = reline.substring(len_pfrom);
      mid = tmp.substring(0,len_pid);

      if (num_of_args == 0) return;
      tmp = tmp.substring(len_pid);
      f = tmp.split(fsep);
      
      int[] len_args = new int[num_of_args];
      for(int i=0;i<num_of_args;i++)len_args[i] = Integer.parseInt(f[i]);
      
      args = new String[num_of_args];
      tmp = MyUtil.join(f, num_of_args,"\\");
      for(int i=0;i<num_of_args;i++){
         args[i] = tmp.substring(0,len_args[i]);
         tmp = tmp.substring(len_args[i]);
      }
   }
   public String encode_args(String[] seqs){
      String[] len_seqs = new String[seqs.length];
      for(int i=0;i<seqs.length;i++){
         len_seqs[i] = String.valueOf(seqs[i].length());
      }
      return MyUtil.join(len_seqs,"\\")+"\\"+MyUtil.join(seqs,"");
   }
   public String packmsg(String op, String[] args){
      String[] s = {mfrom,mid};
      return op+String.valueOf(args.length)+"\\"+encode_args(s)+encode_args(args);
   }
   public String toString(){
      return packmsg(name, args);
   }
}
class MyUtil {
   public static String join(String[] strings,int start, int end, String separator) {
      StringBuffer buf = new StringBuffer();
      for (int i=start;i<end-1;i++){
         buf.append(strings[i]);
         buf.append(separator);
      }
      buf.append(strings[end-1]);
      return buf.toString();
   }
   public static String join(String[] ss,int start,String sep){
      return join(ss,start, ss.length,sep);
   }
   public static String join(String[] ss,String sep){
      return join(ss,0, ss.length,sep);
   }   
   public static boolean inRect(int x, int y, int[] p0,int dx,int dy){
      int x0 = p0[0];
      int y0 = p0[1];
      return (x>x0) && (x< (x0+dx)) && (y>y0) && (y< (y0+dy));
   }
}
