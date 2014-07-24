#include <stdlib.h>
#include <inttypes.h>
#include <ncurses.h>
#include <time.h>
#include <string.h>

#include "colors.h"
#include "list.h"
#include "packet.h"

#define	DETAIL_HEIGHT	10

WINDOW		*detail_window;
WINDOW		*list_window;

uint16_t	pos;
uint16_t	top;
uint16_t	n_packets;
uint16_t	lines;

packet_t *packets[N_PACKETS];

const char *packet_type_to_string(uint8_t type) {
        #define CASE(type, name) case type: { return name; }
        
        switch(type) {
                CASE( TYPE_SERIAL_DATA_IN, "SERIAL_DATA_IN" )
                CASE( TYPE_SERIAL_DATA_OUT, "SERIAL_DATA_OUT" )
                CASE( TYPE_INPUT_CHANGE, "INPUT_CHANGE" )
                CASE( TYPE_ANALOG_0_TRIG, "ANALOG_0_TRIGGER" )
                CASE( TYPE_ANOLOG_1_TRIG, "ANALOG_1_TRIGGER" )
                CASE( TYPE_RF_TAMPER, "RF_TAMPER" )
                CASE( TYPE_RESET, "RESET" )
                CASE( TYPE_STATUS, "STATUS" )
                CASE( TYPE_CHANNEL_BUSY, "CHANNEL_BUSY" )
                CASE( TYPE_CHANNEL_FREE, "CHANNEL_FREE" )
                CASE( TYPE_CHANNEL_JAMMED, "CHANNEL_JAMMED" )
                CASE( TYPE_CHANNEL_TAKEN, "CHANNEL_TAKEN" )
                CASE( TYPE_ACK, "ACK" )
                CASE( TYPE_NAK, "NACK" )
                CASE( TYPE_CID, "CID" )
                CASE( TYPE_NEXT_RECEIVER, "NEXT_RECEIVER" )
                CASE( TYPE_SET_OUTPUTS, "SET_OUTPUTS" )
                CASE( TYPE_SET_PWM, "SET_PWM" )
                CASE( TYPE_GATEWAY_CONFIG, "GATEWAY_CONFIG" )
                CASE( TYPE_GET_CID, "GET_CID" )
                CASE( TYPE_GET_DID, "GET_DID" )
                CASE( TYPE_GET_CONFIG, "GET_CONFIG" )
                CASE( TYPE_TRACEROUTE, "TRACEROUTE" )
                default: { return "ERR_UNKNOWN_VAL"; }
        }
        
        #undef CASE
}


void add_packet_to_list(packet_t *packet) {
        //Scroll packets one down.
        packet_t *tmp = packets[N_PACKETS - 1]; //Save last packet address
        
        uint16_t i;
        for(i = N_PACKETS - 1; i > 0; i--) {
                packets[i] = packets[i - 1];
        }
        
        packets[0] = tmp;
        
        //Add new packet
        
        memcpy(packets[0], packet, sizeof(packet_t));
        if(n_packets < N_PACKETS) {
                n_packets++;
                pos++;
        }
}

void read_packet_to_list() {
        in_packet_t *in_packet = readPacket();
        if(in_packet == NULL) return;
        
        packet_t packet;
        
        packet.dir = DIR_IN;
        packet.timestamp = time(NULL);
        packet.id = in_packet->seq_no;
        packet.addr.addr[0] = in_packet->address[0];
        packet.addr.addr[1] = in_packet->address[1];
        packet.addr.addr[2] = in_packet->address[2];
        packet.addr.addr[3] = in_packet->address[3];
        
        if(in_packet->type == TYPE_SERIAL_IN) {
                packet.type = TYPE_SERIAL_DATA_IN;
        } else if(in_packet->type == TYPE_EVENT) {
                switch(in_packet->data[0]) {
                        case DETAIL_INPUT_CHANGE:	{ packet.type = TYPE_INPUT_CHANGE; break; }
                        case DETAIL_ANALOG_0_TRIG:	{ packet.type = TYPE_ANALOG_0_TRIG; break; }
                        case DETAIL_ANOLOG_1_TRIG:	{ packet.type = TYPE_ANOLOG_1_TRIG; break; }
                        case DETAIL_RF_TAMPER:		{ packet.type = TYPE_RF_TAMPER; break; }
                        case DETAIL_RESET:		{ packet.type = TYPE_RESET; break; }
                        case DETAIL_STATUS:		{ packet.type = TYPE_STATUS; break; }
                        case DETAIL_CHANNEL_BUSY:	{ packet.type = TYPE_CHANNEL_BUSY; break; }
                        case DETAIL_CHANNEL_FREE:	{ packet.type = TYPE_CHANNEL_FREE; break; }
                        case DETAIL_CHANNEL_JAMMED:	{ packet.type = TYPE_CHANNEL_JAMMED; break; }
                        case DETAIL_CHANNEL_TAKEN:	{ packet.type = TYPE_CHANNEL_TAKEN; break; }
                        case DETAIL_ACK:		{ packet.type = TYPE_ACK; break; }
                        case DETAIL_NAK:		{ packet.type = TYPE_NAK; break; }
                        case DETAIL_CID:		{ packet.type = TYPE_CID; break; }
                        case DETAIL_NEXT_RECEIVER: 	{ packet.type = TYPE_NEXT_RECEIVER; break; }
                        case 0x20:/*TODO: Add constant*/{ packet.type = TYPE_TRACEROUTE; break; }
                        default: 			{ packet.type = 0; break; }

                }
        } else {
                packet.type = 0;
        }
        
        packet.latency = in_packet->latency * 10;
        packet.hops = in_packet->hops;
        packet.rssi = in_packet->RSSI;
        
        memcpy(&(packet.data[0]), in_packet, in_packet->len);
        
        add_packet_to_list(&packet);
}

