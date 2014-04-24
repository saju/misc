/*
  The secure listener can generically handle protocols that operate over SSL/TLS.

  The secure listener listens for incoming connections; accepted connections are wrapped into SecureClient objects, their 
  socket channels are registered with a selector thread that can select for i/o. The selector code is implemented 
  by the listener itself. The SecureClient speaks SSL/TLS and offloads actual protocol handling to a Protocol object 
  (currently a http protocol object). The SecureClient objects are pushed into a ThreadPool where the SSL session is 
  managed asynchronously. The SecureListener allows a small thread pool to handle a large number of SSL clients.
*/

package serverfmwk.net;

import java.security.KeyStore;
import java.nio.channels.SocketChannel;
import java.nio.channels.Selector;
import java.nio.channels.SelectionKey;
import javax.net.ssl.KeyManager;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLEngine;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.RejectedExecutionHandler;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;
import java.util.Set;
import java.util.Iterator;

import serverfmwk.Context;
import serverfmwk.error.Errors;
import serverfmwk.error.COREException;
import serverfmwk.config.Configuration;

public class SecureListener extends Listener implements Runnable, RejectedTaskHandler, ThreadFactory, 
							Thread.UncaughtExceptionHandler {
    private SSLContext sslContext;               
    private ArrayBlockingQueue<Client> queue; /* the queue holding submitted SecureClient objects */
    private TPool tpool;                      /* the ThreadPool for this listener */
    private ThreadGroup tGroup;               
    private int tCounter = 0;                 /* static counter to uniquely names threads create by the ThreadFactory */
    private Selector ioSelector;              /* the i/o selector for this listener */
    private Protocol protocol;                /* the protocol this listener is managing */
    private int tClient = 0;

    private final static int CORE_TPOOL_SIZE = 2;

    public SecureListener(int port, int type, Context ctx) throws Exception {
	super(port, type, ctx);
	Configuration cfg = ctx.getConfiguration();

	try {
	    /* Get a Key Manager with our key info */
	    KeyStore store = KeyStore.getInstance("JKS");
	    store.load(new FileInputStream(cfg.getKeyStore()), cfg.getKeyPassword());
      
	    KeyManagerFactory kmf = KeyManagerFactory.getInstance("SunX509");
	    kmf.init(store, cfg.getKeyPassword());
      
	    /* setup an ssl context with our keys. We only speak SSLv3 and TLSv1 */
	    SSLContext sslCtx = SSLContext.getInstance("SSLv3");
	    sslCtx.init(kmf.getKeyManagers(), null, null);
	    this.sslContext = sslCtx;

	} catch (Exception e) {
	    throw new COREException(Errors.SSL_ENGINE_CONFIGURE_ERROR, e);
	}

	/* setup the ThreadGroup */
	tGroup = new ThreadGroup(getName());
	tGroup.setDaemon(true);
    
	/* setup the blocking queue */
	this.queue = new ArrayBlockingQueue(cfg.getQueueSize(this.type));

	/* setup the worker pool */
	this.tpool = new TPool(cfg.getCorePoolSize(this.type), cfg.getMaxPoolSize(this.type), 10, TimeUnit.SECONDS,
			       new ArrayBlockingQueue(cfg.getQueueSize(this.type)), this, this);

	this.protocol = new Protocol(ctx);
	this.ioSelector = Selector.open();

	/* start the async i/o thread */
	new Thread(this.tGroup, this).start();
    }

    public String getName() {
	return "Secure";
    }

    /* called when a Client cannot be processed by the Executor service */
    public void rejectedTask(Runnable r, Exception e) {
	SecureClient client = (SecureClient)r;
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
	Thread t = new Thread(this.tGroup, task, this.getName() + "--" + this.tCounter++);
	t.setUncaughtExceptionHandler(this);
	return t;
    }

    public void run() {
	/* the selector thread for all the i/o events for threads belonging to the secure listeners thread group */
	try {
	    while (true) {
		int num = this.ioSelector.select(20);
		if (num == 0) { /* why were we woken up ? */
		} else {
		    Set<SelectionKey> keys = ioSelector.selectedKeys(); 
		    ctx.getLogger().finest("SECURE LISTENER: GOT SELECTED KEYS");
		    for (Iterator iter = keys.iterator(); iter.hasNext(); ) {
			SelectionKey key = (SelectionKey)iter.next();
			iter.remove();
			SecureClient client = (SecureClient)key.attachment();
			if (key.isReadable()) 
			    client.markReadable();
			else if (key.isWritable())
			    client.markWriteable();
			ctx.getLogger().finest("SECURE LISTENER: EXECUTING THE CLIENT TASK[1]");
			this.tpool.execute(client);
		    }
		}
		synchronized(this) {}
	    }
	} catch (Exception e) {
	    ctx.getLogger().severe("Error in secure i/o polling thread", e);
	    System.exit(-1);
	}
    }

    public void processConnection() {
	try {
	    synchronized (this) {
		ioSelector.wakeup();
		SocketChannel ch = this.channel.accept();
		ctx.getLogger().finest("SecureListener: I have got a new request, processing...");
		SecureClient s = new SecureClient(ch, this.sslContext.createSSLEngine(), this.ioSelector, this.protocol,
						  this.tClient++);
		ctx.getLogger().finest("SecureListener: Executing the task...");
		this.tpool.execute(s);
	    }
	} catch (Exception e) {
	    ctx.getLogger().severe("Error while accepting new client connection", e);
	}
    }
}
