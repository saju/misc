/* Create, manage and delete sessions */

package serverfmwk.session;

import java.util.concurrent.ConcurrentHashMap;
import javax.crypto.KeyGenerator;
import java.security.InvalidParameterException;

import serverfmwk.Context;
import serverfmwk.error.COREException;

public class SessionManager {
    private Context ctx;
    private ConcurrentHashMap<Integer, Session> sessionMap;
    private byte keySize;
    private Integer counter;

    public SessionManager(Context ctx) throws Exception {
	this.ctx = ctx;
	this.sessionMap = new ConcurrentHashMap<Integer, Session>();
	this.counter = 0;

	_getKeyStrength();
    }

    private void _getKeyStrength() throws Exception {
	KeyGenerator kgen = KeyGenerator.getInstance("AES");
	byte bytes = 0;

	try {
	    kgen.init(256);
	    bytes = 3;
	} catch(InvalidParameterException p0) {
	    try {
		kgen.init(192);
		bytes = 2;
	    } catch(InvalidParameterException p1) {
		try {
		    kgen.init(128);
		    bytes = 1;
		} catch(InvalidParameterException p2) {
		    throw new COREException("AES key strength is too low. Check your Java Security documentation", 
					    p2);
		}
	    }
	}
	
	this.keySize = bytes;
    }
    
    private Integer _uniqueId() {
	return this.counter++;
    }

    public Session createSession() throws Exception {
	Integer id = _uniqueId();
	Session session = new Session(id, this.keySize);

	this.sessionMap.put(id, session);
	return session;
    }

    private Session getSession(Integer id) {
	return this.sessionMap.get(id);
    }
}
	