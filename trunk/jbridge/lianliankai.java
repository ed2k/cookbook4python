import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.Robot;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.awt.Point;
import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;

import javax.imageio.ImageIO;
import javax.swing.JFrame;
import javax.swing.JPanel;
import java.util.*;

// TODO: computer help to find matched pair
public class lianliankai {
	static int DELTA = 5; // distance to the edge
	static int CardWidth = 38-DELTA-DELTA;
	static int CardHeight = 35-DELTA-DELTA;
	Point[] marker = new Point[2];
	Point tableUperCorner;
	BufferedImage[] smallCards = new BufferedImage[42];
	BufferedImage[][] originCards = new BufferedImage[10][14];
	int[][] logicBoard = new int[10][14];
	BufferedImage image;
	ImageSearch igs;
	String [][] cards = new String[4][13];
	int startx,starty;
	JFrame capture;
	JPanel panel;
	Robot robot;
	lianliankai(){
		//for(int i=0;i<4;i++)for(int j=0;j<13;j++)cards[i][j] = "?";
		capture = new JFrame();
		capture.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		//Toolkit kit = Toolkit.getDefaultToolkit();
		//final Dimension d = kit.getScreenSize();
		capture.setSize(40*14,500);
		
		igs = new ImageSearch();
		try{		
			BufferedImage base = ImageIO.read(new File("lianliankai.png"));

			for (int row=0;row<4;row++) for(int col=0;col<10;col++){
				int r = (int) (row*35.5+DELTA);
				smallCards[row*10+col+1] = base.getSubimage(col*39+DELTA,r, CardWidth, CardHeight);
			}
			smallCards[41] = base.getSubimage(10*39+DELTA,DELTA, CardWidth, CardHeight);
			smallCards[0] = base.getSubimage(10*39+DELTA,DELTA+36, CardWidth, CardHeight);
		} catch (Exception e) { e.printStackTrace(); }
    
		panel = new JPanel() {
			public void paintComponent(Graphics g) {
				g.fillRect(0, 0, 600, 500);
				for (int i=0;i<logicBoard.length;i++)for (int j=0;j<logicBoard[i].length;j++){
					//g.drawImage(originCards[i][j],40*j,60+i*40, this);
					if (logicBoard[i][j]==0) continue;
					g.drawImage(smallCards[logicBoard[i][j]],35*j+DELTA,60+i*32+DELTA, this);
				}
				for (int i=0;i<5;i++){
					g.drawImage(smallCards[i+37],38*(i+5),0, this);
				}
				for (int i=0;i<5;i++)
					g.drawImage(smallCards[i],38*i,0, this);
				//g.drawImage(image, 0,0,this);
				BasicStroke stroke = new BasicStroke(4.0f);
				Graphics2D g2 = (Graphics2D) g;
				g2.setStroke(stroke);			
				if (marker [0] != null){
					g.setColor(Color.orange);
					for (int i=0;i<marker.length;i++){
						g2.draw(new Rectangle2D.Double(marker[i].x*35, 60+marker[i].y*32, 35, 32));
					}
				}
			}
		};
		panel.setOpaque(true);
		panel.prepareImage(image, panel);
		//panel.repaint();
		capture.getContentPane().add(panel);
		capture.setVisible(true);	

		//Timer t = new Timer();
		//t.scheduleAtFixedRate(new T(), 100, 2000);
		
		Runnable server = new Runnable() {
			@Override
			public void run() {
				try {
					ServerSocket serverSocket = new ServerSocket(4444);
					while (true){
						Socket clientSocket = null;
						clientSocket = serverSocket.accept();
						PrintWriter out = new PrintWriter(
			                      clientSocket.getOutputStream(), true);
						BufferedReader in = new BufferedReader(
						                        new InputStreamReader(
						                            clientSocket.getInputStream()));
						String inputLine, outputLine="";
				
						try {
						while ((inputLine = in.readLine()) != null) {	
							outputLine = processInput(inputLine);
						    out.println(outputLine);
						    if (outputLine.equals("Bye."))
						        break;
						}  } catch (Exception e) {
							System.out.println(e.getMessage());
						}	
					}
				} catch (Exception e) {
				    System.out.println(e.getMessage());
				    System.exit(-1);
				}
				
			}
			
		};
		new Thread(server).start();
	}
	protected String processInput(String inputLine) {
		if(inputLine.equals("calibrate")){
			return calibrate();
		} else if(inputLine.equals("update")){
			return update();
		} else if(inputLine.startsWith("mark ")){
			mark(inputLine.split(" "));
		}
		return "done";
	}
	private void mark(String[] pos) {
		marker = new Point[pos.length/2];
		for(int i=0;i<marker.length;i++){
			marker[i] = new Point(Integer.parseInt(pos[2*i+2]),Integer.parseInt(pos[2*i+1]));
		}
		panel.repaint();
	}
	private String update() {
		try {
			if (robot == null)robot = new Robot();
			// auto align to table
			image = robot.createScreenCapture(new Rectangle(0,0,700,600));
			
			Point p = tableUperCorner;
			int w = 14*39, h = (int)((double)10*(double)35.5);
			image.flush();
			image = image.getSubimage(p.x, p.y, w, h);
			//image.flush();
			//image = robot.createScreenCapture(new Rectangle(p.x,p.y,w,h));
            System.out.println(p.x+"-up-"+p.y);
			splitImage();
			
			panel.repaint();
			return getLogicBoard();
		} catch (Exception e) {
			e.printStackTrace();
		}
		return "";
	}
	private String calibrate() {
		try {
			if (robot == null)robot = new Robot();
			// auto align to table
			Point p = new Point(0,0);
			image = robot.createScreenCapture(new Rectangle(0,0,700,600));
			//image.flush();
			p = getCorner(smallCards,image);
			int left = p.x,up = p.y;
			if (left>=DELTA)left-=DELTA;
			if(up>=DELTA)up-=DELTA;

			//adjustTablePos();
			int w = 14*39, h = (int)((double)10*(double)35.5);
			if ((left+w)>image.getWidth())w=image.getWidth()-left;
			if ((up+h)>image.getHeight())h=image.getHeight()-up;
			//image = robot.createScreenCapture(new Rectangle(left,up,w,h));
			image = image.getSubimage(left, up, w, h);
			//image.flush();
			tableUperCorner = new Point(left,up);
			System.out.println(left+" "+up);
			splitImage();
			
			panel.repaint();
			return getLogicBoard();
		} catch (Exception e) {
			e.printStackTrace();
		}
		return "";
	}
	private String getLogicBoard() {
		String s = "";
		for (int row=0;row<originCards.length;row++) {
			for(int col=0;col<originCards[row].length;col++){	
				int i = findImageIdx(smallCards,originCards[row][col]);
				logicBoard[row][col]=i;
				s += (i+" "); 				
			}
			s += ",";
		}	
		return s;
	}
	private int findImageIdx(BufferedImage[] templ,	BufferedImage img) {
		for(int i=0;i<templ.length;i++){
			if (igs.isIn(templ[i], img)){
				return i;
			}
		}		
		return 0;
	}
	private void splitImage() {
		for (int row=0;row<originCards.length;row++) for(int col=0;col<originCards[row].length;col++){
			int right = (col+1)*39, down = (row+1)*35;
			if (right<=image.getWidth() && down<=image.getHeight()){
				int c = col*(39), r = (int)((double)row*(double)(35.5)), w = 39, h=35;
				if ( (c+w)> image.getWidth())w=image.getWidth()-c;
				if ((r+h) > image.getHeight())h=image.getHeight()-r;
				originCards[row][col] = image.getSubimage(c,r,w,h);
			}
		}
	}
	/**
	 * @param args 
	 */
	public static void main(String[] args) {
		new lianliankai();

	}

