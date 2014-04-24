package serverfmwk.error;

public class COREException extends Exception {
  public COREException(Throwable t) {
    super(t);
  }
  
  public COREException(String msg, Throwable t) {
    super(msg, t);
  }

  public COREException(String msg) {
    super(msg);
  }

  public COREException(Errors e) {
    this(e.toString());
  }

  public COREException(Errors e, Throwable t) {
    this(e.toString(), t);
  }

  public COREException(String msg, Errors e) {
    this(msg + " : " + e.toString());
  }
}
