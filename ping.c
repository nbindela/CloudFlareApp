/*
 * CSCI 315 Operating Systems Design
 * Author: L. Felipe Perrone
 * Date: 2010-02-16
 * Copyright (c) 2011 Bucknell University
 *
 * Permission is hereby granted, free of charge, to any individual
 * or institution obtaining a copy of this software and associated
 * documentation files (the "Software"), to use, copy, modify, and
 * distribute without restriction, provided that this copyright
 * and permission notice is maintained, intact, in all copies and
 * supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL BUCKNELL UNIVERSITY BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wrappers.h"

#define BUFFER_SIZE 512 // length of message buffer
#define	QLEN 6   // length of request queue

#define FOREVER 1
#define TRUE 1
#define FALSE 0

int num_requests = 0;  // tally of client requests

/*------------------------------------------------------------------------
 * Program:   echod - a server for echo requests
 *
 * Purpose:   repeatedly execute the following:
 *    (0) wait for a connection request from client
 *		(1) wait for a null-terminated string from client
 *		(2) send back the same string to client
 *		(3) close the connection
 *		(4) go back to step (0)
 *
 * Usage:    echod [ port ]
 *
 *		 port  - a port number in user space
 *
 *------------------------------------------------------------------------
 */

int main (int argc, char* argv[]){

	struct sockaddr_in sin;  // structure to hold server's address
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
  memset((char *)&sin,0,sizeof(sin)); // zero out sockaddr structure

  sin.sin_family = AF_INET;	          // set family to Internet
  sin.sin_addr.s_addr = inet_addr(argv[1]);   // set the local IP address
	// verify usage

  port = 8001;
	if (port > 0)	{
		// test for illegal value
	  sin.sin_port = htons((u_short)port);}
	else {
		// print error message and exit
		fprintf(stderr,"ECHOD: bad port number %s\n", argv[1]);
		exit(-1);
	}

	// create socket

	sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sd < 0) {
		perror("ECHOD: socket creation failed");
		exit(-1);
	}

	// assign IP/port number to socket where connections come in

	if (bind(sd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		perror("ECHOD: bind failed");
		exit(-1);
	}

	// set up socket to receive incomming connections

	if (listen(sd, QLEN) < 0) {
		perror("ECHOD: listen failed");
		exit(-1);
	}

	// main server loop - accept and handle requests

	while (FOREVER) {
		alen = sizeof(cad);

		if ( (sd2 = accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
			perror("ECHOD: accept failed\n");
			exit(-1);
		}

		num_requests++;

		// receive the string sent by client
		int sizeInput;
		char output[BUFFER_SIZE];
		Recv(sd2,&sizeInput,sizeof(int),0);
		Recv(sd2,in_msg, sizeInput,0);
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
		Send(sd2, &sizeOutput,sizeof(int),0);
		Send(sd2,output,sizeOutput,0);
		printf("ECHOD sending: %s\n",output);
		output[0] = '\0';

		close(sd2);


	}
}
