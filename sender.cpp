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

void send_packet(int sockfd, int portno, struct sockaddr_in &client_addr, socklen_t len, ifstream &file, unsigned int size, int cwnd, float loss, float corruption, Packet ack_0)
{
	Packet file_packet;
	Packet ack;
	Packet prev_ack;
	unsigned int position = 0;
	unsigned int packet_num = 0;
	unsigned int expected_ack = 1;
	unsigned int file_packet_count = 0;
	unsigned int ack_packet_count = 0;
	int recvlen = -1;
	unsigned int window = cwnd;

	unsigned int max = size / PAYLOAD_SIZE;

	prev_ack = ack_0;

	struct timeval tv;
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) 
	{
	    fprintf(stderr, "Error in timeout\n");
	}

	while (1)
	{

		while (position < size)
		{
			if (packet_num == window)
			{
				window += cwnd;
				break;
			}

			cout << "Position: " << position << endl;
			bzero(&file_packet, PACKET_SIZE);
			initialize_packet(&file_packet, packet_num, PAYLOAD_SIZE - 1, client_addr, portno, FILE_PACKET);
			file.seekg(position);
			file.read(file_packet.payload, PAYLOAD_SIZE - 1);

			printf("Packet Payload: %s\n", file_packet.payload);

			printf("Sending file packet %d\n\n", packet_num);
			packet_dump(&file_packet);
			if ( (sendto(sockfd, (char *) &file_packet, PACKET_SIZE, 0, (struct sockaddr *) &client_addr, len)) < 0)
			{
				fprintf(stderr, "Error: sendto Failed.\n");
				exit(1);
			}
			packet_num++;
			position += PAYLOAD_SIZE - 1;
			file_packet_count++;
			file.clear();
			file.seekg(0, ios::beg);
		}

		ack_packet_count = 0;

		while (ack_packet_count < file_packet_count)
		{
			bzero(&ack, PACKET_SIZE);
			recvlen = recvfrom(sockfd, (char *) &ack, PACKET_SIZE, 0, (struct sockaddr *) &client_addr, &len);
			if (recvlen >= 0)
			{
				if ( (random() % 100) < (loss * 100) )
				{
					printf("ACK Packet loss: Discarding ACK %d...\n", ack.num); 
					//if (ack.num < cwnd) prev_ack = ack_0;
					ack_packet_count++;
					packet_num = prev_ack.num;
					window = packet_num + cwnd;
					position = prev_ack.size_or_offset;
				}
				else if ( (random() % 100) < (corruption * 100) )
				{
					printf("ACK Packet corrupted: Discarding ACK %d...\n", ack.num);
					//if (ack.num < cwnd) prev_ack = ack_0;
					ack_packet_count++;
					packet_num = prev_ack.num;
					window = packet_num + cwnd;
					position = prev_ack.size_or_offset;
				}
				else
				{
					printf("Received ACK:\n");
					packet_dump(&ack);
					ack_packet_count++;
					prev_ack = ack;
					packet_num = ack.num;
					position = ack.size_or_offset;
					window = ack.num + cwnd;
				}
			}
			else
			{
				break;
			}
		}

		file_packet_count = 0;
		if (position >= size) return;
	}
}

int main(int argc, char *argv[])
{
	int sockfd, portno, cwnd, recvlen;
	float loss, corruption;
	struct sockaddr_in serv_addr, client_addr;
	socklen_t len = sizeof(client_addr);
	string filename;
	char buffer[BUFFER_SIZE];

	if (argc != 5)
	{
		fprintf(stderr, "Error: incorrect number of arguments\n");
		fprintf(stderr, "sender <receiver_portnumber> <window_size> <loss probability> <corruption probability>\n");
		exit(1);
	}

	// Save arguments to variables
	portno = atoi(argv[1]);
	cwnd = atoi(argv[2]);
	loss = atof(argv[3]);
	corruption = atof(argv[4]);

	bzero(buffer, BUFFER_SIZE);

	// Open socket 
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
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
		printf("waiting on client\n");
		recvlen = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &client_addr, &len);
		printf("received %d bytes\n", recvlen);
		if (recvlen > 0)
		{
			buffer[recvlen] = 0;
			printf("received filename: \"%s\"\n", buffer);

			ifstream file (buffer);

			// find file size
			file.seekg(0, ios::end);
			unsigned int size = file.tellg();
			string size_to_send = to_string(size);

			if ( (sendto(sockfd, size_to_send.c_str(), size_to_send.length(), 0, (struct sockaddr *) &client_addr, len)) < 0)
			{
				fprintf(stderr, "Error: sendto Failed.\n");
				exit(1);
			}

			Packet ack_0;
			bzero(&ack_0, PACKET_SIZE);

			recvlen = recvfrom(sockfd, (char *) &ack_0, PACKET_SIZE, 0, (struct sockaddr *) &client_addr, &len);
			if (recvlen > 0)
			{
				packet_dump(&ack_0);

				file.seekg(0, ios::beg);

				if (file.is_open())
				{
					send_packet(sockfd, portno, client_addr, len, file, size, cwnd, loss, corruption, ack_0);
					file.close();
				}
				else
				{
					fprintf(stderr, "Error: Unable to open file %s\n", buffer);
					exit(1);
				}
			}
		}
	}	

	























}