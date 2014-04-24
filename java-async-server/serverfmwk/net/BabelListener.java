package serverfmwk.net;

import java.nio.channels.Selector;
import java.nio.channels.SelectionKey;
import java.nio.channels.SocketChannel;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.ThreadFactory;
import java.util.Iterator;
import java.util.Set;
import java.io.IOException;

import serverfmwk.Context;
import serverfmwk.config.Configuration;


public class BabelListener extends Listener implements Runnable, RejectedTaskHandler, ThreadFactory {
    private TPool tpool;
    private ThreadGroup tGroup;
    private Selector ioSelector;
    private ArrayBlockingQueue<Client> queue;
    
    public BabelListener(int port, int type, Context ctx) throws Exception {
	super(port, type, ctx);
	
	/* get the configuration */
	Configuration cfg = ctx.getConfiguration();

	/* setup the thread group */
	tGroup = new ThreadGroup(getName());
	tGroup.setDaemon(true);

	/* setup the blocking queue */
	this.queue = new ArrayBlockingQueue(cfg.getQueueSize(this.type));
	
	
	/* setup the worker pool */
	this.tpool = new TPool(cfg.getCorePoolSize(this.type), cfg.getMaxPoolSize(this.type), 10, TimeUnit.SECONDS, new ArrayBlockingQueue(cfg.getQueueSize(this.type)), this, this);

	/* open the i/o selector */
	this.ioSelector = Selector.open();
	
	/* start the async i/o thread */
	new Thread(this.tGroup, this).start();
    }

    public String getName() {
	return "Babel";
    }
	
    /* called when a Client cannot be processed by the Executor service */
    public void rejectedTask(Runnable r, Exception e) {
	Client client = (Client)r;
	/* XXX: scribble a busy message to client here */
	try {
	    client.getChannel().close();
	} catch (IOException io) {
	    ctx.getLogger().severe("close() error on " + client.getIP(), io);
	}
	if (e != null) 
	    ctx.getLogger().severe("Failed to process client " + client.getIP() + " request " + client.getRequest(), e);
	else				   	   	  
	    ctx.getLogger().severe("Failed to process client " + client.getIP() + " request " + client.getRequest());
    }

    /* called when a Exception is not caught by any local handler in a thread from the workgroup */
    public void uncaughtException(Thread t, Throwable e) {
    	ctx.getLogger().severe("Error while processing client ", new Exception(e));
    }

    /* custom ThreadFactory */
    public Thread newThread(Runnable task) {
	Thread t = new Thread(this.tGroup, task, this.getName());
	//t.setUncaughtExceptionHandler(this);
	return t;
    }
	
    public void run() {
	try {
	    System.out.println("Babel Listener: Thread run()....");
	    while (true) {
		//ctx.getLogger().info("Babel Listener: Waiting for requests...");
		int num = this.ioSelector.select(20);
		if (num == 0) {
		} else {
		    Set<SelectionKey> keys = ioSelector.selectedKeys();
		    System.out.println("BABEL: got some thing...");
		    for (Iterator iter = keys.iterator(); iter.hasNext(); ) {
			SelectionKey key = (SelectionKey) iter.next();
			iter.remove();
			BabelClient client = (BabelClient) key.attachment();
			if (key.isReadable()) {
			    ctx.getLogger().info("Babel Listener: client is readable");
			}
			else if (key.isWritable()) {
			    ctx.getLogger().info("Babel Listener: client is writable");
			}
			this.tpool.execute(client);
		    }
		}
		synchronized(this) {}
	    }
	}
	catch (Exception e) {
	    ctx.getLogger().severe("Error in Babel i/o polling thread", e);
	    System.exit(-1);
	}
    }
    
    public void processConnection() {
	try {
	    synchronized (this) {
		ioSelector.wakeup();
		SocketChannel sc = this.channel.accept();
		ctx.getLogger().finest("Babel Listener: Have got new request, processing...");
		BabelClient c = new BabelClient(sc, this.ioSelector, 0);
		this.tpool.execute(c);
	    }
	}
	catch (Exception e) {
	    ctx.getLogger().severe("Error while accepting new client connection", e);
	}
    }
    
}
