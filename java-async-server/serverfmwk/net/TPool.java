/*
  We need to roll our own ThreadPool because
  1. The JDK threadpool executor prefers keeping tasks in Queue when core_threads are busy.
  2. The threadpool executor does not create new spare_threads until the queue is full.
  3. The threadpool fails requests when there are no free threads available.
  
  We want to give preference to the tasks and not threads
  1. Our threadpool prefers creating spare_threads instead to keeping tasks in the Queue.
  2. Our threadpool keeps spare_threads running for a cooldown time after which they die.
  3. We reject tasks only when the queue is full, regardless if all spare_threads are busy - queue is RAM, we can use more
     RAM (large queues) but less CPU (less threads).

  This code is structured pretty much like Doug Lea's original PooledExecutor except for the points noted above.
*/

package serverfmwk.net;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;

public class TPool {
  private int idleWorker;
  private int poolSize;
  private int coreSize;
  private int maxSize;
  private long coolDown;
  private TimeUnit unit;
  private BlockingQueue<Runnable> queue;
  private ThreadFactory tFactory;
  private RejectedTaskHandler handler;

  public TPool(int coreSize, int maxSize, long coolDown, TimeUnit unit, BlockingQueue<Runnable> queue,
	       ThreadFactory tFactory, RejectedTaskHandler handler) 
    throws IllegalArgumentException, NullPointerException {

    if (coreSize < 0 || maxSize <= 0 || coreSize > maxSize || coolDown < 0) 
      throw new IllegalArgumentException();
    
    if (handler == null || tFactory == null || queue == null) 
      throw new NullPointerException();
    
    this.coreSize = coreSize;
    this.maxSize = maxSize;
    this.coolDown = coolDown;
    this.unit = unit;
    this.queue = queue;
    this.tFactory = tFactory;
    this.handler = handler;
    this.poolSize = 0;
    this.idleWorker = 0;

    /* start the core number of threads, if the queue is non empty these threads will start working immediately */
    while (this.poolSize < this.coreSize)
      newThread();
  }

  private synchronized void newThread() {
    this.poolSize++;
    this.tFactory.newThread(new Worker()).start();
  }

  private synchronized void threadFinished() {
      this.poolSize--; 

      /* maintain coreSize threads */
      while (this.poolSize < this.coreSize) {
	newThread();
      }
  }

  /* this may block indefinitely for coreSize threads. Spare threads will die if they don't get
     any new work within coolDown time */
  private Runnable newTask() throws InterruptedException {
    long tout = 0;

    synchronized(this) {
      if (this.poolSize > this.coreSize)
	tout = this.coolDown;
      this.idleWorker++;
    }
  
    Runnable task;
    try {
      if (tout > 0) 
	task = (Runnable)this.queue.poll(tout, this.unit);
      else 
	task = (Runnable)this.queue.take();
    }
    finally {
      synchronized(this) { this.idleWorker--; }
    }
  
    return task;
  }

  /* if this were a generic threadpool we should synchronize this function, but we know
     that this will only get called from 1 thread */
  public void execute(Runnable task) {
    if (this.queue.remainingCapacity() == 0) {
      /* reject this task */
      this.handler.rejectedTask(task, null);
      return;
    }

    
    if (this.idleWorker == 0 && this.poolSize < this.maxSize) 
	newThread();

    boolean inserted = false;
    Exception e = null;
    try {
      inserted = this.queue.offer(task);
    } catch (Exception ex) {
      e = ex;
    }
    if (!inserted)
      this.handler.rejectedTask(task, e);
  }

    /* the wrapper for running tasks */
  public class Worker implements Runnable {
    private Runnable work;

    public Worker() {
      this.work = null;
    }

    public void run() {
      Runnable task = this.work;
      this.work = null;

      try {
	if (task != null) {
	  task.run();
	  task = null;
	}
	
	while((task = newTask()) != null) {
	  task.run();
	  task = null;
	}
      } catch(InterruptedException ie) {}
      finally {
	threadFinished();
      }
    }
  }
}
