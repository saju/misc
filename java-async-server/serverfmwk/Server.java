package serverfmwk;

import serverfmwk.error.Errors;
import serverfmwk.error.COREException;
import serverfmwk.config.Configuration;
import serverfmwk.log.ServerLogger;
import serverfmwk.net.Listener;
import serverfmwk.net.Acceptor;
import serverfmwk.session.SessionManager;

public class Server implements Context, Runnable {
    private Configuration config;
    private ServerLogger logger;
    private Listener[] listeners;
    private Acceptor acceptor;
    private SessionManager sManager;
    
    public Listener[] getListeners() {
	return listeners;
    }
  
    public ServerLogger getLogger() {
	return logger;
    }
  
    public Configuration getConfiguration() {
	return config;
    }

    public SessionManager getSessionManager() {
	return sManager;
    }
	
    private static void usage() {
	System.err.println("java -jar Server.jar [path_to_conf_file]");
    }

    public static void main(String[] args) throws Exception {

	if (args != null && args.length > 1) {
	    usage();
	    System.exit(-1);
	}
    
	Server server = null;
	try {
	    server = args.length == 0 ? new Server(null) : new Server(args[0]);
	} catch (Exception j) {
	    throw new COREException(Errors.SERVER_CREATION_FAILURE, j);
	}

	try {
	    server.boot();
	} catch (Exception e) {
	    /* the following is correct. we must force an explicit exit because Server may have created
	       other user threads that need to be force killed */
	    e.printStackTrace();
	    System.exit(-1);
	}
    }

    public Server(String configFile) throws Exception {
	this.config = new Configuration(configFile);
    }

    public void boot() throws Exception {

	/* initialize logging */
	logger = new ServerLogger(config);
	    
	/* 
	   initialize networking 
      
	   We will use a plaintext listener & a secure listener for demo here.
	   The code can handle any number of configured listeners.
	*/
	listeners = new Listener[2];
	listeners[0] = Listener.getListener(config.getSecurePort(), Configuration.SECURE_TYPE, this);
	listeners[1] = Listener.getListener(config.getBabelPort(), Configuration.BABEL_TYPE, this);
    
	try {
	    for (Listener l : listeners) 
		l.bind();
	} catch (Exception e) {
	    throw new COREException(Errors.LISTENER_CONFIGURE_ERROR, e);
	}

	Runtime.getRuntime().addShutdownHook(new Thread(this));
    
	banner();

	/* setup the session manager */
	this.sManager = new SessionManager(this);

	/* start the select/accept loop */
	this.acceptor = new Acceptor(this);
	new Thread(this.acceptor).start();
    }

    public void run() {
	// XXX: acceptor.close(); - wakeup acceptor thread from select() and shut it down
	for (Listener l : listeners) {
	    try {
		l.close();
	    } catch(Exception e) {
		logger.severe(Errors.LISTENER_CLOSE_ERROR, l.getName(), e);
	    }
	}
    }

    public void banner() {
	/* tell the world what ports we are starting on */
	System.out.print("Server bound to ports");
	for (Listener l : listeners) 
	    System.out.print(" " + l.getName() + ":" + l.getPort()+",");
	System.out.println("\nServer initialised...");
	logger.info("Server initialised...");
    }
}
