package cn.client;

import java.util.Vector;

import com.google.gwt.core.client.EntryPoint;
import com.google.gwt.user.client.ui.*;
import com.google.gwt.user.client.HTTPRequest;
import com.google.gwt.http.client.URL;
import com.google.gwt.user.client.ResponseTextHandler;

//import freebridge.core.*;

/**
 * Entry point classes define <code>onModuleLoad()</code>.
 */
public class bridge implements EntryPoint {

	/**
	 * This is the entry point method.
	 */
	public void onModuleLoad() {
		final Panels panel = new Panels();
		Button button = new Button("start");
		final TextBox tb = new TextBox();
		tb.setVisibleLength(100);
		tb.setText("/oldlady/bridge-cgi.py?flproxyB=");
		button.addClickListener(new ClickListener() {
			public void onClick(Widget sender) {
				panel.website = tb.getText();
				panel.new_start();
			}
		});
		Button bClear = new Button("clear");
		bClear.addClickListener(new ClickListener() {
			public void onClick(Widget sender) {
				panel.debug=new HTML("");
			}
		});		

		RootPanel.get().add(button);
		RootPanel.get().add(bClear);
		RootPanel.get().add(tb);
		RootPanel.get().add(panel);
	}
}
/**
 * All UI stuff
 */
class Panels extends Composite{
	DockPanel dock ;
	Deal deal;
	TabPanel tabs;
	Grid playGrid,bidGrid, cardCounter,ntrickGrid;
	Grid[] bidSuit; 
	int playHistoryRow;
	HTML debug= new HTML("");
	HorizontalSplitPanel hSplit;
	VerticalPanel[] dockHand;
	Grid[][] hpSuit;
	//------------ integrate with java applete app.
	int hand_id = 0;
	int myseat = 0;
	String bidhistory = "";
	// trick layout
	String trick_data = "";
	String [][] tdata;
	//MessageClient message_client;
	String website = null;
	public void test(){
		show_4hands();
	}
	public void debug(String s){
		debug = new HTML(s+"<br>"+ debug.getHTML());
		hSplit.setRightWidget(debug);
	}
	public String html_center(String s){
		return "<center>"+s+"</center>";
	}
	public Panels() {	
		tdata = new String[4][4];
		ntrickGrid  = new Grid(1,5); 
		ntrickGrid.setWidget(0, 1, new HTML("NS"));
		ntrickGrid.setWidget(0, 3, new HTML("EW"));
		
		bidGrid = new Grid(2,4);
		playGrid = new Grid(13,4); playHistoryRow = 0;
		playGrid.setWidth("95px");
		ScrollPanel scroller = new ScrollPanel(playGrid);
		scroller.setStyleName("ks-layouts-Scroller");
		scroller.setPixelSize(125,125);
		
		DockPanel dock = new DockPanel();
		dockHand = new VerticalPanel[4];
		bidSuit = new Grid[4];		
		hpSuit = new Grid[4][4];
		for (int i =0;i<4;i++){
			dockHand[i] = new VerticalPanel();
			bidSuit[i] = new Grid(1,14);
			if (i==1){
				dockHand[i].add(new HTML("> ")); //cosmetic
				dockHand[i].add(new HTML("West"));
			} else if (i==3) {
				dockHand[i].add(ntrickGrid);
				dockHand[i].add(new HTML("East"));
			}
			for(int j=0;j<4;j++){
				hpSuit[i][j] = new Grid(1,14);
				hpSuit[i][j].setCellSpacing(0);
				hpSuit[i][j].setBorderWidth(0);
				dockHand[i].add(hpSuit[i][j]);
			}
		}
		String seat ="NESW";
		dock.add(dockHand[0], DockPanel.NORTH);
		dock.add(dockHand[2], DockPanel.SOUTH);
		dock.add(dockHand[1], DockPanel.EAST);    
		dock.add(dockHand[3], DockPanel.WEST);
		VerticalPanel cvp = new VerticalPanel();
		Grid g = new Grid(1,4);
		g.setWidth("95px");
		for(int i=0;i<4;i++){
			g.setText(0, i, seat.substring(i,i+1));
			bidGrid.setText(0,i,seat.substring(i,i+1));
		}
		cvp.add(g);
		cvp.add(scroller);
		dock.add(cvp, DockPanel.CENTER);
		dock.setHorizontalAlignment(DockPanel.ALIGN_CENTER);
		dock.setCellHorizontalAlignment(dockHand[0], DockPanel.ALIGN_CENTER);
		dock.setCellHorizontalAlignment(dockHand[2], DockPanel.ALIGN_CENTER);
		
		cardCounter = new Grid(4,14);
		for(int j=0;j<4;j++)cardCounter.setText(j,0, Card.SUIT2STR[3-j]);
		for (int i=1;i<14;i++)for(int j=0;j<4;j++) {
			cardCounter.setWidget(j,i, new Button(Card.rank2str(hpsuitRankIDX(i))));
		}

		VerticalPanel vp = new VerticalPanel();
		vp.add(dock);
		vp.add(cardCounter);
		
		tabs = new TabPanel();
		//Biding input layout is in a seperate funciton
		showBidInputPanel(); 
		tabs.add(vp, "Table");
		//tabs.add(grid, "play");
		tabs.setWidth("100%");
		tabs.selectTab(1);

		hSplit = new HorizontalSplitPanel();
		hSplit.setLeftWidget(tabs);
		initWidget(hSplit);
		hSplit.setSize("100%", "650px");
//	    Timer t = new Timer() {
//	        public void run() {
//	    		MessageClient.send("",owner);	
//	        }
//	      };
//	    t.scheduleRepeating(1000);
	
	}
    void new_start(){
		String[] s = {"N"};
		FloaterMessage m = new FloaterMessage("request_seat",s);
		MessageClient.send(m.toString(),this);	    	
    }
	void showBidInputPanel(){
		VerticalPanel vp = new VerticalPanel();
		ClickListener bidclick = new ClickListener() {
			public void onClick(Widget sender) {
				Button b = (Button)sender;
				debug("-"+b.getText()+"-");
				sendBid(b.getText());
			}
		};
		String [] tmp = {"Pass","Double","Redouble"};
		Grid grid1 = new Grid(1,3);
		for (int i=0;i<3;i++){			
			Button b = new Button(tmp[i]);	
			grid1.setWidget(0, i, b);
			b.addClickListener(bidclick);			
		}

		Grid grid2 = new Grid(7, 5);
		for (int r = 0; r < 7; ++r) {
			for (int c = 0; c < 5; ++c) {
				String s = String.valueOf(r+1)+Card.SUIT2STR[c];
				Button b = new Button(s);
				grid2.setWidget(r, c, b);
				b.addClickListener(bidclick);
			}
		}
		for (int i=0;i<4;i++)vp.add(bidSuit[i]);
		vp.add(bidGrid);
		vp.add(grid1);
		vp.add(grid2);
		tabs.add(vp,"Bids");
	}

