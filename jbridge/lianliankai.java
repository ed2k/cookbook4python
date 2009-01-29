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

// TODO: computer help to find matched pair
public class lianliankai {
	BufferedImage[] smallCards = new BufferedImage[40];
	BufferedImage[][] originCards = new BufferedImage[10][14];
	int[] found = new int[40];
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
			int delta = 4;
			for (int row=0;row<4;row++) for(int col=0;col<10;col++){
				smallCards[row*10+col] = base.getSubimage(col*39+delta,row*35+delta, 38-delta-delta, 35-delta-delta);
			}
		} catch (Exception e) { e.printStackTrace(); }
		for (int i=0;i<40;i++){
			found[i] = -1;
		}      
		panel = new JPanel() {
			public void paintComponent(Graphics g) {
				//g.drawImage(image, 0,60,this);
				for (int i=0;i<10;i++)for (int j=0;j<14;j++){
					g.drawImage(originCards[i][j],40*j,60+i*40, this);
				}
				for (int i=0;i<40;i++){
					if (found[i]!=-1){
					  g.drawImage(smallCards[found[i]],38*i,0, this);
					} else g.drawImage(smallCards[0],38*i,0, this);
				}
			}
		};
		panel.setOpaque(true);
		panel.prepareImage(image, panel);
		//panel.repaint();
		capture.getContentPane().add(panel);
		capture.setVisible(true);	

		Timer t = new Timer();
		t.schedule(new T(), 100, 2000);
	}
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		new lianliankai();

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
	class T extends TimerTask{
			
		public void run() {
			
			try {
				if (robot == null)robot = new Robot();
				//TODO auto align to table
				Point p = new Point(20,100);
				image = robot.createScreenCapture(new Rectangle(p.x+14*39,p.y+10*35));
				image.flush();
				//adjustTablePos();
				image = image.getSubimage(p.x, p.y, image.getWidth()-p.x, image.getHeight()-p.y);
				for (int row=0;row<10;row++) for(int col=0;col<14;col++){
					originCards[row][col] = image.getSubimage(col*39,row*35, 39, 35);
				}
				for (int i=0;i<40;i++){
					found[i] = -1;
				}   
				int idx = 0;
				for(int i=0;i<40;i++){
					if (igs.isIn(smallCards[i], image)){
						System.out.print(" " +i);
						found[idx] = i;
						idx++;
					}
				}
				System.out.println(" ");

				panel.repaint();
			} catch (Exception e) {
				e.printStackTrace();
			}

		}



	}

}

