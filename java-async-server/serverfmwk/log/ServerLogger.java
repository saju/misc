package serverfmwk.log;

import java.util.logging.Logger;
import java.util.logging.Level;
import java.io.IOException;
import java.util.logging.FileHandler;
import java.util.logging.SimpleFormatter;

import serverfmwk.error.Errors;
import serverfmwk.config.Configuration;

/*
 * Logger class for Server
 * subclass of Logger provided by Java
 */
public class ServerLogger extends Logger {

    private Level serverLogLevel;         /* Log Level for the Server */

    /* add the handler to the server */
    public ServerLogger(Configuration config) throws IOException {
	/* get the logger at this level */
	super("serverfmwk.log", null);

	/* XXX */
	boolean append = true;
	
	FileHandler handler = new FileHandler(config.getLogFile(), append);
	serverLogLevel = Level.parse(config.getLogLevel());
	setLevel(serverLogLevel);

	/* Set the mode of logging : TEXT/XML
	 * XXX: Get from configuration file
	 */
	handler.setFormatter(new SimpleFormatter());
	addHandler(handler);
    }


    public void severe(String msg, Exception e) {
	log(Level.SEVERE, msg, e);
    }

    public void severe(Errors err, String msg, Exception e) {
	log(Level.SEVERE, err.toString() + ": " + msg, e);
    }

    public void log(Errors err) {
	log(serverLogLevel, err.toString());
    }

}
