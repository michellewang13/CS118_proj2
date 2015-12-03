#define PACKET_SIZE 1024
#define HEADER_SIZE 20
#define PAYLOAD_SIZE 1004
#define BUFFER_SIZE 256

#include <iostream>
using namespace std;

enum Packet_Type { FILE_PACKET , ACK_PACKET };

struct Packet
{
	Packet_Type type;
	unsigned int source_port;
	unsigned int dest_port;
	unsigned int num;
	unsigned int size_or_offset;
	char payload[PAYLOAD_SIZE];
};

void initialize_packet(Packet * packet, int num, unsigned int size, struct sockaddr_in &addr, int portno, Packet_Type type)
{

	if (type == FILE_PACKET) packet->type = FILE_PACKET;
	else 			  packet->type = ACK_PACKET;

	packet->source_port 	= portno;
	packet->dest_port   	= ntohs(addr.sin_port);
	packet->num 			= num;
	packet->size_or_offset  = size;

}

void packet_dump(Packet * packet)
{
	switch (packet->type)
	{
		case FILE_PACKET:

			cout << "Type: File" << endl;
			cout << "Source Port: " << packet->source_port << endl;
			cout << "Destination Port: " << packet->dest_port << endl;
			cout << "Sequence Number: " << packet->num << endl;
			cout << "Size: " << packet->size_or_offset << endl << endl;
			printf("Payload:\n%s\n", packet->payload);
			break;

		case ACK_PACKET:
			cout << "Type: ACK" << endl;
			cout << "Source Port: " << packet->source_port << endl;
			cout << "Destination Port: " << packet->dest_port << endl;
			cout << "Ack Number: " << packet->num << endl;
			cout << "Offset: " << packet->size_or_offset << endl << endl;
			break;
	}
}