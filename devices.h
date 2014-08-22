#ifndef __DEVICES_H__
#define __DEVICES_H__

// How much memory should be allocated when needed
#define N_DEVICES 100

#include <time.h>
#include "list.h"

typedef struct {
        address_t	address; // Device address
        uint8_t		seq_no; // last packet seq. no.
        time_t		last_packet; // When last packet was received
} device_t;

int init_devices(int starty, int height);
void update_devices(int ch);

#endif //__DEVICES_H__
