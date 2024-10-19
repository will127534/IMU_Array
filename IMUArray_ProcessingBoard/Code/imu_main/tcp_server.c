#include <stdio.h>
#include "pico/stdlib.h"
#include "tcp_server.h"

volatile bool tcp_connected = false;

static err_t tcp_server_close(void *arg);
static err_t tcp_server_result(void *arg, int status);
static err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb);
static void tcp_server_err(void *arg, err_t err);
static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err);

uint8_t tcp_sent_count = 0;

TCP_SERVER_T* tcp_server_init(void) {
    TCP_SERVER_T *state = calloc(1, sizeof(TCP_SERVER_T));
    if (!state) {
        printf("[SERVER] Failed to allocate state\n");
        return NULL;
    }
    return state;
}

void tcp_server_deinit(TCP_SERVER_T *state) {
    if (state) {
        tcp_server_close(state);
        free(state);
    }
}

static err_t tcp_server_close(void *arg) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    err_t err = ERR_OK;
    if (state->client_pcb != NULL) {
        tcp_arg(state->client_pcb, NULL);
        tcp_poll(state->client_pcb, NULL, 0);
        tcp_sent(state->client_pcb, NULL);
        tcp_recv(state->client_pcb, NULL);
        tcp_err(state->client_pcb, NULL);
        err = tcp_close(state->client_pcb);
        if (err != ERR_OK) {
            printf("[SERVER] close failed %d, calling abort\n", err);
            tcp_abort(state->client_pcb);
            err = ERR_ABRT;
        }
        state->client_pcb = NULL;
    }
    if (state->server_pcb) {
        tcp_arg(state->server_pcb, NULL);
        tcp_close(state->server_pcb);
        state->server_pcb = NULL;
    }
    return err;
}

static err_t tcp_server_result(void *arg, int status) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    printf("[SERVER] Client disconnected\n");
    state->client_connected = false;
    tcp_arg(state->client_pcb, NULL);
    tcp_poll(state->client_pcb, NULL, 0);
    tcp_sent(state->client_pcb, NULL);
    tcp_recv(state->client_pcb, NULL);
    tcp_err(state->client_pcb, NULL);
    tcp_close(state->client_pcb);
    state->client_pcb = NULL;
    tcp_connected = false;
    return ERR_OK;
}

static err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    printf("[SERVER] Sent: %u, Rem: %u\n", len, TCP_SND_BUF-tcp_sndbuf(state->client_pcb));
    state->lastPackageSentTimestamp = to_ms_since_boot(get_absolute_time());
    return ERR_OK;
}


err_t tcp_server_send_data(void *arg, struct tcp_pcb *tpcb, uint8_t *data, uint len)
{
    //TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    //printf("Writing to client %d\n", len);
    u16_t available = tcp_sndbuf(tpcb);
    if (available < len) {
        printf("[SERVER] TCP send buffer full, cannot send data\n");
        return ERR_MEM;
    }

    err_t err = tcp_write(tpcb, data, len, TCP_WRITE_FLAG_COPY | TCP_WRITE_FLAG_MORE);
    if (err == ERR_OK) {
        tcp_sent_count +=1;
        if(tcp_sent_count>=5){
            tcp_output(tpcb);
            tcp_sent_count = 0;
        }
    } else {
        printf("[SERVER] Failed to write data %d\n", err);
    }
    return err;
}

err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    //TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    if (!p) {
        return tcp_server_result(arg, -1);
    }
    return ERR_OK;
}

static err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    printf("[SERVER] tcp_server_poll_fn\n");
    
    if(state->client_connected == 1){
        uint32_t packagesent = to_ms_since_boot(get_absolute_time()) - state->lastPackageSentTimestamp;
        if(packagesent > 10000){
            return tcp_server_result(arg, -1); // no response is an error?
        }
        else{
            return ERR_OK;
        }
    }
    else{
        state->client_connected = 1;
        tcp_connected = true;
        return ERR_OK; //tcp_server_result(arg, -1); // no response is an error?
    }
    
}

static void tcp_server_err(void *arg, err_t err) {
    if (err != ERR_ABRT) {
        printf("[SERVER] tcp_client_err_fn %d\n", err);
        tcp_server_result(arg, err);
    }
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    if (err != ERR_OK || client_pcb == NULL) {
        printf("[SERVER] Failure in accept\n");
        tcp_server_result(arg, err);
        return ERR_VAL;
    }
    printf("[SERVER] Client connected\n");

    state->client_pcb = client_pcb;
    tcp_arg(client_pcb, state);
    tcp_sent(client_pcb, tcp_server_sent);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_poll(client_pcb, tcp_server_poll, POLL_TIME_S * 2);
    tcp_err(client_pcb, tcp_server_err);
    state->lastPackageSentTimestamp = to_ms_since_boot(get_absolute_time());
    return ERR_OK;
}

bool tcp_server_open(void *arg) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    printf("[SERVER] Starting server at %s on port %u\n", ip4addr_ntoa(netif_ip4_addr(netif_list)), TCP_PORT);

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        printf("[SERVER] failed to create pcb\n");
        return false;
    }

    err_t err = tcp_bind(pcb, NULL, TCP_PORT);
    if (err) {
        printf("[SERVER] failed to bind to port %u\n", TCP_PORT);
        return false;
    }

    state->server_pcb = tcp_listen_with_backlog(pcb, 1);
    if (!state->server_pcb) {
        printf("[SERVER] failed to listen\n");
        if (pcb) {
            tcp_close(pcb);
        }
        return false;
    }

    tcp_arg(state->server_pcb, state);
    tcp_accept(state->server_pcb, tcp_server_accept);

    return true;
}
