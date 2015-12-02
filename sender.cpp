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

void send_packet(int sockfd, int portno, struct sockaddr_in &client_addr, socklen_t len, ifstream &file, unsigned int size)
{
	Packet file_packet;
	Packet ack;
	unsigned int position = 0;
	unsigned int packet_num = 0;
	unsigned int file_packet_count = 0;
	unsigned int ack_packet_count = 0;
	unsigned int recvlen = 0;

	while (position < size)
	{
		bzero(&file_packet, PACKET_SIZE);
		initialize_packet(&file_packet, packet_num, PAYLOAD_SIZE, client_addr, portno, FILE_PACKET);
		file.seekg(position);
		file.read(file_packet.payload, PAYLOAD_SIZE - 1);

		if ( (sendto(sockfd, (char *) &file_packet, PACKET_SIZE, 0, (struct sockaddr *) &client_addr, len)) < 0)
		{
			fprintf(stderr, "Error: sendto Failed.\n");
			exit(1);
		}
		printf("%s\n", file_packet.payload);
		cout << "Position: " << position << endl;
		position += PAYLOAD_SIZE - 1;
		packet_num++;
		file_packet_count++;
	}

	while (ack_packet_count < file_packet_count)
	{
		bzero(&ack, PACKET_SIZE);
		recvlen = recvfrom(sockfd, (char *) &ack, PACKET_SIZE, 0, (struct sockaddr *) &client_addr, &len);
		if (recvlen > 0)
		{
			packet_dump(&ack);
			ack_packet_count++;
		}
	}




}

int main(int argc, char *argv[])
{
	int sockfd, portno, cwnd, recvlen;
	struct sockaddr_in serv_addr, client_addr;
	socklen_t len = sizeof(client_addr);
	string filename;
	char buffer[BUFFER_SIZE];

	if (argc < 3)
	{
		fprintf(stderr, "Error: incorrect number of arguments\n");
		fprintf(stderr, "sender <receiver_portnumber> <window_size>\n");
		exit(1);
	}

	// Save arguments to variables
	portno = atoi(argv[1]);
	cwnd = atoi(argv[2]);
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
					send_packet(sockfd, portno, client_addr, len, file, size);
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