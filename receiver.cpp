#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <ctime>
#include <strings.h>
#include <string.h>
#include <string>
#include <fstream>
#include <iostream>

#include "packet.h"

using namespace std;

int main(int argc, char *argv[])
{
	int sockfd, portno, bytelen;
	struct sockaddr_in serv_addr, client_addr;
	socklen_t serv_len = sizeof(serv_addr);
	struct hostent *server;
	string filename;

	if (sockfd = socket(AF_INET, SOCK_DGRAM, 0) < 0)
	{
		fprintf(stderr, "Error: opening socket\n");
		exit(1);
	}

	if (server = gethostbyname(argv[1]) == NULL)
	{
		fprintf(stderr, "Error: no host found\n");
		exit(1);
	}

	portno = atoi(argv[2]);
	filename = argv[3];

	bzero((char *) &serv_addr, sizeof(serv_addr));		
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);

	if (sendto(sockfd, filename.c_str(), filename.length(), 0, (struct sockaddr *) &serv_addr, &serv_len) < 0)
	{
		fprintf(stderr, "Error: sendto Failed.\n");
		exit(1);
	}

	// char buffer[BUFFER_SIZE];
	// bzero(buffer, BUFFER_SIZE);
	

}