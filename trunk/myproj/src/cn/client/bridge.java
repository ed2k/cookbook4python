package cn.client;

import java.util.Iterator;

import com.google.gwt.core.client.EntryPoint;
import com.google.gwt.user.client.ui.*;

//import com.google.gwt.user.client.ui.ClickListener;
//import com.google.gwt.user.client.ui.Label;
//import com.google.gwt.user.client.ui.RootPanel;
//import com.google.gwt.user.client.ui.Widget;


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
				panel.test();
			}
		});

		// Assume that the host HTML has elements defined whose
		// IDs are "slot1", "slot2".  In a real app, you probably would not want
		// to hard-code IDs.  Instead, you could, for example, search for all 
		// elements with a particular CSS class and replace them with widgets.
		//
		//RootPanel.get("slot1").add(button);
		RootPanel.get().add(button);
		RootPanel.get().add(panel);
	}
}
/**
 * Demonstrates various panels and the way they lay widgets out.
 */
class Panels extends Composite{
	DockPanel dock ;
	Match currentMatch = null;
	Player p[];    
	Game g;
	HTML debug= new HTML("");
	HorizontalSplitPanel hSplit;
	VerticalPanel dockRHO,dockLHO,dockPar, dockMe;
	HTML strLHO[],strRHO[];
	HTML strPar, strMe;
	public void test(){
		String s = p[1].getHand().toSuitString();
		debug(s);
	}
	public void debug(String s){
		debug = new HTML(s+ debug.getHTML());
		hSplit.setRightWidget(debug);
	}
	public String html_center(String s){
		return "<center>"+s+"</center>";
	}
	public Panels() {	
		p = new Player[4];
		for(int i=0;i<p.length;i++){
			p[i] = new Player(Integer.toString(i));
		}
		HTML contents = new HTML("N  E  S  W  ");
		ScrollPanel scroller = new ScrollPanel(contents);
		//scroller.setStyleName("ks-layouts-Scroller");

		DockPanel dock = new DockPanel();
		dockRHO = new VerticalPanel();
		dockLHO = new VerticalPanel();
		dockMe = new VerticalPanel();
		dockPar = new VerticalPanel();
		strLHO = new HTML[4];
		strRHO = new HTML[4];
		strMe = new HTML("North");
		strPar = new HTML("South");
		dockLHO.add(new HTML("West"));
		dockRHO.add(new HTML("East"));
		dockMe.setHorizontalAlignment(VerticalPanel.ALIGN_CENTER);
		dockPar.setHorizontalAlignment(VerticalPanel.ALIGN_CENTER);
		dockLHO.setHorizontalAlignment(VerticalPanel.ALIGN_LEFT);
		dockRHO.setHorizontalAlignment(VerticalPanel.ALIGN_LEFT);
		dockPar.add(strPar);
		dockMe.add(strMe);    

		for (int i=0;i<4;i++){
			strLHO[i] = new HTML(Card.suit2str(5-(i+1)));
			strRHO[i] = new HTML(Card.suit2str(5-(i+1)));
			dockLHO.add(strLHO[i]);
			dockRHO.add(strRHO[i]);
		}
		dockMe.add(strMe);
		dockPar.add(strPar);

		dock.add(dockPar, DockPanel.NORTH);
		dock.add(dockMe, DockPanel.SOUTH);
		dock.add(dockRHO, DockPanel.EAST);    
		dock.add(dockLHO, DockPanel.WEST);
		dock.add(scroller, DockPanel.CENTER);
		dock.setHorizontalAlignment(DockPanel.ALIGN_CENTER);
		dock.setCellHorizontalAlignment(dockMe, DockPanel.ALIGN_CENTER);
		dock.setCellHorizontalAlignment(dockPar, DockPanel.ALIGN_CENTER);


		VerticalPanel vp = new VerticalPanel();

		Grid grid = new Grid(4, 4);
		for (int r = 0; r < 4; ++r) {
			for (int c = 0; c < 4; ++c) {
				//grid.setWidget(r, c, images.gwtLogo().createImage());
				grid.setWidget(r, c, new Button(""));
			}
		}
		Grid grid2 = new Grid(4, 4);
		for (int r = 0; r < 4; ++r) {
			for (int c = 0; c < 4; ++c) {
				grid2.setWidget(r, c, new Button(""));
			}
		}    
		vp.add(dock);
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

		initMatch();
		g = new Game(currentMatch);
		g.doDeal();
		show_4hands();		
	}


	public void initMatch(){
		//cat.debug("> initMatch()");
		currentMatch = new Match();
		currentMatch.setNorth(p[0]);
		currentMatch.setEast(p[1]);
		currentMatch.setSouth(p[2]);
		currentMatch.setWest(p[3]);
		//cat.debug("< initMatch()");
	}
	public void play()  {
		//cat.debug("> play()");		
		do {
			//System.out.println(intl.getString("STARTING_NEW_GAME"));
			Game g = new Game(currentMatch);
			// let's deal the cards
			//cat.info("Dealing the cards...");
			g.doDeal();

			// let's do the bid stage
			//cat.info("Starting the bid stage...");
			Bid toplay = g.doBidStage();

			// show what we're playing
			if (toplay != null && (toplay.getValue()!=Bid.PASS ||
					toplay.getSuit()!=Bid.IGNORED ||
					toplay.getInsult()!=Bid.IGNORED)){
				//cat.info("We're going to play: "+toplay+" ");
				//System.out.print(intl.getString("BID_TO_PLAY")+" ");
				//System.out.println(toplay.show(intl));
			}

			// next game
			//cat.info("End of game");
			//System.out.println(intl.getString("ENDING_GAME"));
			currentMatch.nextGame();
		} while(currentMatch.getDealer().getOrientation() != Orientation.NORTH);

		//cat.debug("< play()");
	}
	public void show_4hands(){
		Hand hEast = p[1].getHand();
		Hand hWest = p[3].getHand();
		for (int i=0;i<4;i++){
			int suit = 4-i;		
			String w = hWest.toSuitString(suit);
			String e = hEast.toSuitString(suit);
			strLHO[i].setHTML(w);
			strRHO[i].setHTML(e);
		}
		strMe.setHTML(p[2].getHand().toSuitString());
		strPar.setHTML(p[0].getHand().toSuitString());
	}
}

