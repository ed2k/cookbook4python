package cn.client;

import java.util.Iterator;
import java.util.Vector;

import com.google.gwt.core.client.EntryPoint;
import com.google.gwt.user.client.ui.*;

//import com.google.gwt.user.client.ui.ClickListener;
//import com.google.gwt.user.client.ui.Label;
//import com.google.gwt.user.client.ui.RootPanel;
//import com.google.gwt.user.client.ui.Widget;
import com.google.gwt.user.client.Timer;
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
		Button button = new Button("test");
		button.addClickListener(new ClickListener() {
			public void onClick(Widget sender) {
				MessageClient.send("", panel);
				//panel.test();
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
		RootPanel.get().add(panel);
	}
}
/**
 * Demonstrates various panels and the way they lay widgets out.
 */
class Panels extends Composite{
	DockPanel dock ;
	Match currentMatch = null;

	HTML debug= new HTML("");
	HorizontalSplitPanel hSplit;
	VerticalPanel[] dockHand;
	HorizontalPanel[][] hpSuit;
	//------------ integrate with java applete app.
	int hand_id = 0;
	int myseat = 0;
	String bidhistory = "";
	// trick layout
	String trick_data = "";
	String [][] tdata;
	//MessageClient message_client;
	public void test(){
		String[] s = {"AKQJT98765432","","AQ","T987654"};
		tdata[2] = s;
		tdata[1] = s;
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
		for (int i = 0;i<4;i++)for (int j = 0;j<4;j++){
			tdata[i][j] = "";
		}

		HTML contents = new HTML("N  E  S  W  ");
		ScrollPanel scroller = new ScrollPanel(contents);
		//scroller.setStyleName("ks-layouts-Scroller");

		DockPanel dock = new DockPanel();
		dockHand = new VerticalPanel[4];
		hpSuit = new HorizontalPanel[4][4];
		
		//dockLHO.add(new HTML("West"));
//		dockMe.setHorizontalAlignment(VerticalPanel.ALIGN_CENTER);
//		dockRHO.setHorizontalAlignment(VerticalPanel.ALIGN_LEFT);
		for (int i =0;i<4;i++){
			dockHand[i] = new VerticalPanel();
			for (int j = 0;j<4;j++){
				hpSuit[i][j] = new HorizontalPanel();
				dockHand[i].add(hpSuit[i][j]);	
			}
		}

		dock.add(dockHand[0], DockPanel.NORTH);
		dock.add(dockHand[2], DockPanel.SOUTH);
		dock.add(dockHand[1], DockPanel.EAST);    
		dock.add(dockHand[3], DockPanel.WEST);
		dock.add(scroller, DockPanel.CENTER);
		dock.setHorizontalAlignment(DockPanel.ALIGN_CENTER);
		dock.setCellHorizontalAlignment(dockHand[0], DockPanel.ALIGN_CENTER);
		dock.setCellHorizontalAlignment(dockHand[2], DockPanel.ALIGN_CENTER);


		VerticalPanel vp = new VerticalPanel();

		Grid grid = new Grid(4, 4);
		for (int r = 0; r < 4; ++r) {
			for (int c = 0; c < 4; ++c) {
				//grid.setWidget(r, c, images.gwtLogo().createImage());
				grid.setWidget(r, c, new Button(""));
			}
		}
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
		vp.add(dock);vp.add(grid1);
		vp.add(grid2);
		TabPanel tabs = new TabPanel();
		tabs.add(grid, "Bids");
		tabs.add(vp, "Table");
		//tabs.add(grid, "play");
		tabs.setWidth("100%");
		tabs.selectTab(1);

		hSplit = new HorizontalSplitPanel();
		hSplit.setLeftWidget(tabs);
		//hSplit.setRightWidget(debug);
		initWidget(hSplit);
		hSplit.setSize("100%", "450px");

		show_4hands();		
		//message_client = new MessageClient(this);
		//message_client.scheduleRepeating(1000);
		String[] s = {"N"};
		FloaterMessage m = new FloaterMessage("request_seat",s);
		MessageClient.send(m.toString(),this);		
	}



	public void show_4hands(){
		ClickListener cardClick = new ClickListener() {
			public void onClick(Widget sender) {
				CardButton b = (CardButton)sender;
				String card = Card.SUIT2STR[3-b.suit]+b.getText();
				debug("-"+card+"-");
				sendPlay(card.toLowerCase());
			}
		};		
		for (int i=0;i<4;i++)for(int j=0;j<4;j++){
			String suit = tdata[i][j];
			dockHand[i].remove(hpSuit[i][j]);
			hpSuit[i][j] = new HorizontalPanel();
			dockHand[i].add(hpSuit[i][j]);
			hpSuit[i][j].add(new HTML(Card.SUIT2STR[3-j]));
			if(suit == "")continue;
			for (int k=0;k<suit.length();k++){
				Button b = new CardButton(j,suit.substring(k,k+1),cardClick);
				hpSuit[i][j].add(b);
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
			debug("handlData: "+hands);
			String[] hs = hands.split("\\|");
			for(int i=0;i<hs.length;i++){
				System.out.println(hs[i]);
				int seat = Integer.parseInt(hs[i].substring(0,1));
				if (i == 0) myseat = seat;
				int fix = (2+seat-myseat) %4;
				//Hand h = new Hand(hs[i].substring(2));
				tdata[fix] = hs[i].substring(2).split("\\.");
				show_4hands();
			}
		} else if (msgname == 'a'){
			bidhistory = msg.args[1];
			debug(bidhistory);
		} else if (msgname == 'p'){
			trick_data = msg.args[1];
			debug(trick_data);
		}
	}	
	
	public void sendBid(String b){
		char c = b.charAt(0);
		String bid = b.toLowerCase();
		if (c=='P')bid = " p";
		else if (c=='R')bid = "xx";
		else if (c=='D')bid = " x";
		String[] s= {String.valueOf(hand_id),bidhistory+bid};
		MessageClient.send(new FloaterMessage("auction_status",s).toString(), this);
	}
	void sendPlay(String p) {
		// TODO Auto-generated method stub
		String[] s= {String.valueOf(hand_id),trick_data+p};
		MessageClient.send(new FloaterMessage("play",s).toString(), this);		
	}
}

class MessageClient extends Timer implements ResponseTextHandler {
	static String website = "http://127.0.0.1:4080/postit.yaws?flproxyB=";
	Object owner;
	String to_send = "";
	MessageClient(Object caller){
		owner = caller;
	}
	public static void send(String m, Object caller){
		String url = website + URL.encode(m+"\r\n");
		if (m=="")url = website;
		MessageClient mc = new MessageClient(caller);
		HTTPRequest.asyncGet(url, mc);
	}
	public void run(){
		String url = website;
		Panels p = (Panels)owner;		
		if(to_send != ""){
			url = website+to_send;
			p.debug("S> "+url);
		}
		HTTPRequest.asyncGet(url, this);
		to_send = "";
	}
	public void onCompletion(String responseText) {
		// TODO Auto-generated method stub
		Panels p = (Panels)owner;
		String[] lines = responseText.split("\\r\\n");
		p.debug(responseText);
		for (int k=0;k<lines.length;k++) {
			String line = lines[k];	
			if(line.startsWith("nothing")) break; 
			if(line == "") continue;	
			if(line.startsWith("T4"))	continue;
			p.debug(line);	
			System.out.println (line);

			FloaterMessage m = new FloaterMessage(line);
			System.out.print(m.name+":");
			if(m.args != null)
				for(int i=0;i<m.args.length;i++)
					System.out.print(m.args[i]+"|");
			System.out.println("");
			p.handleData(m);
		}
		//this.schedule(1000);
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

class CardButton extends Button{
	int suit;
	CardButton(int suit, String label,  ClickListener listener){
		super(label, listener);
		this.suit = suit;
	}
}