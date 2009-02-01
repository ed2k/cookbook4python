import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.io.*;
import java.net.*; 
import java.text.DateFormat;
import java.lang.*;
import cn.client.*;


//Panel to hold buttons leds and mine field
public class jFloaterPanel extends Panel {
   MineCanvas mineCanvas=new MineCanvas();
   Panel palette = new Panel(new FlowLayout());

   jFloaterPanel() {
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
   Deal deal;
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
   MessageClient mc;
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
      //Timer t = new Timer();
      //t.schedule(new MessageClient(this),1000);
      mc = new MessageClient(this);
      String[] s = {"N"};
      FloaterMessage m = new FloaterMessage("request_seat",s);
      mc.send(m.toString());
   }
   public void sendBid(String bid){
      String[] s= {String.valueOf(hand_id),bidhistory+bid};
      mc.send(new FloaterMessage("auction_status",s).toString());
   }
   public void sendPlay(String p){
      String[] s= {String.valueOf(hand_id),trick_data+p};
      mc.send(new FloaterMessage("play",s).toString());
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
   int adjustSeat(int seat){return (2+seat-myseat)%4;}

   public void drawTrick(Graphics g){

      int dealer = hand_id % 4;
      //Deal deal = new Deal(new Orientation(dealer));
      for(int i=0;i<bidhistory.length()/2;i++){
         Bid bid = new Bid(bidhistory.substring(2*i,2*i+2));
         deal.bid(bid);
      }
      int dummyseat = adjustSeat(deal.dummy.idx());
      //deal.hands[myseat] = new Hand(tdata[2]);
      //deal.hands[deal.dummy.idx()] = new Hand(tdata[dummyseat]);
      String seat = "NESW";
      for(int i=0;i<4;i++)
         g.drawString(seat.substring(i,i+1),trick_layout[0]+i*xGrid,trick_layout[1]-yGrid); 
      int x = trick_layout[0]+deal.player.idx()*xGrid;
      int y = trick_layout[1];
      for(int i=0;i<trick_data.length()/2;i++){
         String card = trick_data.substring(2*i,2*i+2);
         g.drawString(card,x,y); 
         deal.play_card(new Card(card));
         if(deal.currenTrickCompleted()){
            deal.next_trick();
            y+= yGrid;
         }
         x = trick_layout[0]+deal.player.idx()*xGrid;
      }
      //g.drawString("?",x,y);
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
      for(int i=0;i<4;i++){
         if (deal.hands[i] == null) continue;
         for(int j=0;j<4;j++){
            Vector remain = deal.hands[i].selectColour(3-j);
            if (remain.isEmpty()) continue;
            Iterator k = remain.iterator();
            //display += Card.showCardColour(i) + " ";
            int dx=table_layout[adjustSeat(i)][0]+remain.size()*xGrid;
            int dy=table_layout[adjustSeat(i)][1]+yGrid+j*yGrid;
            while (k.hasNext()){
               Card c = (Card) k.next();
               g.drawString(c.rank(),dx,dy);
               dx-= xGrid;
            }
            //g.drawImage(imgButton,dx,dy,dx+sw,dy+sh,sx,0,sx+sw,sh,this);
         }
      }
   }

   public void update(Graphics g) {
      // draw 4 hands on the table
      drawBidInput(g);
      drawBidHistory(g);
      if (trick_data.length()>0)drawTrick(g);
      drawTable(g);
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
         deal = new Deal(new Orientation((hand_id-1) % 4));
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
            deal.hands[seat] = new Hand(tdata[fix]);
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

class HTTPRequest {
   public static String get(String website){
      StringBuffer buf = new StringBuffer();
      try {
         URL url = new URL(website);
         InputStream in = url.openStream();
         // Create a buffered input stream for efficency
         BufferedInputStream bufIn = new BufferedInputStream(in);
         // Repeat until end of file

         for (;;) {
            int data = bufIn.read();
            // Check for EOF
            if (data == -1)  break;
            else {
               buf.append((char)data);
            }
         }
      }catch (Exception e) {
         System.err.println ("Error - " + e);
      }
      return buf.toString();
   }
}
class MessageClient {
   Vector inbox;
   static String website = "http://127.0.0.1:10101/postit.yaws?flproxyB=";
   MineCanvas owner;
   MessageClient(MineCanvas caller){
      owner = caller;
   }
   public void send(String tosend){
      String u = "";
      try {
         u = website+URLEncoder.encode(tosend+"\r\n","utf-8");
      } catch (Exception e){e.printStackTrace();}
      System.out.println(">"+u);
      String responseText = HTTPRequest.get(u);

      String[] lines = responseText.split("\\r\\n");
      //p.debug(responseText);
      for (int k=0;k<lines.length;k++) {
         String line = lines[k];	
         if(line.startsWith("nothing")) break; 
         if(line == "") continue;	
         if(line.startsWith("T4"))	continue;
         //p.debug(line);	
         System.out.println (line);

         FloaterMessage m = new FloaterMessage(line);
         System.out.print(m.name+":");
         if(m.args != null)
            for(int i=0;i<m.args.length;i++)
               System.out.print(m.args[i]+"|");
         System.out.println("");
         owner.handleData(m);
      }

   }

}



