package serverfmwk.net;

import java.nio.channels.SocketChannel;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.io.IOException;

public class Client implements Runnable {
  protected SocketChannel channel;
  protected String ip;
  protected int bytesSent;

  public Client(SocketChannel channel) {
    this.bytesSent = 0;
    this.channel = channel;
    this.ip = ((InetSocketAddress)(channel.socket().getRemoteSocketAddress())).getAddress().getHostAddress();
  }
  
  public int bytesSent() {
    return bytesSent;
  }

  public String getRequest() {
    return "/foo";
  }

  public String getIP() {
    return this.ip;
  }

  public SocketChannel getChannel() {
    return this.channel;
  }

  public void close() throws IOException {
    this.channel.close();
  }

  public void run() {

  }
  
}
