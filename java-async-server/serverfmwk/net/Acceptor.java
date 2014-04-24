package serverfmwk.net;

import java.util.Set;
import java.io.IOException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ClosedChannelException;

import serverfmwk.Context;

public class Acceptor implements Runnable {
  private Context ctx;
  private Selector selector;

  public Acceptor(Context ctx) throws IOException, ClosedChannelException {
    this.ctx = ctx;
    
    /* setup the selector */
    this.selector = Selector.open();

    /* register our channels */
    for (Listener l : ctx.getListeners()) {
	SelectionKey k = l.getChannel().register(this.selector, SelectionKey.OP_ACCEPT, l);
	l.setKey(k);
    }
  }

  public void run() {
    while (true) {
      int num;
      try {
	num = this.selector.select();
      } catch (IOException io) {
	ctx.getLogger().severe("acceptor thread failed in select()", io);
	return;
      }
	
      if (num == 0) {
	/* XXX: check if were woken up by someone */
      } else {
	Set<SelectionKey> keys = this.selector.selectedKeys();
	for (SelectionKey key : keys) {
	  if (key.isAcceptable()) {
	    keys.remove(key);
	    Listener listener = (Listener)key.attachment();
	    ctx.getLogger().finer("Acceptor: processing the connection...");
	    listener.processConnection();
	  }
	}
      }
    }
  }
}
