#include "ntp_sync.h"

#include <stdio.h>
#include <time.h>
#include <string.h>

#include "pico/stdlib.h"

#include "lwip/pbuf.h"
#include "lwip/dns.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"

#include "time_sync.h"

// Global variables
volatile uint64_t sysclock_to_unixtime_offset_us = 0;
volatile uint64_t last_ntp_update_us = 0;
volatile bool ntp_time_valid = false;
volatile time_t ntp_last_received_unixtime = 0;

static void ntp_request(NTP_T *state);
static void ntp_receive(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                        const ip_addr_t *addr, u16_t port);
static void ntp_dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg);

void ntp_init(NTP_T *state) {
    state->ntp_pcb = udp_new();
    if (!state->ntp_pcb) {
        printf("[NTP] Failed to create UDP PCB for NTP\n");
        return;
    }
    udp_recv(state->ntp_pcb, ntp_receive, state);
    state->request_in_progress = false;
}

void ntp_sync_time(NTP_T *state) {
    if (state->request_in_progress) return;

    state->request_in_progress = true;
    err_t err = dns_gethostbyname(NTP_SERVER, &state->ntp_server_address, ntp_dns_found, state);
    if (err == ERR_OK) {
        ntp_request(state);
    } else if (err != ERR_INPROGRESS) {
        printf("[NTP] DNS request failed with error %d\n", err);
        state->request_in_progress = false;
    }
}

static void ntp_dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg) {
    NTP_T *state = (NTP_T *)arg;
    if (ipaddr) {
        state->ntp_server_address = *ipaddr;
        printf("[NTP] NTP server address resolved: %s\n", ipaddr_ntoa(ipaddr));
        ntp_request(state);
    } else {
        printf("[NTP] NTP DNS resolution failed\n");
        state->request_in_progress = false;
    }
}

void ntp_request(NTP_T *state) {
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, NTP_MSG_LEN, PBUF_RAM);
    if (!p) {
        printf("[NTP] Failed to allocate pbuf for NTP request\n");
        return;
    }
    memset(p->payload, 0, NTP_MSG_LEN);
    ((uint8_t *)p->payload)[0] = 0x1B; // NTP request header

    // Get the current system time in microseconds
    uint64_t current_sysclock_us = time_us_64();

    // Convert to NTP timestamp format (seconds since 1900 and fraction)
    uint64_t ntp_time_us = current_sysclock_us + ((uint64_t)NTP_DELTA * 1000000ULL);

    uint32_t seconds_since_1900 = (uint32_t)(ntp_time_us / 1000000ULL);
    uint32_t fraction = (uint32_t)(((ntp_time_us % 1000000ULL) << 32) / 1000000ULL);

    // Prepare the Transmit Timestamp (offset 40)
    uint32_t transmit_seconds = htonl(seconds_since_1900);
    uint32_t transmit_fraction = htonl(fraction);

    memcpy((uint8_t *)p->payload + 40, &transmit_seconds, sizeof(transmit_seconds));
    memcpy((uint8_t *)p->payload + 44, &transmit_fraction, sizeof(transmit_fraction));

    // Store the Transmit Timestamp as a 64-bit NTP timestamp
    state->transmit_timestamp = ((uint64_t)seconds_since_1900 << 32) | fraction;

    udp_sendto(state->ntp_pcb, p, &state->ntp_server_address, NTP_PORT);
    pbuf_free(p);

    state->request_in_progress = true;
}


void ntp_receive(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                 const ip_addr_t *addr, u16_t port) {
    if (p == NULL) return;

    NTP_T *state = (NTP_T *)arg;

    // Get the current system time in microseconds as the Destination Timestamp (T4)
    uint64_t t4_sysclock_us = time_us_64();

    // Read the NTP timestamps from the packet
    uint32_t originate_seconds, originate_fraction;
    uint32_t receive_seconds, receive_fraction;
    uint32_t transmit_seconds, transmit_fraction;

    // Originate Timestamp (T1) - offset 24
    pbuf_copy_partial(p, &originate_seconds, sizeof(originate_seconds), 24);
    pbuf_copy_partial(p, &originate_fraction, sizeof(originate_fraction), 28);
    originate_seconds = ntohl(originate_seconds);
    originate_fraction = ntohl(originate_fraction);

    // Receive Timestamp (T2) - offset 32
    pbuf_copy_partial(p, &receive_seconds, sizeof(receive_seconds), 32);
    pbuf_copy_partial(p, &receive_fraction, sizeof(receive_fraction), 36);
    receive_seconds = ntohl(receive_seconds);
    receive_fraction = ntohl(receive_fraction);

    // Transmit Timestamp (T3) - offset 40
    pbuf_copy_partial(p, &transmit_seconds, sizeof(transmit_seconds), 40);
    pbuf_copy_partial(p, &transmit_fraction, sizeof(transmit_fraction), 44);
    transmit_seconds = ntohl(transmit_seconds);
    transmit_fraction = ntohl(transmit_fraction);

    pbuf_free(p);

    // Convert NTP timestamps to Unix time in microseconds
    uint64_t T1_us = ((uint64_t)(originate_seconds - NTP_DELTA) * 1000000ULL) +
                     (((uint64_t)originate_fraction * 1000000ULL) >> 32);

    uint64_t T2_us = ((uint64_t)(receive_seconds - NTP_DELTA) * 1000000ULL) +
                     (((uint64_t)receive_fraction * 1000000ULL) >> 32);

    uint64_t T3_us = ((uint64_t)(transmit_seconds - NTP_DELTA) * 1000000ULL) +
                     (((uint64_t)transmit_fraction * 1000000ULL) >> 32);

    uint64_t T4_us = t4_sysclock_us;

    // Calculate round-trip delay (delay_us) and clock offset (theta_us) in microseconds
    int64_t delay_us = (int64_t)(T4_us - T1_us) - (int64_t)(T3_us - T2_us);
    int64_t theta_us = ((int64_t)(T2_us - T1_us) + (int64_t)(T3_us - T4_us)) / 2;

    // Update system clock offset
    sysclock_to_unixtime_offset_us = theta_us;
    ntp_last_received_unixtime = T3_us;
    ntp_time_valid = true;
    last_ntp_update_us = T4_us;

    // For debugging
    time_t ntp_time = (time_t)(T3_us / 1000000ULL);
    printf("[NTP] NTP time synchronized: %s", ctime(&ntp_time));
    printf("[NTP] \tOriginate Timestamp (T1): %llu\n", (unsigned long long)T1_us);
    printf("[NTP] \tReceive Timestamp (T2): %llu\n", (unsigned long long)T2_us);
    printf("[NTP] \tTransmit Timestamp (T3): %llu\n", (unsigned long long)T3_us);
    printf("[NTP] \tDestination Timestamp (T4): %llu\n", (unsigned long long)T4_us);
    printf("[NTP] \ttheta: %lld\n", theta_us);
    printf("[NTP] \tdelay: %lld\n", delay_us);

    state->request_in_progress = false;
}