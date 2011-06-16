/**
   Written for Solaris (solaris10-x86).
   
% gcc -fPIC  -g -c -Wall ld_preload_proxy.c
% gcc -shared -o libpreload_proxy.so ld_preload_proxy.o -lc
% export LD_PRELOAD=/path/to/libpreload_proxy.so
% your program [ cvs -d :pserver@@b.com:/home/foo login ]

We override the gethostbyname() and connect() from libnsl and libsocket 
in our "pre-loaded" library.

This code assumes that a calling program will first call gethostbyname() 
to resolve target host and then do a connect().

When gethostbyname() is called, the name passed in by the callee is replaced
by the name of the proxy we wish to connect to. The original gethostbyname() 
from libnsl is now called with the new arguments. The callee program will now 
attempt to connect() to the resolved IP ( but not realising that it will end 
up connecting to the HTTP proxy instead ) The original hostname is stored for 
future reference by connect().

When connect() is called, we retrieve the port number from the sockaddr_in *,
and stuff the proxy's port number in. The orignal connect() from libsocket is
now called to connect to the proxy. A HTTP CONNECT request is sent to the 
proxy to open a connection to the original (previously saved) host:port.

saju.pillai@gmail.com

**/


#include <dlfcn.h>
#include <link.h>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>

#define LIBNSL "/lib/libnsl.so.1"
#define LIBSOCK "/lib/libsocket.so.1"
#define GHNAME "gethostbyname"
#define CONNECT "connect"

#define PROXY "your.proxy.com"
#define PORT "80"

void *libnsl_handle = (void *)0, *libsock_handle = (void *)0;

struct hostent *(*o_ghname)(const char *, int, int, char **) = (void *)0;

int (*o_connect)(int, const struct sockaddr_in *, int) = (void *)0;

char actual[1024];

struct hostent *gethostbyname(const char *name, int main, int argc, 
			      char **argv)
{
  char *err;
  
  if (!libnsl_handle) {
    libnsl_handle = dlopen(LIBNSL, RTLD_LAZY);
    if (!libnsl_handle) {
      fprintf(stderr, "Failed to load %s\n", LIBNSL);
      exit(-1);
    }
  }

  if (!o_ghname) {
    o_ghname = dlsym(libnsl_handle, GHNAME);
    if ((err = dlerror()) != NULL) {
      fprintf(stderr, "Could not get %s handle: err %s\n", GHNAME, err);
      exit(-1);
    }
  }
  strcpy(actual, name);
  return (o_ghname(PROXY, main, argc, argv));
}
    

int connect(int s, const struct sockaddr_in *name, int namelen)
{
  int port, rval;
  char *err, cbuff[4096];

  if (!libsock_handle) {
    libsock_handle = dlopen(LIBSOCK, RTLD_LAZY);
    if (!libsock_handle) {
      fprintf(stderr, "Failed to load %s\n", LIBSOCK);
      exit(-1);
    }
  }

  if (!o_connect) {
    o_connect = dlsym(libsock_handle, CONNECT);
    if ((err = dlerror()) != NULL) {
      fprintf(stderr, "Could not get %s handle: err %s\n", CONNECT, err);
      exit(-1);
    }
  }
  
  if (!o_ghname) {
    fprintf(stderr, "gethostbyname() should be called atleast once"
	    " before calling connect()\n");
    exit(-1);
  }

  port = ntohs(name->sin_port);
  name->sin_port = htons(atoi(PORT));

  /**
     name->sin_addr must already be correct because we spoofed the name
     in a previous call to gethostbyname()
  **/
  rval = o_connect(s, name, namelen);
  if (rval < 0)
    return rval;
  

  /**
     send CONNECT to proxy and slurp up the response
  **/
  sprintf(cbuff,"CONNECT %s:%d HTTP/1.0\n\n", actual, port);
  write(s, cbuff, strlen(cbuff));


  /**
     proxy can take a arbitrary amount of time to come back ...
     .. do a blocking read.
  **/
  port = read(s, cbuff, 4096);
  return rval;
}
