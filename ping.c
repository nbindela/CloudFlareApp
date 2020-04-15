/**
Nicholas Bindela
CloudFlare Software Engineering Application
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv){

  struct hostent *host_name;

  if(argc < 2){
    perror("Incorrect number of arguments\nUsage: ./ping [hostname or IP]")
    return 1;
  }
  if((host_name = gethostbyname(argv)) != NULL){
    printf("Hostname is %s\n",*host_name->h_name);
  }

  return 0;
}
