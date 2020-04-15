/**
Nicholas Bindela
CloudFlare Software Engineering Application
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char **argv){

  struct hostent *host_name;
  char *IP;

  if(argc < 2){
    perror("Incorrect number of arguments\nUsage: ./ping [hostname or IP]");
    exit(1);
  }
  if((host_name = gethostbyname(argv[1])) != NULL){
    printf("Hostname is %s\n",host_name->h_name);
  }
  else{
    printf("Could not resolve hostname");
  }

  IP = inet_ntoa(*((struct in_addr*)host_name->h_addr_list[0]));
  printf("Host IP is: %s\n", IP);

  return 0;
}
