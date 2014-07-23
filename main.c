#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <string.h>

#include "packet.h"
#include "list.h"
#include "colors.h"

const char *serialPort = "/dev/ttyUSB0";

void init_ncurses();

int main()
{
        if(openSerialPort(serialPort) != 0) return -1;
        
        init_ncurses();
        if(init_list(0, LINES) != 0) return -1;
        
        bool running = true;
        int ch;
        while(running) {
                ch = getch();
                
                update_list(ch);
                
                if(ch == 'q' || ch == 'Q') {
                        running = false;
                }
                

        }
        endwin();
        closeSerialPort();
        
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