	public void show_4hands(){
		ClickListener cardClick = new ClickListener() {
			public void onClick(Widget sender) {
				// check if in the right suit
				CardButton b = (CardButton)sender;
				//check if it is my turn
				if (b.seat != adjustSeat(deal.player.idx())){
					debug("not your turn");
					return;
				}
				Card c = deal.trick.lead; 
				if (c != null){
					Vector remain = deal.hands[deal.player.idx()].selectColour(c.getColour());
					if(!remain.isEmpty() && (b.suit != c.getColour())){
						debug("not the right suit");
						return ;
					}
				}
				String card = Card.SUIT2STR[b.suit]+b.getText();
				debug("-"+card+"-");
				sendPlay(card.toLowerCase());
				//showTrickPlayed(card);
			}
		};		
		for (int i=0;i<4;i++)for(int j=0;j<4;j++)
			for(int k=1;k<14;k++)hpSuit[i][j].clearCell(0, k);
		for (int i=0;i<4;i++)for(int j=0;j<4;j++){
			String suit = tdata[i][j];
			hpSuit[i][j].setWidget(0,0,new HTML(Card.SUIT2STR[3-j]));
			if(suit == "")continue;
			for (int k=0;k<suit.length();k++){
				String card = suit.substring(k,k+1);
				Button b = new CardButton(i,3-j,card,cardClick);
				hpSuit[i][j].setWidget(0,hpsuitRankIDX(Card.ridx(card)),b);
				//hpSuit[i].setWidget(j,k+1,b);
			}
		}
		for(int j=0;j<4;j++)
			for(int k=1;k<14;k++)bidSuit[j].clearCell(0, k);
		for(int j=0;j<4;j++){
			String suit = tdata[2][j];
			bidSuit[j].setWidget(0,0,new HTML(Card.SUIT2STR[3-j]));
			for (int k=0;k<suit.length();k++){
				String card = suit.substring(k,k+1);
				bidSuit[j].setWidget(0,hpsuitRankIDX(Card.ridx(card)),new HTML(card));
			}
		}		
		//if (trick_data.length()>0)showTrickPlayed();
		if (deal.contract != null)
			ntrickGrid.setWidget(0, 0, new HTML(deal.contract.toString()));		
	}
	void trackBidHistory(){
	    int pos = (hand_id-1) % 4;
	    int row = 0;
	    bidGrid.resizeRows(1); //clear history
	    bidGrid.resizeRows(2);
	    for(int i=0;i<pos;i++){
	    	playGrid.setText(0, i, "--");
	    }
		for(int i=0;i<bidhistory.length()/2;i++){
			String b = bidhistory.substring(2*i,2*i+2);
			Bid bid = new Bid(b);
			deal.bid(bid);
			playGrid.setText(row, pos, b);
			bidGrid.setText(1+row, pos, b);
			pos++;
			if (pos > 3){
				pos = 0;
				row++;
				bidGrid.resizeRows(2+row);
			}		
			playGrid.setWidget(row, pos, new HTML("?"));			
		}		
	}

