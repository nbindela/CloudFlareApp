/*
 * Nicholas Bindela
 * CloudFlare Application
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

#define BUFFER_SIZE 512 // length of message buffer
#define	QLEN 6   // length of request queue

#define FOREVER 1
#define TRUE 1
#define FALSE 0

int num_requests = 0;  // tally of client requests

int lookuphost(const char *host, char *ip_str){
  struct addrinfo hints, *res;
  int err;
  void *ptr;
  
  memset(&hints,0,sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  
  err = getaddrinfo(host,NULL,&hints,&res);
  if(err != 0){
    perror("getaddrinfo returned an error");
    return -1;
  }

  while(res != NULL){
    inet_ntop(res->ai_family, res->ai_addr->sa_data,ip_str,100);

    switch(res->ai_family){
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
  
  // The memset call is ESSENTIAL!
  // if you don't do this every time you create a sockaddr struct, you will
  // see some pretty strange behaviour
  

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
  
  if (connect(sd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    perror("PING: connection failed");
    exit(-1);
  }
  //printf("PING %s (%s)\n",argv[1],(inet_ntop(sin.sin_addr)));
  
  // main server loop - accept and handle requests
  printf("PING %s (%s)\n",argv[1], ip_str);
  
  while (FOREVER) {
    alen = sizeof(cad);
    
    if ( (sd2 = accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
      perror("PING: accept failed\n");
      exit(-1);
    }
    
    num_requests++;
    
    // receive the string sent by client
    int sizeInput;
    char output[BUFFER_SIZE];
    recv(sd2,&sizeInput,sizeof(int),0);
    recv(sd2,in_msg, sizeInput,0);
    printf("ECHOD recievied: %s\n",in_msg);
    
    char *temp;
    
    temp = strtok(in_msg, " ");
    while(temp != NULL){
      strcat(output, temp);
      temp = strtok(NULL," ");
      if(temp != NULL){
	strcat(output," ");
      }
    }
    strcat(output,"\0");
    
    
    // send the received string back to client
    int sizeOutput = strlen(output);
    send(sd2, &sizeOutput,sizeof(int),0);
    send(sd2,output,sizeOutput,0);
    printf("ECHOD sending: %s\n",output);
    output[0] = '\0';
    
    close(sd2);
    
    
  }
}
