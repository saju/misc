package serverfmwk.error;

public enum Errors {
    OK,
    FAIL,
    SERVER_CREATION_FAILURE,
    CONFIG_FILE_INVALID_ON_DISK,
    CONFIG_FILE_BAD_PERMS_ON_DISK,
    SSL_ENGINE_CONFIGURE_ERROR,
    LISTENER_CONFIGURE_ERROR,
    LISTENER_CLOSE_ERROR,
    SERVER_CLOSE_OK,
    SERVER_START_OK,
    ACCEPTOR_SELECT_FAILURE,
    ;

  private static String strings[] = {
    "OK",
    "Failure",
    "Failed to create Server instance",
    "Supplied config file does not exit",
    "Supplied config file cannot be read",
    "SSL Engine could not be configured",
    "Listener could not be configured",
    "Listener could not be closed properly",
    "Server shutdown completed. Bye...",
    "Server started...",
    "Acceptor thread failed while selecting listeners",
  };

  public String toString() {
    return strings[this.ordinal()];
  }
}
