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
	int sockfd, portno, recvlen;
	struct sockaddr_in serv_addr, client_addr;
	socklen_t serv_len = sizeof(serv_addr);
	struct hostent *server;
	string filename;

	if (argc < 4)
	{
		fprintf(stderr, "Error: incorrect number of arguments\n");
		fprintf(stderr, "receiver <sender hostname> <sender portnumber> <filename>\n");
		exit(1);
	}

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		fprintf(stderr, "Error: opening socket\n");
		exit(1);
	}

	if ( (server = gethostbyname(argv[1])) == NULL)
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

	if ( (sendto(sockfd, filename.c_str(), filename.length(), 0, (struct sockaddr *) &serv_addr, serv_len)) < 0)
	{
		fprintf(stderr, "Error: sendto Failed.\n");
		exit(1);
	}

	char buffer[BUFFER_SIZE];
	bzero(buffer, BUFFER_SIZE);

	recvlen = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &serv_addr, &serv_len);
	unsigned int file_size = stoul(buffer, NULL, 0); 

	printf("File size: %d bytes\n", file_size);

	Packet ack;
	bzero(&ack, PACKET_SIZE);

	// If file size received, send ACK 0
	if (recvlen > 0)
	{
		initialize_packet(&ack, 0, 0, serv_addr, portno, ACK_PACKET);

		if ( (sendto(sockfd, (char *) &ack, PACKET_SIZE, 0, (struct sockaddr *) &serv_addr, serv_len)) < 0)
		{
			fprintf(stderr, "Error: sendto Failed.\n");
			exit(1);
		}
	}

	Packet file_packet;
	bzero(&file_packet, PACKET_SIZE);

	int file_offset = 0;

	// Open file to be written to
	ofstream file;
	file.open("test_directory/test");

	while (1)
	{
		printf("Waiting on server\n");
		recvlen = recvfrom(sockfd, (char *) &file_packet, PACKET_SIZE, 0, (struct sockaddr *) &serv_addr, &serv_len);
		printf("received %d bytes\n", recvlen);
		if (recvlen > 0)
		{
			file_offset += file_packet.size_or_offset;
			if (file_offset >= file_size) file_offset = file_size;

			packet_dump(&file_packet);

			file << file_packet.payload;

			initialize_packet(&ack, file_packet.num + 1, file_offset, serv_addr, portno, ACK_PACKET);

			if ( (sendto(sockfd, (char *) &ack, PACKET_SIZE, 0, (struct sockaddr *) &serv_addr, serv_len)) < 0)
			{
				fprintf(stderr, "Error: sendto Failed.\n");
				exit(1);
			}
		}
		if (file_offset == file_size)
		{
			printf("Finished receiving file\n");
			file.close();
			exit(0);
		}

	}







}