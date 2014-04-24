/* 
   less than half @ss implementation of a rfc 2616 server. Handles known URL's and doesn't allow 
   persistent connections - we don't need them

   Note that the protocol engine is designed to be thread safe and shared across multiple clients/threads.
   The per client session is actually stored in the Context object (look at getNewContext())

   XXX: this should be an abstract class with the actual protocol impl's extending this class
*/

package serverfmwk.net;

import java.nio.ByteBuffer;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.FileInputStream;

import serverfmwk.Context;
import serverfmwk.session.SessionManager;
import serverfmwk.session.Session;

public class Protocol {
    private Context ctx;
    private static byte[] babelAddr;

    public final static int ReadData  = 0;
    public final static int WriteData = 1;
    public final static int Done  = 2;
    public final static int None = 3;
    public final static String INDEX = "/";
    public final static String APP = "/app";
    public final static String KEY = "/key";
    private static byte []app;

    public final static String index_page = 
	"<html><title>ServerFmwk</title></html>     " +
	"<body><center><h2>ServerFmwk - It works</h2></center> " +
	"<p><center>Machete dont tweet</center></p> " +
	"</body></html>";

    public final static String app_page =
	"<html><title>ServerFmwk</title></html>" +
	"<body><center><h2>ServerFmwk</h2></center>" +
	"<p><center>Deadlocking threads since 1854</p></center>" +
	"</body></html>";

    public final static String key_page = 
	"<html><title>ServerFmwk</title></html>" +
	"<body><center><h2>ServerFmwk</h2></center>" +
	"<p><center>Blinding the watchmaker since 1859</p></center>" +
	"</body></html>";

    /* XXX: this is ugly, protocol now knows about Server internals. It will be cleaner if Configuration implements a interface
       that can be used to transfer protocol specific config to the Protocol object, but I don't have the patience for
       it now.
    */
    public Protocol(Context ctx) throws FileNotFoundException, IOException {
	/* get the location of the application SWF, for now this is a single file */
	String fname = ctx.getConfiguration().getAppFile();
	File file = new File(fname);

	if (!file.exists())
	    throw new FileNotFoundException(fname + " does not exist");

	/* stuff the app into memory */
	int len = (int)file.length();
	if (len == 0) 
	    throw new IOException(fname + " is 0 bytes in size");

	app = new byte[len];
	FileInputStream is = new FileInputStream(file);
	is.read(app, 0, len);

	this.ctx = ctx;

	/* 
	   Get the addr the babel server is listening on. First get the ipv4 addr, if not bound then the
	   ipv6 addr. 

	   XXX: Currently we bind to all interfaces by default hence the '*'. Fix this so configuration
	   can tell you exactly which interface the Babel listener is bound to.
	*/
	babelAddr = ("*" + ":" + ctx.getConfiguration().getBabelPort()).getBytes();
    }

    public int parse(ByteBuffer in, ByteBuffer out, Object object) {
	PContext p = (PContext)object;

	if (p.done) {
	    ctx.getLogger().info("End request " + p.uri);
	    return Done;
	}

	if (p.wbuff != null) {
	    if (out.remaining() < p.wbuff.length - p.mark) {
		int len = out.remaining();
		out.put(p.wbuff, p.mark, len);
		p.mark += len;
		p.done = false;
	    } else {
		int len = p.wbuff.length - p.mark;
		out.put(p.wbuff, p.mark, len);
		p.mark = 0;
		p.wbuff = null;
		p.done = true;
	    }
	    return WriteData;
	}

	copy(p, in);

	/* See if we got enough bytes to read the request line */
	if (haveRequestLine(p)) {
	    return processRequestLine(p, out);
	} else  {
	    return ReadData;
	} 
    }

    private int processRequestLine(PContext p, ByteBuffer out) {
	int len = p.buffer.length();
	char [] seq = new char[len];
	/* we can store the bytes as chars because HTTP headers are ascii */
	p.buffer.getChars(0, len -1, seq, 0);
	
	/* check method is GET */
	if (seq[0] != 'G' || seq[1] != 'E' || seq[2] != 'T' || seq[3] != ' ') {
	    return badHttpMethod(p, out);
	}
	/* scan for the next space */
	int i;
	for (i = 4; seq[i] != ' ' && i < len; i++);
	if (seq[i] != ' ')
	    return fileNotFound(p, out);
	String uri = new String(seq, 4, i - 4);
	return serviceURI(p, uri, out);
    }

