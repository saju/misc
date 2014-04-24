package serverfmwk.net;

import java.io.IOException;
import java.nio.channels.SelectionKey;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.InetSocketAddress;

import serverfmwk.Context;
import serverfmwk.log.ServerLogger;
import serverfmwk.config.Configuration;

/**
 * <p>   </p>
 */

public class Listener {
    protected int type;                 /* type of Listener */
    protected int port;                    /* the port number this listener is bound to */
    protected ServerSocketChannel channel; /* the selectable channel for this listener */
    protected Context ctx;                 /* the global context */
    protected ServerLogger logger;           /* the global logger */
    protected SocketAddress addr;          /* the full inet address this listener is bound to */
    protected SelectionKey key;
    
  
    public static Listener getListener(int port, int type, Context ctx) throws Exception {
	if (type == Configuration.SECURE_TYPE) 
	    return new SecureListener(port, type, ctx);
	else if (type == Configuration.BABEL_TYPE)
	    return new BabelListener(port, type, ctx);
	else
	    return new Listener(port, type, ctx);
    }

    public Listener(int port, int type, Context ctx) throws Exception {
	this.port    = port;
	this.type    = type;
	this.ctx     = ctx;

	/* XXX: maybe we should install our own SocketFactoryImpl to garauntee that
	   a customer's VM doesn't spook us */
	this.channel = ServerSocketChannel.open();

	/* XXX : currently binds to *:port */
	this.addr    = new InetSocketAddress(port);

	this.channel.configureBlocking(false);
    }

    public void bind() throws Exception {
	this.channel.socket().bind(this.addr);
    }

    public void close() throws Exception {
	this.channel.close();
    }

    public String getName() {
	return "Plain";
    }

    public int getPort() {
	return this.port;
    }

    public int getType() {
	return this.type;
    }
  
    public ServerSocketChannel getChannel() {
	return this.channel;
    }

    public void setKey(SelectionKey key) {
	this.key = key;
    }

    /** 
     * <p> this default implementation of processConnection() does nothing.
     * Protocol specific listeners must implement their own connection processing
     * logic, which includes asynchronous handling of the request and protocol logic. </p>
     */
    public void processConnection() {
	try {
	    SocketChannel client = this.channel.accept();
	    client.close();
	} catch(Exception e) {
	    ctx.getLogger().severe("Error while processing client connection", e);
	}
    }
}
