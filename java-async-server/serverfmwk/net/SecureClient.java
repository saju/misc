/*
  generic tls/ssl server
  
  this class implements a state transition engine that can switch states within the same thread or
  over thread boundaries. In this implementation all i/o state switches are done over thread boundaries 
  thus allowing a small thread pool to manage a large number of tls/ssl speaking clients.

  this class uses an opaque Protocol object that can implement any higher level protocol (for eg http)
*/
package serverfmwk.net;

import javax.net.ssl.SSLEngine;
import javax.net.ssl.SSLException;
import javax.net.ssl.SSLEngineResult;
import javax.net.ssl.SSLEngineResult.Status;
import javax.net.ssl.SSLEngineResult.HandshakeStatus;
import java.nio.channels.SelectionKey;
import java.nio.channels.SocketChannel;
import java.nio.channels.Selector;
import java.nio.ByteBuffer;
import java.io.IOException;

public class SecureClient extends Client {

    private int me;

    private SSLEngine engine;
    private Protocol protocol;
    private Object   context;
    private SelectionKey key;   /* the selection key for this channel */
    
    private int state;
    private boolean shookHands;
    private boolean shutdown;

    private ByteBuffer inApp;
    private ByteBuffer outApp;
    private ByteBuffer inNw;
    private ByteBuffer outNw;
    private ByteBuffer blank;

    /* states */
    private final static int UNWRAP_DATA    = 0;
    private final static int WRAP_DATA      = 1;
    private final static int READ_DATA      = 2;
    private final static int WRITE_DATA     = 3;
    private final static int PEER_CLOSE     = 4;
    private final static int PROTOCOL_PARSE = 5;
    private final static int PEER_READABLE  = 6;
    private final static int PEER_WRITEABLE = 7;
    private final static int RUN_TASKS      = 8;
    private final static int DROP_THRU      = 9;
    private final static int DO_CLOSE       = 10;

    public SecureClient(SocketChannel channel, SSLEngine engine, Selector selector, Protocol protocol, int me) throws Exception {
	super(channel);

	this.engine = engine;

	/* non blocking server mode */
	this.channel.configureBlocking(false);
	this.engine.setUseClientMode(false);

	this.inApp  = ByteBuffer.allocate(engine.getSession().getApplicationBufferSize());
	this.outApp = ByteBuffer.allocate(engine.getSession().getApplicationBufferSize());
	this.inNw   = ByteBuffer.allocate(engine.getSession().getPacketBufferSize());
	this.outNw  = ByteBuffer.allocate(engine.getSession().getPacketBufferSize());
	this.blank  = ByteBuffer.allocate(0);
	
	this.context = protocol.getNewContext();
	this.protocol = protocol;
	this.shookHands = false;
	this.shutdown   = false;

	/* we are a server so we start by reading data */
	this.state = UNWRAP_DATA;
	this.key = channel.register(selector, 0, this);
	this.outApp.flip();

	this.me = me;
    }

    /* 
       the actual state transition code. 

       the do-while loop is an optimization. functions can decide to stay in the same thread of execution by
       not breaking out of the loop. currently the i/o and close functions break out of the loop. This causes
       the current task to finish and will cause the thread to either terminate or go back to the thread pool.
       The SecureListener's selector thread re-submits this task back to the thread pool on a i/o event, thus
       continuing the session 
    */
    public void run() {
	boolean loop = false;
	try {
	    do {
		if (state == UNWRAP_DATA) {
		    loop = unwrap();
		} else if (state == WRAP_DATA) {
		    loop = wrap();
		} else if (state == READ_DATA) {
		    loop = readPeer();
		} else if (state == WRITE_DATA) {
		    loop = writePeer();
		} else if (state == PEER_CLOSE) {
		    loop = peerClose();
		} else if (state == PROTOCOL_PARSE) {
		    loop = protocolParse();
		} else if (state == PEER_READABLE) {
		    loop = _readPeer();
		} else if (state == PEER_WRITEABLE) {
		    loop = _writePeer();
		} else if (state == RUN_TASKS) {
		    loop = runTasks();
		} else if (state == DO_CLOSE) {
		    loop = doClose();
		} else 
		    loop = false;
	    } while (loop);
	    
	} catch (Exception e) {
	    try {
		if (channel.isOpen())
		    channel.close();
	    } catch(Exception e1) {}
	}
    }

    /* called from the SecureListener, marks state to readable so when another thread picks up 
       this task it will know that it should read() */
    public void markReadable() {
	state = PEER_READABLE;
	key.interestOps(0);
    }

    public void markWriteable() {
	state = PEER_WRITEABLE;
	key.interestOps(0);
    }

    public boolean _readPeer() throws Exception {
	int bytes = channel.read(inNw);
	if (bytes == -1) { /* EOF */
	    state = PEER_CLOSE;
	} else
	    state = UNWRAP_DATA;
	return true;
    }

    public boolean _writePeer() throws Exception {
	int to_write = outNw.remaining();
	int bytes = channel.write(outNw);

	if (shutdown) {
	    channel.close();
	    state = DROP_THRU;
	    return false;
	} else {
	    state = PROTOCOL_PARSE;
	    return true;
	}
    }

    /* tasks are currently run from the same thread. It is not worth the effort to 
       scatter tasks in the thread pool and synchronize the result.*/
    private boolean runTasks() throws Exception {
	Runnable task;
	while((task = engine.getDelegatedTask()) != null)
	    task.run();
	state = UNWRAP_DATA;
	return true;
    }
    
