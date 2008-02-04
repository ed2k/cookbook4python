package cn.client;

public class BadTypeException extends Exception {
    public BadTypeException(String msg){
	super(msg);
	printStackTrace();// for debug
    }
}
