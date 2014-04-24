package serverfmwk.net;

import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.SocketChannel;
import java.nio.ByteBuffer;
import java.io.IOException;

public class BabelClient extends Client {

    private int me;
    private int state;
    private SelectionKey key;
    
    
    public BabelClient(SocketChannel sc, Selector selector, int me) throws Exception {
	super (sc);
	
	this.channel.configureBlocking(false);
	this.me = me;
	this.key = sc.register(selector, 0, this);
	
	
    }

    
    public void run() {
	ByteBuffer buf = ByteBuffer.allocate(1024);
	try {
	    int bytes = channel.read(buf);
	    
	    //buf.flip();
	    //System.out.println("buf: " + buf.toString());
	    byte[] array = buf.array();
	    String str = new String(array);
	    System.out.println("I got " + str +" " + bytes + " bytes");
	    
	}
	catch (IOException ie) {
	    ie.printStackTrace();
	    return;
	}
	
    }
    
}
