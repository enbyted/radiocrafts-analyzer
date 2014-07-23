#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <string.h>

#include "packet.h"

#define N_PACKETS 500

#define DIR_NONE	0
#define DIR_OUT		1
#define DIR_IN		2

typedef struct {
        uint8_t addr[4];
} address_t;

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

const char *serialPort = "/dev/ttyUSB0";

WINDOW *list_window;
WINDOW *detail_window;
packet_t *packets[N_PACKETS];

uint16_t lines;
uint16_t n_packets;
uint16_t top;
uint16_t pos;

void init_ncurses();
void init_windows();
int init_memory();

void update_list();
void update_detail();

void read_packet();
void add_packet(packet_t *packet);

const char *packetTypeToString(uint8_t type);

#define COLOR_HEADER	1

uint16_t err;

int main()
{
        lines = LINES - 11;
        n_packets = 0;
        top = 2;
        pos = 5;
        
        if(openSerialPort(serialPort) != 0) return -1;
        if(init_memory() != 0) return -1;
        
        init_ncurses();

        lines = LINES - 11;
        n_packets = 0;
        top = 0;
        pos = 0;

        init_windows();
        bool running = true;
        
        int ch;
        i = 0;
        while(running) {
                i++;
                update_list(); 
                
                read_packet();
                ch = getch();
                
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
                } else if(ch == 'q' || ch == 'Q') {
                        running = false;
                }
                
                if(top > pos) {
                        top = pos;
                }
                uint16_t bottom = top + lines - 1;
                mvwprintw(detail_window, 0, 0, "bottom: %d, top: %d, pos: %d, lines: %d, i: %d, err: %d             ", bottom, top, pos, lines, i, err);
                wrefresh(detail_window);
                if(bottom < pos) {
                        bottom = pos;
                        top = bottom - lines + 1;
                }
        }
        endwin();
        closeSerialPort();
        
	return 0;
}

const char *packetTypeToString(uint8_t type) {
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
                default: { err = type; return "ERR_UNKNOWN_VAL"; }
        }
        
        #undef CASE
}


void add_packet(packet_t *packet) {
        //Scroll packets one down.
        packet_t *tmp = packets[N_PACKETS - 1]; //Save last packet address
        
        uint16_t i;
        for(i = N_PACKETS - 1; i > 0; i--) {
                packets[i] = packets[i - 1];
                
        }
        
        packets[0] = tmp;
        
        //Add new packet
        
        memcpy(packets[0], packet, sizeof(packet_t));
        if(n_packets < N_PACKETS) n_packets++;
}

void read_packet() {
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
                        default: 			{ packet.type = 0; in_packet->type = in_packet->data[0] * 1000; break; }

                }
        } else {
                err = in_packet->type * 100;
                packet.type = 0;
        }
        
        packet.latency = in_packet->latency * 10;
        packet.hops = in_packet->hops;
        packet.rssi = in_packet->RSSI;
        
        memcpy(&(packet.data[0]), in_packet, in_packet->len);
        
        add_packet(&packet);
}

void update_list() {
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
                mvwprintw(list_window, h, 35, "%s", packetTypeToString(packets[i]->type));
                mvwprintw(list_window, h, 57, "%dms", packets[i]->latency);
                mvwprintw(list_window, h, 66, "%d", packets[i]->hops);
                mvwprintw(list_window, h, 72, "%ddBm", packets[i]->rssi);
        }
        
        wrefresh(list_window);        
}

void update_detail() {
        wprintw(detail_window, "Not implemented yet");
}

int init_memory() {
        void *mem = calloc(N_PACKETS, sizeof(packet_t));
        if(mem == NULL) return -1;
        
        uint16_t i;
        for(i = 0; i < N_PACKETS; i++) {
                packets[i] = mem + (i * sizeof(packet_t));
                packets[i]->dir = DIR_NONE;
        }
        
        return 0;
}

void init_ncurses() {
        initscr();
        start_color();
        cbreak();
        keypad(stdscr, TRUE);
        noecho();
        curs_set(0);
        refresh();
        timeout(0);
        
        init_pair(COLOR_HEADER, COLOR_BLACK, COLOR_CYAN);
}

void init_windows() {
        detail_window	= newwin(10, COLS, 0, 0);
        list_window	= newwin(LINES - 10, COLS, 10, 0);
        

        wrefresh(list_window);
        wrefresh(detail_window);
}