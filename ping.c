/**
Nicholas Bindela
CloudFlare Software Engineering Application
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
<<<<<<< HEAD
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
=======
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
>>>>>>> 4507c8dd2ddd3ea219831d5902acb0847409ad22

#define BUF_SIZE 500

<<<<<<< HEAD
int
main(int argc, char *argv[])
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s;
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    ssize_t nread;
    char buf[BUF_SIZE];

    if (argc != 2) {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(NULL, argv[1], &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully bind(2).
       If socket(2) (or bind(2)) fails, we (close the socket
       and) try the next address. */
=======
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
>>>>>>> 4507c8dd2ddd3ea219831d5902acb0847409ad22

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype,
                rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0){
            printf("Success\n");
            break;                  /* Success */
          }

        close(sfd);
    }

    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);           /* No longer needed */
    return 0;
  }
