import java.awt.image.BufferedImage;
import java.io.File;
import java.awt.Point;
import javax.imageio.ImageIO;

//all image samples are at cards.png
public class ImageSearch {
	int startx, starty;
	BufferedImage[] smallCards, bigCards, smallSuits, bigSuits;
	BufferedImage tableCorner, bidImage;
	static String trans = "AKQJT98765432"; 
	static String trans2 = "SHDC";
	
	ImageSearch(){
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
	}
	Point findTableCorner(BufferedImage image){
		return isIn(tableCorner,image,0,0,image.getWidth(),image.getHeight(),new PixelMatch());
	}
	Point findBidImage(BufferedImage image){
		return isIn(bidImage,image,0,0,image.getWidth(),image.getHeight(),new PixelMatch());
	}	
	char findBigCard(BufferedImage image){
		return findBigCard(image,0,0,image.getWidth(),image.getHeight(), new MatchBlue());
	}
	char findBigSuit(BufferedImage image){
		for (int i=0;i<bigSuits.length;i++){
			if (isIn(bigSuits[i], image, 0, 0, image.getWidth(), image.getHeight(), new MatchBlue()) != null)
				return trans2.charAt(i);	
		}
		return 0;		
	}
	char findBigCard(BufferedImage image ,int initx,int inity, int width, int height, PixelOpts op){
		for (int i=0;i<bigCards.length;i++){
			if (isIn(bigCards[i], image, initx, inity, width, height, op) != null)
				return trans.charAt(i);	
		}
		return 0;
	}
	int grey(int rgb){
		int r = (rgb & 0x00ff0000) >> 16;
		int g = (rgb & 0x0000ff00) >> 8;
		int b = rgb & 0x000000ff;
		int a =  33+(126-33)*(r+g+b/3)/255 ;
		return (a << 16) | (a << 8) | a;
	}
	int getB(int rgb){
		return rgb & 0x000000ff;
	}
	char scale2ascii(int rgb){
		int g = grey(rgb);
		char c = (char)(g& 0xff);
		return c;
	}
	private void find(String[] src, BufferedImage image ) {
		for(int y=0;y<(image.getHeight()-src.length);y++){
			StringBuffer sb = new StringBuffer();
			for(int x=0;x<image.getWidth();x++){
				int p = image.getRGB(x, y);
				sb.append(scale2ascii(p));
			}
			String s = sb.toString();
			int idx = s.indexOf(src[0]); 
			if (idx == -1) continue;
			if (!matchAscii(src,image,idx,y))continue;
			System.out.println("->"+idx+","+y);
			startx = idx;
			starty = y;
		}			
	}
	//determin if src image is within image
	static public Point isIn(BufferedImage src, BufferedImage image, PixelOpts op) {
		return isIn(src, image, 0,0, image.getWidth(),image.getHeight(), op);
	}
	public boolean isIn(BufferedImage src, BufferedImage img) {
		PixelMatch op = new PixelMatch();
		if(null ==  isIn(src,img, op))return false;
		return true;
	}
	public Point findImage(BufferedImage src, BufferedImage img) {
		PixelMatch op = new PixelMatch();
		return  isIn(src,img, op);
	}

	class MatchBlue implements PixelOpts {	
		public boolean match(int a, int b) {
			return getB(a) == getB(b);
		}		
	}
	class PixelMatch implements PixelOpts{
		public boolean match(int a, int b) {
			return a == b;
		}		
	}
	//determin if src image is within image
	static public Point isIn(BufferedImage src, BufferedImage image, int initx,int inity, int width, int height, PixelOpts op){
		if (src.getHeight()> height || src.getWidth()>width)return null;
		if ((initx+src.getWidth())> image.getWidth() || (inity+src.getHeight())> image.getHeight()) return null;
		for(int y=inity; y<(inity+height-src.getHeight());y++){
			for(int x=initx; x<(initx+width-src.getWidth());x++){
				if (match(src,image,x,y, op))return new Point(x,y);
				//System.out.println("->"+idx+","+y);				
			}
		}				
		return null;
	}

	static private boolean match(BufferedImage src, BufferedImage image, int startx, int starty, PixelOpts op) {
		for(int i=0;i<src.getWidth();i++)for(int j=0;j<src.getHeight();j++){
			if(!op.match(src.getRGB(i, j),image.getRGB(startx+i, starty+j))) return false;
		}
		return true;
	}

	private boolean matchAscii(String[] src, BufferedImage image, int startx, int starty) {
		for(int i=0;i<src[0].length();i++)for(int j=0;j<src.length;j++){
			int y = starty + j;
			if (y > image.getHeight())return false;
			int p = image.getRGB(startx+i, y);
			char c = scale2ascii(p);
			if (src[j].charAt(i) != c) return false;
		}
		return true;
	}
	// direction 0,row
	static boolean isLine(BufferedImage image,int start, int direction){
		return true;
	}
	static Point findLines(BufferedImage image,int startx, int starty){
		for(int x=startx;x<image.getWidth();x++){
			
		}
		return new Point(0,0);
	}
}

interface PixelOpts {
	boolean match(int a, int b);
}