	/*
	 * return where to display a card
	 */
	int hpsuitRankIDX(int rank){ return 15-rank;}
	
	void showTrickPlayed(String card){
		int dummyseat = adjustSeat(deal.dummy.idx());
		int dummy = deal.dummy.idx();		
		int p = deal.player.idx();
		Card c = new Card(card);
		//remove card from table
		if (p==myseat)hpSuit[2][c.sidx()].clearCell(0,hpsuitRankIDX(c.ridx())); 
		else if(p==dummy)hpSuit[dummyseat][c.sidx()].clearCell(0,hpsuitRankIDX(c.ridx()));
		cardCounter.getWidget(c.sidx(), hpsuitRankIDX(c.ridx())).addStyleDependentName("played");
		Button b = new Button(c.rank());
		b.addStyleDependentName("played");
		hpSuit[adjustSeat(p)][c.sidx()].setWidget(0,hpsuitRankIDX(c.ridx()), b);
		Card lead = deal.trick.lead;
		if (lead != null){
			if (c.getColour() != lead.getColour()){
				hpSuit[adjustSeat(p)][lead.sidx()].setWidget(0,0, new HTML("-"));
			}
		}
		// record in the history
		deal.play_card(c);
		playGrid.setWidget(playHistoryRow, p, new HTML(card));
		if(deal.currenTrickCompleted()){
			deal.next_trick();
			playHistoryRow++;
			for (int i=0;i<2;i++){
				int t = deal.tricks_taken[i]; 	
				ntrickGrid.setWidget(0, i*2, new HTML(String.valueOf(t)));
			}
		}
		if (playHistoryRow < 13)
			playGrid.setWidget(playHistoryRow, deal.player.idx(), new HTML("?"));
	}

	int adjustSeat(int seat){return (2+seat-myseat)%4;}
	public void handleData(FloaterMessage msg){
		//TODO, update graphic once all date has been processed
		// ??? not working with substring(0,1) == "*"
		char msgname = msg.name.charAt(0);
		if(msgname == '*'){
			hand_id = Integer.parseInt(msg.args[0]);
			String hands = msg.args[2];
			bidhistory = msg.args[3];
			trick_data = msg.args[4];
			debug("handlData: "+hands);
			for (int i = 0;i<4;i++)for (int j = 0;j<4;j++)tdata[i][j] = "";			
			String[] hs = hands.split("\\|");
			for(int i=0;i<hs.length;i++){
				System.out.println(hs[i]);
				int seat = Integer.parseInt(hs[i].substring(0,1));
				if (i == 0) myseat = seat;
				int fix = adjustSeat(seat);
				String[] tmp = hs[i].substring(2).split("\\.");
				for(int j=0;j<tmp.length;j++)
					//if (tmp[j])
					tdata[fix][j]=tmp[j]; 
			}
			clearCardCounter();
			process();		
		} else if (msgname == 'a'){
			bidhistory = msg.args[1];
			debug(bidhistory);
			process();
		} else if (msgname == 'p'){
			trick_data = msg.args[1];
			debug(trick_data);
			process();
		}
	}	
	