    /* protocol handover. Decrypted data is supplied to the protocol engine. Protocol can 
       choose to ask us for more data, write some protocol data back to us or choose to do 
       a protocol level close */
    private boolean protocolParse() throws Exception {
	inApp.flip(); 
	outApp.compact();
	int action = protocol.parse(inApp, outApp, this.context);
	outApp.flip(); inApp.compact();

	if (action == Protocol.ReadData) 
	    state = UNWRAP_DATA;
	else if (action == Protocol.WriteData)
	    state = WRAP_DATA;
	else if (action == Protocol.Done)
	    state = DO_CLOSE;
	return true;
    }

    /*
      SSL/TLS level close. Once we close the engine's outbound stream, we still need to send
      the close_notify out */
    private boolean doClose() throws Exception {
	engine.closeOutbound();
	state = WRAP_DATA;
	shutdown = true;
	return true;
    }
    
    /*
      called when the remote peer closed before we did
    */
    private boolean peerClose() throws Exception {
	if (shutdown) {
	    if (channel.isOpen())
		channel.close();
	    state = DROP_THRU;
	    return false;
	}
	shutdown = true;
	boolean bad_close = false;
	/* peer closed the transport */
	try {
	    engine.closeInbound();
	} catch (SSLException e) {
	    bad_close = true; /* transport level closure without a SSL level closure - bastard */
	}

	engine.closeOutbound();
	if (!bad_close && !this.channel.socket().isOutputShutdown()) {
	    state = WRAP_DATA;
	    return true;
	} 
	
	channel.close();
	state = DROP_THRU;
	return false;
    }
    
    private boolean readPeer() throws Exception {
	/* if we have data in the buffers use it */
	if (inNw.position() != 0)  {
	    state = UNWRAP_DATA;
	    return true;
	} else {
	    /* mark key for readability and finish this thread */
	    state = DROP_THRU;
	    key.interestOps(SelectionKey.OP_READ);
	    return false;
	}
    }

    private boolean writePeer() throws Exception {
	/* mark key for writeability */
	key.interestOps(SelectionKey.OP_WRITE);
	state = DROP_THRU;
	return false;
    }

    /*  regular synchronous write for use in handshaking */
    private void syncWritePeer() throws Exception {
	channel.write(outNw);
    }

    /* encryption */
    private boolean wrap()  throws Exception {
	SSLEngineResult result;

	if (!shookHands || shutdown) {
	    /* we are handshaking or shutting down. engine will generate own data */
	    outNw.clear();
	    result = engine.wrap(blank, outNw);
	    outNw.flip();

	    switch (result.getStatus()) {
	    case BUFFER_UNDERFLOW:
		/* An underflow during handshake is a bug in the engine. Anyway we will try an half hearted attempt
		   to fix the underflow */
		state = WRAP_DATA;
		return true;
	    case BUFFER_OVERFLOW:
		throw new SSLException("Unexpected buffer overflow in wrap()");
	    case CLOSED:
		state = PEER_CLOSE;
		return true;
	    case OK:
		/* XXX: we are performing a synchronous write() here. It is not easy to perform the write() in an 
		   async manner and then resume processing from this point on. */
		syncWritePeer();
		switch (result.getHandshakeStatus()) {
		case NOT_HANDSHAKING:
		    throw new SSLException("SSLEngine is fubar, should be handshaking but doesn't want to");
		case NEED_UNWRAP:
		    state = UNWRAP_DATA;
		    return true;
		case NEED_WRAP:
		    state = WRAP_DATA;
		    return true;
		case FINISHED:
		    /* finished handshaking, protocol can decide if it wants to read() or write() */
		    state = PROTOCOL_PARSE;
		    shookHands = true;
		    return true;
		}
	    }
	} else { 
	    /* handshake is done, this data should go to client */
	    outNw.clear();
	    result = engine.wrap(outApp, outNw);
	    outNw.flip();

	    switch (result.getStatus())  {
	    case BUFFER_OVERFLOW:
		throw new SSLException("Unexpected Buffer overflow in wrap()");
	    case BUFFER_UNDERFLOW:
		/* ask protocol for more data */
		state = PROTOCOL_PARSE;
		return true;
	    case CLOSED:
		state = PEER_CLOSE;
		return true;
	    case OK:
		state = WRITE_DATA;
		return true;
	    }
	}
	return true;
    }

    /* decryption */
    private boolean unwrap() throws Exception {
	inNw.flip();
	SSLEngineResult result = engine.unwrap(inNw, inApp);
	inNw.compact();

	switch (result.getStatus()) {
	    
	case BUFFER_UNDERFLOW:
	    /* read data off n/w */
	    state = READ_DATA;
	    return true;
	case BUFFER_OVERFLOW:
	    throw new SSLException("Unexpected Buffer overflow in unwrap()");
	case CLOSED:
	    state = PEER_CLOSE;
	    return true;
	case OK:
	    if (!shookHands) {
		/* we are in a handshake */
		switch (result.getHandshakeStatus()) {
		case NOT_HANDSHAKING:
		    throw new SSLException("SSLEngine is fubar, should be handshaking but doesn't want to");
		case NEED_UNWRAP:
		    state = UNWRAP_DATA;
		    return true;
		case NEED_WRAP:
		    state = WRAP_DATA;
		    return true;
		case NEED_TASK:
		    state = RUN_TASKS;
		    return true;
		case FINISHED:
		    shookHands = true;
		    state = PROTOCOL_PARSE;
		    return true;
		} 
	    } else { /* we are done handshaking, we can consume this data */
		state = PROTOCOL_PARSE;
		return true;
	    }
	}
	return true;
    }
}
