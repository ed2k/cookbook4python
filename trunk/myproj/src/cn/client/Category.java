package cn.client;

public class Category {
	static Category _obj = new Category();
	static public Category getInstance(String name){
		return _obj;
	}
	public void debug(String s){
		output(s);				
	}
	public void fatal(String s){
		output(s);				
	}
	public void error(String s){
		output(s);				
	}
	public void info(String s){
		output(s);		
	}
	public void warn(String s){
		output(s);
	}
	void output(String s){
		System.out.println(s);
	}
}
