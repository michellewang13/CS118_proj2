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
	float loss, corruption;
	struct sockaddr_in serv_addr, client_addr;
	socklen_t serv_len = sizeof(serv_addr);
	struct hostent *server;
	string filename;

	if (argc != 6)
	{
		fprintf(stderr, "Error: incorrect number of arguments\n");
		fprintf(stderr, "receiver <sender hostname> <sender portnumber> <filename> <loss probability> <corruption probability>\n");
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
	loss = atof(argv[4]);
	corruption = atof(argv[5]);

	bzero((char *) &serv_addr, sizeof(serv_addr));		
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);

	struct timeval tv;
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) 
	{
	    fprintf(stderr, "Error in timeout\n");
	}


	// Send filename
	if ( (sendto(sockfd, filename.c_str(), filename.length(), 0, (struct sockaddr *) &serv_addr, serv_len)) < 0)
	{
		fprintf(stderr, "Error: sendto Failed.\n");
		exit(1);
	}

	char buffer[BUFFER_SIZE];
	bzero(buffer, BUFFER_SIZE);

	// Receive file size
	recvlen = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &serv_addr, &serv_len);
	unsigned int file_size = stoul(buffer, NULL, 0); 

	printf("Received file size: %d bytes\n", file_size);

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
		printf("Sent ACK 0 back to server\n\n");
	}

	Packet file_packet;
	bzero(&file_packet, PACKET_SIZE);

	int file_offset = 0;
	int prev_offset = 0;
	unsigned int expected_packet = 0;

	// Open file to be written to
	ofstream file;
	file.open("test_directory/test");

	int i = 0;

	while (1)
	{
		printf("Waiting on server\n\n");
		recvlen = recvfrom(sockfd, (char *) &file_packet, PACKET_SIZE, 0, (struct sockaddr *) &serv_addr, &serv_len);
		if (recvlen > 0)
		{

			if ( (random() % 100) < (loss * 100) )
			{
				printf("File packet %d loss: Discarding...\n", file_packet.num); 
				continue;
			}
			if ( (random() % 100) < (corruption * 100) )
			{
				printf("File packet %d corrupted: Discarding...\n", file_packet.num);
				continue;
			}

			if (expected_packet == file_packet.num)
			{

				file_offset += file_packet.size_or_offset;
				if (file_offset >= file_size) file_offset = file_size;

				expected_packet++;

				printf("Sending ACK %d\n\n", expected_packet);

				printf("Received file packet:\n");
				packet_dump(&file_packet);
				file << file_packet.payload;

				// send ack back to server
				initialize_packet(&ack, expected_packet, file_offset, serv_addr, portno, ACK_PACKET);

				if ( (sendto(sockfd, (char *) &ack, PACKET_SIZE, 0, (struct sockaddr *) &serv_addr, serv_len)) < 0)
				{
					fprintf(stderr, "Error: sendto Failed.\n");
					exit(1);	
				}
			}
			else
			{
				// Send duplicate ACK back to server
				printf("Out of order file packet %d, expected %d\n", file_packet.num, expected_packet);
				printf("Sending ACK %d\n", expected_packet);
				initialize_packet(&ack, expected_packet, file_offset, serv_addr, portno, ACK_PACKET);

				if ( (sendto(sockfd, (char *) &ack, PACKET_SIZE, 0, (struct sockaddr *) &serv_addr, serv_len)) < 0)
				{
					fprintf(stderr, "Error: sendto Failed.\n");
					exit(1);	
				}
				packet_dump(&ack);
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