#include <stdlib.h>
#include <inttypes.h>
#include <ncurses.h>
#include <time.h>
#include <string.h>

#include "colors.h"
#include "list.h"
#include "devices.h"

#define	DETAIL_HEIGHT	10

static WINDOW	*detail_window;
static WINDOW	*list_window;

static uint16_t	pos;
static uint16_t	top;
static uint16_t	n_devices;
static uint16_t	lines;

device_t *devices[N_DEVICES];

void add_device_to_list(device_t *device) {
        //Scroll packets one down.
        device_t *tmp = devices[N_DEVICES - 1]; //Save last packet address
        
        uint16_t i;
        for(i = N_DEVICES - 1; i > 0; i--) {
                devices[i] = devices[i - 1];
        }
        
        devices[0] = tmp;
        
        //Add new packet
        
        memcpy(devices[0], device, sizeof(device_t));
        if(n_devices < N_DEVICES) {
                n_devices++;
                pos++;
        }
}

static void update_window_list() {
        werase(list_window);


        wprintw(list_window, "!DIR  TIME      ID     ADDRESS      TYPE                  LATENCY  HOPS  RSSI");
        //		      OUT  HH:MM:SS  65535  AA:BB:CC:DD  SERIAL_DATA_OUT       1000ms   100   -123dBm
        mvwchgat(list_window, 0, 0, -1, COLOR_PAIR(COLOR_HEADER), 1, NULL);
/*        
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
 */       
        wrefresh(list_window);        
}

static void update_window_detail() {
//        packet_t *packet = packets[pos];
        werase(detail_window);

//        struct tm tm = *localtime(&packet->timestamp);

        wrefresh(detail_window);
}

void update_devices(int ch) {
        //Handle user input
        if(ch == KEY_UP) {
                if(pos > 0) { 
                        pos--;
                } else {
                        pos = n_devices - 1;
                }
        } else if(ch == KEY_DOWN) {
                if(pos < n_devices - 1) {
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
                if(pos + lines > n_devices) {
                        pos = n_devices - 1;
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
        
//        // Read packet from serial port 
//        read_packet_to_list();
        
        // Update windows
        update_window_list();
        update_window_detail();
}

int init_devices(int starty, int height) {
        lines 		= LINES - DETAIL_HEIGHT - 1;
        detail_window	= newwin(DETAIL_HEIGHT, COLS, 0, 0);
        list_window	= newwin(height - DETAIL_HEIGHT, COLS, 10, 0);
        
        wrefresh(list_window);
        wrefresh(detail_window);
        
        void *mem = calloc(N_DEVICES, sizeof(device_t));
        if(mem == NULL) return -1;
        
        uint16_t i;
        for(i = 0; i < N_DEVICES; i++) {
                devices[i] = mem + (i * sizeof(device_t));
        }
        
        return 0;
}
