#ifndef __LIST_H__
#define __LIST_H__

#define N_PACKETS 500

#define DIR_NONE	0
#define DIR_OUT		1
#define DIR_IN		2

#define TYPE_SERIAL_DATA_IN	1
#define TYPE_SERIAL_DATA_OUT	2
#define TYPE_INPUT_CHANGE     	3
#define TYPE_ANALOG_0_TRIG    	4
#define TYPE_ANOLOG_1_TRIG    	5
#define TYPE_RF_TAMPER        	6
#define TYPE_RESET            	7
#define TYPE_STATUS           	8
#define TYPE_CHANNEL_BUSY     	9
#define TYPE_CHANNEL_FREE     	10
#define TYPE_CHANNEL_JAMMED   	11
#define TYPE_CHANNEL_TAKEN    	12
#define TYPE_ACK              	13
#define TYPE_NAK              	14
#define TYPE_CID              	15
#define TYPE_NEXT_RECEIVER    	16
#define TYPE_SET_OUTPUTS	17
#define TYPE_SET_PWM		18
#define TYPE_GATEWAY_CONFIG	19
#define TYPE_GET_CID		20
#define TYPE_GET_DID		21
#define TYPE_GET_CONFIG		22
#define TYPE_TRACEROUTE		23

typedef struct {
        uint8_t addr[4];
} address_t;

typedef struct {
        time_t timestamp;
        uint8_t dir;
        uint16_t id;
        uint8_t type;
        address_t addr;
        uint8_t hops;
        uint16_t latency;
        int16_t rssi;
        uint8_t data[150];
} packet_t;

const char *packet_type_to_string(uint8_t type);

void read_packet_to_list();
void add_packet_to_list(packet_t *packet);

int init_list(int starty, int height);
void update_list(int ch);

#endif //__LIST_H__