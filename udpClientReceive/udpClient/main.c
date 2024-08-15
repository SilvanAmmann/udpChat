#include <netdb.h>      /* network database library */
#include <sys/socket.h> /* sockets */
#include <arpa/inet.h>  /* address conversions */
#include <unistd.h>
#include <string.h>   /* memset, strlen */
#include <stdio.h>    /* printf */

#define BUFSIZE (10*1024) /* size of buffer, max 64 KByte */

static unsigned char ibuf[BUFSIZE]; /* receive buffer */
static char obuf[BUFSIZE]; /* receive buffer */

int main(void) {
  struct sockaddr_in myaddr; /* our own address */
  struct sockaddr_in remaddr; /* remote address */
  int fd, i;
  socklen_t slen=sizeof(remaddr);
  int recvlen;    /* # bytes in acknowledgment message */
  int port = 1234; /* port to be used */
  const char *msg;
  const char *host = "146.136.90.45"; /* IP of host */
  char ipBuf[64]; /* in case 'host' is a hostname and not an IP address, this will hold the IP address */

  /* create a socket */
  if ((fd=socket(AF_INET, SOCK_DGRAM, 0))==-1) {
    printf("socket created\n");
  }

  /* bind it to all local addresses and pick any port number */
  memset((char *)&myaddr, 0, sizeof(myaddr));
  myaddr.sin_family = AF_INET;
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  myaddr.sin_port = htons(0);

  if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
    perror("bind failed");
    return -1;
  }

  /* now define remaddr, the address to whom we want to send messages */
  /* For convenience, the host address is expressed as a numeric IP address */
  /* that we will convert to a binary format via inet_aton */
  memset((char *) &remaddr, 0, sizeof(remaddr));
  remaddr.sin_family = AF_INET;
  remaddr.sin_port = htons(port);
  if (inet_aton(host, &remaddr.sin_addr)==0) {
    fprintf(stderr, "inet_aton() failed\n");
    return -1;
  }

#if 0
  /* set socket receive timeout */
  struct timeval tv;
  tv.tv_sec = 5;
  tv.tv_usec = 0;
  if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
    perror("Error");
  }
#endif

  while (1){
    /* now let's send the messages */
    /*printf("Type message:");
    msg = fgets(obuf, BUFSIZE, stdin);
    if (sendto(fd, msg, strlen(msg), 0, (struct sockaddr *)&remaddr, slen)==-1) {
      perror("sendto");
      return -1;
    }*/

    /* now receive an acknowledgment from the server */
    recvlen = recvfrom(fd, ibuf, sizeof(ibuf), 0, (struct sockaddr *)&remaddr, &slen);
    if (recvlen >= 0) {
      ibuf[recvlen] = '\0'; /* expect a printable string - terminate it */
      printf("response: \n%s\n", ibuf);
    } else { /* timeout */
      printf("socket receive timeout\n");
    }
  }
  close(fd);
  return 0; /* ok */
}

