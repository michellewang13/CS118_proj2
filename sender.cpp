#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h> 
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <fstream> 
#include <sstream>

#include "packet.h"

using namespace std;

void parse(int sockfd, int portno, struct sockaddr_in &client, socklen_t len, char * buffer)
{

}

int main(int argc, char *argv[])
{
	int sockfd, portno, cwnd;
	struct sockaddr_in serv_addr, client_addr;
	socklen_t len = sizeof(client_addr);
	string filename;
	char buffer[BUFFER_SIZE];

	if (argc < 3)
	{
		fprintf(stderr, "Error: incorrect number of arguments\n");
		fprintf(stderr, "receiver <receiver_portnumber> <window_size>\n");
		exit(1);
	}

	// Save arguments to variables
	portno = atoi(argv[1]);
	cwnd = atoi(argv[2]);
	bzero(buffer, BUFFER_SIZE);

	// Open socket 
	if ( sockfd = socket(AF_INET, SOCK_DGRAM, 0) < 0 )
	{
		fprintf(stderr, "Error: opening socket\n");
		exit(1);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;	

	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		fprintf(stderr, "Error: on binding");	
		exit(1);
	}

	while (1)
	{
		printf("waiting on port\n");
		recvlen = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &client_addr, &len);
		printf("received %d bytes\n", recvlen);
		if (recvlen > 0)
		{
			buffer[recvlen] = 0;
			printf("received message: \"%s\"\n", buffer);
		}

	}	

	























}