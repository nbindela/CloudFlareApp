/*
 * Nicholas Bindela
 * CloudFlare Application
 *
 * Application to emulate the ping Unix command
 *
 * Usage: sudo ./ping [hostname or IPv4 address]
 * sudo must be used when operating with RAW sockets
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <signal.h>
#include <netinet/ip_icmp.h>

#define BUFFER_SIZE 512 // length of message buffer
#define	QLEN 6   // length of request queue

#define FOREVER 1
#define TRUE 1
#define FALSE 0

//Packet Size
#define PACKET_S 64

//Default Port
#define PORT_NO 0

//How long to sleep after finishing a ping request
#define PING_SLEEP_RATE 1000000

//Program will timeout after not recieving packets after this delay(in seconds)
#define RECV_TIMEOUT 1


// Define the Ping Loop
int loop=1;

//Struct to hold packets recieved
struct packet
{
  struct icmphdr hdr;
  char msg[PACKET_S-sizeof(struct icmphdr)];
};


/**
 *Fucntion for doing a reverse DNS lookup
 *Returns 0,-1 based on whether the function executed correctly
 */
int lookuphost(const char *host, char *ip_str){
  struct addrinfo hints, *res;
  int err;
  void *ptr;

  //Allocate memory for the address hints parameter
  memset(&hints,0,sizeof(hints));
  hints.ai_family = AF_UNSPEC; //Set family for IPv4 or IPv6

  err = getaddrinfo(host,NULL,&hints,&res); //Put address info into addrinfo struct res
  if(err != 0){
    //Failed to get ip from domain name
    perror("getaddrinfo returned an error");
    return -1;
  }

  if(res != NULL){
    inet_ntop(res->ai_family, res->ai_addr->sa_data,ip_str,100);  //Store the resulting address in the location that the ip_str points to

    switch(res->ai_family){ //Switch case for IPv4 and IPv6
    case AF_INET:
      ptr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
      break;
    case AF_INET6:
      ptr = &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr;
      break;
    }
    inet_ntop(res->ai_family,ptr,ip_str,100); //Store first ip instance
    return 0;

  }
  return 1;
}
/**
 * Function to preform a checksum on the incoming data
 * Returns the value of the checksum as an unsigned int
 */
