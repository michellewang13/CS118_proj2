#define MAX_PACKET_SIZE 1024
#define HEADER_SIZE 16
#define PAYLOAD_SIZE 1008
#define BUFFER_SIZE 256

struct Packet
{
	enum type { FILE, ACK };
	unsigned int source_port;
	unsigned int dest_port;
	unsigned int num;
	unsigned int size_or_offset;
	char payload[PAYLOAD_SIZE];
};