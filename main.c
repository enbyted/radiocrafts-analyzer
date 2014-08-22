#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <string.h>

#include "packet.h"
#include "list.h"
#include "devices.h"
#include "colors.h"

const char *serialPort = "/dev/ttyUSB0";
static bool running;
int init_ncurses();
void close_ncurses();
void update_ncurses(int ch);

int main(int argc, char *argv[])
{
        running = true;
        int ch;
        
        if(init_packet(serialPort) != 0) return -1;
        if(init_ncurses() != 0) return -1;
        if(init_list(0, LINES / 2) != 0) return -1;
        if(init_devices(LINES / 2 + 1, LINES / 2) != 0) return -1;
        
        while(running) {
                ch = getch();
                
                update_packet(ch);
                update_ncurses(ch);
                update_list(ch);
                
                //TODO: Add help somewhere
        }
        
        close_list();
        close_ncurses();
        close_packet();
        
	return 0;
}

void update_ncurses(int ch) {
        if(ch == 'q' || ch == 'Q') {
                running = false;
        }
}

int init_ncurses() {
        initscr();
        start_color();
        cbreak();
        keypad(stdscr, TRUE);
        noecho();
        curs_set(0);
        refresh();
        timeout(0);
        
        init_pair(COLOR_HEADER, COLOR_BLACK, COLOR_CYAN);
        
        return 0;
}

void close_ncurses() {
        endwin();
}