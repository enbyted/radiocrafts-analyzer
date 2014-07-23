#include <inttypes.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>

#include "packet.h"

static uint8_t buffer[255];
static uint8_t seq_no = 0;
int fd;

int openSerialPort(const char *name) {
    if(fd != 0) closeSerialPort();
    
    struct termios tty;
    
    fd = open(name, O_RDWR | O_NOCTTY | O_SYNC);
    if(fd < 0) {
        printf("Cannot open %s!\n", name);
        return -1;
    }
    
    tcgetattr(fd, &tty);
    tty.c_cflag = B19200 | CRTSCTS | CS8 | CREAD;
    tty.c_iflag = IGNPAR;
    tty.c_oflag = 0;
    tty.c_lflag = 0;
    tty.c_cc[VTIME] = 0;
    tty.c_cc[VMIN] = 0;
    tcsetattr(fd, TCSANOW, &tty);
    
    return 0;
}

void closeSerialPort() {
    close(fd);
    fd = 0;
}

void sendPacket() {
    out_packet_t *packet = (out_packet_t *)(&buffer[0]);
    packet->seq_no = ++seq_no;

    uint8_t i;
    
    #ifdef DEBUG_OUT_PACKET
        for(i = 0; i < packet->len; i++) {
            printf("%d ", ((uint8_t *)packet)[i]);
        }
        printf("\n");
    #endif
    
    i = write(fd, &buffer[0], packet->len);
    printf("Wrote %d bytes\n", i);
}

void sendCommand(uint8_t addr[], uint8_t type, uint8_t data1, uint8_t data2) {
    out_packet_t *packet = (out_packet_t *)(&buffer[0]);
    data_control_t *data = (data_control_t *)(&packet->data[0]);
    packet->len = 0x0A;
    uint8_t i;
    for(i = 0; i < 4; i++) {
        packet->address[i] = addr[i];
    }
    packet->type = 0x03;
    data->arg = type;
    data->data1 = data1;
    data->data2 = data2;
    
    sendPacket();
}

void setGPIOFunction(uint8_t addr[], uint8_t gpio, uint8_t function, uint8_t trigger) {
    out_packet_t *packet = (out_packet_t *)(&buffer[0]);
    data_configuration_t *config;
    packet->len = 0x28;
    packet->address[0] = addr[0];
    packet->address[1] = addr[1];
    packet->address[2] = addr[2];
    packet->address[3] = addr[3];
    packet->type = 0x03;
    config = (data_configuration_t *)(&packet->data[0]);
    config->arg = 0x03;
    
    uint8_t i;
    for(i = 0; i < 32; i++) {
        config->data[i] = 0;
    }
    
    config->data[0] = GPIO_BASE + gpio;
    config->data[1] = function;
    config->data[2] = GPIO_BASE + 8 + gpio;
    config->data[3] = trigger;
    
    sendPacket();
}

void sendData(uint8_t addr[], void *data, uint8_t n) {
    out_packet_t *packet = (out_packet_t *)(&buffer[0]);
    packet->len = 7 + n;
    uint8_t i;
    for(i = 0; i < 4; i++) {
        packet->address[i] = addr[i];
    }
    
    packet->type = 0x11;
    for(i = 0; i < n; i++) {
        packet->data[i] = ((uint8_t *)data)[i];
    }
    
    sendPacket();
}

const char* packetType(uint8_t type) {
    switch(type) {
        case TYPE_EVENT: {
            return "TYPE_EVENT";
        }
        case TYPE_SERIAL_IN: {
            return "TYPE_SERIAL_IN";
        }
        default: {
            return "ERR_UNKNOWN";
        }
    }
}

const char* msgDetail(uint8_t detail) {
    switch(detail) {
        case DETAIL_INPUT_CHANGE: { return "DETAIL_INPUT_CHANGE"; }
        case DETAIL_ANALOG_0_TRIG: { return "DETAIL_ANALOG_0_TRIG"; }
        case DETAIL_ANOLOG_1_TRIG: { return "DETAIL_ANOLOG_1_TRIG"; }
        case DETAIL_RF_TAMPER: { return "DETAIL_RF_TAMPER"; }
        case DETAIL_RESET: { return "DETAIL_RESET"; }
        case DETAIL_STATUS: {return "DETAIL_STATUS"; }
        case DETAIL_CHANNEL_BUSY: { return "DETAIL_CHANNEL_BUSY"; }
        case DETAIL_CHANNEL_FREE: { return "DETAIL_CHANNEL_FREE"; }
        case DETAIL_CHANNEL_JAMMED: { return "DETAIL_CHANNEL_JAMMED"; }
        case DETAIL_CHANNEL_TAKEN: { return "DETAIL_CHANNEL_TAKEN"; }
        case DETAIL_ACK: { return "DETAIL_ACK"; }
        case DETAIL_NAK: { return "DETAIL_NAK"; }
        case DETAIL_CID: { return "DETAIL_CID"; }
        case DETAIL_NEXT_RECEIVER: { return "DETAIL_NEXT_RECEIVER"; }
        default: { return "ERR_UNKNOWN"; }
    }
}

void printPacketInfo(in_packet_t *packet) {
    if(packet == NULL) return;
    
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    
    printf("[%d:%d:%d] ", tm.tm_hour, tm.tm_min, tm.tm_sec);
    printf("From %d:%d:%d:%d", packet->address[0], packet->address[1], packet->address[2], packet->address[3]);
    printf(", No.: %d", packet->seq_no);
    printf(", hops: %d", packet->hops);
    printf(", latency: %dms", packet->latency * 10);
    printf(", %s", packetType(packet->type));
    if(packet->type == TYPE_EVENT) {
        printf(", %s", msgDetail(packet->data[0]));
    } else if(packet->type == TYPE_SERIAL_IN) {
        data_serial_t *data = (data_serial_t *)&(packet->data[0]);
        printf(", block %d", data->block_no);
        printf(" ( ");
        uint8_t i;
        for(i = 0; i < (packet->len - 18); i++) {
            printf("%d ", data->data[i]);
            if(i > 15) {
                printf("... ");
                break;
            }
        }
        printf(")");
    }
    
    printf("\n");
}

in_packet_t* readPacket() {
    in_packet_t *ret = (in_packet_t *)(&buffer[0]);
    
    int len = read(fd, &buffer[0], 1); // Packet len
    if( len < 1 || buffer[0] < 18 ) return NULL;
    uint8_t i;
    for(i = 1; i < buffer[0]; i++) {
        uint8_t amt = read(fd, &buffer[i], buffer[0] - i);
        if(amt < 0) return NULL;
        i += amt;
    }
    
    uint16_t tmp16 = ret->seq_no;
    uint8_t tmp1 = tmp16 & 0xFF;
    uint8_t tmp2 = (tmp16 >> 8) & 0xFF;
    ret->seq_no = (tmp1 << 8) | tmp2; // Data is comming in big-endian, converting to little-endian
    
    tmp16 = ret->latency;
    tmp1 = tmp16 & 0xFF;
    tmp2 = (tmp16 >> 8) & 0xFF;
    ret->latency = (tmp1 << 8) | tmp2; // Same here
    
    return ret;
}

