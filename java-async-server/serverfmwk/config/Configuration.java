package serverfmwk.config;

import java.io.File;
import java.io.FileInputStream;
import java.util.Properties;

import serverfmwk.error.COREException;
import serverfmwk.error.Errors;

public class Configuration {
    private final static int SECURE_PORT = 9000;
    private final static int BABEL_PORT  = 9001;
    private final static String LOG_FILE = "server.log";
    private final static String LOG_LEVEL = "INFO";
    private final static String KEY_STORE = "keys";
    private final static String KEY_PASSWORD = "welcome";
    private final static int SECURE_QUEUE_SIZE = 5;
    private final static int BABEL_QUEUE_SIZE = 20;
    private final static int SECURE_MAX_POOL_SIZE = 10;
    private final static int BABEL_MAX_POOL_SIZE = 40;
    private final static int BABEL_CORE_POOL_SIZE = 10;
    private final static int SECURE_CORE_POOL_SIZE = 5;
    private final static String APP_FILE = "od.swf";

    public final static int SECURE_TYPE = 0;
    public final static int BABEL_TYPE = 1;

    private int securePort;
    private int babelPort;
    private String logFile;
    private String logLevel;
    private char[] keyPass;
    private String keyFile;
    private int secureQueueSize;
    private int babelQueueSize;
    private int secureCorePoolSize;
    private int babelCorePoolSize;
    private int secureMaxPoolSize;
    private int babelMaxPoolSize;
    private String appFile;

    private Properties props;

    public Configuration(String filename) throws Exception {
	if (filename == null)
	    loadDefaults();
	else {
	    try {
		loadConfigFile(filename);
	    } catch(Exception e) {
		throw new COREException("Configuration load error", e);
	    }
	}
    }

    public String getLogFile() {
	return logFile;
    }

    public String getLogLevel() {
	return logLevel;
    }

    public int getSecurePort() {
	return securePort;
    }

    public int getBabelPort() {
	return babelPort;
    }

    public String getKeyStore() {
	return keyFile;
    }

    public char[] getKeyPassword() {
	return keyPass;
    }


    public int getQueueSize(int type) {
	if (type == SECURE_TYPE)
	    return this.secureQueueSize;
	else if (type == BABEL_TYPE)
	    return this.babelQueueSize;
	else
	    return 0;
    }

    public int getMaxPoolSize(int type) {
	if (type == SECURE_TYPE)
	    return this.secureMaxPoolSize;
	else if (type == BABEL_TYPE)
	    return this.babelMaxPoolSize;
	else
	    return 0;
    }

    public int getCorePoolSize(int type) {
	if (type == SECURE_TYPE)
	    return this.secureCorePoolSize;
	else if (type == BABEL_TYPE)
	    return this.babelCorePoolSize;
	else
	    return 0;
    }

    public String getAppFile() {
	return this.appFile;
    }

    private void loadDefaults() {
	this.securePort         = SECURE_PORT;
	this.babelPort          = BABEL_PORT;
	this.logFile            = LOG_FILE;
	this.logLevel           = LOG_LEVEL;
	this.keyFile            = KEY_STORE;
	this.keyPass            = KEY_PASSWORD.toCharArray();
	this.secureQueueSize    = SECURE_QUEUE_SIZE;
	this.babelQueueSize     = BABEL_QUEUE_SIZE;
	this.secureMaxPoolSize  = SECURE_MAX_POOL_SIZE;
	this.babelMaxPoolSize   = BABEL_MAX_POOL_SIZE;
	this.secureCorePoolSize = SECURE_CORE_POOL_SIZE;
	this.babelCorePoolSize  = BABEL_CORE_POOL_SIZE;
	this.appFile            = APP_FILE;
    }

    private void loadConfigFile(String filename) throws Exception {
	File configFile = new File(filename);
	if (!configFile.exists() || !configFile.isFile()) 
	    throw new COREException(filename, Errors.CONFIG_FILE_INVALID_ON_DISK);

	if (!configFile.canRead()) 
	    throw new COREException(filename, Errors.CONFIG_FILE_BAD_PERMS_ON_DISK);

	props = new Properties();
	props.load(new FileInputStream(filename));

	/* fetch properties we want, if not found set to default */
	this.securePort         = getIntValue("SECURE_PORT", SECURE_PORT);
	this.babelPort          = getIntValue("BABEL_PORT", BABEL_PORT);
	this.logFile            = getStringValue("LOG_FILE", LOG_FILE);
	this.logLevel           = getStringValue("LOG_LEVEL", LOG_LEVEL);
	this.keyFile            = getStringValue("KEY_STORE", KEY_STORE);
	this.keyPass            = getStringValue("KEY_PASSWORD", KEY_PASSWORD).toCharArray();
	this.secureQueueSize    = getIntValue("SECURE_QUEUE_SIZE", SECURE_QUEUE_SIZE);
	this.babelQueueSize     = getIntValue("BABEL_QUEUE_SIZE", BABEL_QUEUE_SIZE);
	this.secureMaxPoolSize  = getIntValue("SECURE_MAX_POOL_SIZE", SECURE_MAX_POOL_SIZE);
	this.babelMaxPoolSize   = getIntValue("BABEL_MAX_POOL_SIZE", BABEL_MAX_POOL_SIZE);
	this.secureCorePoolSize = getIntValue("SECURE_CORE_POOL_SIZE", SECURE_CORE_POOL_SIZE);
	this.babelCorePoolSize  = getIntValue("BABEL_CORE_POOL_SIZE", BABEL_CORE_POOL_SIZE);
	this.appFile            = getStringValue("APP_FILE", APP_FILE);
    }

    private String getStringValue(String key, String defaultValue) {
	String x = this.props.getProperty(key);
	if (x == null)
	    return defaultValue;
	else
	    return x;
    }

    private int getIntValue(String key, int defaultValue) {
	String x = this.props.getProperty(key);
	if (x == null) 
	    return defaultValue;
	else 
	    return Integer.parseInt(x);
    }
}