unsigned short csum(void *data, unsigned int size){
    unsigned short *temp = (unsigned short *) data;
    unsigned int sum=0;
    unsigned short result;

    while(size > 1){
      sum += *temp++;
      if(sum >> 16){ //If the sum went overflows
        sum = (sum >> 16) + (sum & 0xFFFF); //Add the overflow
      }
      size -= 2; //2 bytes

    }
    if(size == 1){ //If odd number of bytes add last byte
      sum += *(unsigned char *)temp;
    }

    sum = (sum >> 16) + (sum & 0xFFFF); //Add overflow
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

/**
 *Function for handling interrupts
 */
void interruptHandler(int i){
  //Used to end loop when interrupted
  loop = 0;
}

/**
 *Function for handling pings and the ping loop
 *Params: ping_sd is a socket file descriptor to be used for pinging
 *ping_addr is a sockaddr_in struct that holds properties of the socket
 *ping_ip is a string that points to the ip being pinged
 *rev_host is the argument passed to the commandline that acts as out 'real value' of the host
 */
void send_ping(int ping_sd, struct sockaddr_in *ping_addr,
                char *ping_ip, char *rev_host)
{
    int ttl_val=64, msg_count=0, i, addr_len, flag=1,msg_received_count=0;

    struct packet pckt;
    struct sockaddr_in r_addr;
    struct timespec time_start, time_end, tfs, tfe;
    long double rtt_msec=0, total_msec=0;
    struct timeval tv_out;
    tv_out.tv_sec = RECV_TIMEOUT;
    tv_out.tv_usec = 0;

    clock_gettime(CLOCK_MONOTONIC, &tfs);


    // set socket options for TTL
    if (setsockopt(ping_sd, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0)
    {
        printf("\nSetting socket options error\n");
        return;
    }

    // setting timeout setting for socket
    setsockopt(ping_sd, SOL_SOCKET, SO_RCVTIMEO,(const char*)&tv_out, sizeof tv_out);

    while(loop) //Loop for recieving packets
    {
        // flag is if packet was actually sent
        flag=1;

        //get data for packet
        bzero(&pckt, sizeof(pckt));

        pckt.hdr.type = ICMP_ECHO;
        pckt.hdr.un.echo.id = getpid();

        for ( i = 0; i < sizeof(pckt.msg)-1; i++ ){
            pckt.msg[i] = i+'0';
	}

        pckt.msg[i] = 0;
        pckt.hdr.un.echo.sequence = msg_count++;
        pckt.hdr.checksum = csum(&pckt, sizeof(pckt)); //Calculate and store Check Sum on data


        usleep(PING_SLEEP_RATE);

        //send packet
        clock_gettime(CLOCK_MONOTONIC, &time_start);
        if ( sendto(ping_sd, &pckt, sizeof(pckt), 0,
           (struct sockaddr*) ping_addr,
            sizeof(*ping_addr)) <= 0)
        {
            printf("\nPacket Sending Failed!\n");
            flag=0;
        }

        //receive packet
        addr_len=sizeof(r_addr);

        if ( recvfrom(ping_sd, &pckt, sizeof(pckt), 0,
             (struct sockaddr*)&r_addr, &addr_len) <= 0
              && msg_count>1)
        {
            printf("\nPacket receive failed!\n");
        }

        else
        {
            clock_gettime(CLOCK_MONOTONIC, &time_end);

            double timeElapsed = ((double)(time_end.tv_nsec - time_start.tv_nsec))/1000000.0;
            rtt_msec = (time_end.tv_sec- time_start.tv_sec) * 1000.0 + timeElapsed;

            // if packet was not sent, don't receive
            if(flag)
            {
                if(!(pckt.hdr.type ==69 && pckt.hdr.code==0))
                {
                    printf("Error..Packet received with ICMP type %d code %d\n", pckt.hdr.type, pckt.hdr.code);
                }
                else
                {
                    printf("%d bytes from (%s) (%s) msg_seq=%d ttl=%d rtt = %Lf ms.\n", packet_S, rev_host, ping_ip, msg_count, ttl_val, rtt_msec);

                    msg_received_count++;
                }
            }
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &tfe);
    double timeElapsed = ((double)(tfe.tv_nsec - tfs.tv_nsec))/1000000.0;

    total_msec = (tfe.tv_sec-tfs.tv_sec)*1000.0+timeElapsed;

    printf("\n===%s ping statistics===\n", ping_ip);
    printf("\n%d packets sent, %d packets received, %f percent packet loss. Total time: %Lf ms.\n\n", msg_count, msg_received_count, ((msg_count - msg_received_count)/msg_count) * 100.0, total_msec);
}

int main (int argc, char* argv[]){


  char *address;
  struct sockaddr_in *sinp;


  struct sockaddr_in cad;  // structure to hold client's address
  int sd, sd2;	           // socket descriptors
  int port;		             // protocol port number
  socklen_t alen;	         // length of address
  char in_msg[BUFFER_SIZE];  // buffer for incoming message

  // prepare address data structure

  if (argc != 2) {
    printf("Usage: %s [ address ]\n", argv[0]);
    exit(-1);
  }



  //Try to get address
  unsigned char buf[sizeof(struct in_addr)];
  struct addrinfo *host;
  char ip_str[INET_ADDRSTRLEN];
  struct sockaddr_in sin;
  memset((char *)&sin,0,sizeof(sin)); // zero out sockaddr structure

  if((inet_pton(AF_INET,argv[1],buf ) > 0)){//IPV4
    sin.sin_family = AF_INET;	          // set family to IP4
    inet_pton(AF_INET,argv[1],&sin.sin_addr);   // set the local IP address
    inet_ntop(AF_INET,&sin.sin_addr,ip_str,sizeof(ip_str)); //convert ip to string format
    }
  else if(lookuphost(argv[1],ip_str) == 0){//Domain Name
    sin.sin_family = AF_INET;
    inet_pton(AF_INET,ip_str,&sin.sin_addr);   // set the local IP address
    }
  else{
    printf("Could not resolve host\n");
    exit(1);
  }
  sin.sin_port = htons(12345);
	// verify usage


  // create socket

  sd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (sd < 0) {
    perror("PING: socket creation failed");
    exit(-1);
  }

  // assign IP/port number to socket where connections come in
  signal(SIGINT,interruptHandler);

  send_ping(sd, &sin,ip_str,argv[1]);


}
