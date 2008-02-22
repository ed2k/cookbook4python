package cn.client;
import cn.client.*;

public class FloaterMessage {
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

