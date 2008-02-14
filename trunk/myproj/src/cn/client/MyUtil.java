package cn.client;

public class MyUtil {
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
