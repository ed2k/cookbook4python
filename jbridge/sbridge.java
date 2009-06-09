import java.awt.Graphics;
import java.awt.Rectangle;
import java.awt.Robot;
import java.awt.image.BufferedImage;
import java.awt.Point;
import java.io.File;

import javax.imageio.ImageIO;
import javax.swing.JFrame;
import javax.swing.JPanel;
import java.util.*;
import cn.client.*;

public class sbridge {
	//image sample to match with
	BufferedImage[] smallCards, bigCards, smallSuits, bigSuits;
	BufferedImage tableCorner, bidImage;
	// image from 4 players
	BufferedImage image, left, right, up, down;
	ImageSearch igs;
	String [][] cards = new String[4][13];
	int startx,starty;
	JFrame capture;
	JPanel panel;
	Robot robot;
	private Point tableUpperCorner;
	sbridge(){
		String root= ".\\";
		smallCards = new BufferedImage[13];
		bigCards = new BufferedImage[13];
		smallSuits = new BufferedImage[4];
		bigSuits = new BufferedImage[4];
		try{		
			BufferedImage base = ImageIO.read(new File(root+"cards.png"));
			for (int i=0;i<13;i++){
				smallCards[i] = base.getSubimage(i*8, 0, 8, 9);
				bigCards[i] = base.getSubimage(i*10, 20, 10, 11);
			}
			for (int i=0;i<4;i++){
				smallSuits[i] = base.getSubimage(i*9, 9, 9, 11);;
				bigSuits[i] = base.getSubimage(i*10, 43, 10, 9);;
			}
			tableCorner = base.getSubimage(0, 31, 11, 43-31);
			bidImage = base.getSubimage(41, 43, 17, 11);
		} catch (Exception e) { e.printStackTrace(); }
		
		for(int i=0;i<4;i++)for(int j=0;j<13;j++)cards[i][j] = "?";
		capture = new JFrame();
		capture.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		//Toolkit kit = Toolkit.getDefaultToolkit();
		//final Dimension d = kit.getScreenSize();
		capture.setSize(500,500);
		//Rectangle rect = new Rectangle(d);
		
		igs = new ImageSearch();
        
		//		for(int j=0;j<9;j++){
//			StringBuffer sb = new StringBuffer();
//			for(int i=0;i<8;i++){
//				int p = source.getRGB(i, j);
//				sb.append(scale2ascii(p));
//			}
//			src[j] = sb.toString();
//			System.out.println(src[j]);
//		}
		panel = new JPanel() {
			private static final long serialVersionUID = 1L;

			public void paintComponent(Graphics g) {
				if (left == null)return;
				g.drawImage(left, 0, 0,  this);
				g.drawImage(right, 50, 0, this);
				g.drawImage(up, 100, 0, this);
				g.drawImage(down, 150, 0,  this);
				g.drawImage(image, 0,60,this);
				//g.drawRect(startx, starty, 8, 9);
				//g.drawImage(source, 200,0, this);
				for (int i=0;i<13;i++){
					g.drawImage(igs.bigCards[i],200+i*10,0, this);
				}
				for (int i=0;i<4;i++){
					g.drawImage(igs.bigSuits[i],200+i*10,11, this);
				}
				g.drawImage(igs.bidImage,200,20,this);
			}
		};
		panel.setOpaque(true);
		panel.prepareImage(image, panel);
		//panel.repaint();
		capture.getContentPane().add(panel);
		capture.setVisible(true);	
		
		Timer t = new Timer();
		t.scheduleAtFixedRate(new T(), 1000, 1000);
	}
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		new sbridge();

	}
	void find(BufferedImage img){
		if (igs.findBidImage(img) != null){
			for(int i=0;i<4;i++)for(int j=0;j<13;j++)cards[i][j] = "?";
			return;
		}
		StringBuffer s = new StringBuffer();
		char c = igs.findBigSuit(img);
		if (c == 0)	return;
		s.append(c);
		c = igs.findBigCard(img);
		if (c == 0) return;
		s.append(c);
		Card card = new Card(s.toString());
		int suit = card.getColour();
		int rank = card.getValue()-2;
		if (cards[suit][rank].charAt(0) != '?') return;
		cards[suit][rank] = card.rank();
		System.out.print(card.toString()+" ");
		for (int i =0;i<13;i++){
			System.out.print(cards[suit][i]);
		}
		System.out.println();
		
	}
	public static BufferedImage rotate90(BufferedImage src){		
		int nw = src.getHeight();
		BufferedImage tgt = new BufferedImage(nw,src.getWidth(),BufferedImage.TYPE_INT_ARGB);

		for(int x = 0;x<src.getWidth();x++)for (int y=0;y<nw;y++)
			tgt.setRGB(nw-1-y, x, src.getRGB(x,y));
		return tgt;
	}
	void adjustTablePos(){
		image = robot.createScreenCapture(new Rectangle(800,500));
		image.flush();
		Point p = igs.findTableCorner(image);
		if (p!= null)System.out.println(p.toString()); 
	}
	private void calibrate() {
		try {
			if (robot == null)robot = new Robot();
			// auto align to table
			image = robot.createScreenCapture(new Rectangle(0,0,700,600));
			//image.flush();
			Point p = igs.findImage(tableCorner,image);
			if (p==null)return;
			int left = p.x,up = p.y;

			//adjustTablePos();
			int w = 14*39, h = (int)((double)10*(double)35.5);
			if ((left+w)>image.getWidth())w=image.getWidth()-left;
			if ((up+h)>image.getHeight())h=image.getHeight()-up;
			//image = robot.createScreenCapture(new Rectangle(left,up,w,h));
			image = image.getSubimage(left, up, w, h);
			//image.flush();
			tableUpperCorner = new Point(left,up);
			System.out.println(left+" "+up);

		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	class T extends TimerTask{
			
		public void run() {
			
			try {
				calibrate();
				if (robot == null)robot = new Robot();
				
				//TODO auto align to table
				Point p = tableUpperCorner;
				image = robot.createScreenCapture(new Rectangle(p.x, p.y,p.x+200,p.y+200));
				
				//image.flush();
				//adjustTablePos();
				//image = image.getSubimage(p.x, p.y, image.getWidth()-p.x, image.getHeight()-p.y);
				int w = 50,h=50;
				int x = 100,y=100;
				left = rotate90(image.getSubimage(x-100, y-10, w, h));
				right = rotate90(image.getSubimage(x+50, y-10, w, h));
				up = image.getSubimage(x, y-100, w, h);
				down = image.getSubimage(x, y-20, w, h);
				find(up);
				find(down);
				find(left);
				find(right);
				

				panel.repaint();
			} catch (Exception e) {
				e.printStackTrace();
			}

		}



	}

}

