#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "lwip/tcp.h"

#define TCP_PORT 4242
#define POLL_TIME_S 3

typedef struct {
    struct tcp_pcb *server_pcb;
    struct tcp_pcb *client_pcb;
    bool client_connected;
    uint32_t lastPackageSentTimestamp;
} TCP_SERVER_T;

TCP_SERVER_T* tcp_server_init(void);
void tcp_server_deinit(TCP_SERVER_T *state);
bool tcp_server_open(void *arg);
err_t tcp_server_send_data(void *arg, struct tcp_pcb *tpcb, uint8_t *data, uint len);

extern volatile bool tcp_connected;

#endif // TCP_SERVER_H
