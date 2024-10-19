#ifndef GPSDO_H
#define GPSDO_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "hardware/pio.h"

// Constants
#define SYS_CLOCK_KHz (240 * 1000) // Adjust as needed
#define WINDOW_SIZE 10

// Function Prototypes
void gpsdo_init(PIO pio, uint sm, uint offset, uint pin);
void gpsdo_handle_pps(void);

// External Variables
extern volatile uint64_t last_pps_sysclock_us;
extern volatile bool pps_received;
extern volatile time_t gps_last_received_unixtime;
extern volatile bool gps_time_valid;

#endif // GPSDO_H
