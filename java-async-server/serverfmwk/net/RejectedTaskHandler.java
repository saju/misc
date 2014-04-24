package serverfmwk.net;

public interface RejectedTaskHandler {
    public void rejectedTask(Runnable r, Exception e);
}