	private void clearCardCounter() {
		// TODO Auto-generated method stub
		for(int i=0;i<4;i++)for(int j=1;j<14;j++)
			cardCounter.getWidget(i, j).removeStyleDependentName("played");
	}
	public void process() {
		// TODO Auto-generated method stub
		deal = new Deal(new Orientation((hand_id-1) % 4));
		for (int i=0;i<4;i++)for(int j=0;j<13;j++)playGrid.clearCell(j,i);
		
		trackBidHistory();
		show_4hands();

		if (deal.trick == null) return;
		playHistoryRow = 0;
		for (int i=0;i<4;i++)for(int j=0;j<13;j++)playGrid.clearCell(j,i);
		int dummyseat = adjustSeat(deal.dummy.idx());		
		deal.hands[myseat] = new Hand(tdata[2]);
		deal.hands[deal.dummy.idx()] = new Hand(tdata[dummyseat]);

		for(int i=0;i<trick_data.length()/2;i++){
			String card = trick_data.substring(2*i,2*i+2);
			showTrickPlayed(card);
		}				
		
	}
	public void sendBid(String b){
		//check if it is my turn first
		if (((hand_id-1+myseat+bidhistory.length()/2)%4) != 0) return;	
		//if (bidhistory.length()>=6 && bid)
		char c = b.charAt(0);
		String bid = b.toLowerCase();
		if (c=='P')bid = " p";
		else if (c=='R')bid = "xx";
		else if (c=='D')bid = " x";
		bidhistory += bid;
		String[] s= {String.valueOf(hand_id),bidhistory};
		MessageClient.send(new FloaterMessage("auction_status",s).toString(), this);
	}
	void sendPlay(String p) {
		// TODO Auto-generated method stub
		trick_data += p;
		String[] s= {String.valueOf(hand_id),trick_data};
		showTrickPlayed(p);
		MessageClient.send(new FloaterMessage("play",s).toString(), this);		
	}
}

class MessageClient implements ResponseTextHandler {
	//static String website = "http://127.0.0.1:10101/postit.yaws?flproxyB=";
	//static String website = "http://142.133.118.126:10101/postit.yaws?flproxyB=";
	static String website = "192.168.0.104:10101/postit.yaws?flproxyB=";
	static String MSG_SEPERATOR="<f/>";
	Object owner;
	String to_send = "";
	MessageClient(Object caller){
		owner = caller;
	}
	public static void send(String m, Object caller){
		Panels p = (Panels)caller;
		if (p.website != null )website = p.website;
		String url = website + URL.encode(m+MSG_SEPERATOR);
		System.out.println(url);
		if (m=="")url = website;
		MessageClient mc = new MessageClient(caller);
		HTTPRequest.asyncGet(url, mc);
	}

	public void onCompletion(String responseText) {
		// TODO Auto-generated method stub
		Panels p = (Panels)owner;
		String[] lines = responseText.split(MSG_SEPERATOR);
		p.debug("response:"+responseText);
		for (int k=0;k<lines.length;k++) {
			String line = lines[k];	
			if(line.startsWith("nothing")) break; 
			if(line == "") continue;	
			if(line.startsWith("T4"))	continue;
			p.debug("line:"+line);	
			System.out.println (line);
			char a = line.charAt(0); 
			if( a != '*' && a !='a' && a!= 'p') continue;	

			FloaterMessage m = new FloaterMessage(line);
			System.out.print(m.name+":");
			if(m.args != null)
				for(int i=0;i<m.args.length;i++)
					System.out.print(m.args[i]+"|");
			System.out.println("");
			p.handleData(m);
		}
		// leave graphic updating the last thing
		//p.process();
	}
}


class CardButton extends Button{
	int seat;
	int suit;
	CardButton(int seat,int suit, String label,  ClickListener listener){
		super(label, listener);
		this.suit = suit;
		this.seat = seat;
	}
}