void update_window_list() {
        werase(list_window);
        wprintw(list_window, "DIR  TIME      ID     ADDRESS      TYPE                  LATENCY  HOPS  RSSI");
        //		      OUT  HH:MM:SS  65535  AA:BB:CC:DD  SERIAL_DATA_OUT       1000ms   100   -123dBm
        mvwchgat(list_window, 0, 0, -1, COLOR_PAIR(COLOR_HEADER), 1, NULL);
        
        uint16_t i;
        uint16_t h;
        uint16_t bottom = ( (top + lines) < n_packets )?(top + lines):n_packets;
        address_t *addr;
        
        for(i = top; i < bottom; i++) {
                if(pos == i) {
                        wattrset(list_window, A_REVERSE);
                } else {
                        wattrset(list_window, A_NORMAL);
                }
                
                for(h = 0; h < COLS; h++) {
                        mvwprintw(list_window, i - top + 1, h, " ");
                }
                
                struct tm tm = *localtime(&packets[i]->timestamp);
                h = i - top + 1;
                addr = &packets[i]->addr;
                mvwprintw(list_window, h, 0, "%s", (packets[i]->dir==DIR_IN)?"IN":((packets[i]->dir==DIR_OUT)?"OUT":"NONE"));
                mvwprintw(list_window, h, 5, "%.2d:%.2d:%.2d", tm.tm_hour, tm.tm_min, tm.tm_sec);
                mvwprintw(list_window, h, 15, "%d", packets[i]->id);
                mvwprintw(list_window, h, 22, "%.2X:%.2X:%.2X:%.2X", addr->addr[0], addr->addr[1], addr->addr[2], addr->addr[3]);
                mvwprintw(list_window, h, 35, "%s", packet_type_to_string(packets[i]->type));
                mvwprintw(list_window, h, 57, "%dms", packets[i]->latency);
                mvwprintw(list_window, h, 66, "%d", packets[i]->hops);
                mvwprintw(list_window, h, 72, "%ddBm", packets[i]->rssi);
        }
        
        wrefresh(list_window);        
}

