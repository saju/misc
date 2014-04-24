package serverfmwk;

import serverfmwk.log.ServerLogger;
import serverfmwk.net.Listener;
import serverfmwk.config.Configuration;
import serverfmwk.session.SessionManager;

public interface Context {
    /* returns the listeners for this server */
    public Listener[] getListeners();

    /* returns the logger for this server */
    public ServerLogger getLogger();

    /* returns the Configuration for this server */
    public Configuration getConfiguration();
    
    /* return the SessionManager */
    public SessionManager getSessionManager();
}
