#ifndef NTP_SYNC_H
#define NTP_SYNC_H

#include "lwip/udp.h"

#define NTP_SERVER "pool.ntp.org"
#define NTP_PORT 123
#define NTP_MSG_LEN 48
#define NTP_DELTA 2208988800UL // Seconds between 1 Jan 1900 and 1 Jan 1970

typedef struct {
    ip_addr_t ntp_server_address;
    struct udp_pcb *ntp_pcb;
    bool request_in_progress;
    uint64_t transmit_timestamp; // T1 in microseconds
    uint64_t t1_sysclock_us;
} NTP_T;

void ntp_init(NTP_T *state);
void ntp_sync_time(NTP_T *state);

extern volatile uint64_t last_ntp_update_us;
extern volatile bool ntp_time_valid;
extern volatile time_t ntp_last_received_unixtime;

#endif // NTP_SYNC_H