    private byte[] makeError(String code, String line) {
	return new String ("HTTP/1.1 " + code + " " + line + "\r\n" 
			   + "Connection: close\r\n\r\n" 
			   + "<html><title>" + code + " " + line + "</title>"
			   + "<body>" + code + " " + line + "</body></html>").getBytes();
    }
    
    private int badHttpMethod(PContext p, ByteBuffer out) {
	out.put(makeError("405", "Method Not Allowed"));
	p.done = true;
	return WriteData;
    }

    private int fileNotFound(PContext p, ByteBuffer out) {
	out.put(makeError("404", "File not Found"));
	p.done = true;
	return WriteData;
    }

    private byte[] addBytes(byte[] a, byte[] b) {
	int i, j;
	byte[] c = new byte[a.length + b.length];
	for (i = 0; i < a.length; i++)
	    c[i] = a[i];
	
	for (j = 0; j < b.length; j++)
	    c[i+j] = b[j];
	return c;
    }

    private String lenHeader(byte []a) {
	return "Content-length: " + a.length + "\r\n";
    }

    private String lenHeader(String a) {
	return lenHeader(a.getBytes());
    }

    private int serviceURI(PContext p, String uri, ByteBuffer out) {
	byte []page;
	String OKHeader = "HTTP/1.1 200 OK\r\nConnection: close\r\n";
	String InternalError = "HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n";
	String fini = "\r\n";

	p.uri = uri;

	if (INDEX.equals(uri)) {
	    page = (OKHeader + lenHeader(index_page) + fini + index_page).getBytes();
	    copy(p, out, page);
	} else if (APP.equals(uri)) {
	    page = addBytes((OKHeader + "Content-type: application/x-shockwave-flash\r\n" + 
			     lenHeader(app) + fini).getBytes(), app);
	    copy(p, out, page);
	} else if (KEY.equals(uri)) {
	    /* setup a session for this client */
	    SessionManager mgr = ctx.getSessionManager();
	    try {
		Session client = mgr.createSession();

		/* get the Session "Hello" message from the session object  + the addr the babel listener is bound to*/
		byte []msg = addBytes(client.getHello(), babelAddr);
		page = addBytes((OKHeader + lenHeader(msg) + fini).getBytes(), msg);
	    } catch (Exception e) {
		ctx.getLogger().severe("Unable to create new Session", e);
		page = (InternalError + fini + "Internal Error").getBytes();
	    }
	    copy(p, out, page);
	} else
	    fileNotFound(p, out);
	
	ctx.getLogger().info("Start request " + p.uri);
	return WriteData;
    }

    private boolean haveRequestLine(PContext p) {
	/* the shortest request line will be "GET / HTTP/1.1\r\n",
	   the longest will be "GET /app HTTP/1.1\r\n" */
	if ("GET / HTTP/1.1\r\n".length() > p.buffer.length()) 
	    return false;
	else 
	    return true;
    }

    private void copy(PContext p, ByteBuffer in) {
	if (in.remaining() > 0) {
	    byte []out = new byte[in.remaining()];
	    in.get(out);
	    p.buffer.append(new String(out));
	}
    }

    private void copy(PContext p, ByteBuffer out, byte[] page) {
	if (out.remaining() > page.length) {
	    out.put(page);
	    p.done = true;
	} else {
	    p.wbuff = page;
	    p.mark = out.remaining();
	    out.put(page, 0, out.remaining());
	    p.done = false;

	}
    }

    public Object getNewContext() {
	return new PContext();
    }

    public class PContext {
	public StringBuffer buffer;
	public byte[] wbuff;
	public int mark;
	public boolean done;
	public String uri;

	public PContext() {
	    done = false;
	    mark = 0;
	    wbuff = null;
	    uri = null;
	    buffer = new StringBuffer(); 
	}
    }
}
