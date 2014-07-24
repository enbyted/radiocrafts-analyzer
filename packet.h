#ifndef __PACKET_H__
#define __PACKET_H__

#include <inttypes.h>

#ifndef NULL
#define NULL 0
#endif

#ifndef bool
#define bool uint8_t
#define true 1
#define false 0
#endif

#define GPIO_BASE	16
#define FUN_OUT_HIGH	0x00
#define FUN_IN		0x01
#define FUN_ALT		0x02
#define FUN_OUT_LOW	0x04
#define FUN_ANALOGUE_IN	FUN_ALT
#define FUN_PWN		FUN_ALT

#define TRIG_NONE	0x00
#define TRIG_RISING	0x01
#define TRIG_FALLING	0x02
#define TRIG_BOTH	(TRIG_RISING | TRIG_FALLING)

#define CMD_SET_OUT	0x01
#define CMD_SET_PWM	0x02
#define CMD_GATEWAY_CONFIG 0x05
#define CMD_GET_CID	0x10
#define CMD_GET_STATUS	0x11
#define CMD_GET_DID	0x12
#define CMD_GET_CONFIG	0x13
#define CMD_RESET	0x15
#define CMD_TRACEROUTE	0x16

#define TYPE_EVENT		0x02
#define TYPE_SERIAL_IN		0x10

#define DETAIL_INPUT_CHANGE	0x01
#define DETAIL_ANALOG_0_TRIG	0x02
#define DETAIL_ANOLOG_1_TRIG	0x03
#define DETAIL_RF_TAMPER	0x06
#define DETAIL_RESET		0x08
#define DETAIL_STATUS		0x09
#define DETAIL_CHANNEL_BUSY	0x0A
#define DETAIL_CHANNEL_FREE	0x0B
#define DETAIL_CHANNEL_JAMMED	0x0C
#define DETAIL_CHANNEL_TAKEN	0x0D
#define DETAIL_ACK		0x10
#define DETAIL_NAK		0x11
#define DETAIL_CID		0x12
#define DETAIL_NEXT_RECEIVER	0x13

#pragma pack(push)
#pragma pack(1)

typedef struct {
    uint8_t len;
    uint8_t address[4];
    uint8_t seq_no;
    uint8_t type;
    uint8_t data[120];
} out_packet_t;

typedef struct {
    uint8_t arg;
    uint8_t data1;
    uint8_t data2;
} data_control_t;

typedef struct {
    uint8_t arg;
    uint8_t data[32];
} data_configuration_t;

typedef struct {
    uint8_t len;
    uint8_t sys_id[4];
    uint8_t address[4];
    uint8_t RSSI;
    uint8_t network_level;
    uint8_t hops;
    uint16_t seq_no;
    uint16_t latency;
    uint8_t type;
    uint8_t data[121];
} in_packet_t;

typedef struct {
    uint8_t detail;
    uint8_t data[2];
    uint8_t locator[4];
    uint8_t temp;
    uint8_t voltage;
    uint8_t inputs;
    uint16_t analog0;
    uint16_t analog1;
    uint16_t hw_version;
    uint16_t fw_version;
} data_event_t;

typedef struct {
    uint8_t block_no;
    uint8_t data[120];
} data_serial_t;

#pragma pack(pop)

typedef void (*InPacketHandler)(in_packet_t *packet);
#define N_IN_HANDLERS	5

//#define DEBUG_OUT_PACKET 1

//int openSerialPort(const char *name);
//void closeSerialPort();

int init_packet(const char *portName);
void close_packet();
void update_packet(int ch);

void sendCommand(uint8_t addr[], uint8_t type, uint8_t data1, uint8_t data2);
void setGPIOFunction(uint8_t addr[], uint8_t gpio, uint8_t function, uint8_t trigger);
void sendData(uint8_t addr[], void *data, uint8_t n);

const char* packetType(uint8_t type);
const char* msgDetail(uint8_t detail);
void printPacketInfo(in_packet_t *packet);

int addInPacketHandler(InPacketHandler handler);
void removeInPacketHandler(int i);

//in_packet_t* readPacket();
//bool isPacketAvailable();

#endif
