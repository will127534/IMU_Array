#ifndef TIME_SYNC_H
#define TIME_SYNC_H

#include <stdbool.h>
#include <stdint.h>

extern volatile bool ntp_time_valid;
extern volatile uint64_t sysclock_to_unixtime_offset_us;

#endif // TIME_SYNC_H