	// find any image in templ from src
	Point findImage(BufferedImage[] templ, BufferedImage src){
		for(int i=0;i<templ.length;i++){
			Point p = igs.findImage(templ[i], src);
			if (p != null){
				return p;
			}
		}			
		return null;
	}
	public Point getCorner(BufferedImage[] templ, BufferedImage src){
		int leftX=0, leftY=0;
		Point p = findImage(templ,src);
		if (p==null) return new Point(leftX,leftY);
		leftX = p.x;
		leftY = p.y;
		while (true) {
			int nx = leftX;
			BufferedImage img = src.getSubimage(0,0,nx,src.getHeight());
			Point n = findImage(templ,img);
			if (n==null)break;
			leftX = n.x;
		}
		
		while (true) {
			int ny = leftY;
			BufferedImage img = src.getSubimage(0,0,src.getWidth(),ny);
			Point n = findImage(templ,img);
			if (n==null)break;
			leftY = n.y;
		}	
		return new Point(leftX,leftY);
	}
	public Point getCornerDownRight(BufferedImage[] templ, BufferedImage src){
		int leftX=src.getWidth(), leftY=src.getHeight();
		Point p = findImage(templ,src);
		if (p==null) return new Point(leftX,leftY);
		leftX = p.x;
		leftY = p.y;
		while (true) {
			int nx = leftX+templ[0].getWidth();
			BufferedImage img = src.getSubimage(nx,0,src.getWidth()-nx,src.getHeight());
			Point n = findImage(templ,img);
			if (n==null)break;
			leftX += nx+n.x;
		}
		
		while (true) {
			int ny = leftY+templ[0].getHeight();
			BufferedImage img = src.getSubimage(0,ny,src.getWidth(),src.getHeight()-ny);
			Point n = findImage(templ,img);
			if (n==null)break;
			leftY = ny+n.y;
		}	
		return new Point(leftX+templ[0].getWidth(),leftY+templ[0].getHeight());
	}	
	class T extends TimerTask{
			
		public void run() {
			
			try {
				panel.repaint();
			} catch (Exception e) {
				e.printStackTrace();
			}

		}



	}

}