void update_window_detail() {
        packet_t *packet = packets[pos];
        werase(detail_window);
        mvwprintw(detail_window, 2, 2, "TEST");

        struct tm tm = *localtime(&packet->timestamp);

        if(packet->dir == DIR_NONE) return;
        else if(packet->dir == DIR_OUT) {
                mvwprintw(detail_window, 0, 0, "Outgoing packet   Sent at: %.2d:%.2d:%.2d        To: %.2X:%.2X:%.2X:%.2X", 
                        packet->addr.addr[0], packet->addr.addr[1], packet->addr.addr[2], packet->addr.addr[3]
                );
                
                switch(packet->type) {
                        default: {
                                mvwprintw(detail_window, 2, 0, "Unsupported packet type, please check if it's the newwest version of Radiocrafts Analyzer. If so, please send a bug report at https://github.com/enbyted/radiocrafts-analyzer");
                        }
                }
        } else if(packet->dir == DIR_IN) {
                mvwprintw(detail_window, 0, 0, "Incoming packet   Received at: %.2d:%.2d:%.2d  From: %.2X:%.2X:%.2X:%.2X", 
                        tm.tm_hour, tm.tm_min, tm.tm_sec,
                        packet->addr.addr[0], packet->addr.addr[1], packet->addr.addr[2], packet->addr.addr[3]
                );
                in_packet_t *in_packet = (in_packet_t *)&packet->data[0];
                switch(packet->type) {
                        case TYPE_SERIAL_DATA_IN: {
                                data_serial_t *data = (data_serial_t *)&in_packet->data[0];
                                mvwprintw(detail_window, 2, 0, "Received following serial data (First HEX notation, then ASCII)");
                                
                                wmove(detail_window, 4, 0);
                                
                                uint8_t n;
                                uint16_t header_size = &data->data[0] - &in_packet->len;
                                for(n = 0; n < in_packet->len - header_size; n++) {
                                        wprintw(detail_window, "%.2X ", data->data[n]);
                                }
                                wprintw(detail_window, "\n");
                                for(n = 0; n < in_packet->len - header_size; n++) {
                                        if(data->data[n] >= ' ' || data->data[n] <= '~') {
                                                wprintw(detail_window, "%c", data->data[n]);
                                        }
                                }
                                
                                break;
                        }
                        case TYPE_STATUS:
                        case TYPE_RESET: {
                                data_event_t *data = (data_event_t *) &in_packet->data[0];
                                switch(data->data[1]) {
                                        case 0x00: {
                                                mvwprintw(detail_window, 2, 0, "IMA Packet");
                                                break;
                                        }
                                        case 0x01: {
                                                mvwprintw(detail_window, 2, 0, "IMA Packet caused by device power on reset");
                                                break;
                                        }
                                        case 0x02: {
                                                mvwprintw(detail_window, 2, 0, "IMA Packet caused by device external reset");
                                                break;
                                        }
                                        case 0x03: {
                                                mvwprintw(detail_window, 2, 0, "IMA Packet caused by device reset from config/sleep");
                                                break;
                                        }
                                        case 0x04: {
                                                mvwprintw(detail_window, 2, 0, "IMA Packet caused by reset command");
                                                break;
                                        }
                                        case 0x05: {
                                                mvwprintw(detail_window, 2, 0, "IMA Packet caused by device watchdog reset");
                                                break;
                                        }
                                }
                                
                                mvwprintw(detail_window, 4, 0, "Module temperature: %d°C", ((int16_t)data->temp) - 128);
                                mvwprintw(detail_window, 5, 0, "Module voltage: %fV", data->voltage * 0.03f);

                                wattrset(detail_window, A_BOLD);
                                mvwprintw(detail_window, 7, 8, "Analogue inputs");
                                wattrset(detail_window, A_NORMAL);
                                
                                mvwprintw(detail_window, 8, 0, "Analogue input 0 (GPIO0): %.2fV", data->analog0 * 0.03f);
                                mvwprintw(detail_window, 9, 0, "Analogue input 1 (GPIO1): %.2fV", data->analog1 * 0.03f);
                                
                                wattrset(detail_window, A_BOLD);
                                mvwprintw(detail_window, 1, 51, "Digital inputs");
                                wattrset(detail_window, A_NORMAL);

                                uint8_t j;
                                for(j = 0; j < 8; j++) {
                                        if(data->inputs & (1<<j)) {
                                                mvwprintw(detail_window, j + 2, 50, "Input GPIO%d high", j);
                                        } else {
                                                mvwprintw(detail_window, j + 2, 50, "Input GPIO%d low", j);
                                        }
                                }
                                
                                
                                break;
                        }
                        default: {
                                mvwprintw(detail_window, 2, 0, "Unsupported packet type, please check if it's the newest version of Radiocrafts Analyzer. If so, please send a bug report at https://github.com/enbyted/radiocrafts-analyzer");
                        }
                }
        }
        
        wrefresh(detail_window);
}

void update_list(int ch) {
        //Handle user input
        if(ch == KEY_UP) {
                if(pos > 0) { 
                        pos--;
                } else {
                        pos = n_packets - 1;
                }
        } else if(ch == KEY_DOWN) {
                if(pos < n_packets - 1) {
                        pos++;
                } else {
                        pos = 0;
                }
        } else if(ch == KEY_PPAGE) {
                if(pos < lines) {
                        pos = 0;
                } else {
                        pos -= lines;
                }
        } else if(ch == KEY_NPAGE) {
                if(pos + lines > n_packets) {
                        pos = n_packets - 1;
                } else {
                        pos += lines;
                        top += lines;
                }
        }
        // Scroll list if nessesary
        uint16_t bottom = top + lines - 1;
        if(top > pos) {
                top = pos;
        }
        if(bottom < pos) {
                bottom = pos;
                top = bottom - lines + 1;
        }
        
        // Read packet from serial port 
        read_packet_to_list();
        
        // Update windows
        update_window_list();
        update_window_detail();
}

int init_list(int starty, int height) {
        lines = LINES - DETAIL_HEIGHT - 1;
        detail_window	= newwin(DETAIL_HEIGHT, COLS, 0, 0);
        list_window	= newwin(height - DETAIL_HEIGHT, COLS, 10, 0);

        wrefresh(list_window);
        wrefresh(detail_window);

        void *mem = calloc(N_PACKETS, sizeof(packet_t));
        if(mem == NULL) return -1;
        
        uint16_t i;
        for(i = 0; i < N_PACKETS; i++) {
                packets[i] = mem + (i * sizeof(packet_t));
                packets[i]->dir = DIR_NONE;
        }
        
        return 0;
}
